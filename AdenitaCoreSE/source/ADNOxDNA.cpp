#include "ADNSaveAndLoad.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNNumericParsing.hpp"
#include "SBString.hpp"

#include <array>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>

namespace {

// oxDNA reduced length units are independent of the nanometers used in the UI.
// https://lorenzo-rovigatti.github.io/oxDNA/configurations.html
constexpr double oxDNALengthInNm = 0.8518;

struct TopologyRecord {
	int strand;
	char base;
	int threePrime;
	int fivePrime;
	size_t line;
};

struct StrandTraversal {
	std::vector<int> indices;
	bool circular{ false };
};

struct ConfigurationRecord {
	SBPosition3 position;
	ADNFrameUtils::Frame frame;
};

// Keep input errors distinct from implementation or allocation errors.
class InputError : public std::runtime_error {
public:
	InputError(const std::string& path, size_t line, const std::string& message)
		: std::runtime_error(path + (line ? ":" + std::to_string(line) : "") + ": " + message) {}
};

class RecordReader {
public:
	explicit RecordReader(const std::string& path) : path_(path), input_(SBCContainerString::pathFromUtf8(path)) {
		if (!input_) fail("Cannot open file");
	}
	bool next(std::vector<std::string>& tokens) {
		tokens.clear();
		std::string line;
		while (std::getline(input_, line)) {
			++line_;
			std::istringstream row(line);
			std::string token;
			while (row >> token) tokens.push_back(token);
			if (!tokens.empty()) return true;
		}
		if (input_.bad()) fail("Cannot read file");
		return false;
	}
	[[noreturn]] void fail(const std::string& message) const { throw InputError(path_, line_, message); }
	int integer(const std::string& text) const {
		int value;
		if (!ADNNumericParsing::tryInteger(text, value)) fail("Invalid integer: " + text);
		return value;
	}
	double number(const std::string& text) const {
		double value;
		if (!ADNNumericParsing::tryDouble(text, value)) fail("Invalid finite number: " + text);
		return value;
	}
	size_t line() const { return line_; }
private:
	std::string path_;
	std::ifstream input_;
	size_t line_{ 0 };
};

ADNFrameUtils::Frame importFrame(const ADNFrameUtils::Vec3& a1,
	const ADNFrameUtils::Vec3& a3, bool legacy, const RecordReader& reader) {
	using namespace ADNFrameUtils;
	// Reject degenerate inputs rather than inventing a default orientation.
	const double n1 = std::hypot(a1.x, a1.y, a1.z);
	const double n3 = std::hypot(a3.x, a3.y, a3.z);
	if (!std::isfinite(n1) || !std::isfinite(n3) || n1 <= 1e-12 || n3 <= 1e-12)
		reader.fail("Degenerate nucleotide orientation");
	const Vec3 first = a1 / n1;
	const Vec3 normal = a3 / n3;
	if (std::abs(dot(first, normal)) > 1e-5) reader.fail("Nucleotide orientation axes must be orthogonal");
	Frame frame;
	if (legacy) {
		// Invert the former exporter: a1 = -e2, a3 = -e1.
		frame.e2 = -first;
		frame.e1 = -normal;
		frame.e3 = cross(frame.e1, frame.e2);
	}
	else {
		// Adenita e2 points into the base, and e3 points 5' -> 3'.
		// oxDNA a3 points 3' -> 5': https://github.com/lorenzo-rovigatti/tacoxDNA/blob/master/src/libs/pdb.py
		frame.e2 = first;
		frame.e3 = -normal;
		frame.e1 = cross(frame.e2, frame.e3);
	}
	return orthonormalized(frame);
}

std::vector<StrandTraversal> validateTopology(const std::vector<TopologyRecord>& records,
	int strands, const std::string& path) {
	std::map<int, std::vector<int>> groups;
	const int count = static_cast<int>(records.size());
	for (int i = 0; i < count; ++i) {
		const auto& row = records[i];
		groups[row.strand].push_back(i);
		for (const int neighbor : { row.threePrime, row.fivePrime }) {
			if (neighbor < -1 || neighbor >= count) throw InputError(path, row.line, "Neighbor index is out of range");
			if (neighbor >= 0 && records[neighbor].strand != row.strand)
				throw InputError(path, row.line, "Backbone link crosses strand identifiers");
		}
		if ((row.threePrime >= 0 && records[row.threePrime].fivePrime != i) ||
			(row.fivePrime >= 0 && records[row.fivePrime].threePrime != i))
			throw InputError(path, row.line, "Backbone links are not reciprocal");
	}
	if (groups.size() != static_cast<size_t>(strands)) throw InputError(path, 1, "Declared strand count does not match topology");
	std::vector<StrandTraversal> traversals;
	for (const auto& group : groups) {
		int start = group.second.front();
		int fiveEnds = 0, threeEnds = 0;
		for (int index : group.second) {
			if (records[index].fivePrime == -1) { ++fiveEnds; start = index; }
			if (records[index].threePrime == -1) ++threeEnds;
		}
		const bool circular = fiveEnds == 0 && threeEnds == 0;
		if (!circular && (fiveEnds != 1 || threeEnds != 1))
			throw InputError(path, records[start].line, "A strand must be one linear chain or one closed cycle");
		StrandTraversal traversal;
		traversal.circular = circular;
		std::set<int> visited;
		int current = start;
		while (current >= 0 && visited.insert(current).second) {
			traversal.indices.push_back(current);
			current = records[current].threePrime;
		}
		if (visited.size() != group.second.size() || (circular ? current != start : current != -1))
			throw InputError(path, records[start].line, "Strand contains disconnected topology components");
		traversals.push_back(std::move(traversal));
	}
	return traversals;
}

} // namespace

ADNLoader::OxDNAImportResult ADNLoader::InputFromOxDNA(const std::string& topoFile, const std::string& configFile) {
	return InputFromOxDNA(topoFile, configFile, OxDNAImportOptions{});
}

ADNLoader::OxDNAImportResult ADNLoader::InputFromOxDNA(const std::string& topoFile,
	const std::string& configFile, const OxDNAImportOptions& options) {
	try {
		RecordReader topology(topoFile);
		std::vector<std::string> tokens;
		if (!topology.next(tokens) || tokens.size() != 2) topology.fail("Expected classic topology header: nucleotide-count strand-count");
		const int count = topology.integer(tokens[0]);
		const int strandCount = topology.integer(tokens[1]);
		if (count < 0 || strandCount < 0 || strandCount > count || (count > 0 && strandCount == 0))
			topology.fail("Invalid declared nucleotide or strand count");
		std::vector<TopologyRecord> records;
		while (topology.next(tokens)) {
			if (records.size() >= static_cast<size_t>(count)) topology.fail("More topology records than declared");
			if (tokens.size() != 4) topology.fail("Expected strand, base, and two neighbor indices");
			const int strand = topology.integer(tokens[0]);
			if (strand < (options.olderAdenitaExport ? 0 : 1)) topology.fail("Invalid strand identifier");
			if (tokens[1].size() != 1 || std::string("ACGT").find(tokens[1][0]) == std::string::npos) {
				// Earlier Adenita exports used R for an unspecified nucleotide.
				if (!options.olderAdenitaExport || (tokens[1] != "R" && tokens[1] != "N")) topology.fail("Unsupported nucleotide base: " + tokens[1]);
			}
			int n3 = topology.integer(tokens[2]), n5 = topology.integer(tokens[3]);
			if (options.olderAdenitaExport) std::swap(n3, n5);
			records.push_back({ strand, tokens[1][0], n3, n5, topology.line() });
		}
		if (records.size() != static_cast<size_t>(count)) topology.fail("Fewer topology records than declared");
		const auto traversals = validateTopology(records, strandCount, topoFile);

		RecordReader configuration(configFile);
		for (const auto& header : { std::make_pair("t", size_t(3)), std::make_pair("b", size_t(5)), std::make_pair("E", size_t(5)) }) {
			if (!configuration.next(tokens) || tokens.size() != header.second || tokens[0] != header.first || tokens[1] != "=")
				configuration.fail("Invalid or missing configuration header");
			for (size_t i = 2; i < tokens.size(); ++i) {
				const double value = configuration.number(tokens[i]);
				if (tokens[0] == "b" && value < 0) configuration.fail("Negative box dimension");
			}
		}
		std::vector<ConfigurationRecord> coordinates;
		while (configuration.next(tokens)) {
			if (coordinates.size() >= records.size()) configuration.fail("More configuration records than declared");
			if (tokens.size() != 9 && tokens.size() != 15) configuration.fail("Expected 9 or 15 configuration fields");
			std::array<double, 15> values{};
			for (size_t i = 0; i < tokens.size(); ++i) values[i] = configuration.number(tokens[i]);
			const double scale = options.olderAdenitaExport ? 1000.0 : 1000.0 * oxDNALengthInNm;
			for (size_t i = 0; i < 3; ++i) if (!std::isfinite(values[i] * scale)) configuration.fail("Position exceeds supported coordinate range");
			coordinates.push_back({ SBPosition3(SBQuantity::picometer(values[0] * scale), SBQuantity::picometer(values[1] * scale), SBQuantity::picometer(values[2] * scale)),
				importFrame({values[3], values[4], values[5]}, {values[6], values[7], values[8]}, options.olderAdenitaExport, configuration) });
		}
		if (coordinates.size() != records.size()) configuration.fail("Missing nucleotide configuration records");

		// No model nodes exist until both files and every traversal are validated.
		SBPointer<ADNPart> part = new ADNPart();
		for (const auto& traversal : traversals) {
			SBPointer<ADNSingleStrand> strand = new ADNSingleStrand();
			strand->SetDefaultName();
			part->RegisterSingleStrand(strand);
			for (int index : traversal.indices) {
				SBPointer<ADNNucleotide> nucleotide = new ADNNucleotide();
				nucleotide->Init();
				const char base = records[index].base;
				nucleotide->setNucleotideType(base == 'R' || base == 'N' ? DNABlocks::DI : ADNModel::ResidueNameToType(base));
				nucleotide->SetPosition(coordinates[index].position);
				ADNFrameAdapters::setFrame(*nucleotide, coordinates[index].frame);
				part->RegisterNucleotideThreePrime(strand, nucleotide);
			}
			strand->setCircularFlag(traversal.circular);
		}
		BuildTopScales(part);
		return { false, part, {} };
	}
	catch (const InputError& error) { return { true, nullptr, error.what() }; }
	catch (const std::filesystem::filesystem_error& error) { return { true, nullptr, error.what() }; }
}

namespace {

/// \brief Complete validated configuration and topology text awaiting publication.
struct OxDNAExportText {
	std::string configuration;
	std::string topology;
};

/// \brief Prepares a complete export before any destination file can be truncated.
/// \param singleStrands Strands to export in classic oxDNA order.
/// \param options Box dimensions in nanometers.
/// \return Validated configuration and topology text.
/// \throws std::invalid_argument If model values or box dimensions cannot be exported.
OxDNAExportText prepareOxDNAExport(SBPointerIndexer<ADNSingleStrand> singleStrands,
	const ADNAuxiliary::OxDNAOptions& options) {
	for (const double size : { options.boxSizeX_, options.boxSizeY_, options.boxSizeZ_ })
		if (!std::isfinite(size) || size < 0 || !std::isfinite(size / oxDNALengthInNm))
			throw std::invalid_argument("oxDNA export requires finite non-negative box dimensions");
	std::ostringstream outConf, outTopo;
	// Failed staging must not be mistaken for a complete export, including allocation failures in a stream.
	outConf.exceptions(std::ios::badbit | std::ios::failbit);
	outTopo.exceptions(std::ios::badbit | std::ios::failbit);
	std::vector<std::vector<SBPointer<ADNNucleotide>>> strands;
	std::map<ADNNucleotide*, size_t> indices;
	// One index map drives topology and coordinates, including circular closure.
	for (auto strand : singleStrands) {
		if (strand == nullptr) continue;
		std::vector<SBPointer<ADNNucleotide>> nucleotides;
		for (auto nucleotide = strand->GetThreePrime(); nucleotide != nullptr; nucleotide = nucleotide->GetPrev()) {
			if (indices.count(nucleotide())) throw std::invalid_argument("Duplicate nucleotide in oxDNA export");
			if (std::string("ACGT").find(nucleotide->getOneLetterNucleotideTypeString()) == std::string::npos || nucleotide->getOneLetterNucleotideTypeString().size() != 1)
				throw std::invalid_argument("oxDNA export requires A, C, G, or T nucleotides");
			indices.emplace(nucleotide(), indices.size());
			nucleotides.push_back(nucleotide);
		}
		if (!nucleotides.empty()) strands.push_back(std::move(nucleotides));
	}
	const auto neighborIndex = [&](SBPointer<ADNNucleotide> nucleotide) {
		if (nucleotide == nullptr) return std::string("-1");
		const auto found = indices.find(nucleotide());
		if (found == indices.end()) throw std::invalid_argument("oxDNA export contains a neighbor outside the exported strands");
		return std::to_string(found->second);
	};
	outConf << std::setprecision((std::numeric_limits<double>::max_digits10));
	outConf << "t = 0\nb = " << options.boxSizeX_ / oxDNALengthInNm << ' ' << options.boxSizeY_ / oxDNALengthInNm << ' ' << options.boxSizeZ_ / oxDNALengthInNm << "\nE = 0 0 0\n";
	outTopo << indices.size() << ' ' << strands.size() << '\n';
	for (size_t strand = 0; strand < strands.size(); ++strand) {
		for (const auto& nucleotide : strands[strand]) {
			const auto position = nucleotide->GetPosition();
			for (unsigned int axis = 0; axis < 3; ++axis)
				if (!std::isfinite(position[axis].getValue()))
					throw std::invalid_argument("oxDNA export requires finite nucleotide coordinates");
			const auto frame = ADNFrameAdapters::sanitizedFrame(*nucleotide());
			const auto a1 = frame.e2;
			const auto a3 = -frame.e3;
			outConf << position[0].getValue() / (1000.0 * oxDNALengthInNm) << ' ' << position[1].getValue() / (1000.0 * oxDNALengthInNm) << ' ' << position[2].getValue() / (1000.0 * oxDNALengthInNm)
				<< ' ' << a1.x << ' ' << a1.y << ' ' << a1.z << ' ' << a3.x << ' ' << a3.y << ' ' << a3.z << " 0 0 0 0 0 0\n";
			const auto n3 = nucleotide->GetNext(true);
			const auto n5 = nucleotide->GetPrev(true);
			outTopo << strand + 1 << ' ' << nucleotide->getOneLetterNucleotideTypeString() << ' '
				<< neighborIndex(n3) << ' ' << neighborIndex(n5) << '\n';
		}
	}
	return { outConf.str(), outTopo.str() };
}

/// \brief Writes previously validated text and reports stream failures.
/// \param text Complete export text.
/// \param outConf Configuration output stream.
/// \param outTopo Topology output stream.
/// \throws std::runtime_error If either stream cannot accept the output.
void writeOxDNAExport(const OxDNAExportText& text, std::ofstream& outConf, std::ofstream& outTopo) {
	outConf << text.configuration;
	outTopo << text.topology;
	if (!outConf || !outTopo) throw std::runtime_error("Could not write oxDNA configuration or topology");
}

} // namespace

void ADNLoader::SingleStrandsToOxDNA(SBPointerIndexer<ADNSingleStrand> singleStrands,
	std::ofstream& outConf, std::ofstream& outTopo, const ADNAuxiliary::OxDNAOptions& options) {
	writeOxDNAExport(prepareOxDNAExport(singleStrands, options), outConf, outTopo);
}

void ADNLoader::OutputToOxDNA(SBPointer<ADNPart> part, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options) {
	if (part == nullptr) throw std::invalid_argument("Cannot export a missing part to oxDNA");
	SBPointerIndexer<ADNPart> parts;
	parts.addReferenceTarget(part());
	OutputToOxDNA(parts, folder, options);
}

void ADNLoader::OutputToOxDNA(SBPointerIndexer<ADNPart> parts, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options) {
	SBPointerIndexer<ADNSingleStrand> strands;
	for (auto part : parts) {
		if (part == nullptr) throw std::invalid_argument("Cannot export a missing part to oxDNA");
		for (auto strand : part->GetSingleStrands()) strands.addReferenceTarget(strand);
	}
	// Validation may fail late in a strand. Prepare both complete files before replacing an existing export.
	const auto text = prepareOxDNAExport(strands, options);
	const auto directory = SBCContainerString::pathFromUtf8(folder);
	std::ofstream outConf(directory / "config.conf"), outTopo(directory / "topo.top");
	if (!outConf || !outTopo) throw std::runtime_error("Could not open oxDNA output files in " + folder);
	writeOxDNAExport(text, outConf, outTopo);
	outConf.close();
	outTopo.close();
	if (!outConf || !outTopo) throw std::runtime_error("Could not finish writing oxDNA output files in " + folder);
}

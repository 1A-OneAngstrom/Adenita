#include "DASCadnano.hpp"
#include "ADNBackbone.hpp"
#include "ADNSidechain.hpp"

#include "rapidjson/error/en.h"

#include <filesystem>
#include <set>
#include <sstream>

namespace {

bool setValidationError(std::string& error, const std::string& message) {

	error = message;
	return false;

}

bool readIntMember(const rapidjson::Value& object,
	const char* memberName,
	int& out,
	std::string& error,
	const std::string& context) {

	if (!object.IsObject())
		return setValidationError(error, context + " must be a JSON object.");

	const auto memberIt = object.FindMember(memberName);
	if (memberIt == object.MemberEnd())
		return setValidationError(error, context + " is missing the '" + std::string(memberName) + "' field.");
	if (!memberIt->value.IsInt())
		return setValidationError(error, context + " field '" + std::string(memberName) + "' must be an integer.");

	out = memberIt->value.GetInt();
	return true;

}

bool readArrayMember(const rapidjson::Value& object,
	const char* memberName,
	const rapidjson::Value*& out,
	std::string& error,
	const std::string& context) {

	if (!object.IsObject())
		return setValidationError(error, context + " must be a JSON object.");

	const auto memberIt = object.FindMember(memberName);
	if (memberIt == object.MemberEnd())
		return setValidationError(error, context + " is missing the '" + std::string(memberName) + "' array.");
	if (!memberIt->value.IsArray())
		return setValidationError(error, context + " field '" + std::string(memberName) + "' must be an array.");

	out = &memberIt->value;
	return true;

}

bool readVec4(const rapidjson::Value& value,
	vec4& out,
	std::string& error,
	const std::string& context) {

	if (!value.IsArray() || value.Size() != 4)
		return setValidationError(error, context + " must be an array with 4 integer entries.");

	for (rapidjson::SizeType i = 0; i < value.Size(); ++i) {
		if (!value[i].IsInt())
			return setValidationError(error, context + " must contain only integer entries.");
	}

	out.n0 = value[0].GetInt();
	out.n1 = value[1].GetInt();
	out.n2 = value[2].GetInt();
	out.n3 = value[3].GetInt();
	return true;

}

bool validateLink(int linkedVstrandId,
	int linkedPosition,
	const std::map<int, Vstrand>& vstrands,
	std::string& error,
	const std::string& context) {

	const bool noLink = linkedVstrandId == -1 && linkedPosition == -1;
	const bool hasLink = linkedVstrandId >= 0 && linkedPosition >= 0;
	if (!noLink && !hasLink)
		return setValidationError(error, context + " must use either [-1, -1] or a non-negative [vstrand, position] pair.");
	if (!hasLink) return true;

	const auto vstrandIt = vstrands.find(linkedVstrandId);
	if (vstrandIt == vstrands.end())
		return setValidationError(error, context + " references missing vstrand " + std::to_string(linkedVstrandId) + ".");

	if (linkedPosition >= vstrandIt->second.totalLength_)
		return setValidationError(error,
			context + " references position " + std::to_string(linkedPosition) +
			" outside vstrand " + std::to_string(linkedVstrandId) +
			" length " + std::to_string(vstrandIt->second.totalLength_) + ".");

	return true;

}

bool validateConnection(const vec4& entry,
	const std::map<int, Vstrand>& vstrands,
	std::string& error,
	const std::string& context) {

	return validateLink(entry.n0, entry.n1, vstrands, error, context + " previous link") &&
		validateLink(entry.n2, entry.n3, vstrands, error, context + " next link");

}

}

void DASCadnano::ResetState() {

	json_ = CadnanoJSONFile{};
	vGrid_ = VGrid{};
	cellBsMap_.clear();
	ntPositions_.clear();
	ssId_.clear();
	lastKey = -1;
	lastError_.clear();
	conformation3D_ = nullptr;
	conformation2D_ = nullptr;
	conformation1D_ = nullptr;

}

bool DASCadnano::Fail(const std::string& message) {

	lastError_ = message;
	ADNLogger::LogError(message);
	return false;

}

bool DASCadnano::ParseJSON(const std::string& filename) {

	FILE* fp = nullptr;
	const auto closeFile = [&fp]() {
		if (fp != nullptr) {
			fclose(fp);
			fp = nullptr;
		}
	};

	try {

		std::filesystem::path filepath = std::filesystem::u8path(filename);
#ifdef _WIN32
		// convert to a wide string (UTF-8) to take care of special characters
		fp = _wfopen(filepath.c_str(), L"rb");
#else
		fp = fopen(filepath.c_str(), "rb");
#endif

	}
	catch (...) {

		return Fail("Adenita couldn't open the cadnano file.");

	}

	if (fp == nullptr)
		return Fail("Adenita couldn't open the cadnano file.");

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document d;
	d.ParseStream(is);

	if (d.HasParseError()) {

		std::ostringstream message;
		message << "The cadnano file is not valid JSON";
		if (const char* parseError = rapidjson::GetParseError_En(d.GetParseError())) {
			message << " (" << parseError << " near offset " << d.GetErrorOffset() << ")";
		}
		message << ".";
		closeFile();
		return Fail(message.str());

	}

	if (!d.IsObject()) {

		closeFile();
		return Fail("The cadnano file must contain a JSON object at the root.");

	}

	// check for save version
	double versionValue = 0.0;
	if (rapidjson::Value* version = rapidjson::Pointer("/format").Get(d)) {
		if (!version->IsNumber()) {
			closeFile();
			return Fail("The cadnano file has a non-numeric 'format' value.");
		}
		versionValue = version->GetDouble();
	}

	bool parsed = false;
	if (versionValue < 3.0) {
		parsed = ParseCadnanoLegacy(d);
	}
	else {
		parsed = ParseCadnanoFormat3(d);
	}

	closeFile();
	return parsed;

}

bool DASCadnano::ParseCadnanoFormat3(rapidjson::Document&) {

	return Fail("Cadnano format 3.0 is not supported yet.");

}

bool DASCadnano::ParseCadnanoLegacy(rapidjson::Document& d) {

	if (d.HasMember("name") && d["name"].IsString())
		json_.name_ = d["name"].GetString();

	std::string validationError;
	const rapidjson::Value* vstrandsVal = nullptr;
	if (!readArrayMember(d, "vstrands", vstrandsVal, validationError, "Cadnano file"))
		return Fail(validationError);
	if (vstrandsVal->Empty())
		return Fail("Cadnano file does not contain any virtual strands.");

	int totalCount = -1;
	for (rapidjson::SizeType i = 0; i < vstrandsVal->Size(); ++i) {

		const rapidjson::Value& vstrandVal = (*vstrandsVal)[i];
		if (!vstrandVal.IsObject())
			return Fail("Cadnano vstrand entry " + std::to_string(static_cast<int>(i)) + " must be a JSON object.");

		const std::string vstrandContext = "Cadnano vstrand " + std::to_string(static_cast<int>(i));
		Vstrand vstrand;

		if (!readIntMember(vstrandVal, "num", vstrand.num_, validationError, vstrandContext) ||
			!readIntMember(vstrandVal, "col", vstrand.col_, validationError, vstrandContext) ||
			!readIntMember(vstrandVal, "row", vstrand.row_, validationError, vstrandContext))
			return Fail(validationError);

		if (json_.vstrands_.find(vstrand.num_) != json_.vstrands_.end())
			return Fail("Cadnano file contains duplicate vstrand id " + std::to_string(vstrand.num_) + ".");

		const rapidjson::Value* scafVals = nullptr;
		const rapidjson::Value* stapVals = nullptr;
		const rapidjson::Value* loopVals = nullptr;
		const rapidjson::Value* skipVals = nullptr;
		if (!readArrayMember(vstrandVal, "scaf", scafVals, validationError, vstrandContext) ||
			!readArrayMember(vstrandVal, "stap", stapVals, validationError, vstrandContext) ||
			!readArrayMember(vstrandVal, "loop", loopVals, validationError, vstrandContext) ||
			!readArrayMember(vstrandVal, "skip", skipVals, validationError, vstrandContext))
			return Fail(validationError);

		if (stapVals->Size() != scafVals->Size() || loopVals->Size() != scafVals->Size() || skipVals->Size() != scafVals->Size()) {
			return Fail("Cadnano vstrand " + std::to_string(vstrand.num_) + " has inconsistent array lengths.");
		}

		int count = 0;
		for (rapidjson::SizeType k = 0; k < scafVals->Size(); ++k) {
			vec4 elem;
			if (!readVec4((*scafVals)[k], elem, validationError,
				"Cadnano vstrand " + std::to_string(vstrand.num_) + " scaffold entry " + std::to_string(static_cast<int>(k))))
				return Fail(validationError);
			vstrand.scaf_.insert(std::make_pair(count, elem));

			//start point
			if (elem.n0 == -1 && elem.n1 == -1 && elem.n2 != -1 && elem.n3 != -1) {
				json_.scaffoldStartPositions_.push_back(std::make_pair(vstrand.num_, count));
			}

			++count;
		}

		if (count == 0)
			return Fail("Cadnano vstrand " + std::to_string(vstrand.num_) + " does not contain any positions.");

		totalCount = count;  // all vhelix have the same count
		vstrand.totalLength_ = totalCount;
		if (totalCount != static_cast<int>(scafVals->Size()))
			return Fail("Cadnano vstrand " + std::to_string(vstrand.num_) + " contains too many positions.");

		if (json_.vstrands_.empty()) {
			totalCount = count;
		}
		else if (count != json_.vstrands_.begin()->second.totalLength_) {
			return Fail("Cadnano virtual strands must all have the same number of positions.");
		}

		count = 0;
		for (rapidjson::SizeType k = 0; k < stapVals->Size(); ++k) {

			vec4 elem;
			if (!readVec4((*stapVals)[k], elem, validationError,
				"Cadnano vstrand " + std::to_string(vstrand.num_) + " staple entry " + std::to_string(static_cast<int>(k))))
				return Fail(validationError);
			vstrand.stap_.insert(std::make_pair(count, elem));

			if (elem.n0 == -1 && elem.n1 == -1 && elem.n2 != -1 && elem.n3 != -1) {

				vec2 stapleStart;
				stapleStart.n0 = vstrand.num_;
				stapleStart.n1 = count;

				json_.stapleStarts_.push_back(stapleStart);

			}

			++count;

		}

		count = 0;
		for (rapidjson::SizeType k = 0; k < loopVals->Size(); ++k) {

			if (!(*loopVals)[k].IsInt())
				return Fail("Cadnano vstrand " + std::to_string(vstrand.num_) + " loop entry " + std::to_string(static_cast<int>(k)) + " must be an integer.");
			const int loopValue = (*loopVals)[k].GetInt();
			if (loopValue < 0)
				return Fail("Cadnano vstrand " + std::to_string(vstrand.num_) + " loop entry " + std::to_string(static_cast<int>(k)) + " must be non-negative.");
			vstrand.loops_.insert(std::make_pair(count, loopValue));
			++count;

		}

		count = 0;
		for (rapidjson::SizeType k = 0; k < skipVals->Size(); ++k) {

			if (!(*skipVals)[k].IsInt())
				return Fail("Cadnano vstrand " + std::to_string(vstrand.num_) + " skip entry " + std::to_string(static_cast<int>(k)) + " must be an integer.");
			vstrand.skips_.insert(std::make_pair(count, (*skipVals)[k].GetInt()));
			++count;

		}

		// create tubes
		bool start_tube = false;
		int init_pos = -1;
		int end_pos = -1;
		for (int i = 0; i < vstrand.totalLength_; ++i) {

			vec4 scaf_pos = vstrand.scaf_[i];
			vec4 stap_pos = vstrand.stap_[i];
			if (IsThereBase(scaf_pos) || IsThereBase(stap_pos)) {

				if (!start_tube) {

					start_tube = true;
					init_pos = i;
					end_pos = -1;

				}

			}
			else {

				if (start_tube) {

					start_tube = false;
					end_pos = i - 1;
					VTube tube;
					tube.vStrandId_ = vstrand.num_;
					tube.initPos_ = init_pos;
					tube.endPos_ = end_pos;
					vGrid_.AddTube(tube);
					init_pos = -1;

				}

			}

		}

		if (start_tube) {

			VTube tube;
			tube.vStrandId_ = vstrand.num_;
			tube.initPos_ = init_pos;
			tube.endPos_ = vstrand.totalLength_ - 1;
			vGrid_.AddTube(tube);

		}

		json_.vstrands_.insert(std::make_pair(vstrand.num_, vstrand));

	}

	for (const auto& vstrandPair : json_.vstrands_) {

		const Vstrand& vstrand = vstrandPair.second;
		for (const auto& scafPair : vstrand.scaf_) {
			const std::string context = "Cadnano scaffold entry at vstrand " + std::to_string(vstrand.num_) + ", position " + std::to_string(scafPair.first);
			if (!validateConnection(scafPair.second, json_.vstrands_, validationError, context))
				return Fail(validationError);
		}

		for (const auto& stapPair : vstrand.stap_) {
			const std::string context = "Cadnano staple entry at vstrand " + std::to_string(vstrand.num_) + ", position " + std::to_string(stapPair.first);
			if (!validateConnection(stapPair.second, json_.vstrands_, validationError, context))
				return Fail(validationError);
		}

	}

	if (totalCount % 32 == 0) {
		json_.lType_ = LatticeType::Square;
	}
	else if (totalCount % 21 == 0) {
		json_.lType_ = LatticeType::Honeycomb;
	}
	else {

		// error
		if (totalCount == -1) {
			return Fail("Adenita couldn't find a compatible lattice: design seems empty.");
		}
		else {
			return Fail("Adenita couldn't find a compatible lattice: number of vHelix positions = " + std::to_string(totalCount) + ".");
		}

	}

	vGrid_.CreateLattice(json_.lType_);
	return true;

}

SBPointer<ADNPart> DASCadnano::CreateCadnanoModel() {

	SBPointer<ADNPart> part = new ADNPart();

	CreateEdgeMap(part);
	ADNLogger::LogDebug(std::string("Cadnano module > Double strands created"));
	if (!CreateScaffold(part)) return nullptr;
	if (json_.scaffoldStartPositions_.size() > 0) {
		ADNLogger::LogDebug(std::string("Cadnano module > Scaffold(s) created"));
	}
	else {
		ADNLogger::LogError(std::string("Adenita couldn't detect a scaffold. Circular scaffolds won't be detected"));
	}
	if (!CreateStaples(part)) return nullptr;
	ADNLogger::LogDebug(std::string("Cadnano module > Staples created"));

	return part;

}

void DASCadnano::CreateEdgeMap(SBPointer<ADNPart> part) {

	const auto& tubes = vGrid_.vDoubleStrands_;

	for (auto& tube : tubes) {

		Vstrand* vs = &json_.vstrands_[tube.vStrandId_];
		const SBPosition3 initPos = vGrid_.GetGridCellPos3D(tube.initPos_, vs->row_, vs->col_);
		const SBPosition3 endPos = vGrid_.GetGridCellPos3D(tube.endPos_, vs->row_, vs->col_);
		const int length = tube.endPos_ - tube.initPos_ + 1;
		const SBVector3 dir = (endPos - initPos).normalizedVersion();

		std::map<std::pair<int, int>, SBPointer<ADNBaseSegment>> positions;
		if (cellBsMap_.find(vs) != cellBsMap_.end()) positions = cellBsMap_.at(vs);

		SEConfig& config = SEConfig::GetInstance();

		int bs_number = tube.initPos_;
		// fix crossovers for square lattice
		double initAng = 0.0;
		if (json_.lType_ == LatticeType::Square) initAng = 7 * ADNConstants::BP_ROT;

		// every tube is a double strand
		SBPointer<ADNDoubleStrand> ds = new ADNDoubleStrand();
		part->RegisterDoubleStrand(ds);
		bool firstBs = true;
		ds->SetInitialTwistAngle(initAng);

		// position offset due to skip pairs
		int posOffset = 0;

		SBPosition3 fp = initPos;
		for (int i = 0; i < length; ++i) {

			// take into account the loops, we place for now base segments of loops in the same position as the previous base segment
			int max_iter = vs->loops_[tube.initPos_ + i];
			bool skip = vs->skips_[tube.initPos_ + i] == -1;
			if (max_iter > 0) max_iter = 1;  // a loop is contained in a base segment
			for (int k = 0; k <= max_iter; k++) {

				int z = tube.initPos_ + i - posOffset;
				fp = vGrid_.GetGridCellPos3D(z, vs->row_, vs->col_);
				double factor = 1.0;
				if ((vs->row_ + vs->col_) % 2 == 0) factor = -1.0;

				SBPointer<ADNBaseSegment> bs = new ADNBaseSegment();
				if (skip) {
					bs->SetCell(new ADNSkipPair());
					--bs_number;
					++posOffset;
				}
				else if (k > 0) {
					bs->SetCell(new ADNLoopPair());
				}
				else {
					bs->SetCell(new ADNBasePair());
				}
				part->RegisterBaseSegmentEnd(ds, bs);
				bs->SetNumber(bs_number);
				bs->SetPosition(fp);
				bs->SetE3(ADNAuxiliary::SBVectorToUblasVector(dir));

				std::pair<int, int> key = std::make_pair(tube.initPos_ + i, k);
				positions.insert(std::make_pair(key, bs));
				++bs_number;

			}

		}

		cellBsMap_[vs] = positions;

	}

}

bool DASCadnano::CreateScaffold(SBPointer<ADNPart> part) {

	for (auto& p : json_.scaffoldStartPositions_) {

		//look for stating point of scaffold in vstrand
		int startVstrand = p.first;
		int startVstrandPos = p.second;

		//create the scaffold strand
		SBPointer<ADNSingleStrand> scaff = new ADNSingleStrand();
		scaff->setName("Scaffold");
		scaff->setScaffoldFlag(true);
		part->RegisterSingleStrand(scaff);
		AddSingleStrandToMap(scaff);

		//trace scaffold through vstrands
		if (!TraceSingleStrand(startVstrand, startVstrandPos, scaff, part))
			return false;

	}

	return true;

}

bool DASCadnano::CreateStaples(SBPointer<ADNPart> part) {

	//find number of staples and their starting points
	std::vector<vec2> stapleStarts = json_.stapleStarts_;  //vstrand id and position on vstrand
	std::string numStaplesString;
	numStaplesString += "Cadnano module > Detected " + std::to_string(stapleStarts.size()) + " staples";
	ADNLogger::LogDebug(numStaplesString);

	auto& vstrands = json_.vstrands_;
	int sid = 1; //because scaffold is chain 0

	for (vec2 curStapleStart : stapleStarts) {

		int vStrandId = curStapleStart.n0;
		int z = curStapleStart.n1;
		const auto vstrandIt = vstrands.find(vStrandId);
		if (vstrandIt == vstrands.end())
			return Fail("Cadnano staple references missing vstrand " + std::to_string(vStrandId) + ".");
		if (vstrandIt->second.stap_.find(z) == vstrandIt->second.stap_.end())
			return Fail("Cadnano staple references missing position " + std::to_string(z) + " in vstrand " + std::to_string(vStrandId) + ".");
		if (cellBsMap_.find(&vstrandIt->second) == cellBsMap_.end())
			return Fail("Cadnano staple on vstrand " + std::to_string(vStrandId) + " is missing base-segment data.");

		SBPointer<ADNSingleStrand> staple = new ADNSingleStrand();
		staple->setName("Staple " + std::to_string(sid));
		staple->setStructuralID(sid);
		++sid;
		staple->setScaffoldFlag(false);
		part->RegisterSingleStrand(staple);
		AddSingleStrandToMap(staple);

		if (!TraceSingleStrand(vStrandId, z, staple, part, false))
			return false;

	}

	return true;

}

bool DASCadnano::TraceSingleStrand(int startVStrand, int startVStrandPos, SBPointer<ADNSingleStrand> ss, SBPointer<ADNPart> part, bool scaf) {

	if (part == nullptr) return Fail("Cadnano import cannot trace a strand into a null part.");

	//trace scaffold through vstrands
	auto& vstrands = json_.vstrands_;
	const std::string strandType = scaf ? "scaffold" : "staple";

	const auto startVstrandIt = vstrands.find(startVStrand);
	if (startVstrandIt == vstrands.end())
		return Fail("Cadnano " + strandType + " start references missing vstrand " + std::to_string(startVStrand) + ".");
	const auto startEntryIt = scaf ? startVstrandIt->second.scaf_.find(startVStrandPos) : startVstrandIt->second.stap_.find(startVStrandPos);
	if (startEntryIt == (scaf ? startVstrandIt->second.scaf_.end() : startVstrandIt->second.stap_.end()))
		return Fail("Cadnano " + strandType + " start references missing position " + std::to_string(startVStrandPos) + " in vstrand " + std::to_string(startVStrand) + ".");

	int vStrandId = startVStrand;
	int z = startVStrandPos;
	vec4 curVstrandElem = startEntryIt->second;
	int count = 0;
	std::set<std::pair<int, int>> visitedPositions;

	while (true) {

		if (!visitedPositions.insert(std::make_pair(vStrandId, z)).second)
			return Fail("Cadnano " + strandType + " path revisits vstrand " + std::to_string(vStrandId) + " position " + std::to_string(z) + ".");

		const auto vstrandIt = vstrands.find(vStrandId);
		if (vstrandIt == vstrands.end())
			return Fail("Cadnano " + strandType + " references missing vstrand " + std::to_string(vStrandId) + ".");

		const auto bsPositionsIt = cellBsMap_.find(&vstrandIt->second);
		if (bsPositionsIt == cellBsMap_.end())
			return Fail("Cadnano " + strandType + " on vstrand " + std::to_string(vStrandId) + " is missing base-segment data.");

		const auto& bs_positions = bsPositionsIt->second;
		const auto skipIt = vstrandIt->second.skips_.find(z);
		if (skipIt == vstrandIt->second.skips_.end())
			return Fail("Cadnano " + strandType + " is missing skip data at vstrand " + std::to_string(vStrandId) + " position " + std::to_string(z) + ".");

		if (skipIt->second != -1) {

			const auto loopIt = vstrandIt->second.loops_.find(z);
			if (loopIt == vstrandIt->second.loops_.end())
				return Fail("Cadnano " + strandType + " is missing loop data at vstrand " + std::to_string(vStrandId) + " position " + std::to_string(z) + ".");

			int max_iter = loopIt->second;
			bool left = true;
			if (scaf && vStrandId % 2 == 0) left = false;
			else if (!scaf && vStrandId % 2 != 0) left = false;

			for (int k = 0; k <= max_iter; k++) {

				//add loop
				SBPointer<ADNNucleotide> nt = new ADNNucleotide();
				nt->Init();
				part->RegisterNucleotideThreePrime(ss, nt);
				ntPositions_.insert(std::make_pair(nt(), count));
				++count;

				const SBPosition3 pos3D = vGrid_.GetGridCellPos3D(z, vstrandIt->second.row_, vstrandIt->second.col_);

				// fetch base segment
				int p = 0;
				if (k > 0) p = 1;
				SBPointer<ADNBaseSegment> bs = nullptr;
				std::pair<int, int> key = std::make_pair(z, p);
				if (bs_positions.find(key) != bs_positions.end()) {
					bs = bs_positions.at(key);
				}
				if (bs == nullptr) continue;

				if (k == 0) {

					// even for a loop nfirst base is always BasePair
					SBPointer<ADNCell> c = bs->GetCell();
					if (c != nullptr && c->GetCellType() == CellType::BasePair) {

						SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(c());
						if (left) bp->SetLeftNucleotide(nt);
						else bp->SetRightNucleotide(nt);

						bp->PairNucleotides();
						nt->SetBaseSegment(bs);

					}
					else {

						std::string msg = "Expected BasePair but found another cell type";
						ADNLogger::LogDebug(msg);

					}

				}
				else {

					// We have a loop
					SBPointer<ADNCell> c = bs->GetCell();
					if (c != nullptr && c->GetCellType() == CellType::LoopPair) {

						SBPointer<ADNLoopPair> lp = static_cast<ADNLoopPair*>(c());
						SBPointer<ADNLoop> loop;
						if (left) loop = lp->GetLeftLoop();
						else loop = lp->GetRightLoop();

						if (loop == nullptr) {

							// first time we need to create
							loop = new ADNLoop();
							if (left) {
								lp->SetLeftLoop(loop);
								loop->SetStart(nt);
							}
							else {
								lp->SetRightLoop(loop);
								loop->SetEnd(nt);
							}

						}

						loop->AddNucleotide(nt);
						if (left) loop->SetEnd(nt);
						else loop->SetStart(nt);
						nt->SetBaseSegment(bs);

					}
					else {

						std::string msg = "Expected LoopPair but found another cell type";
						ADNLogger::LogDebug(msg);

					}

				}

				nt->SetPosition(pos3D);
				nt->SetBackbonePosition(pos3D);
				nt->SetSidechainPosition(pos3D);

			}

		}

		if (curVstrandElem.n2 == -1 && curVstrandElem.n3 == -1) {
			break;
		}
		if (curVstrandElem.n2 < 0 || curVstrandElem.n3 < 0)
			return Fail("Cadnano " + strandType + " has an incomplete next link at vstrand " + std::to_string(vStrandId) + " position " + std::to_string(z) + ".");

		//find next scaffold element
		const auto nextVstrandIt = vstrands.find(curVstrandElem.n2);
		if (nextVstrandIt == vstrands.end())
			return Fail("Cadnano " + strandType + " next link references missing vstrand " + std::to_string(curVstrandElem.n2) + ".");

		const auto nextEntryIt = scaf ? nextVstrandIt->second.scaf_.find(curVstrandElem.n3) : nextVstrandIt->second.stap_.find(curVstrandElem.n3);
		if (nextEntryIt == (scaf ? nextVstrandIt->second.scaf_.end() : nextVstrandIt->second.stap_.end()))
			return Fail("Cadnano " + strandType + " next link references missing position " + std::to_string(curVstrandElem.n3) + " in vstrand " + std::to_string(curVstrandElem.n2) + ".");

		vStrandId = curVstrandElem.n2;
		z = curVstrandElem.n3;
		curVstrandElem = nextEntryIt->second;

	}

	return true;

}

bool DASCadnano::CreateConformations(SBPointer<ADNPart> part) {

	if (part == nullptr)
		return Fail("Cadnano import cannot create conformations for a null part.");

	const std::string name = part->getName();
	SBNodeIndexer nodeIndexer;
	part->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

	conformation3D_ = new ADNConformation(name + " 3D", nodeIndexer);
	conformation2D_ = new ADNConformation(name + " 2D", nodeIndexer);
	conformation1D_ = new ADNConformation(name + " 1D", nodeIndexer);

	SBPosition3 center3D;
	SBPosition3 center2D;
	SBPosition3 center1D;
	int n1D = 0;
	int n3D = 0;
	// find out centers
	for (auto it = cellBsMap_.begin(); it != cellBsMap_.end(); ++it) {

		Vstrand* vs = it->first;
		int vStrandId = vs->num_;
		std::map<std::pair<int, int>, SBPointer<ADNBaseSegment>> values = it->second;

		for (auto jt = values.begin(); jt != values.end(); ++jt) {

			std::pair<int, int> bsNumAndLoop = jt->first;
			SBPointer<ADNBaseSegment> bs = jt->second;
			int z = bsNumAndLoop.first;
			auto nts = bs->GetNucleotides();
			SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

				SBPointer<ADNSingleStrand> ss = nt->GetStrand();
				SBPosition3 pos2D = vGrid_.GetGridCellPos2D(vStrandId, z, ss->IsScaffold());
				SBPosition3 pos1D = vGrid_.GetGridCellPos1D(ssId_[ss()], ntPositions_[nt()]);
				center3D += nt->GetPosition();
				++n3D;
				center2D += pos2D;
				if (!ss->IsScaffold()) {

					center1D += pos1D;
					++n1D;  // remove scaffold in case it's too long

				}

			}

		}

	}

	if (n3D == 0)
		return Fail("Cadnano import produced no nucleotides, so conformations could not be created.");

	center3D /= n3D;
	center2D /= n3D;
	if (n1D > 0) center1D /= n1D;
	else center1D = center3D;

	// set positions
	for (auto it = cellBsMap_.begin(); it != cellBsMap_.end(); ++it) {

		Vstrand* vs = it->first;
		int vStrandId = vs->num_;
		std::map<std::pair<int, int>, SBPointer<ADNBaseSegment>> values = it->second;

		for (auto jt = values.begin(); jt != values.end(); ++jt) {

			std::pair<int, int> bsNumAndLoop = jt->first;
			SBPointer<ADNBaseSegment> bs = jt->second;
			int z = bsNumAndLoop.first;
			auto nts = bs->GetNucleotides();
			SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

				SBPointer<ADNSingleStrand> ss = nt->GetStrand();
				SBPosition3 pos2D = vGrid_.GetGridCellPos2D(vStrandId, z, ss->IsScaffold());
				SBPosition3 pos1D = vGrid_.GetGridCellPos1D(ssId_[ss()], ntPositions_[nt()]);
				pos2D = pos2D - center2D + center3D;
				pos1D = pos1D - center1D + center3D;

				if (nt->GetBackbone() != nullptr) {

					auto ats = nt->GetBackbone()->GetAtoms();
					SB_FOR(SBPointer<ADNAtom> at, ats) {
						conformation2D_->setPosition(at(), pos2D);
						conformation1D_->setPosition(at(), pos1D);
					}

				}

				if (nt->GetSidechain() != nullptr) {

					auto ats = nt->GetSidechain()->GetAtoms();
					SB_FOR(SBPointer<ADNAtom> at, ats) {
						conformation2D_->setPosition(at(), pos2D);
						conformation1D_->setPosition(at(), pos1D);
					}

				}

			}

		}

	}

	return true;

}

const std::string& DASCadnano::GetLastError() const {
	return lastError_;
}

SBPointer<ADNConformation> DASCadnano::Get3DConformation() {
	return conformation3D_;
}

SBPointer<ADNConformation> DASCadnano::Get2DConformation() {
	return conformation2D_;
}

SBPointer<ADNConformation> DASCadnano::Get1DConformation() {
	return conformation1D_;
}

SBPointer<ADNPart> DASCadnano::CreateCadnanoPart(std::string file) {

	ResetState();
	if (!ParseJSON(file))
		return nullptr;
	ADNLogger::LogDebug(std::string("Cadnano design parsed"));
	if (vGrid_.vDoubleStrands_.size() == 0) {

		Fail("Adenita couldn't create a cadnano model from the parsed helices.");
		return nullptr;

	}

	return CreateCadnanoModel();

}

DNABlocks DASCadnano::GetComplementaryBase(DNABlocks type) {
	return ADNModel::GetComplementaryBase(type);
}

bool DASCadnano::IsThereBase(vec4 data) {

	bool base = false;

	if ((data.n0 != -1 && data.n1 != -1) || (data.n2 != -1 && data.n3 != -1)) base = true;

	return base;

}

void DASCadnano::AddSingleStrandToMap(SBPointer<ADNSingleStrand> ss) {

	int key = lastKey + 1;
	ssId_.insert(std::make_pair(ss(), key));
	lastKey = key;

}

void VGrid::CreateLattice(LatticeType lType) {

	// in cadnano 2.0 square lattice is 50x50, honeycomb is 30 x 32
	int maxRows = 55;
	int maxCols = 55;
	if (lType == LatticeType::Honeycomb) {
		maxRows = 30;
		maxCols = 32;
	}

	lattice_ = DASLattice(lType, dh_diameter_, maxRows, maxCols);

}

void VGrid::AddTube(VTube tube) {
	vDoubleStrands_.push_back(tube);
}

SBPosition3 VGrid::GetGridCellPos3D(int z, unsigned int row, unsigned int column) {

	auto zPos = z * ADNConstants::BP_RISE;
	LatticeCell lc = lattice_.GetLatticeCell(row, column);
	SBPosition3 pos = SBPosition3(SBQuantity::nanometer(zPos), SBQuantity::nanometer(lc.x_), SBQuantity::nanometer(lc.y_));
	return pos;

}

SBPosition3 VGrid::GetGridCellPos2D(int vStrandId, int z, bool isScaffold) {

	double hPos = vStrandId * ADNConstants::DH_DIAMETER;
	double zPos = z * ADNConstants::BP_RISE;

	if (vStrandId % 2 == 0) {
		if (!isScaffold) hPos += ADNConstants::BP_CADNANO_DIST;
	}
	else {
		if (isScaffold) hPos += ADNConstants::BP_CADNANO_DIST;
	}

	SBPosition3 pos(SBQuantity::nanometer(zPos), SBQuantity::nanometer(hPos), SBQuantity::nanometer(0.0));

	return pos;

}

SBPosition3 VGrid::GetGridCellPos1D(int ssId, int ntNum) {

	const double x = ssId * ADNConstants::DH_DIAMETER * 0.25;
	const double y = -ntNum / 2.0;

	SBPosition3 pos(SBQuantity::nanometer(x), SBQuantity::nanometer(y), SBQuantity::nanometer(0.0));

	return pos;

}

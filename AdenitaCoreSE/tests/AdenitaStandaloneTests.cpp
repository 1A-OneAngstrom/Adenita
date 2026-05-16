/// \file AdenitaStandaloneTests.cpp
/// \brief Standalone smoke tests for Adenita code that does not launch SAMSON.

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "ADNBaseSegment.hpp"
#include "ADNCell.hpp"
#include "ADNArray.hpp"
#include "ADNConfig.hpp"
#include "ADNConfigFileIO.hpp"
#include "ADNConfigJson.hpp"
#include "ADNJsonValidation.hpp"
#include "ADNPart.hpp"
#include "ADNSaveAndLoad.hpp"
#include "ADNScaffoldReader.hpp"
#include "DASDaedalus.hpp"
#include "SBCHeapExport.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace {

struct TestFailure {
	std::string name;
	std::string message;
	std::string file;
	std::string function;
	int line{ 0 };
};

std::vector<TestFailure> failures;

void recordFailure(const std::string& name,
	const std::string& message,
	const char* file,
	int line,
	const char* function) {

	failures.push_back({ name, message, file ? file : "", function ? function : "", line });

}

void requireTrueAt(const std::string& name,
	bool condition,
	const std::string& message,
	const char* file,
	int line,
	const char* function) {

	if (!condition)
		recordFailure(name, message, file, line, function);

}

template <typename T>
void requireEqualAt(const std::string& name,
	const T& actual,
	const T& expected,
	const char* file,
	int line,
	const char* function) {

	if (!(actual == expected))
		recordFailure(name, "Unexpected value.", file, line, function);

}

template <typename Callable>
void requireThrowsIntAt(const std::string& name,
	Callable callable,
	int expectedValue,
	const char* file,
	int line,
	const char* function) {

	try {
		callable();
		recordFailure(name, "Expected an integer exception.", file, line, function);
	}
	catch (int value) {
		if (value != expectedValue)
			recordFailure(name, "Unexpected exception value.", file, line, function);
	}
	catch (...) {
		recordFailure(name, "Expected an integer exception but caught a different type.", file, line, function);
	}

}

#define requireTrue(name, condition, message) requireTrueAt((name), (condition), (message), __FILE__, __LINE__, __func__)
#define requireEqual(name, actual, expected) requireEqualAt((name), (actual), (expected), __FILE__, __LINE__, __func__)
#define requireThrowsInt(name, callable, expectedValue) requireThrowsIntAt((name), (callable), (expectedValue), __FILE__, __LINE__, __func__)

rapidjson::Document parseJson(const char* json) {

	rapidjson::Document document;
	document.Parse(json);
	return document;

}

std::string serializeJson(const rapidjson::Document& document) {

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document.Accept(writer);
	return buffer.GetString();

}

std::filesystem::path temporaryConfigPath(const std::string& filename) {

	return std::filesystem::temp_directory_path() / filename;

}

std::filesystem::path repoDataPath(const std::string& filename) {

	return std::filesystem::path(__FILE__).parent_path().parent_path() / "data" / filename;

}

ublas::vector<double> vector3(double x, double y, double z) {

	ublas::vector<double> value(3);
	value[0] = x;
	value[1] = y;
	value[2] = z;
	return value;

}

SBPosition3 positionAngstrom(double x, double y, double z) {

	return SBPosition3(SBQuantity::angstrom(x), SBQuantity::angstrom(y), SBQuantity::angstrom(z));

}

std::map<int, SBPosition3> makeTetrahedronVertices() {

	return {
		{ 0, positionAngstrom(0.0, 0.0, 0.0) },
		{ 1, positionAngstrom(1.0, 0.0, 0.0) },
		{ 2, positionAngstrom(0.0, 1.0, 0.0) },
		{ 3, positionAngstrom(0.0, 0.0, 1.0) }
	};

}

std::map<int, std::vector<int>> makeTetrahedronFaces() {

	return {
		{ 0, { 0, 1, 2 } },
		{ 1, { 0, 3, 1 } },
		{ 2, { 1, 3, 2 } },
		{ 3, { 2, 3, 0 } }
	};

}

std::map<int, SBPosition3> makeTriangleVertices() {

	return {
		{ 0, positionAngstrom(0.0, 0.0, 0.0) },
		{ 1, positionAngstrom(1.0, 0.0, 0.0) },
		{ 2, positionAngstrom(0.0, 1.0, 0.0) }
	};

}

std::map<int, std::vector<int>> makeTriangleFaces() {

	return {
		{ 0, { 0, 1, 2 } }
	};

}

std::map<int, SBPosition3> makeOpenSquareVertices() {

	return {
		{ 0, positionAngstrom(0.0, 0.0, 0.0) },
		{ 1, positionAngstrom(1.0, 0.0, 0.0) },
		{ 2, positionAngstrom(1.0, 1.0, 0.0) },
		{ 3, positionAngstrom(0.0, 1.0, 0.0) }
	};

}

std::map<int, std::vector<int>> makeOpenSquareFaces() {

	return {
		{ 0, { 0, 1, 2 } },
		{ 1, { 0, 2, 3 } }
	};

}

size_t countStructuralChild(const SBStructuralGroup& parent, const SBStructuralNode* child) {

	if (child == nullptr) return 0;

	const SBPointerList<SBStructuralNode>* children = parent.getChildren();
	if (children == nullptr) return 0;

	size_t count = 0;
	SB_FOR(SBStructuralNode * node, *children) {
		if (node == child) ++count;
	}
	return count;

}

SBPointer<ADNNucleotide> createSyntheticNucleotide(DNABlocks type,
	double x,
	double y,
	double z,
	const ublas::vector<double>& e2) {

	SBPointer<ADNNucleotide> nucleotide = new ADNNucleotide();
	nucleotide->Init();
	const SBPosition3 sbPosition = SBPosition3(SBQuantity::nanometer(x), SBQuantity::nanometer(y), SBQuantity::nanometer(z));
	nucleotide->setNucleotideType(type);
	nucleotide->SetBackbonePosition(sbPosition);
	nucleotide->SetSidechainPosition(sbPosition);
	nucleotide->SetE1(vector3(0.0, 1.0, 0.0));
	nucleotide->SetE2(e2);
	nucleotide->SetE3(vector3(0.0, 0.0, 1.0));
	return nucleotide;

}

SBPointer<ADNPart> createPartWithBrokenTopScaleLinks(double pairedX) {

	SBPointer<ADNPart> part = new ADNPart();
	SBPointer<ADNSingleStrand> strand = new ADNSingleStrand();
	SBPointer<ADNSingleStrand> pairedStrand = new ADNSingleStrand();
	SBPointer<ADNNucleotide> nucleotide = createSyntheticNucleotide(SBResidue::ResidueType::DA, 0.0, 0.0, 0.0, vector3(1.0, 0.0, 0.0));
	SBPointer<ADNNucleotide> pairedNucleotide = createSyntheticNucleotide(SBResidue::ResidueType::DT, pairedX, 0.0, 0.0, vector3(-1.0, 0.0, 0.0));

	part->RegisterSingleStrand(strand);
	part->RegisterSingleStrand(pairedStrand);
	part->RegisterNucleotideThreePrime(strand, nucleotide);
	part->RegisterNucleotideThreePrime(pairedStrand, pairedNucleotide);

	nucleotide->setEndType(ADNNucleotide::EndType::NotEnd);
	pairedNucleotide->setEndType(ADNNucleotide::EndType::NotEnd);

	return part;

}

struct CircularStrandFixture {
	SBPointer<ADNPart> part;
	SBPointer<ADNSingleStrand> strand;
	SBPointer<ADNNucleotide> fivePrime;
	SBPointer<ADNNucleotide> middle;
	SBPointer<ADNNucleotide> threePrime;
};

CircularStrandFixture createCircularStrandFixture() {

	CircularStrandFixture fixture;
	fixture.part = new ADNPart();
	fixture.part->setName("Circular Part");
	fixture.strand = new ADNSingleStrand();
	fixture.strand->setName("Circular Strand");
	fixture.part->RegisterSingleStrand(fixture.strand);

	fixture.fivePrime = createSyntheticNucleotide(SBResidue::ResidueType::DA, 0.0, 0.0, 0.0, vector3(1.0, 0.0, 0.0));
	fixture.middle = createSyntheticNucleotide(SBResidue::ResidueType::DT, 1.0, 0.0, 0.0, vector3(0.0, 1.0, 0.0));
	fixture.threePrime = createSyntheticNucleotide(SBResidue::ResidueType::DG, 2.0, 0.0, 0.0, vector3(-1.0, 0.0, 0.0));

	fixture.part->RegisterNucleotideThreePrime(fixture.strand, fixture.fivePrime);
	fixture.part->RegisterNucleotideThreePrime(fixture.strand, fixture.middle);
	fixture.part->RegisterNucleotideThreePrime(fixture.strand, fixture.threePrime);
	fixture.strand->setCircularFlag(true);

	return fixture;

}

struct TrackedValue {
	TrackedValue() { ++liveCount; }
	TrackedValue(const TrackedValue& other) : value(other.value) { ++liveCount; }
	TrackedValue(TrackedValue&& other) noexcept : value(other.value) { ++liveCount; }

	~TrackedValue() {
		--liveCount;
		++destructedCount;
	}

	TrackedValue& operator=(const TrackedValue& other) {
		value = other.value;
		return *this;
	}

	TrackedValue& operator=(TrackedValue&& other) noexcept {
		value = other.value;
		return *this;
	}

	static void resetCounters() {
		liveCount = 0;
		destructedCount = 0;
	}

	int value{ 0 };
	static int liveCount;
	static int destructedCount;
};

int TrackedValue::liveCount = 0;
int TrackedValue::destructedCount = 0;

void testConstructionAndAccess() {

	ADNArray<int> values(3, 2);
	requireEqual("construction dimension", values.GetDim(), static_cast<std::size_t>(3));
	requireEqual("construction element count", values.GetNumElements(), static_cast<std::size_t>(2));
	requireTrue("construction backing array", values.GetArray() != nullptr, "Expected a backing array.");

	values(0, 0) = 10;
	values(0, 1) = 11;
	values(0, 2) = 12;
	values(1, 0) = 20;
	values(1, 1) = 21;
	values(1, 2) = 22;

	requireEqual("two dimensional read", values(1, 2), 22);
	requireEqual("one dimensional read", values(1), 20);

}

void testCopyConstructorDeepCopy() {

	ADNArray<int> original(2, 2);
	original(0, 0) = 1;
	original(0, 1) = 2;
	original(1, 0) = 3;
	original(1, 1) = 4;

	ADNArray<int> copy(original);
	original(0, 0) = 99;

	requireEqual("copy constructor dimension", copy.GetDim(), static_cast<std::size_t>(2));
	requireEqual("copy constructor element count", copy.GetNumElements(), static_cast<std::size_t>(2));
	requireEqual("copy constructor deep copy", copy(0, 0), 1);
	requireEqual("copy constructor copied value", copy(1, 1), 4);

}

void testAssignmentCopiesValues() {

	ADNArray<int> source(2, 2);
	source(0, 0) = 5;
	source(0, 1) = 6;
	source(1, 0) = 7;
	source(1, 1) = 8;

	ADNArray<int> assigned(1);
	assigned(0) = 42;
	assigned = source;
	source(1, 1) = 80;

	requireEqual("assignment dimension", assigned.GetDim(), static_cast<std::size_t>(2));
	requireEqual("assignment element count", assigned.GetNumElements(), static_cast<std::size_t>(2));
	requireEqual("assignment copied first value", assigned(0, 0), 5);
	requireEqual("assignment copied last value", assigned(1, 1), 8);

}

void testAssignmentReleasesPreviousStorage() {

	TrackedValue::resetCounters();

	{
		ADNArray<TrackedValue> source(3);
		source(0).value = 1;
		source(1).value = 2;
		source(2).value = 3;

		ADNArray<TrackedValue> assigned(2);
		assigned(0).value = 10;
		assigned(1).value = 20;

		assigned = source;

		requireEqual("tracked assignment dimension", assigned.GetDim(), static_cast<std::size_t>(1));
		requireEqual("tracked assignment element count", assigned.GetNumElements(), static_cast<std::size_t>(3));
		requireEqual("tracked assignment copied first value", assigned(0).value, 1);
		requireEqual("tracked assignment copied last value", assigned(2).value, 3);

		assigned = assigned;

		requireEqual("tracked self assignment first value", assigned(0).value, 1);
		requireEqual("tracked self assignment last value", assigned(2).value, 3);
	}

	requireEqual("tracked assignment released all values", TrackedValue::liveCount, 0);
	requireTrue("tracked assignment destructed values", TrackedValue::destructedCount > 0, "Expected tracked values to be destructed.");

}

void testMoveSemantics() {

	ADNArray<int> source(2, 2);
	source(0, 0) = 1;
	source(0, 1) = 2;
	source(1, 0) = 3;
	source(1, 1) = 4;

	ADNArray<int> moved(std::move(source));
	requireEqual("move constructor dimension", moved.GetDim(), static_cast<std::size_t>(2));
	requireEqual("move constructor element count", moved.GetNumElements(), static_cast<std::size_t>(2));
	requireEqual("move constructor copied value", moved(1, 1), 4);

	ADNArray<int> assigned(1);
	assigned(0) = 9;
	assigned = std::move(moved);
	requireEqual("move assignment dimension", assigned.GetDim(), static_cast<std::size_t>(2));
	requireEqual("move assignment element count", assigned.GetNumElements(), static_cast<std::size_t>(2));
	requireEqual("move assignment copied value", assigned(0, 1), 2);

}

void testRows() {

	ADNArray<int> values(3, 2);
	values(0, 0) = 1;
	values(0, 1) = 2;
	values(0, 2) = 3;
	values(1, 0) = 4;
	values(1, 1) = 5;
	values(1, 2) = 6;

	ADNArray<int> row = values.GetRow(1);
	requireEqual("get row dimension", row.GetDim(), static_cast<std::size_t>(1));
	requireEqual("get row element count", row.GetNumElements(), static_cast<std::size_t>(3));
	requireEqual("get row first value", row(0), 4);
	requireEqual("get row last value", row(2), 6);

	ADNArray<int> replacement(3);
	replacement(0) = 9;
	replacement(1) = 8;
	replacement(2) = 7;
	values.SetRow(0, replacement);

	requireEqual("set row first value", values(0, 0), 9);
	requireEqual("set row last value", values(0, 2), 7);

}

void testExceptions() {

	const int outOfBounds = 30;
	const int dimensionMismatch = 31;

	ADNArray<int> values(2, 2);
	requireThrowsInt("two dimensional out of bounds", [&values]() { (void)values(2, 0); }, outOfBounds);
	requireThrowsInt("one dimensional out of bounds", [&values]() { (void)values(2); }, outOfBounds);

	ADNArray<int> invalidRow(2, 1);
	auto setInvalidRow = [&values, &invalidRow]() { values.SetRow(0, invalidRow); };
	requireThrowsInt("set row dimension mismatch", setInvalidRow, dimensionMismatch);

}

void testScaffoldReaderSkipsBlankLinesAndHeaders() {

	const std::vector<std::string> lines{
		"",
		">first scaffold",
		"ACG",
		"",
		">second scaffold",
		"tnn",
		"   ",
		"  gca  "
	};

	requireEqual("scaffold reader skips blank lines and headers",
		ADNScaffoldReader::readScaffoldLines(lines),
		std::string("ACGTNNGCA"));

}

void testScaffoldReaderAcceptsMissingInitialHeader() {

	const std::vector<std::string> lines{
		"AC",
		"GT",
		">later header",
		"not-a-sequence",
		"NN"
	};

	requireEqual("scaffold reader accepts missing initial header",
		ADNScaffoldReader::readScaffoldLines(lines),
		std::string("ACGTNN"));

}

void testConfigJsonStringMemberCopiesAddedValue() {

	rapidjson::Document document;
	document.SetObject();

	std::string ntthalPath = "C:/tools/ntthal/" + std::string(128, 'a') + ".exe";
	const std::string expected = ntthalPath;

	ADNConfigJson::setStringMember(document, "ntthal", ntthalPath);

	for (char& c : ntthalPath)
		c = 'z';

	requireEqual("config json copied added string member",
		std::string(document["ntthal"].GetString()),
		expected);

	const std::string serialized = serializeJson(document);
	requireTrue("config json serializes added string member",
		serialized.find(expected) != std::string::npos,
		"Expected serialized JSON to contain the copied string value.");

}

void testConfigJsonStringMemberCopiesUpdatedValue() {

	rapidjson::Document document;
	document.SetObject();

	ADNConfigJson::setStringMember(document, "scaffCustomFilename", std::string("initial.fasta"));

	std::string scaffoldPath = "C:/scaffolds/" + std::string(128, 'c') + ".fasta";
	const std::string expected = scaffoldPath;

	ADNConfigJson::setStringMember(document, "scaffCustomFilename", scaffoldPath);

	for (char& c : scaffoldPath)
		c = 'x';

	requireEqual("config json copied updated string member",
		std::string(document["scaffCustomFilename"].GetString()),
		expected);
	requireEqual("config json updated existing member count",
		document.MemberCount(),
		static_cast<rapidjson::SizeType>(1));

	const std::string serialized = serializeJson(document);
	requireTrue("config json serializes updated string member",
		serialized.find(expected) != std::string::npos,
		"Expected serialized JSON to contain the copied string value.");

}

void testConfigFileIoClosesWrittenAndReadFiles() {

	const std::filesystem::path path = temporaryConfigPath("adenita_config_file_io_round_trip.json");
	std::error_code errorCode;
	std::filesystem::remove(path, errorCode);

	rapidjson::Document document;
	document.SetObject();
	ADNConfigJson::setStringMember(document, "ntthal", std::string("C:/tools/ntthal.exe"));

	requireTrue("config file io writes document",
		ADNConfigFileIO::writeDocumentToFile(path.string(), document),
		"Expected JSON document write to succeed.");

	rapidjson::Document readDocument;
	requireTrue("config file io reads document",
		ADNConfigFileIO::readDocumentFromFile(path.string(), readDocument),
		"Expected JSON document read to succeed.");
	requireEqual("config file io round trips string",
		std::string(readDocument["ntthal"].GetString()),
		std::string("C:/tools/ntthal.exe"));

	errorCode.clear();
	const bool removed = std::filesystem::remove(path, errorCode);
	requireTrue("config file io closes file handles",
		removed && !errorCode,
		"Expected the temporary file to be removable after read and write.");

}

void testConfigFileIoReportsFailuresAndClosesInvalidReads() {

	rapidjson::Document document;
	document.SetObject();

	const std::filesystem::path directoryPath = std::filesystem::temp_directory_path();
	requireTrue("config file io reports write open failure",
		!ADNConfigFileIO::writeDocumentToFile(directoryPath.string(), document),
		"Expected writing to a directory path to fail.");

	const std::filesystem::path path = temporaryConfigPath("adenita_config_file_io_invalid.json");
	{
		std::ofstream invalidJson(path);
		invalidJson << "{ invalid json";
	}

	rapidjson::Document readDocument;
	requireTrue("config file io reports invalid read",
		!ADNConfigFileIO::readDocumentFromFile(path.string(), readDocument),
		"Expected invalid JSON to be rejected.");

	std::error_code errorCode;
	const bool removed = std::filesystem::remove(path, errorCode);
	requireTrue("config file io closes invalid read handle",
		removed && !errorCode,
		"Expected the invalid JSON file to be removable after read failure.");

}

void testConcatenate() {

	ADNArray<int> left(2, 2);
	left(0, 0) = 1;
	left(0, 1) = 2;
	left(1, 0) = 3;
	left(1, 1) = 4;

	ADNArray<int> right(2, 1);
	right(0, 0) = 5;
	right(0, 1) = 6;

	std::unique_ptr<ADNArray<int>> combined(ADNArray<int>::Concatenate(left, right));
	requireEqual("concatenate dimension", combined->GetDim(), static_cast<std::size_t>(2));
	requireEqual("concatenate element count", combined->GetNumElements(), static_cast<std::size_t>(3));
	requireEqual("concatenate left first value", (*combined)(0, 0), 1);
	requireEqual("concatenate left last value", (*combined)(1, 1), 4);
	requireEqual("concatenate right first value", (*combined)(2, 0), 5);
	requireEqual("concatenate right last value", (*combined)(2, 1), 6);

	const int dimensionMismatch = 31;
	ADNArray<int> mismatch(3, 1);
	auto concatenateMismatch = [&left, &mismatch]() {
		std::unique_ptr<ADNArray<int>> ignored(ADNArray<int>::Concatenate(left, mismatch));
	};
	requireThrowsInt("concatenate dimension mismatch", concatenateMismatch, dimensionMismatch);

}

void testCircularSingleStrandWrapsWithoutChangingSequenceOrder() {

	const CircularStrandFixture fixture = createCircularStrandFixture();

	requireTrue("circular strand wrap 3 to 5",
		fixture.threePrime->GetNext(true) == fixture.fivePrime,
		"Expected circular strands to wrap from 3' to 5' when requested.");
	requireTrue("circular strand wrap 5 to 3",
		fixture.fivePrime->GetPrev(true) == fixture.threePrime,
		"Expected circular strands to wrap from 5' to 3' when requested.");
	requireTrue("circular strand keeps linear next pointer",
		fixture.threePrime->GetNext() == nullptr,
		"Expected the physical 3' next pointer to remain open.");
	requireTrue("circular strand keeps linear prev pointer",
		fixture.fivePrime->GetPrev() == nullptr,
		"Expected the physical 5' prev pointer to remain open.");
	requireEqual("circular strand sequence remains finite", fixture.strand->GetSequence(), std::string("ATG"));

}

void testModernJsonValidation() {

	auto valid = parseJson(R"json({
		"version": 0.5,
		"name": "valid part",
		"singleStrands": {
			"1": {
				"chainName": "strand",
				"isScaffold": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"10": {
						"type": "A",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"tag": "",
						"next": -1,
						"prev": -1,
						"pair": -1
					}
				}
			}
		},
		"doubleStrands": {
			"2": {
				"initialTwistAngle": 0.0,
				"firstBaseSegment": 20,
				"lastBaseSegment": 20,
				"bases": {
					"20": {
						"position": "0,0,0",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"number": 0,
						"cell": {
							"type": 0,
							"left": 10,
							"right": -1
						},
						"next": -1,
						"previous": -1
					}
				}
			}
		}
	})json");

	requireTrue("valid modern json", ADNLoader::JsonValidation::isValidModernPart(valid, 0.5), "Expected valid modern part JSON.");

	auto validWithCircular = parseJson(R"json({
		"version": 0.5,
		"name": "valid circular part",
		"singleStrands": {
			"1": {
				"chainName": "strand",
				"isScaffold": true,
				"isCircular": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"10": {
						"type": "A",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"tag": "",
						"next": -1,
						"prev": -1,
						"pair": -1
					}
				}
			}
		},
		"doubleStrands": {}
	})json");
	requireTrue("valid modern circular json", ADNLoader::JsonValidation::isValidModernPart(validWithCircular, 0.5), "Expected optional circular metadata to be accepted.");

	auto missingName = parseJson(R"json({
		"version": 0.5,
		"singleStrands": {},
		"doubleStrands": {}
	})json");
	requireTrue("modern json missing name", !ADNLoader::JsonValidation::isValidModernPart(missingName, 0.5), "Expected missing name to be rejected.");

	auto malformedNucleotide = parseJson(R"json({
		"version": 0.5,
		"name": "invalid part",
		"singleStrands": {
			"1": {
				"chainName": "strand",
				"isScaffold": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"10": {
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"tag": "",
						"next": -1,
						"prev": -1,
						"pair": -1
					}
				}
			}
		},
		"doubleStrands": {}
	})json");
	requireTrue("modern json malformed nucleotide", !ADNLoader::JsonValidation::isValidModernPart(malformedNucleotide, 0.5), "Expected malformed nucleotide to be rejected.");

	auto invalidVector = parseJson(R"json({
		"version": 0.5,
		"name": "invalid vector",
		"singleStrands": {
			"1": {
				"chainName": "strand",
				"isScaffold": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"10": {
						"type": "A",
						"position": "0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"tag": "",
						"next": -1,
						"prev": -1,
						"pair": -1
					}
				}
			}
		},
		"doubleStrands": {}
	})json");
	requireTrue("modern json invalid vector", !ADNLoader::JsonValidation::isValidModernPart(invalidVector, 0.5), "Expected invalid vector string to be rejected.");

	auto invalidCircularType = parseJson(R"json({
		"version": 0.5,
		"name": "invalid circular type",
		"singleStrands": {
			"1": {
				"chainName": "strand",
				"isScaffold": true,
				"isCircular": "true",
				"fivePrimeId": 10,
				"nucleotides": {
					"10": {
						"type": "A",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"tag": "",
						"next": -1,
						"prev": -1,
						"pair": -1
					}
				}
			}
		},
		"doubleStrands": {}
	})json");
	requireTrue("modern json invalid circular type", !ADNLoader::JsonValidation::isValidModernPart(invalidCircularType, 0.5), "Expected non-boolean circular metadata to be rejected.");

	auto validParts = parseJson(R"json({
		"version": 0.5,
		"parts": {
			"part0": {
				"name": "valid part",
				"singleStrands": {},
				"doubleStrands": {}
			}
		}
	})json");
	requireTrue("valid parts json", ADNLoader::JsonValidation::isValidPartsDocument(validParts, 0.5), "Expected valid parts document.");

}

void testLegacyJsonValidation() {

	auto validLegacy = parseJson(R"json({
		"version": 0.3,
		"name": "legacy part",
		"strands": {
			"0": {
				"id": 1,
				"chainName": "strand",
				"isScaffold": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"0": {
						"id": 10,
						"type": "A",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"next": -1,
						"prev": -1,
						"pair": {
							"strandId": -1,
							"pairId": -1
						}
					}
				}
			}
		},
		"doubleStrands": {
			"0": {
				"id": 2,
				"initialTwistAngle": 0.0,
				"size": 1,
				"bsStartId": 20
			}
		},
		"joints": {
			"0": {
				"id": 30,
				"position": "0,0,0"
			}
		},
		"bases": {
			"0": {
				"double_strand": 2,
				"source": 30,
				"target": 30,
				"number": 0,
				"normal": "0,1,0",
				"direction": "0,0,1",
				"u": "1,0,0",
				"cell": {
					"type": 0,
					"left": {
						"strand_id": 1,
						"nt_id": 10
					},
					"right": {
						"strand_id": -1,
						"nt_id": -1
					}
				}
			}
		}
	})json");

	requireTrue("valid legacy json", ADNLoader::JsonValidation::isValidLegacyPart(validLegacy, 0.3), "Expected valid legacy part JSON.");

	auto validLegacyWithCircular = parseJson(R"json({
		"version": 0.3,
		"name": "legacy circular part",
		"strands": {
			"0": {
				"id": 1,
				"chainName": "strand",
				"isScaffold": true,
				"isCircular": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"0": {
						"id": 10,
						"type": "A",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"next": -1,
						"prev": -1,
						"pair": {
							"strandId": -1,
							"pairId": -1
						}
					}
				}
			}
		},
		"doubleStrands": {
			"0": {
				"id": 2,
				"initialTwistAngle": 0.0,
				"size": 1,
				"bsStartId": 20
			}
		},
		"joints": {
			"0": {
				"id": 30,
				"position": "0,0,0"
			}
		},
		"bases": {
			"0": {
				"id": 20,
				"position": "0,0,0",
				"next": -1,
				"prev": -1,
				"double_strand": 2,
				"source": 30,
				"target": 30,
				"number": 0,
				"normal": "0,1,0",
				"direction": "0,0,1",
				"u": "1,0,0",
				"cell": {
					"type": 0,
					"left": {
						"strand_id": 1,
						"nt_id": 10
					},
					"right": {
						"strand_id": -1,
						"nt_id": -1
					}
				}
			}
		}
	})json");
	requireTrue("valid legacy circular json", ADNLoader::JsonValidation::isValidLegacyPart(validLegacyWithCircular, 0.3), "Expected optional circular metadata in legacy JSON to be accepted.");

	auto missingVersion = parseJson(R"json({
		"name": "legacy part",
		"strands": {},
		"doubleStrands": {},
		"joints": {},
		"bases": {}
	})json");
	requireTrue("legacy json missing version", !ADNLoader::JsonValidation::isValidLegacyPart(missingVersion, 0.0), "Expected missing version to be rejected.");

	auto malformedPair = parseJson(R"json({
		"version": 0.3,
		"name": "legacy part",
		"strands": {
			"0": {
				"id": 1,
				"chainName": "strand",
				"isScaffold": true,
				"fivePrimeId": 10,
				"nucleotides": {
					"0": {
						"id": 10,
						"type": "A",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"next": -1,
						"prev": -1,
						"pair": {}
					}
				}
			}
		},
		"doubleStrands": {},
		"joints": {},
		"bases": {}
	})json");
	requireTrue("legacy json malformed pair", !ADNLoader::JsonValidation::isValidLegacyPart(malformedPair, 0.3), "Expected malformed legacy pair to be rejected.");

	auto invalidLegacyCircularType = parseJson(R"json({
		"version": 0.3,
		"name": "legacy invalid circular type",
		"strands": {
			"0": {
				"id": 1,
				"chainName": "strand",
				"isScaffold": true,
				"isCircular": "true",
				"fivePrimeId": 10,
				"nucleotides": {
					"0": {
						"id": 10,
						"type": "A",
						"e1": "1,0,0",
						"e2": "0,1,0",
						"e3": "0,0,1",
						"position": "0,0,0",
						"backboneCenter": "0,0,0",
						"sidechainCenter": "0,0,0",
						"next": -1,
						"prev": -1,
						"pair": {
							"strandId": -1,
							"pairId": -1
						}
					}
				}
			}
		},
		"doubleStrands": {
			"0": {
				"id": 2,
				"initialTwistAngle": 0.0,
				"size": 1,
				"bsStartId": 20
			}
		},
		"joints": {
			"0": {
				"id": 30,
				"position": "0,0,0"
			}
		},
		"bases": {
			"0": {
				"id": 20,
				"position": "0,0,0",
				"next": -1,
				"prev": -1,
				"double_strand": 2,
				"source": 30,
				"target": 30,
				"number": 0,
				"normal": "0,1,0",
				"direction": "0,0,1",
				"u": "1,0,0",
				"cell": {
					"type": 0,
					"left": {
						"strand_id": 1,
						"nt_id": 10
					},
					"right": {
						"strand_id": -1,
						"nt_id": -1
					}
				}
			}
		}
	})json");
	requireTrue("legacy json invalid circular type", !ADNLoader::JsonValidation::isValidLegacyPart(invalidLegacyCircularType, 0.3), "Expected legacy circular metadata to be boolean when present.");

}

void testCircularSingleStrandJsonRoundTrip() {

	const CircularStrandFixture fixture = createCircularStrandFixture();

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	writer.StartObject();
	writer.Key("version");
	writer.Double(ADNConstants::JSON_FORMAT_VERSION);
	ADNLoader::SavePartToJson(fixture.part, writer);
	writer.EndObject();

	rapidjson::Document document;
	document.Parse(buffer.GetString());
	const bool parsed = !document.HasParseError() && document.IsObject();
	requireTrue("circular json round-trip parse", parsed, "Expected the serialized circular strand JSON to parse.");
	if (!parsed) return;

	requireTrue("circular json round-trip validates",
		ADNLoader::JsonValidation::isValidModernPart(document, ADNConstants::JSON_FORMAT_VERSION),
		"Expected the serialized circular strand JSON to validate.");

	const bool hasSingleStrands = document.HasMember("singleStrands") && document["singleStrands"].IsObject() && document["singleStrands"].MemberCount() == 1;
	requireTrue("circular json round-trip has one strand", hasSingleStrands, "Expected a single serialized strand.");
	if (!hasSingleStrands) return;

	auto strandIt = document["singleStrands"].MemberBegin();
	const rapidjson::Value& strandValue = strandIt->value;
	requireTrue("circular json round-trip writes circular flag",
		strandValue.HasMember("isCircular") && strandValue["isCircular"].IsBool() && strandValue["isCircular"].GetBool(),
		"Expected serialized strands to preserve circular metadata.");

	const bool hasNucleotides = strandValue.HasMember("nucleotides") && strandValue["nucleotides"].IsObject();
	requireTrue("circular json round-trip writes nucleotides", hasNucleotides, "Expected serialized nucleotides.");
	if (!hasNucleotides) return;

	const rapidjson::Value& nucleotides = strandValue["nucleotides"];
	const std::string fivePrimeId = std::to_string(strandValue["fivePrimeId"].GetInt());
	const std::string threePrimeId = std::to_string(strandValue["threePrimeId"].GetInt());
	auto fivePrimeIt = nucleotides.FindMember(fivePrimeId.c_str());
	auto threePrimeIt = nucleotides.FindMember(threePrimeId.c_str());
	const bool hasFivePrime = fivePrimeIt != nucleotides.MemberEnd();
	const bool hasThreePrime = threePrimeIt != nucleotides.MemberEnd();
	requireTrue("circular json round-trip keeps five prime nucleotide", hasFivePrime, "Expected serialized five-prime nucleotide data.");
	requireTrue("circular json round-trip keeps three prime nucleotide", hasThreePrime, "Expected serialized three-prime nucleotide data.");
	if (!hasFivePrime || !hasThreePrime) return;

	requireEqual("circular json round-trip keeps linear five prime prev", fivePrimeIt->value["prev"].GetInt(), -1);
	requireEqual("circular json round-trip keeps linear three prime next", threePrimeIt->value["next"].GetInt(), -1);

	SBPointer<ADNPart> reloaded = ADNLoader::LoadPartFromJson(document, ADNConstants::JSON_FORMAT_VERSION);
	requireTrue("circular json round-trip reloads part", reloaded != nullptr, "Expected the serialized circular strand to reload.");
	if (reloaded == nullptr) return;

	SBPointer<ADNSingleStrand> reloadedStrand = nullptr;
	auto singleStrands = reloaded->GetSingleStrands();
	SB_FOR(SBPointer<ADNSingleStrand> strand, singleStrands) {
		reloadedStrand = strand;
		break;
	}

	requireTrue("circular json round-trip reloads strand", reloadedStrand != nullptr, "Expected the reloaded part to contain a strand.");
	if (reloadedStrand == nullptr) return;

	requireTrue("circular json round-trip preserves circular flag",
		reloadedStrand->IsCircular(),
		"Expected the reloaded strand to remain circular.");
	requireEqual("circular json round-trip preserves sequence", reloadedStrand->GetSequence(), std::string("ATG"));
	requireTrue("circular json round-trip preserves wrap 3 to 5",
		reloadedStrand->GetThreePrime()->GetNext(true) == reloadedStrand->GetFivePrime(),
		"Expected the reloaded circular strand to wrap from 3' to 5'.");
	requireTrue("circular json round-trip preserves wrap 5 to 3",
		reloadedStrand->GetFivePrime()->GetPrev(true) == reloadedStrand->GetThreePrime(),
		"Expected the reloaded circular strand to wrap from 5' to 3'.");
	requireTrue("circular json round-trip keeps linear topology after reload",
		reloadedStrand->GetThreePrime()->GetNext() == nullptr && reloadedStrand->GetFivePrime()->GetPrev() == nullptr,
		"Expected the reloaded circular strand to keep a linear stored topology.");

}

void testBuildTopScalesHandlesBrokenNucleotideLinks() {

	SBPointer<ADNPart> part = createPartWithBrokenTopScaleLinks(1.0);

	ADNLoader::BuildTopScales(part);

	requireTrue("build top scales handles broken nucleotide links",
		part->GetNumberOfBaseSegments() > 0,
		"Expected top-scale construction to complete and create base segments.");

}

void testBuildTopScalesParametrizedHandlesBrokenNucleotideLinks() {

	SBPointer<ADNPart> part = createPartWithBrokenTopScaleLinks(-1.0);

	ADNLoader::BuildTopScalesParametrized(part,
		SBQuantity::nanometer(1.2),
		SBQuantity::nanometer(0.0),
		49.0);

	requireTrue("build top scales parametrized handles broken nucleotide links",
		part->GetNumberOfBaseSegments() > 0,
		"Expected parametrized top-scale construction to complete and create base segments.");

}

void testBaseSegmentSetCellReplacesChild() {

	ADNBaseSegment baseSegment;
	SBPointer<ADNBasePair> firstCell = new ADNBasePair();
	SBPointer<ADNSkipPair> secondCell = new ADNSkipPair();

	baseSegment.SetCell(firstCell());
	requireTrue("base segment stores initial cell",
		baseSegment.GetCell()() == firstCell(),
		"Expected the initial cell pointer to be stored.");
	requireEqual("base segment initial cell child count",
		countStructuralChild(baseSegment, firstCell()),
		static_cast<size_t>(1));

	baseSegment.SetCell(secondCell());
	requireTrue("base segment stores replacement cell",
		baseSegment.GetCell()() == secondCell(),
		"Expected the replacement cell pointer to be stored.");
	requireEqual("base segment removes old cell child",
		countStructuralChild(baseSegment, firstCell()),
		static_cast<size_t>(0));
	requireEqual("base segment replacement cell child count",
		countStructuralChild(baseSegment, secondCell()),
		static_cast<size_t>(1));

	baseSegment.SetCell(nullptr);
	requireTrue("base segment clears cell pointer",
		baseSegment.GetCell() == nullptr,
		"Expected clearing the cell to reset the stored pointer.");
	requireEqual("base segment clears replacement cell child",
		countStructuralChild(baseSegment, secondCell()),
		static_cast<size_t>(0));

}

void testLoopPairSettersReplaceOnlySelectedChild() {

	ADNLoopPair loopPair;
	SBPointer<ADNLoop> firstLeft = new ADNLoop();
	SBPointer<ADNLoop> secondLeft = new ADNLoop();
	SBPointer<ADNLoop> firstRight = new ADNLoop();
	SBPointer<ADNLoop> secondRight = new ADNLoop();

	loopPair.SetLeftLoop(firstLeft);
	loopPair.SetRightLoop(firstRight);
	requireEqual("loop pair initial left child count",
		countStructuralChild(loopPair, firstLeft()),
		static_cast<size_t>(1));
	requireEqual("loop pair initial right child count",
		countStructuralChild(loopPair, firstRight()),
		static_cast<size_t>(1));

	loopPair.SetLeftLoop(secondLeft);
	requireTrue("loop pair stores replacement left loop",
		loopPair.GetLeftLoop() == secondLeft,
		"Expected the replacement left loop pointer to be stored.");
	requireTrue("loop pair preserves existing right loop",
		loopPair.GetRightLoop() == firstRight,
		"Expected replacing the left loop to preserve the right loop pointer.");
	requireEqual("loop pair removes old left child",
		countStructuralChild(loopPair, firstLeft()),
		static_cast<size_t>(0));
	requireEqual("loop pair replacement left child count",
		countStructuralChild(loopPair, secondLeft()),
		static_cast<size_t>(1));
	requireEqual("loop pair right child remains attached",
		countStructuralChild(loopPair, firstRight()),
		static_cast<size_t>(1));

	loopPair.SetRightLoop(secondRight);
	requireTrue("loop pair stores replacement right loop",
		loopPair.GetRightLoop() == secondRight,
		"Expected the replacement right loop pointer to be stored.");
	requireEqual("loop pair left child remains attached",
		countStructuralChild(loopPair, secondLeft()),
		static_cast<size_t>(1));
	requireEqual("loop pair removes old right child",
		countStructuralChild(loopPair, firstRight()),
		static_cast<size_t>(0));
	requireEqual("loop pair replacement right child count",
		countStructuralChild(loopPair, secondRight()),
		static_cast<size_t>(1));

	SBPointer<ADNLoop> nullLoop = nullptr;
	loopPair.SetLeftLoop(nullLoop);
	loopPair.SetRightLoop(nullLoop);
	requireTrue("loop pair clears left loop",
		loopPair.GetLeftLoop() == nullptr,
		"Expected clearing the left loop to reset the stored pointer.");
	requireTrue("loop pair clears right loop",
		loopPair.GetRightLoop() == nullptr,
		"Expected clearing the right loop to reset the stored pointer.");
	requireEqual("loop pair clears left child",
		countStructuralChild(loopPair, secondLeft()),
		static_cast<size_t>(0));
	requireEqual("loop pair clears right child",
		countStructuralChild(loopPair, secondRight()),
		static_cast<size_t>(0));

}

void testPolyhedronRebuildClearsPreviousTopology() {

	DASPolyhedron polyhedron;
	polyhedron.BuildPolyhedron(makeTetrahedronVertices(), makeTetrahedronFaces());
	requireEqual("polyhedron initial vertex count",
		polyhedron.GetNumVertices(),
		static_cast<size_t>(4));
	requireEqual("polyhedron initial face count",
		polyhedron.GetNumFaces(),
		static_cast<size_t>(4));
	requireEqual("polyhedron initial edge count",
		polyhedron.GetEdges().size(),
		static_cast<size_t>(6));

	polyhedron.BuildPolyhedron(makeTriangleVertices(), makeTriangleFaces());
	requireEqual("polyhedron rebuild vertex count",
		polyhedron.GetNumVertices(),
		static_cast<size_t>(3));
	requireEqual("polyhedron rebuild face count",
		polyhedron.GetNumFaces(),
		static_cast<size_t>(1));
	requireEqual("polyhedron rebuild edge count",
		polyhedron.GetEdges().size(),
		static_cast<size_t>(3));

}

void testPolyhedronEdgeLookupDoesNotAllocatePlaceholders() {

	DASPolyhedron polyhedron;
	polyhedron.BuildPolyhedron(makeOpenSquareVertices(), makeOpenSquareFaces());
	Vertices vertices = polyhedron.GetVertices();
	const size_t originalEdgeCount = polyhedron.GetEdges().size();

	DASEdge* existingEdge = DASPolyhedron::GetEdgeByVertices(vertices.at(0), vertices.at(2));
	requireTrue("polyhedron edge lookup finds existing edge",
		existingEdge != nullptr,
		"Expected lookup to find the shared diagonal edge.");

	DASEdge* missingEdge = DASPolyhedron::GetEdgeByVertices(vertices.at(1), vertices.at(3));
	requireTrue("polyhedron edge lookup returns null for missing edge",
		missingEdge == nullptr,
		"Expected lookup to return null when vertices do not share an edge.");
	requireEqual("polyhedron missing lookup does not add edge",
		polyhedron.GetEdges().size(),
		originalEdgeCount);

}

void testPlyLoaderWeldsDuplicatedAssimpVertices() {

	const std::filesystem::path path = temporaryConfigPath("adenita_assimp_duplicate_vertex_cube.ply");
	std::ofstream file(path);
	file << R"(ply
format ascii 1.0
comment Created by Open Asset Import Library
element vertex 24
property float x
property float y
property float z
property float nx
property float ny
property float nz
property float s
property float t
element face 12
property list uchar int vertex_index
end_header
1 -1 -1 0 -1 0 0 1
1 -1 1 0 -1 0 0 2
-1 -1 1 0 -1 0 -1 2
-1 -1 -1 0 -1 0 -1 1
1 1 -0.999998987 0 1 0 0 0
-1 1 -1 0 1 0 -1 0
-1 1 1 0 1 0 -1 1
0.999998987 1 1.00000095 0 1 0 0 1
1 -1 -1 1 0 0 0 1
1 1 -0.999998987 1 0 0 0 0
0.999998987 1 1.00000095 1 0 0 -1 0
1 -1 1 1 0 0 -1 1
1 -1 1 0 0 1 0 1
0.999998987 1 1.00000095 0 0 1 0 0
-1 1 1 0 0 1 -1 0
-1 -1 1 0 0 1 -1 1
-1 -1 1 -1 0 0 0 1
-1 1 1 -1 0 0 0 0
-1 1 -1 -1 0 0 1 0
-1 -1 -1 -1 0 0 1 1
1 1 -0.999998987 0 0 -1 0 1
1 -1 -1 0 0 -1 0 2
-1 -1 -1 0 0 -1 -1 2
-1 1 -1 0 0 -1 -1 1
3 0 1 2
3 0 2 3
3 4 5 6
3 4 6 7
3 8 9 10
3 8 10 11
3 12 13 14
3 12 14 15
3 16 17 18
3 16 18 19
3 20 21 22
3 20 22 23
)";
	file.close();

	requireTrue("assimp-style ply fixture written",
		std::filesystem::exists(path),
		"Expected the temporary duplicated-vertex PLY fixture to exist.");
	requireTrue("assimp-style ply recognized",
		DASPolyhedron::isPLYFile(path.string()),
		"Expected the duplicated-vertex PLY fixture to be recognized as valid.");

	DASPolyhedron polyhedron(path.string());
	requireEqual("assimp-style ply welds duplicated vertices",
		polyhedron.GetNumVertices(),
		static_cast<size_t>(8));
	requireEqual("assimp-style ply keeps triangular faces",
		polyhedron.GetNumFaces(),
		static_cast<size_t>(12));

	bool allEdgesPaired = true;
	for (DASEdge* edge : polyhedron.GetEdges()) {
		if (edge == nullptr || edge->halfEdge_ == nullptr || edge->halfEdge_->pair_ == nullptr) {
			allEdgesPaired = false;
			break;
		}
	}

	requireTrue("assimp-style ply builds closed half-edge mesh",
		allEdgesPaired,
		"Expected duplicated position vertices to be welded so every edge has a paired half-edge.");

}

void testDaedalusPlyRegressionDoesNotCrashOnTeardown() {

	const std::filesystem::path path = repoDataPath("01_tetrahedron.ply");
	requireTrue("daedalus regression ply file created",
		std::filesystem::exists(path),
		"Expected the bundled tetrahedron PLY fixture to exist.");
	requireTrue("daedalus regression ply file recognized",
		DASPolyhedron::isPLYFile(path.string()),
		"Expected the bundled tetrahedron PLY fixture to be recognized as valid.");

	SEConfig& config = SEConfig::GetInstance();
	const bool originalCustomMeshModel = config.custom_mesh_model;
	config.custom_mesh_model = false;

	SBPointer<ADNPart> part = nullptr;
	{
		DASDaedalus alg;
		part = alg.ApplyAlgorithm("", path.string(), true);
		requireTrue("daedalus regression generated part",
			part != nullptr,
			"Expected Daedalus to create a part from the regression tetrahedron mesh.");
	}

	config.custom_mesh_model = originalCustomMeshModel;

	requireTrue("daedalus regression kept generated part alive",
		part != nullptr,
		"Expected the generated part to outlive the temporary DASDaedalus instance.");
	requireTrue("daedalus regression created base segments",
		part != nullptr && part->GetNumberOfBaseSegments() > 0,
		"Expected the generated part to contain base segments.");
	requireTrue("daedalus regression created single strands",
		part != nullptr && part->GetNumberOfSingleStrands() > 0,
		"Expected the generated part to contain strands.");

	SBPointer<ADNSingleStrand> scaffold = nullptr;
	bool foundLinearStaple = false;
	if (part != nullptr) {
		auto singleStrands = part->GetSingleStrands();
		SB_FOR(SBPointer<ADNSingleStrand> strand, singleStrands) {
			if (strand == nullptr) continue;
			if (strand->IsScaffold()) {
				scaffold = strand;
			}
			else if (!strand->IsCircular()) {
				foundLinearStaple = true;
			}
		}
	}

	requireTrue("daedalus regression created scaffold strand",
		scaffold != nullptr,
		"Expected the generated part to contain a scaffold strand.");
	requireTrue("daedalus regression scaffold is circular",
		scaffold != nullptr && scaffold->IsCircular(),
		"Expected the routed scaffold strand to be circular.");
	requireTrue("daedalus regression keeps staples linear",
		foundLinearStaple,
		"Expected at least one generated staple strand to remain linear.");

}

void testDaedalusInstanceCanRunTwice() {

	const std::filesystem::path path = repoDataPath("01_tetrahedron.ply");
	requireTrue("daedalus reuse regression ply file exists",
		std::filesystem::exists(path),
		"Expected the bundled tetrahedron PLY fixture to exist.");

	SEConfig& config = SEConfig::GetInstance();
	const bool originalCustomMeshModel = config.custom_mesh_model;
	config.custom_mesh_model = false;

	DASDaedalus alg;
	SBPointer<ADNPart> firstPart = alg.ApplyAlgorithm("", path.string(), true);
	SBPointer<ADNPart> secondPart = alg.ApplyAlgorithm("", path.string(), true);

	config.custom_mesh_model = originalCustomMeshModel;

	requireTrue("daedalus reuse first part generated",
		firstPart != nullptr && firstPart->GetNumberOfBaseSegments() > 0,
		"Expected the first run to create a part with base segments.");
	requireTrue("daedalus reuse second part generated",
		secondPart != nullptr && secondPart->GetNumberOfBaseSegments() > 0,
		"Expected the second run to create a part with base segments.");

}

} // namespace

int main() {

	testConstructionAndAccess();
	testCopyConstructorDeepCopy();
	testAssignmentCopiesValues();
	testAssignmentReleasesPreviousStorage();
	testMoveSemantics();
	testRows();
	testExceptions();
	testScaffoldReaderSkipsBlankLinesAndHeaders();
	testScaffoldReaderAcceptsMissingInitialHeader();
	testConfigJsonStringMemberCopiesAddedValue();
	testConfigJsonStringMemberCopiesUpdatedValue();
	testConfigFileIoClosesWrittenAndReadFiles();
	testConfigFileIoReportsFailuresAndClosesInvalidReads();
	testConcatenate();
	testCircularSingleStrandWrapsWithoutChangingSequenceOrder();
	testModernJsonValidation();
	testLegacyJsonValidation();
	testCircularSingleStrandJsonRoundTrip();
	testBuildTopScalesHandlesBrokenNucleotideLinks();
	testBuildTopScalesParametrizedHandlesBrokenNucleotideLinks();
	testBaseSegmentSetCellReplacesChild();
	testLoopPairSettersReplaceOnlySelectedChild();
	testPolyhedronRebuildClearsPreviousTopology();
	testPolyhedronEdgeLookupDoesNotAllocatePlaceholders();
	testPlyLoaderWeldsDuplicatedAssimpVertices();
	testDaedalusPlyRegressionDoesNotCrashOnTeardown();
	testDaedalusInstanceCanRunTwice();

	if (!failures.empty()) {
		for (const auto& failure : failures)
			std::cerr << "[FAIL] " << failure.file << ":" << failure.line << " in " << failure.function << " - " << failure.name << ": " << failure.message << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Adenita standalone tests passed." << std::endl;
	return EXIT_SUCCESS;

}

/// \file AdenitaStandaloneTests.cpp
/// \brief Standalone smoke tests for Adenita code that does not launch SAMSON.

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ADNArray.hpp"
#include "ADNConfigJson.hpp"
#include "ADNJsonValidation.hpp"
#include "ADNScaffoldReader.hpp"

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
	testConcatenate();
	testModernJsonValidation();
	testLegacyJsonValidation();

	if (!failures.empty()) {
		for (const auto& failure : failures)
			std::cerr << "[FAIL] " << failure.file << ":" << failure.line << " in " << failure.function << " - " << failure.name << ": " << failure.message << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Adenita standalone tests passed." << std::endl;
	return EXIT_SUCCESS;

}

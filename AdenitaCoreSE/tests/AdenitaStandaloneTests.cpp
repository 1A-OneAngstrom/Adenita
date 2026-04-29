/// \file AdenitaStandaloneTests.cpp
/// \brief Standalone smoke tests for Adenita code that does not launch SAMSON.

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "ADNArray.hpp"

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

} // namespace

int main() {

	testConstructionAndAccess();
	testCopyConstructorDeepCopy();
	testAssignmentCopiesValues();
	testRows();
	testExceptions();

	if (!failures.empty()) {
		for (const auto& failure : failures)
			std::cerr << "[FAIL] " << failure.file << ":" << failure.line << " in " << failure.function << " - " << failure.name << ": " << failure.message << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Adenita standalone tests passed." << std::endl;
	return EXIT_SUCCESS;

}

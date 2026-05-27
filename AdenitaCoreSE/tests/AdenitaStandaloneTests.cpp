/// \file AdenitaStandaloneTests.cpp
/// \brief Standalone smoke tests for Adenita code that does not launch SAMSON.

#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "ADNBaseSegment.hpp"
#include "ADNAtom.hpp"
#include "ADNCell.hpp"
#include "ADNArray.hpp"
#include "ADNConfig.hpp"
#include "ADNConstants.hpp"
#include "ADNConfigFileIO.hpp"
#include "ADNConfigJson.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNFrameUtils.hpp"
#include "ADNGeometrySynchronization.hpp"
#include "ADNJsonValidation.hpp"
#include "ADNLoop.hpp"
#include "ADNNucleotide.hpp"
#include "ADNNodeValidation.hpp"
#include "ADNPart.hpp"
#include "ADNSaveAndLoad.hpp"
#include "ADNScaffoldReader.hpp"
#include "DASCadnano.hpp"
#include "DASAlgorithms.hpp"
#include "DASBackToTheAtom.hpp"
#include "DASDaedalus.hpp"
#include "PIPrimer3.hpp"
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

void requireNearAt(const std::string& name,
	double actual,
	double expected,
	double tolerance,
	const char* file,
	int line,
	const char* function) {

	if (std::fabs(actual - expected) > tolerance)
		recordFailure(name, "Unexpected numeric value.", file, line, function);

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

template <typename Callable>
void requireThrowsRuntimeErrorAt(const std::string& name,
	Callable callable,
	const char* file,
	int line,
	const char* function) {

	try {
		callable();
		recordFailure(name, "Expected a runtime_error exception.", file, line, function);
	}
	catch (const std::runtime_error&) {
	}
	catch (...) {
		recordFailure(name, "Expected a runtime_error exception but caught a different type.", file, line, function);
	}

}

#define requireTrue(name, condition, message) requireTrueAt((name), (condition), (message), __FILE__, __LINE__, __func__)
#define requireEqual(name, actual, expected) requireEqualAt((name), (actual), (expected), __FILE__, __LINE__, __func__)
#define requireNear(name, actual, expected, tolerance) requireNearAt((name), (actual), (expected), (tolerance), __FILE__, __LINE__, __func__)
#define requireThrowsInt(name, callable, expectedValue) requireThrowsIntAt((name), (callable), (expectedValue), __FILE__, __LINE__, __func__)
#define requireThrowsRuntimeError(name, callable) requireThrowsRuntimeErrorAt((name), (callable), __FILE__, __LINE__, __func__)

void requireVecNear(const std::string& name,
	const ADNFrameUtils::Vec3& actual,
	const ADNFrameUtils::Vec3& expected,
	double tolerance) {

	requireNear(name + " x", actual.x, expected.x, tolerance);
	requireNear(name + " y", actual.y, expected.y, tolerance);
	requireNear(name + " z", actual.z, expected.z, tolerance);

}

void requirePositionNear(const std::string& name,
	const SBPosition3& actual,
	const SBPosition3& expected,
	double tolerance) {

	requireNear(name + " x", actual[0].getValue(), expected[0].getValue(), tolerance);
	requireNear(name + " y", actual[1].getValue(), expected[1].getValue(), tolerance);
	requireNear(name + " z", actual[2].getValue(), expected[2].getValue(), tolerance);

}

void requireFrameNear(const std::string& name,
	const ADNFrameUtils::Frame& actual,
	const ADNFrameUtils::Frame& expected,
	double tolerance) {

	requireVecNear(name + " e1", actual.e1, expected.e1, tolerance);
	requireVecNear(name + " e2", actual.e2, expected.e2, tolerance);
	requireVecNear(name + " e3", actual.e3, expected.e3, tolerance);

}

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

void writeTextFile(const std::filesystem::path& path, const std::string& contents) {

	std::ofstream file(path, std::ios::binary);
	file << contents;

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

ADNFrameUtils::Vec3 vecFromPosition(const SBPosition3& position) {

	return ADNFrameUtils::Vec3{
		position[0].getValue(),
		position[1].getValue(),
		position[2].getValue()
	};

}

SBPosition3 positionFromVec(const ADNFrameUtils::Vec3& position) {

	return positionAngstrom(position.x, position.y, position.z);

}

SBPosition3 positionFromRawVec(const ADNFrameUtils::Vec3& position) {

	return SBPosition3(
		SBQuantity::picometer(position.x),
		SBQuantity::picometer(position.y),
		SBQuantity::picometer(position.z));

}

double distanceValue(const SBPosition3& first, const SBPosition3& second) {

	return (first - second).norm().getValue();

}

ADNFrameUtils::Vec3 rotatedAroundOrigin(const ADNFrameUtils::Mat3& rotation,
	const SBPosition3& position) {

	return ADNFrameUtils::rotated(rotation, vecFromPosition(position));

}

void rotateNucleotideGeometryOnly(SBPointer<ADNNucleotide> nucleotide,
	const ADNFrameUtils::Mat3& rotation) {

	nucleotide->SetBackbonePosition(positionFromVec(
		ADNFrameUtils::rotated(rotation, vecFromPosition(nucleotide->GetBackbonePosition()))));
	nucleotide->SetSidechainPosition(positionFromVec(
		ADNFrameUtils::rotated(rotation, vecFromPosition(nucleotide->GetSidechainPosition()))));

}

void rotateNucleotideGeometryOnlyRaw(SBPointer<ADNNucleotide> nucleotide,
	const ADNFrameUtils::Mat3& rotation) {

	nucleotide->SetBackbonePosition(positionFromRawVec(
		ADNFrameUtils::rotated(rotation, vecFromPosition(nucleotide->GetBackbonePosition()))));
	nucleotide->SetSidechainPosition(positionFromRawVec(
		ADNFrameUtils::rotated(rotation, vecFromPosition(nucleotide->GetSidechainPosition()))));

}

void rotateBaseSegmentGeometryOnlyRaw(SBPointer<ADNBaseSegment> baseSegment,
	const ADNFrameUtils::Mat3& rotation) {

	baseSegment->SetPosition(positionFromRawVec(
		ADNFrameUtils::rotated(rotation, vecFromPosition(baseSegment->GetPosition()))));

	auto nucleotides = baseSegment->GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide != nullptr)
			rotateNucleotideGeometryOnlyRaw(nucleotide, rotation);

	}

}

void rotateBaseSegmentGeometryOnly(SBPointer<ADNBaseSegment> baseSegment,
	const ADNFrameUtils::Mat3& rotation) {

	baseSegment->SetPosition(positionFromVec(
		ADNFrameUtils::rotated(rotation, vecFromPosition(baseSegment->GetPosition()))));

	auto nucleotides = baseSegment->GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide != nullptr)
			rotateNucleotideGeometryOnly(nucleotide, rotation);

	}

}

double backboneSidechainAbsDot(SBPointer<ADNNucleotide> nucleotide,
	const ADNFrameUtils::Vec3& direction) {

	using namespace ADNFrameUtils;

	const Vec3 backboneSidechain =
		vecFromPosition(nucleotide->GetSidechainPosition()) -
		vecFromPosition(nucleotide->GetBackbonePosition());
	return std::abs(dot(normalized(backboneSidechain), normalized(direction)));

}

ADNFrameUtils::Vec3 sidechainPlaneNormal(SBPointer<ADNNucleotide> nucleotide) {

	std::vector<ADNFrameUtils::Vec3> points;
	auto atoms = nucleotide->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> atom, atoms) {

		if (atom != nullptr && !atom->IsInADNBackbone())
			points.push_back(vecFromPosition(atom->getPosition()));

	}

	if (points.size() < 3) return ADNFrameUtils::Vec3{};

	ADNFrameUtils::Vec3 center{};
	for (const ADNFrameUtils::Vec3& point : points)
		center = center + point;
	center = center / static_cast<double>(points.size());

	ublas::matrix<double> centered(points.size(), 3);
	for (std::size_t i = 0; i < points.size(); ++i) {

		const ADNFrameUtils::Vec3 point = points[i] - center;
		centered(i, 0) = point.x;
		centered(i, 1) = point.y;
		centered(i, 2) = point.z;

	}

	const ublas::vector<double> normal = ADNVectorMath::CalculatePlane(centered);
	return ADNFrameUtils::normalized(ADNFrameUtils::Vec3{ normal[0], normal[1], normal[2] });

}

ADNFrameUtils::Vec3 localBaseSegmentAxis(SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment == nullptr) return ADNFrameUtils::Vec3{};

	SBPointer<ADNBaseSegment> previous = baseSegment->GetPrev(true);
	SBPointer<ADNBaseSegment> next = baseSegment->GetNext(true);

	ADNFrameUtils::Vec3 axis{};
	if (previous != nullptr && next != nullptr)
		axis = vecFromPosition(next->GetPosition()) - vecFromPosition(previous->GetPosition());
	else if (next != nullptr)
		axis = vecFromPosition(next->GetPosition()) - vecFromPosition(baseSegment->GetPosition());
	else if (previous != nullptr)
		axis = vecFromPosition(baseSegment->GetPosition()) - vecFromPosition(previous->GetPosition());

	if (ADNFrameUtils::isNearlyZero(axis))
		axis = ADNFrameAdapters::sanitizedFrame(*baseSegment).e3;

	return ADNFrameUtils::normalized(axis);

}

void rotateNucleotideAroundRaw(SBPointer<ADNNucleotide> nucleotide,
	const ADNFrameUtils::Vec3& center,
	const ADNFrameUtils::Mat3& rotation) {

	if (nucleotide == nullptr) return;

	const auto rotatePosition = [&](const SBPosition3& position) {

		return positionFromRawVec(center + ADNFrameUtils::rotated(rotation, vecFromPosition(position) - center));

	};

	nucleotide->SetBackbonePosition(rotatePosition(nucleotide->GetBackbonePosition()));
	nucleotide->SetSidechainPosition(rotatePosition(nucleotide->GetSidechainPosition()));

}

void applyTemplatePhaseToBaseSegmentGeometry(SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment == nullptr) return;

	const ADNFrameUtils::Vec3 center = vecFromPosition(baseSegment->GetPosition());
	const ADNFrameUtils::Vec3 axis = localBaseSegmentAxis(baseSegment);
	const ADNFrameUtils::Mat3 rotation = ADNFrameUtils::rotationAroundAxis(
		axis,
		ADNGeometrySynchronization::baseSegmentReconstructionPhaseRadians(*baseSegment));

	auto nucleotides = baseSegment->GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		rotateNucleotideAroundRaw(nucleotide, center, rotation);

	}

}

SBPointer<ADNAtom> firstAtomByName(SBPointer<ADNNucleotide> nucleotide,
	const std::string& atomName) {

	if (nucleotide == nullptr) return nullptr;
	auto atoms = nucleotide->GetAtomsByName(atomName);
	if (atoms.size() == 0) return nullptr;
	return *atoms.begin();

}

double pToPreviousO3Distance(SBPointer<ADNNucleotide> nucleotide) {

	if (nucleotide == nullptr) return -1.0;

	SBPointer<ADNNucleotide> previous = nucleotide->GetPrev(true);
	if (previous == nullptr) return -1.0;

	SBPointer<ADNAtom> phosphate = firstAtomByName(nucleotide, "P");
	SBPointer<ADNAtom> previousO3 = firstAtomByName(previous, "O3'");
	if (phosphate == nullptr || previousO3 == nullptr) return -1.0;

	return distanceValue(phosphate->getPosition(), previousO3->getPosition());

}

ADNFrameUtils::Vec3 averageGeneratedAtomPosition(SBPointer<ADNNucleotide> nucleotide) {

	ADNFrameUtils::Vec3 sum{};
	std::size_t count = 0;
	auto atoms = nucleotide->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> atom, atoms) {

		if (atom == nullptr) continue;
		sum = sum + vecFromPosition(atom->getPosition());
		++count;

	}

	if (count == 0) return ADNFrameUtils::Vec3{};
	return sum / static_cast<double>(count);

}

void reconstructBackboneSidechainFromFrame(SBPointer<ADNNucleotide> nucleotide,
	double halfDistance) {

	using namespace ADNFrameUtils;

	const Vec3 center = vecFromPosition(nucleotide->GetPosition());
	const Vec3 e2 = normalized(ADNFrameAdapters::frameFromOrientable(*nucleotide).e2);
	nucleotide->SetBackbonePosition(positionFromVec(center - e2 * halfDistance));
	nucleotide->SetSidechainPosition(positionFromVec(center + e2 * halfDistance));

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

std::string readTextFile(const std::filesystem::path& path) {

	std::ifstream file(path);
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

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

SBPointer<ADNNucleotide> createFrameNucleotide(double x) {

	SBPointer<ADNNucleotide> nucleotide = new ADNNucleotide();
	nucleotide->Init();
	nucleotide->setNucleotideType(SBResidue::ResidueType::DA);
	nucleotide->SetBackbonePosition(positionAngstrom(x, -0.5, 0.0));
	nucleotide->SetSidechainPosition(positionAngstrom(x, 0.5, 0.0));
	nucleotide->SetE1(vector3(0.0, 0.0, 0.0));
	nucleotide->SetE2(vector3(0.0, 0.0, 0.0));
	nucleotide->SetE3(vector3(0.0, 0.0, 0.0));
	return nucleotide;

}

struct ThreeNucleotideFrameFixture {
	SBPointer<ADNSingleStrand> strand;
	SBPointer<ADNNucleotide> previous;
	SBPointer<ADNNucleotide> nucleotide;
	SBPointer<ADNNucleotide> next;
};

ThreeNucleotideFrameFixture createThreeNucleotideFrameFixture() {

	ThreeNucleotideFrameFixture fixture;
	fixture.strand = new ADNSingleStrand();
	fixture.previous = createFrameNucleotide(-1.0);
	fixture.nucleotide = createFrameNucleotide(0.0);
	fixture.next = createFrameNucleotide(1.0);
	fixture.strand->AddNucleotideThreePrime(fixture.previous);
	fixture.strand->AddNucleotideThreePrime(fixture.nucleotide);
	fixture.strand->AddNucleotideThreePrime(fixture.next);
	return fixture;

}

struct BaseSegmentFrameFixture {
	SBPointer<ADNPart> part;
	SBPointer<ADNDoubleStrand> doubleStrand;
	SBPointer<ADNSingleStrand> leftStrand;
	SBPointer<ADNSingleStrand> rightStrand;
	SBPointer<ADNBaseSegment> previous;
	SBPointer<ADNBaseSegment> baseSegment;
	SBPointer<ADNBaseSegment> next;
};

SBPointer<ADNBaseSegment> createBasePairSegment(double x) {

	SBPointer<ADNBaseSegment> baseSegment = new ADNBaseSegment(CellType::BasePair);
	baseSegment->SetPosition(positionAngstrom(x, 0.0, 0.0));

	SBPointer<ADNNucleotide> left = createSyntheticNucleotide(
		SBResidue::ResidueType::DA, x, -0.5, 0.0, vector3(0.0, 1.0, 0.0));
	SBPointer<ADNNucleotide> right = createSyntheticNucleotide(
		SBResidue::ResidueType::DT, x, 0.5, 0.0, vector3(0.0, -1.0, 0.0));

	left->SetBackbonePosition(positionAngstrom(x, -0.8, 0.0));
	left->SetSidechainPosition(positionAngstrom(x, -0.2, 0.0));
	left->SetE1(vector3(0.0, 0.0, -1.0));
	left->SetE2(vector3(0.0, 1.0, 0.0));
	left->SetE3(vector3(1.0, 0.0, 0.0));

	right->SetBackbonePosition(positionAngstrom(x, 0.8, 0.0));
	right->SetSidechainPosition(positionAngstrom(x, 0.2, 0.0));
	right->SetE1(vector3(0.0, 0.0, -1.0));
	right->SetE2(vector3(0.0, -1.0, 0.0));
	right->SetE3(vector3(-1.0, 0.0, 0.0));

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(baseSegment->GetCell()());
	basePair->AddPair(left, right);
	left->SetBaseSegment(baseSegment);
	right->SetBaseSegment(baseSegment);

	return baseSegment;

}

SBPointer<ADNNucleotide> getLeftNucleotide(SBPointer<ADNBaseSegment> baseSegment);
SBPointer<ADNNucleotide> getRightNucleotide(SBPointer<ADNBaseSegment> baseSegment);

BaseSegmentFrameFixture createBaseSegmentFrameFixture(double spacing = 1.0) {

	BaseSegmentFrameFixture fixture;
	fixture.part = new ADNPart();
	fixture.doubleStrand = new ADNDoubleStrand();
	fixture.leftStrand = new ADNSingleStrand();
	fixture.rightStrand = new ADNSingleStrand();
	fixture.part->RegisterDoubleStrand(fixture.doubleStrand);
	fixture.part->RegisterSingleStrand(fixture.leftStrand);
	fixture.part->RegisterSingleStrand(fixture.rightStrand);

	fixture.previous = createBasePairSegment(-spacing);
	fixture.baseSegment = createBasePairSegment(0.0);
	fixture.next = createBasePairSegment(spacing);

	fixture.part->RegisterBaseSegmentEnd(fixture.doubleStrand, fixture.previous);
	fixture.part->RegisterBaseSegmentEnd(fixture.doubleStrand, fixture.baseSegment);
	fixture.part->RegisterBaseSegmentEnd(fixture.doubleStrand, fixture.next);

	fixture.part->RegisterNucleotideThreePrime(fixture.leftStrand, getLeftNucleotide(fixture.previous));
	fixture.part->RegisterNucleotideThreePrime(fixture.leftStrand, getLeftNucleotide(fixture.baseSegment));
	fixture.part->RegisterNucleotideThreePrime(fixture.leftStrand, getLeftNucleotide(fixture.next));
	fixture.part->RegisterNucleotideThreePrime(fixture.rightStrand, getRightNucleotide(fixture.previous));
	fixture.part->RegisterNucleotideThreePrime(fixture.rightStrand, getRightNucleotide(fixture.baseSegment));
	fixture.part->RegisterNucleotideThreePrime(fixture.rightStrand, getRightNucleotide(fixture.next));

	return fixture;

}

SBPointer<ADNNucleotide> getLeftNucleotide(SBPointer<ADNBaseSegment> baseSegment) {

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(baseSegment->GetCell()());
	return basePair->GetLeftNucleotide();

}

SBPointer<ADNNucleotide> getRightNucleotide(SBPointer<ADNBaseSegment> baseSegment) {

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(baseSegment->GetCell()());
	return basePair->GetRightNucleotide();

}

struct AtomicGenerationFixture {
	SBPointer<ADNPart> part;
	SBPointer<ADNDoubleStrand> doubleStrand;
	SBPointer<ADNSingleStrand> leftStrand;
	SBPointer<ADNSingleStrand> rightStrand;
	SBPointer<ADNBaseSegment> baseSegment;
	SBPointer<ADNNucleotide> left;
	SBPointer<ADNNucleotide> right;
};

AtomicGenerationFixture createAtomicGenerationFixture() {

	AtomicGenerationFixture fixture;
	fixture.part = new ADNPart();
	fixture.doubleStrand = new ADNDoubleStrand();
	fixture.leftStrand = new ADNSingleStrand();
	fixture.rightStrand = new ADNSingleStrand();
	fixture.baseSegment = new ADNBaseSegment(CellType::BasePair);

	fixture.part->RegisterDoubleStrand(fixture.doubleStrand);
	fixture.part->RegisterSingleStrand(fixture.leftStrand);
	fixture.part->RegisterSingleStrand(fixture.rightStrand);

	fixture.left = createSyntheticNucleotide(
		SBResidue::ResidueType::DA, 0.0, 0.0, 0.0, vector3(0.0, 1.0, 0.0));
	fixture.right = createSyntheticNucleotide(
		SBResidue::ResidueType::DT, 0.0, 0.0, 0.0, vector3(0.0, -1.0, 0.0));

	fixture.left->SetBackbonePosition(positionAngstrom(0.0, -0.9, 0.0));
	fixture.left->SetSidechainPosition(positionAngstrom(0.0, -0.3, 0.0));
	fixture.right->SetBackbonePosition(positionAngstrom(0.0, 0.9, 0.0));
	fixture.right->SetSidechainPosition(positionAngstrom(0.0, 0.3, 0.0));
	fixture.baseSegment->SetPosition(positionAngstrom(0.0, 0.0, 0.0));

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(fixture.baseSegment->GetCell()());
	basePair->AddPair(fixture.left, fixture.right);
	fixture.left->SetBaseSegment(fixture.baseSegment);
	fixture.right->SetBaseSegment(fixture.baseSegment);

	fixture.part->RegisterNucleotideThreePrime(fixture.leftStrand, fixture.left);
	fixture.part->RegisterNucleotideThreePrime(fixture.rightStrand, fixture.right);
	fixture.part->RegisterBaseSegmentEnd(fixture.doubleStrand, fixture.baseSegment);

	return fixture;

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

void testFrameUtilsRotateFrameAroundZ() {

	using namespace ADNFrameUtils;

	constexpr double pi = 3.141592653589793238462643383279502884;
	const Frame frame{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 }
	};

	const Frame rotatedFrame = rotated(rotationZ(0.5 * pi), frame);

	requireVecNear("frame utils rotated e1", rotatedFrame.e1, Vec3{ 0.0, 1.0, 0.0 }, 1.0e-9);
	requireVecNear("frame utils rotated e2", rotatedFrame.e2, Vec3{ -1.0, 0.0, 0.0 }, 1.0e-9);
	requireVecNear("frame utils rotated e3", rotatedFrame.e3, Vec3{ 0.0, 0.0, 1.0 }, 1.0e-9);
	requireTrue("frame utils rotated frame remains valid",
		isOrthonormalRightHanded(rotatedFrame, 1.0e-9),
		"Expected rotated frame to remain orthonormal and right-handed.");

}

void testFrameUtilsOrthonormalizationRepairsSmallDrift() {

	using namespace ADNFrameUtils;

	const Frame noisy{
		Vec3{ 1.0, 0.001, 0.0 },
		Vec3{ 0.0, 0.999, 0.002 },
		Vec3{ 0.001, 0.0, 0.998 }
	};

	const Frame repaired = orthonormalized(noisy);

	requireTrue("frame utils repairs drifted frame",
		isOrthonormalRightHanded(repaired, 1.0e-9),
		"Expected repaired frame to be orthonormal and right-handed.");
	requireNear("frame utils repaired e1 unit", norm(repaired.e1), 1.0, 1.0e-9);
	requireNear("frame utils repaired e2 unit", norm(repaired.e2), 1.0, 1.0e-9);
	requireNear("frame utils repaired e3 unit", norm(repaired.e3), 1.0, 1.0e-9);

}

void testFrameUtilsInvalidFrameFallsBack() {

	using namespace ADNFrameUtils;

	const Frame invalid{
		Vec3{ 0.0, 0.0, 0.0 },
		Vec3{ 0.0, 0.0, 0.0 },
		Vec3{ 0.0, 0.0, 0.0 }
	};

	const Frame repaired = orthonormalized(invalid);

	requireTrue("frame utils invalid frame fallback is valid",
		isOrthonormalRightHanded(repaired, 1.0e-9),
		"Expected invalid frame fallback to be orthonormal and right-handed.");
	requireVecNear("frame utils fallback e1", repaired.e1, Vec3{ 1.0, 0.0, 0.0 }, 1.0e-9);
	requireVecNear("frame utils fallback e2", repaired.e2, Vec3{ 0.0, 1.0, 0.0 }, 1.0e-9);
	requireVecNear("frame utils fallback e3", repaired.e3, Vec3{ 0.0, 0.0, 1.0 }, 1.0e-9);

}

void testFrameUtilsRigidRotationPreservesDistances() {

	using namespace ADNFrameUtils;

	const Vec3 a{ 1.0, 2.0, 3.0 };
	const Vec3 b{ 4.0, 6.0, 8.0 };
	const double before = norm(b - a);

	const Mat3 rotation = rotationZ(1.234);
	const double after = norm(rotated(rotation, b) - rotated(rotation, a));

	requireNear("frame utils rotation preserves distance", after, before, 1.0e-9);

}

void testFrameUtilsDerivesRotatedMockGeometryFrame() {

	using namespace ADNFrameUtils;

	constexpr double pi = 3.141592653589793238462643383279502884;
	const Vec3 previousCenter{ -1.0, 0.0, 0.0 };
	const Vec3 nextCenter{ 1.0, 0.0, 0.0 };
	const Vec3 backbone{ 0.0, -0.5, 0.0 };
	const Vec3 sidechain{ 0.0, 0.5, 0.0 };

	const Frame original = frameFromE2AndTangent(sidechain - backbone, nextCenter - previousCenter);
	const Mat3 rotation = rotationZ(0.5 * pi);

	const Vec3 rotatedPreviousCenter = rotated(rotation, previousCenter);
	const Vec3 rotatedNextCenter = rotated(rotation, nextCenter);
	const Vec3 rotatedBackbone = rotated(rotation, backbone);
	const Vec3 rotatedSidechain = rotated(rotation, sidechain);
	const Frame derivedAfterRotation = frameFromE2AndTangent(
		rotatedSidechain - rotatedBackbone,
		rotatedNextCenter - rotatedPreviousCenter);
	const Frame expectedAfterRotation = rotated(rotation, original);

	requireTrue("frame utils mock geometry original valid",
		isOrthonormalRightHanded(original, 1.0e-9),
		"Expected mock geometry frame to be valid.");
	requireTrue("frame utils mock geometry rotated valid",
		isOrthonormalRightHanded(derivedAfterRotation, 1.0e-9),
		"Expected rotated mock geometry frame to be valid.");
	requireVecNear("frame utils mock geometry e1", derivedAfterRotation.e1, expectedAfterRotation.e1, 1.0e-9);
	requireVecNear("frame utils mock geometry e2", derivedAfterRotation.e2, expectedAfterRotation.e2, 1.0e-9);
	requireVecNear("frame utils mock geometry e3", derivedAfterRotation.e3, expectedAfterRotation.e3, 1.0e-9);

}

void testFrameAdaptersSanitizeAndRotateOrientable() {

	constexpr double pi = 3.141592653589793238462643383279502884;

	Orientable orientable;
	orientable.SetE1(vector3(1.0, 0.001, 0.0));
	orientable.SetE2(vector3(0.0, 0.999, 0.002));
	orientable.SetE3(vector3(0.001, 0.0, 0.998));

	ADNFrameAdapters::sanitizeFrame(orientable);
	const ADNFrameUtils::Frame sanitized = ADNFrameAdapters::frameFromOrientable(orientable);

	requireTrue("frame adapters sanitize orientable",
		ADNFrameUtils::isOrthonormalRightHanded(sanitized, 1.0e-9),
		"Expected orientable frame to be sanitized.");

	ADNFrameAdapters::rotateFrame(orientable, ADNFrameUtils::rotationZ(0.5 * pi));
	const ADNFrameUtils::Frame rotated = ADNFrameAdapters::frameFromOrientable(orientable);
	const ADNFrameUtils::Frame expected = ADNFrameUtils::rotated(ADNFrameUtils::rotationZ(0.5 * pi), sanitized);

	requireTrue("frame adapters rotate orientable",
		ADNFrameUtils::isOrthonormalRightHanded(rotated, 1.0e-9),
		"Expected rotated orientable frame to stay valid.");
	requireVecNear("frame adapters rotated e1", rotated.e1, expected.e1, 1.0e-9);
	requireVecNear("frame adapters rotated e2", rotated.e2, expected.e2, 1.0e-9);
	requireVecNear("frame adapters rotated e3", rotated.e3, expected.e3, 1.0e-9);

}

void testTemplateFramePreparationRoundTripLeftSide() {

	using namespace ADNFrameUtils;

	const Frame canonical = frameFromE2AndTangent(
		Vec3{ 0.2, 0.9, 0.3 },
		Vec3{ 1.0, 0.1, 0.7 });
	const double phase = 0.73;
	const Frame leftSide = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
		canonical,
		ADNGeometrySynchronization::TemplateSide::Left,
		phase);
	const Frame recovered = ADNGeometrySynchronization::nucleotideSideFrameToCanonicalBaseSegmentFrame(
		leftSide,
		ADNGeometrySynchronization::TemplateSide::Left,
		phase);

	requireTrue("template frame left target valid",
		isOrthonormalRightHanded(leftSide, 1.0e-9),
		"Expected left template target frame to remain valid.");
	requireFrameNear("template frame left round trip", recovered, canonical, 1.0e-9);

}

void testTemplateFramePreparationRoundTripRightSide() {

	using namespace ADNFrameUtils;

	const Frame canonical = frameFromE2AndTangent(
		Vec3{ -0.3, 0.8, 0.4 },
		Vec3{ 0.7, -0.2, 1.0 });
	const double phase = -0.51;
	const Frame leftSide = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
		canonical,
		ADNGeometrySynchronization::TemplateSide::Left,
		phase);
	const Frame rightSide = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
		canonical,
		ADNGeometrySynchronization::TemplateSide::Right,
		phase);
	const Frame recovered = ADNGeometrySynchronization::nucleotideSideFrameToCanonicalBaseSegmentFrame(
		rightSide,
		ADNGeometrySynchronization::TemplateSide::Right,
		phase);

	requireVecNear("template frame right e1 sign", rightSide.e1, leftSide.e1, 1.0e-9);
	requireVecNear("template frame right e2 sign", rightSide.e2, -leftSide.e2, 1.0e-9);
	requireVecNear("template frame right e3 sign", rightSide.e3, -leftSide.e3, 1.0e-9);
	requireFrameNear("template frame right round trip", recovered, canonical, 1.0e-9);

}

void testTemplateFramePreparationTargetStateIsStable() {

	using namespace ADNFrameUtils;

	const Frame canonical = frameFromE2AndTangent(
		Vec3{ 0.1, -0.6, 0.8 },
		Vec3{ 0.9, 0.3, 0.4 });
	const double phase = 1.24;
	const Frame rightTarget = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
		canonical,
		ADNGeometrySynchronization::TemplateSide::Right,
		phase);
	const Frame canonicalAgain = ADNGeometrySynchronization::nucleotideSideFrameToCanonicalBaseSegmentFrame(
		rightTarget,
		ADNGeometrySynchronization::TemplateSide::Right,
		phase);
	const Frame rightTargetAgain = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
		canonicalAgain,
		ADNGeometrySynchronization::TemplateSide::Right,
		phase);

	requireFrameNear("template frame canonical target is stable", canonicalAgain, canonical, 1.0e-9);
	requireFrameNear("template frame side target is stable", rightTargetAgain, rightTarget, 1.0e-9);

}

void testTemplateFrameHandednessAcrossPhases() {

	using namespace ADNFrameUtils;

	constexpr double pi = 3.141592653589793238462643383279502884;
	const Frame canonical = identityFrame();
	const double phases[] = { 0.0, pi / 6.0, pi / 2.0, pi, -pi / 3.0 };
	const ADNGeometrySynchronization::TemplateSide sides[] = {
		ADNGeometrySynchronization::TemplateSide::Left,
		ADNGeometrySynchronization::TemplateSide::Right
	};

	for (double phase : phases) {
		for (ADNGeometrySynchronization::TemplateSide side : sides) {

			const Frame sideFrame = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
				canonical,
				side,
				phase);

			requireTrue("template frame side handedness",
				isOrthonormalRightHanded(sideFrame, 1.0e-9),
				"Expected converted nucleotide-side frame to remain right-handed.");
			requireTrue("template frame side determinant positive",
				determinant(sideFrame) > 0.0,
				"Expected converted nucleotide-side frame determinant to be positive.");

		}
	}

}

void testTemplateFrameBasePlaneNormalsStayCoplanar() {

	using namespace ADNFrameUtils;

	const Frame canonical = frameFromE2AndTangent(
		Vec3{ 0.2, 0.8, 0.5 },
		Vec3{ 0.9, -0.1, 0.7 });
	const double phases[] = { 0.0, 0.37, 1.5707963267948966, -0.62 };

	for (double phase : phases) {

		const Frame leftSide = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
			canonical,
			ADNGeometrySynchronization::TemplateSide::Left,
			phase);
		const Frame rightSide = ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
			canonical,
			ADNGeometrySynchronization::TemplateSide::Right,
			phase);

		const double normalAbsDot = std::abs(dot(normalized(leftSide.e3), normalized(rightSide.e3)));
		const double pairDirectionLeftNormalAbsDot =
			std::abs(dot(normalized(leftSide.e2), normalized(leftSide.e3)));

		requireTrue("template frame base normals coplanar",
			normalAbsDot > 0.999,
			"Expected left and right base-plane normals to be parallel or antiparallel.");
		requireTrue("template frame pair direction lies in plane",
			pairDirectionLeftNormalAbsDot < 1.0e-9,
			"Expected base-pair direction to lie in the base-pair plane.");

	}

}

void testCanonicalTemplateFrameFromCurrentGeometryTracksBaseSegmentAxis() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();
	fixture.doubleStrand->SetInitialTwistAngle(18.0);

	const Frame original =
		ADNGeometrySynchronization::canonicalTemplateFrameFromCurrentGeometry(*fixture.baseSegment);
	const Vec3 originalAxis = localBaseSegmentAxis(fixture.baseSegment);

	requireTrue("canonical template current geometry valid",
		isOrthonormalRightHanded(original, 1.0e-9),
		"Expected canonical template frame from current geometry to be right-handed.");
	requireTrue("canonical template current geometry axis",
		std::abs(dot(normalized(original.e3), originalAxis)) > 0.999,
		"Expected canonical template frame e3 to follow the base-segment centerline.");

	const Mat3 rigidRotation = rotationAroundAxis(Vec3{ 0.4, -0.3, 0.9 }, 0.71);
	rotateBaseSegmentGeometryOnlyRaw(fixture.previous, rigidRotation);
	rotateBaseSegmentGeometryOnlyRaw(fixture.baseSegment, rigidRotation);
	rotateBaseSegmentGeometryOnlyRaw(fixture.next, rigidRotation);

	const Frame rotatedFrame =
		ADNGeometrySynchronization::canonicalTemplateFrameFromCurrentGeometry(*fixture.baseSegment);
	const Vec3 rotatedAxis = localBaseSegmentAxis(fixture.baseSegment);

	requireTrue("canonical template rotated geometry valid",
		isOrthonormalRightHanded(rotatedFrame, 1.0e-9),
		"Expected rotated canonical template frame to be right-handed.");
	requireTrue("canonical template rotated geometry axis",
		std::abs(dot(normalized(rotatedFrame.e3), rotatedAxis)) > 0.999,
		"Expected rotated canonical template frame e3 to follow the rotated base-segment centerline.");
	requireFrameNear("canonical template current geometry equivariant",
		rotatedFrame,
		rotated(rigidRotation, original),
		1.0e-8);

}

void testCanonicalTemplateFrameFromCurrentGeometryDoesNotMutateBaseSegment() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();
	ADNGeometrySynchronization::syncBaseSegmentFrameFromGeometry(*fixture.baseSegment);
	const Frame before = ADNFrameAdapters::sanitizedFrame(*fixture.baseSegment);

	const Frame canonical =
		ADNGeometrySynchronization::canonicalTemplateFrameFromCurrentGeometry(*fixture.baseSegment);
	const Frame after = ADNFrameAdapters::sanitizedFrame(*fixture.baseSegment);

	requireTrue("canonical template non mutating result valid",
		isOrthonormalRightHanded(canonical, 1.0e-9),
		"Expected non-mutating canonical template frame to be valid.");
	requireFrameNear("canonical template leaves base segment frame unchanged",
		after,
		before,
		1.0e-12);

}

void testNucleotideSetPositionTranslatesBackboneAndSidechain() {

	SBPointer<ADNNucleotide> nucleotide = new ADNNucleotide();
	nucleotide->Init();
	nucleotide->SetBackbonePosition(positionAngstrom(0.0, 0.0, 0.0));
	nucleotide->SetSidechainPosition(positionAngstrom(2.0, 0.0, 0.0));

	nucleotide->SetPosition(positionAngstrom(3.0, 0.0, 0.0));

	requirePositionNear("nucleotide set position updates center",
		nucleotide->GetPosition(),
		positionAngstrom(3.0, 0.0, 0.0),
		1.0e-9);
	requirePositionNear("nucleotide set position translates backbone",
		nucleotide->GetBackbonePosition(),
		positionAngstrom(2.0, 0.0, 0.0),
		1.0e-9);
	requirePositionNear("nucleotide set position translates sidechain",
		nucleotide->GetSidechainPosition(),
		positionAngstrom(4.0, 0.0, 0.0),
		1.0e-9);

}

void testGeometrySynchronizationDerivesNucleotideFrame() {

	SBPointer<ADNSingleStrand> strand = new ADNSingleStrand();
	SBPointer<ADNNucleotide> previous = new ADNNucleotide();
	SBPointer<ADNNucleotide> nucleotide = new ADNNucleotide();
	SBPointer<ADNNucleotide> next = new ADNNucleotide();

	auto initializeNucleotide = [](SBPointer<ADNNucleotide> nt, double x) {

		nt->Init();
		nt->SetBackbonePosition(positionAngstrom(x, -0.5, 0.0));
		nt->SetSidechainPosition(positionAngstrom(x, 0.5, 0.0));
		nt->SetE1(vector3(0.0, 0.0, 0.0));
		nt->SetE2(vector3(0.0, 0.0, 0.0));
		nt->SetE3(vector3(0.0, 0.0, 0.0));

	};

	initializeNucleotide(previous, -1.0);
	initializeNucleotide(nucleotide, 0.0);
	initializeNucleotide(next, 1.0);
	strand->AddNucleotideThreePrime(previous);
	strand->AddNucleotideThreePrime(nucleotide);
	strand->AddNucleotideThreePrime(next);

	ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*nucleotide);
	const ADNFrameUtils::Frame frame = ADNFrameAdapters::frameFromOrientable(*nucleotide);

	requireTrue("geometry synchronization nucleotide frame valid",
		ADNGeometrySynchronization::validateNucleotideGeometry(*nucleotide),
		"Expected synchronized nucleotide frame to be valid.");
	requireVecNear("geometry synchronization nucleotide e1", frame.e1, ADNFrameUtils::Vec3{ 0.0, 0.0, -1.0 }, 1.0e-9);
	requireVecNear("geometry synchronization nucleotide e2", frame.e2, ADNFrameUtils::Vec3{ 0.0, 1.0, 0.0 }, 1.0e-9);
	requireVecNear("geometry synchronization nucleotide e3", frame.e3, ADNFrameUtils::Vec3{ 1.0, 0.0, 0.0 }, 1.0e-9);

}

void testGeometryValidationRejectsStaleNucleotideFrame() {

	using namespace ADNFrameUtils;

	ThreeNucleotideFrameFixture fixture = createThreeNucleotideFrameFixture();

	ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*fixture.nucleotide);
	const Frame original = ADNFrameAdapters::frameFromOrientable(*fixture.nucleotide);

	requireTrue("stale nucleotide original frame valid",
		ADNGeometrySynchronization::validateNucleotideGeometry(*fixture.nucleotide),
		"Expected original nucleotide frame to align with geometry.");

	const Mat3 rotation = ADNFrameUtils::rotationAroundAxis(Vec3{ 1.0, 1.0, 1.0 }, 1.0471975511965976);
	rotateNucleotideGeometryOnly(fixture.previous, rotation);
	rotateNucleotideGeometryOnly(fixture.nucleotide, rotation);
	rotateNucleotideGeometryOnly(fixture.next, rotation);

	const Frame stale = ADNFrameAdapters::frameFromOrientable(*fixture.nucleotide);
	const ADNGeometrySynchronization::FrameGeometryAlignment staleAlignment =
		ADNGeometrySynchronization::analyzeNucleotideFrameAlignment(*fixture.nucleotide);

	requireTrue("stale nucleotide frame remains orthonormal",
		isOrthonormalRightHanded(stale, 1.0e-9),
		"Expected stale frame to remain mathematically valid.");
	requireTrue("stale nucleotide geometry alignment fails",
		!staleAlignment.primaryDirectionAligned || !staleAlignment.tangentDirectionAligned,
		"Expected stale frame to disagree with rotated nucleotide geometry.");
	requireTrue("stale nucleotide validation fails",
		!ADNGeometrySynchronization::validateNucleotideGeometry(*fixture.nucleotide),
		"Expected validation to reject a stale but valid nucleotide frame.");

	ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*fixture.nucleotide);
	const Frame synchronized = ADNFrameAdapters::frameFromOrientable(*fixture.nucleotide);
	const Frame expected = rotated(rotation, original);

	requireTrue("synchronized nucleotide validation passes",
		ADNGeometrySynchronization::validateNucleotideGeometry(*fixture.nucleotide),
		"Expected synchronization to realign nucleotide frame with geometry.");
	requireFrameNear("synchronized nucleotide frame tracks rotation", synchronized, expected, 1.0e-8);

}

void testGeometryValidationRejectsStaleBaseSegmentFrame() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();

	ADNGeometrySynchronization::syncBaseSegmentFrameFromGeometry(*fixture.baseSegment);
	const Frame original = ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment);

	requireTrue("stale base segment original frame valid",
		ADNGeometrySynchronization::validateBaseSegmentGeometry(*fixture.baseSegment),
		"Expected original base-segment frame to align with geometry.");

	const Mat3 rotation = ADNFrameUtils::rotationAroundAxis(Vec3{ 0.3, 0.7, 0.2 }, 0.9);
	rotateBaseSegmentGeometryOnly(fixture.previous, rotation);
	rotateBaseSegmentGeometryOnly(fixture.baseSegment, rotation);
	rotateBaseSegmentGeometryOnly(fixture.next, rotation);

	const Frame stale = ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment);
	const ADNGeometrySynchronization::FrameGeometryAlignment staleAlignment =
		ADNGeometrySynchronization::analyzeBaseSegmentFrameAlignment(*fixture.baseSegment);

	requireTrue("stale base segment frame remains orthonormal",
		isOrthonormalRightHanded(stale, 1.0e-9),
		"Expected stale base-segment frame to remain mathematically valid.");
	requireTrue("stale base segment geometry alignment fails",
		!staleAlignment.primaryDirectionAligned || !staleAlignment.tangentDirectionAligned,
		"Expected stale frame to disagree with rotated base-segment geometry.");
	requireTrue("stale base segment validation fails",
		!ADNGeometrySynchronization::validateBaseSegmentGeometry(*fixture.baseSegment),
		"Expected validation to reject a stale but valid base-segment frame.");

	ADNGeometrySynchronization::syncBaseSegmentFrameFromGeometry(*fixture.baseSegment);
	const Frame synchronized = ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment);
	const Frame expected = rotated(rotation, original);

	requireTrue("synchronized base segment validation passes",
		ADNGeometrySynchronization::validateBaseSegmentGeometry(*fixture.baseSegment),
		"Expected synchronization to realign base-segment frame with geometry.");
	requireFrameNear("synchronized base segment frame tracks rotation", synchronized, expected, 1.0e-8);

}

void testGeometryEditBarrierPreservesRotatedNucleotideDirection() {

	using namespace ADNFrameUtils;

	ThreeNucleotideFrameFixture staleFixture = createThreeNucleotideFrameFixture();
	ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*staleFixture.nucleotide);
	const Frame staleOriginal = ADNFrameAdapters::frameFromOrientable(*staleFixture.nucleotide);
	const Mat3 rotation = rotationZ(1.5707963267948966);
	const Vec3 expectedDirection = rotated(rotation, staleOriginal.e2);

	rotateNucleotideGeometryOnly(staleFixture.previous, rotation);
	rotateNucleotideGeometryOnly(staleFixture.nucleotide, rotation);
	rotateNucleotideGeometryOnly(staleFixture.next, rotation);
	reconstructBackboneSidechainFromFrame(staleFixture.nucleotide, 0.5);

	requireTrue("geometry edit without barrier uses stale direction",
		backboneSidechainAbsDot(staleFixture.nucleotide, expectedDirection) < 0.85,
		"Expected reconstruction without pre-sync to use the stale frame direction.");

	ThreeNucleotideFrameFixture synchronizedFixture = createThreeNucleotideFrameFixture();
	ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*synchronizedFixture.nucleotide);
	const Frame synchronizedOriginal = ADNFrameAdapters::frameFromOrientable(*synchronizedFixture.nucleotide);
	const Vec3 synchronizedExpectedDirection = rotated(rotation, synchronizedOriginal.e2);

	rotateNucleotideGeometryOnly(synchronizedFixture.previous, rotation);
	rotateNucleotideGeometryOnly(synchronizedFixture.nucleotide, rotation);
	rotateNucleotideGeometryOnly(synchronizedFixture.next, rotation);
	ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*synchronizedFixture.nucleotide);
	reconstructBackboneSidechainFromFrame(synchronizedFixture.nucleotide, 0.5);

	requireTrue("geometry edit with barrier preserves rotated direction",
		backboneSidechainAbsDot(synchronizedFixture.nucleotide, synchronizedExpectedDirection) > 0.999,
		"Expected pre-sync to preserve the rotated geometry direction during reconstruction.");

}

void testFrameUtilsRotationAroundAxisMatchesZRotation() {

	using namespace ADNFrameUtils;

	constexpr double pi = 3.141592653589793238462643383279502884;
	const Mat3 arbitraryAxisRotation = ADNFrameUtils::rotationAroundAxis(Vec3{ 0.0, 0.0, 1.0 }, 0.5 * pi);
	const Mat3 zRotation = rotationZ(0.5 * pi);
	const Vec3 vector{ 1.0, 0.0, 0.0 };

	requireVecNear("axis rotation matches z rotation",
		rotated(arbitraryAxisRotation, vector),
		rotated(zRotation, vector),
		1.0e-12);

}

void testRotateDoubleStrandGeometryPreservesDistancesAfterRigidTransform() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();
	ADNGeometrySynchronization::syncPartFramesFromGeometry(*fixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);

	const Mat3 samsonMoveRotation = ADNFrameUtils::rotationAroundAxis(Vec3{ 0.3, 0.7, 1.0 }, 0.63);
	rotateBaseSegmentGeometryOnlyRaw(fixture.previous, samsonMoveRotation);
	rotateBaseSegmentGeometryOnlyRaw(fixture.baseSegment, samsonMoveRotation);
	rotateBaseSegmentGeometryOnlyRaw(fixture.next, samsonMoveRotation);

	ADNGeometrySynchronization::syncPartFramesBeforeGeometryEdit(*fixture.part);

	SBPointer<ADNNucleotide> left = getLeftNucleotide(fixture.baseSegment);
	SBPointer<ADNNucleotide> right = getRightNucleotide(fixture.baseSegment);
	const double pairDistance = distanceValue(left->GetPosition(), right->GetPosition());
	const double leftBackboneSidechainDistance =
		distanceValue(left->GetBackbonePosition(), left->GetSidechainPosition());
	const double rightBackboneSidechainDistance =
		distanceValue(right->GetBackbonePosition(), right->GetSidechainPosition());

	const Frame frameBefore = ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment);
	const double delta = -0.41;
	const Mat3 expectedFrameRotation = ADNFrameUtils::rotationAroundAxis(frameBefore.e3, delta);

	ADNGeometrySynchronization::rotateDoubleStrandGeometry(*fixture.doubleStrand, delta);
	ADNGeometrySynchronization::syncPartFramesAfterGeometryEdit(*fixture.part);

	const Frame frameAfter = ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment);
	requireNear("rotate double strand preserves pair distance",
		distanceValue(left->GetPosition(), right->GetPosition()),
		pairDistance,
		1.0e-6);
	requireNear("rotate double strand preserves left backbone sidechain distance",
		distanceValue(left->GetBackbonePosition(), left->GetSidechainPosition()),
		leftBackboneSidechainDistance,
		1.0e-6);
	requireNear("rotate double strand preserves right backbone sidechain distance",
		distanceValue(right->GetBackbonePosition(), right->GetSidechainPosition()),
		rightBackboneSidechainDistance,
		1.0e-6);
	requireFrameNear("rotate double strand rotates frame",
		frameAfter,
		rotated(expectedFrameRotation, frameBefore),
		1.0e-8);
	requireTrue("rotate double strand keeps base segment geometry valid",
		ADNGeometrySynchronization::validateBaseSegmentGeometry(*fixture.baseSegment),
		"Expected synchronized rotated base-segment frame to remain valid.");

}

void testRotateDoubleStrandGeometryFullTurnReturnsToStart() {

	using namespace ADNFrameUtils;

	constexpr double pi = 3.141592653589793238462643383279502884;
	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();
	ADNGeometrySynchronization::syncPartFramesFromGeometry(*fixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);

	SBPointer<ADNNucleotide> left = getLeftNucleotide(fixture.baseSegment);
	SBPointer<ADNNucleotide> right = getRightNucleotide(fixture.baseSegment);
	const SBPosition3 leftBackbone = left->GetBackbonePosition();
	const SBPosition3 leftSidechain = left->GetSidechainPosition();
	const SBPosition3 rightBackbone = right->GetBackbonePosition();
	const SBPosition3 rightSidechain = right->GetSidechainPosition();
	const Frame frame = ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment);

	for (int i = 0; i < 12; ++i)
		ADNGeometrySynchronization::rotateDoubleStrandGeometry(*fixture.doubleStrand, 2.0 * pi / 12.0);

	ADNGeometrySynchronization::syncPartFramesAfterGeometryEdit(*fixture.part);

	requirePositionNear("full turn left backbone", left->GetBackbonePosition(), leftBackbone, 1.0e-6);
	requirePositionNear("full turn left sidechain", left->GetSidechainPosition(), leftSidechain, 1.0e-6);
	requirePositionNear("full turn right backbone", right->GetBackbonePosition(), rightBackbone, 1.0e-6);
	requirePositionNear("full turn right sidechain", right->GetSidechainPosition(), rightSidechain, 1.0e-6);
	requireFrameNear("full turn base segment frame",
		ADNFrameAdapters::frameFromOrientable(*fixture.baseSegment),
		frame,
		1.0e-8);

}

void testTwisterTemplateReconstructionIsEquivariantAfterRigidTransform() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture originalFixture = createBaseSegmentFrameFixture();
	BaseSegmentFrameFixture transformedFixture = createBaseSegmentFrameFixture();
	originalFixture.doubleStrand->SetInitialTwistAngle(12.0);
	transformedFixture.doubleStrand->SetInitialTwistAngle(12.0);

	ADNGeometrySynchronization::syncPartFramesFromGeometry(*originalFixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);
	ADNGeometrySynchronization::syncPartFramesFromGeometry(*transformedFixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);

	const Mat3 samsonMoveRotation = ADNFrameUtils::rotationAroundAxis(Vec3{ 0.4, -0.2, 1.0 }, 0.77);
	rotateBaseSegmentGeometryOnlyRaw(transformedFixture.previous, samsonMoveRotation);
	rotateBaseSegmentGeometryOnlyRaw(transformedFixture.baseSegment, samsonMoveRotation);
	rotateBaseSegmentGeometryOnlyRaw(transformedFixture.next, samsonMoveRotation);

	DASBackToTheAtom btta;
	ADNGeometrySynchronization::prepareBaseSegmentFrameForTemplateReconstruction(*originalFixture.baseSegment);
	ADNGeometrySynchronization::prepareBaseSegmentFrameForTemplateReconstruction(*transformedFixture.baseSegment);
	btta.SetNucleotidePosition(originalFixture.baseSegment, true);
	btta.SetNucleotidePosition(transformedFixture.baseSegment, true);
	ADNGeometrySynchronization::syncPartFramesAfterGeometryEdit(*originalFixture.part);
	ADNGeometrySynchronization::syncPartFramesAfterGeometryEdit(*transformedFixture.part);

	SBPointer<ADNNucleotide> originalLeft = getLeftNucleotide(originalFixture.baseSegment);
	SBPointer<ADNNucleotide> originalRight = getRightNucleotide(originalFixture.baseSegment);
	SBPointer<ADNNucleotide> transformedLeft = getLeftNucleotide(transformedFixture.baseSegment);
	SBPointer<ADNNucleotide> transformedRight = getRightNucleotide(transformedFixture.baseSegment);
	const Frame originalFrame = ADNFrameAdapters::frameFromOrientable(*originalFixture.baseSegment);
	const Frame transformedFrame = ADNFrameAdapters::frameFromOrientable(*transformedFixture.baseSegment);

	requireVecNear("twister equivariant left backbone",
		vecFromPosition(transformedLeft->GetBackbonePosition()),
		rotatedAroundOrigin(samsonMoveRotation, originalLeft->GetBackbonePosition()),
		1.0e-6);
	requireVecNear("twister equivariant left sidechain",
		vecFromPosition(transformedLeft->GetSidechainPosition()),
		rotatedAroundOrigin(samsonMoveRotation, originalLeft->GetSidechainPosition()),
		1.0e-6);
	requireVecNear("twister equivariant right backbone",
		vecFromPosition(transformedRight->GetBackbonePosition()),
		rotatedAroundOrigin(samsonMoveRotation, originalRight->GetBackbonePosition()),
		1.0e-6);
	requireVecNear("twister equivariant right sidechain",
		vecFromPosition(transformedRight->GetSidechainPosition()),
		rotatedAroundOrigin(samsonMoveRotation, originalRight->GetSidechainPosition()),
		1.0e-6);
	requireFrameNear("twister equivariant base segment frame",
		transformedFrame,
		rotated(samsonMoveRotation, originalFrame),
		1.0e-8);

}

void testTwisterTemplateReconstructionDoesNotAccumulatePhase() {

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();
	fixture.doubleStrand->SetInitialTwistAngle(17.0);
	ADNGeometrySynchronization::syncPartFramesFromGeometry(*fixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);
	ADNGeometrySynchronization::prepareBaseSegmentFrameForTemplateReconstruction(*fixture.baseSegment);
	const ADNFrameUtils::Frame canonicalFrame =
		ADNFrameAdapters::sanitizedFrame(*fixture.baseSegment);

	DASBackToTheAtom btta;
	btta.SetNucleotidePosition(fixture.baseSegment, true);

	SBPointer<ADNNucleotide> left = getLeftNucleotide(fixture.baseSegment);
	SBPointer<ADNNucleotide> right = getRightNucleotide(fixture.baseSegment);
	const SBPosition3 leftBackbone = left->GetBackbonePosition();
	const SBPosition3 leftSidechain = left->GetSidechainPosition();
	const SBPosition3 rightBackbone = right->GetBackbonePosition();
	const SBPosition3 rightSidechain = right->GetSidechainPosition();

	ADNFrameAdapters::setFrame(*fixture.baseSegment, canonicalFrame);
	btta.SetNucleotidePosition(fixture.baseSegment, true);

	requirePositionNear("twister target left backbone", left->GetBackbonePosition(), leftBackbone, 1.0e-9);
	requirePositionNear("twister target left sidechain", left->GetSidechainPosition(), leftSidechain, 1.0e-9);
	requirePositionNear("twister target right backbone", right->GetBackbonePosition(), rightBackbone, 1.0e-9);
	requirePositionNear("twister target right sidechain", right->GetSidechainPosition(), rightSidechain, 1.0e-9);

}

void testDASReconstructionSideFramesRemainRightHanded() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture();
	fixture.doubleStrand->SetInitialTwistAngle(21.0);

	DASBackToTheAtom btta;
	ADNGeometrySynchronization::prepareBaseSegmentFrameForTemplateReconstruction(*fixture.baseSegment);
	btta.SetNucleotidePosition(fixture.baseSegment, true);

	const Frame twistedLeft = ADNFrameAdapters::sanitizedFrame(*getLeftNucleotide(fixture.baseSegment));
	const Frame twistedRight = ADNFrameAdapters::sanitizedFrame(*getRightNucleotide(fixture.baseSegment));

	requireTrue("twisted left frame right handed",
		isOrthonormalRightHanded(twistedLeft, 1.0e-9),
		"Expected twisted left nucleotide frame to remain right-handed.");
	requireTrue("twisted right frame right handed",
		isOrthonormalRightHanded(twistedRight, 1.0e-9),
		"Expected twisted right nucleotide frame to remain right-handed.");
	requireTrue("twisted side base normals coplanar",
		std::abs(dot(normalized(twistedLeft.e3), normalized(twistedRight.e3))) > 0.999,
		"Expected twisted nucleotide base normals to be parallel or antiparallel.");

	ADNGeometrySynchronization::prepareBaseSegmentFrameForTemplateReconstruction(*fixture.baseSegment);
	btta.UntwistNucleotidesPosition(fixture.baseSegment);

	const Frame untwistedLeft = ADNFrameAdapters::sanitizedFrame(*getLeftNucleotide(fixture.baseSegment));
	const Frame untwistedRight = ADNFrameAdapters::sanitizedFrame(*getRightNucleotide(fixture.baseSegment));

	requireTrue("untwisted left frame right handed",
		isOrthonormalRightHanded(untwistedLeft, 1.0e-9),
		"Expected untwisted left nucleotide frame to remain right-handed.");
	requireTrue("untwisted right frame right handed",
		isOrthonormalRightHanded(untwistedRight, 1.0e-9),
		"Expected untwisted right nucleotide frame to remain right-handed.");
	requireTrue("untwisted side base normals coplanar",
		std::abs(dot(normalized(untwistedLeft.e3), normalized(untwistedRight.e3))) > 0.999,
		"Expected untwisted nucleotide base normals to be parallel or antiparallel.");

}

void testComplementPlacementPreservesExistingNucleotideGeometry() {

	SBPointer<ADNPart> part = new ADNPart();
	SBPointer<ADNDoubleStrand> doubleStrand = new ADNDoubleStrand();
	SBPointer<ADNSingleStrand> leftStrand = new ADNSingleStrand();
	SBPointer<ADNSingleStrand> rightStrand = new ADNSingleStrand();
	SBPointer<ADNBaseSegment> baseSegment = new ADNBaseSegment(CellType::BasePair);
	baseSegment->SetPosition(positionAngstrom(0.0, 0.0, 0.0));

	part->RegisterDoubleStrand(doubleStrand);
	part->RegisterSingleStrand(leftStrand);
	part->RegisterSingleStrand(rightStrand);
	part->RegisterBaseSegmentEnd(doubleStrand, baseSegment);

	SBPointer<ADNNucleotide> existing = createSyntheticNucleotide(
		SBResidue::ResidueType::DA, 0.0, -0.5, 0.0, vector3(0.0, 1.0, 0.0));
	existing->SetBackbonePosition(positionAngstrom(0.0, -0.8, 0.0));
	existing->SetSidechainPosition(positionAngstrom(0.0, -0.2, 0.0));
	SBPointer<ADNNucleotide> created = createSyntheticNucleotide(
		SBResidue::ResidueType::DT, 3.0, 3.0, 3.0, vector3(0.0, -1.0, 0.0));

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(baseSegment->GetCell()());
	basePair->AddPair(existing, created);
	existing->SetBaseSegment(baseSegment);
	created->SetBaseSegment(baseSegment);
	part->RegisterNucleotideThreePrime(leftStrand, existing);
	part->RegisterNucleotideThreePrime(rightStrand, created);

	const SBPosition3 existingPosition = existing->GetPosition();
	const SBPosition3 existingBackbone = existing->GetBackbonePosition();
	const SBPosition3 existingSidechain = existing->GetSidechainPosition();
	const SBPosition3 createdPlaceholder = created->GetPosition();

	SBPointerIndexer<ADNNucleotide> createdNucleotides;
	createdNucleotides.addReferenceTarget(created());

	DASBackToTheAtom btta;
	btta.SetPositionsForNewNucleotides(part,
		createdNucleotides,
		DASBackToTheAtom::NewNucleotidePlacementMode::PositionInputNucleotidesOnly);

	requirePositionNear("complement placement preserves existing center",
		existing->GetPosition(),
		existingPosition,
		1.0e-9);
	requirePositionNear("complement placement preserves existing backbone",
		existing->GetBackbonePosition(),
		existingBackbone,
		1.0e-9);
	requirePositionNear("complement placement preserves existing sidechain",
		existing->GetSidechainPosition(),
		existingSidechain,
		1.0e-9);
	requireTrue("complement placement moves created nucleotide",
		distanceValue(created->GetPosition(), createdPlaceholder) > 1.0,
		"Expected complementary placement to update only the newly created nucleotide.");

}

void testSingleStrandAtomGenerationUsesExistingNucleotideCenter() {

	SBPointer<ADNPart> part = new ADNPart();
	SBPointer<ADNDoubleStrand> doubleStrand = new ADNDoubleStrand();
	SBPointer<ADNSingleStrand> strand = new ADNSingleStrand();
	SBPointer<ADNBaseSegment> baseSegment = new ADNBaseSegment(CellType::BasePair);
	baseSegment->SetPosition(positionAngstrom(0.0, 0.0, 0.0));

	part->RegisterDoubleStrand(doubleStrand);
	part->RegisterSingleStrand(strand);
	part->RegisterBaseSegmentEnd(doubleStrand, baseSegment);

	SBPointer<ADNNucleotide> nucleotide = createSyntheticNucleotide(
		SBResidue::ResidueType::DA, 0.0, -5.0, 0.0, vector3(0.0, 1.0, 0.0));
	nucleotide->SetBackbonePosition(positionAngstrom(0.0, -5.3, 0.0));
	nucleotide->SetSidechainPosition(positionAngstrom(0.0, -4.7, 0.0));

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(baseSegment->GetCell()());
	basePair->SetLeftNucleotide(nucleotide);
	nucleotide->SetBaseSegment(baseSegment);
	part->RegisterNucleotideThreePrime(strand, nucleotide);

	const SBPosition3 nucleotidePosition = nucleotide->GetPosition();
	const SBPosition3 backbonePosition = nucleotide->GetBackbonePosition();
	const SBPosition3 sidechainPosition = nucleotide->GetSidechainPosition();

	DASBackToTheAtom btta;
	btta.GenerateAllAtomModel(part, false);

	requirePositionNear("single strand atom generation preserves center",
		nucleotide->GetPosition(),
		nucleotidePosition,
		1.0e-9);
	requirePositionNear("single strand atom generation preserves backbone",
		nucleotide->GetBackbonePosition(),
		backbonePosition,
		1.0e-9);
	requirePositionNear("single strand atom generation preserves sidechain",
		nucleotide->GetSidechainPosition(),
		sidechainPosition,
		1.0e-9);
	requireTrue("single strand atom generation adds atoms",
		nucleotide->GetAtoms().size() > 0,
		"Expected atom generation to populate the existing nucleotide.");

	const ADNFrameUtils::Vec3 atomCenter = averageGeneratedAtomPosition(nucleotide);
	const double distanceToNucleotide =
		ADNFrameUtils::norm(atomCenter - vecFromPosition(nucleotide->GetPosition()));
	const double distanceToBaseSegment =
		ADNFrameUtils::norm(atomCenter - vecFromPosition(baseSegment->GetPosition()));
	requireTrue("single strand atom generation uses nucleotide center",
		distanceToNucleotide < distanceToBaseSegment,
		"Expected generated atoms to stay closer to the existing nucleotide than to the absent complementary side.");

}

void testAllAtomGenerationPreservesSynchronizedNucleotideGeometry() {

	AtomicGenerationFixture fixture = createAtomicGenerationFixture();
	ADNGeometrySynchronization::syncPartFramesFromGeometry(*fixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);

	const ADNFrameUtils::Mat3 samsonMoveRotation =
		ADNFrameUtils::rotationAroundAxis(ADNFrameUtils::Vec3{ 0.3, 0.2, 1.0 }, 0.55);
	rotateBaseSegmentGeometryOnlyRaw(fixture.baseSegment, samsonMoveRotation);
	ADNGeometrySynchronization::syncPartFramesBeforeGeometryEdit(*fixture.part);

	const SBPosition3 leftBackbone = fixture.left->GetBackbonePosition();
	const SBPosition3 leftSidechain = fixture.left->GetSidechainPosition();
	const SBPosition3 rightBackbone = fixture.right->GetBackbonePosition();
	const SBPosition3 rightSidechain = fixture.right->GetSidechainPosition();

	DASBackToTheAtom btta;
	btta.GenerateAllAtomModel(fixture.part, false);

	requirePositionNear("all atom generation preserves left backbone",
		fixture.left->GetBackbonePosition(),
		leftBackbone,
		1.0e-9);
	requirePositionNear("all atom generation preserves left sidechain",
		fixture.left->GetSidechainPosition(),
		leftSidechain,
		1.0e-9);
	requirePositionNear("all atom generation preserves right backbone",
		fixture.right->GetBackbonePosition(),
		rightBackbone,
		1.0e-9);
	requirePositionNear("all atom generation preserves right sidechain",
		fixture.right->GetSidechainPosition(),
		rightSidechain,
		1.0e-9);
	requireTrue("all atom generation adds atoms",
		fixture.left->GetAtoms().size() > 0 && fixture.right->GetAtoms().size() > 0,
		"Expected generated atomic details on both nucleotides.");

	const ADNFrameUtils::Vec3 leftNormal = sidechainPlaneNormal(fixture.left);
	const ADNFrameUtils::Vec3 rightNormal = sidechainPlaneNormal(fixture.right);
	requireTrue("all atom generation left base plane normal",
		!ADNFrameUtils::isNearlyZero(leftNormal),
		"Expected generated left nucleotide base atoms to define a plane.");
	requireTrue("all atom generation right base plane normal",
		!ADNFrameUtils::isNearlyZero(rightNormal),
		"Expected generated right nucleotide base atoms to define a plane.");
	requireTrue("all atom generation base planes coplanar",
		std::abs(ADNFrameUtils::dot(leftNormal, rightNormal)) > 0.95,
		"Expected generated paired bases to be coplanar.");

}

void testAllAtomGenerationAlignsBasePlanesAndBackboneAfterRigidTransform() {

	using namespace ADNFrameUtils;

	BaseSegmentFrameFixture fixture = createBaseSegmentFrameFixture(3.4);
	fixture.doubleStrand->SetInitialTwistAngle(14.0);
	// Atom generation preserves existing coarse geometry; seed this fixture
	// with helical base-pair phases before applying the SAMSON-like transform.
	applyTemplatePhaseToBaseSegmentGeometry(fixture.previous);
	applyTemplatePhaseToBaseSegmentGeometry(fixture.baseSegment);
	applyTemplatePhaseToBaseSegmentGeometry(fixture.next);

	ADNGeometrySynchronization::syncPartFramesFromGeometry(*fixture.part,
		ADNGeometrySynchronization::SyncReason::ManualRepair);

	const Mat3 samsonMoveRotation =
		rotationAroundAxis(Vec3{ -0.2, 0.5, 1.0 }, 0.66);
	rotateBaseSegmentGeometryOnlyRaw(fixture.previous, samsonMoveRotation);
	rotateBaseSegmentGeometryOnlyRaw(fixture.baseSegment, samsonMoveRotation);
	rotateBaseSegmentGeometryOnlyRaw(fixture.next, samsonMoveRotation);
	ADNGeometrySynchronization::syncPartFramesBeforeGeometryEdit(*fixture.part);

	SBPointer<ADNNucleotide> left = getLeftNucleotide(fixture.baseSegment);
	SBPointer<ADNNucleotide> right = getRightNucleotide(fixture.baseSegment);
	const SBPosition3 leftBackbone = left->GetBackbonePosition();
	const SBPosition3 leftSidechain = left->GetSidechainPosition();
	const SBPosition3 rightBackbone = right->GetBackbonePosition();
	const SBPosition3 rightSidechain = right->GetSidechainPosition();

	DASBackToTheAtom btta;
	btta.GenerateAllAtomModel(fixture.part, false);

	requirePositionNear("all atom axis generation preserves left backbone",
		left->GetBackbonePosition(),
		leftBackbone,
		1.0e-9);
	requirePositionNear("all atom axis generation preserves left sidechain",
		left->GetSidechainPosition(),
		leftSidechain,
		1.0e-9);
	requirePositionNear("all atom axis generation preserves right backbone",
		right->GetBackbonePosition(),
		rightBackbone,
		1.0e-9);
	requirePositionNear("all atom axis generation preserves right sidechain",
		right->GetSidechainPosition(),
		rightSidechain,
		1.0e-9);

	const Vec3 leftNormal = sidechainPlaneNormal(left);
	const Vec3 rightNormal = sidechainPlaneNormal(right);
	const Vec3 axis = localBaseSegmentAxis(fixture.baseSegment);
	requireTrue("all atom axis generation left base plane normal",
		!isNearlyZero(leftNormal),
		"Expected generated left base atoms to define a plane.");
	requireTrue("all atom axis generation right base plane normal",
		!isNearlyZero(rightNormal),
		"Expected generated right base atoms to define a plane.");
	requireTrue("all atom axis generation base planes coplanar",
		std::abs(dot(leftNormal, rightNormal)) > 0.95,
		"Expected generated paired bases to stay coplanar.");
	requireTrue("all atom axis generation base planes follow axis",
		std::min(std::abs(dot(leftNormal, axis)), std::abs(dot(rightNormal, axis))) > 0.85,
		"Expected generated base-plane normals to follow the local base-segment axis.");

	const double leftBackboneLink = pToPreviousO3Distance(left);
	const double rightBackboneLink = pToPreviousO3Distance(right);
	requireTrue("all atom axis generation left P-O3 distance",
		leftBackboneLink >= 100.0 && leftBackboneLink <= 250.0,
		"Expected left generated P to previous O3' distance to remain plausible, got " +
		std::to_string(leftBackboneLink) + " pm.");
	requireTrue("all atom axis generation right P-O3 distance",
		rightBackboneLink >= 100.0 && rightBackboneLink <= 250.0,
		"Expected right generated P to previous O3' distance to remain plausible, got " +
		std::to_string(rightBackboneLink) + " pm.");

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

void testGenerateSequenceHonorsLengthAlphabetAndMaxGs() {

	const std::string sequence = DASAlgorithms::GenerateSequence(1.0, 2, 200);
	requireEqual("generated sequence length",
		sequence.size(),
		static_cast<size_t>(200));
	requireTrue("generated sequence alphabet",
		sequence.find_first_not_of("ACGT") == std::string::npos,
		"Expected generated sequence to contain only DNA bases.");
	requireTrue("generated sequence max consecutive G",
		sequence.find("GGG") == std::string::npos,
		"Expected generated sequence to honor max consecutive G limit.");

	const std::string emptySequence = DASAlgorithms::GenerateSequence(0.5, 2, 0);
	requireTrue("generated empty sequence",
		emptySequence.empty(),
		"Expected zero-size sequence request to return an empty string.");

}

void testDaedalusEdgeSizeQuantizationBoundaries() {

	requireEqual("edge size clamps below minimum",
		DASDaedalus::CalculateEdgeSize(SBQuantity::nanometer(1.0)),
		31);
	requireEqual("edge size rounds to nearest full turn",
		DASDaedalus::CalculateEdgeSize(SBQuantity::nanometer(ADNConstants::BP_RISE * 4.0 * 10.5)),
		42);
	requireEqual("edge size next full turn boundary",
		DASDaedalus::CalculateEdgeSize(SBQuantity::nanometer(ADNConstants::BP_RISE * 5.0 * 10.5)),
		52);

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

void testSerializedNodeValidation() {

	SBPointer<ADNNucleotide> nucleotide = new ADNNucleotide();
	SBPointer<ADNLoop> loop = new ADNLoop();
	SBNodeIndexer nodeIndexer;
	nodeIndexer.addNode(nucleotide());
	nodeIndexer.addNode(loop());

	const unsigned int nucleotideIndex = nodeIndexer.getIndex(nucleotide());
	const unsigned int loopIndex = nodeIndexer.getIndex(loop());
	const unsigned int nullIndex = static_cast<unsigned int>(-1);

	SBPointer<ADNNucleotide> resolvedNucleotide =
		ADNNodeValidation::GetSerializedAdenitaNode<ADNNucleotide>(nodeIndexer, nucleotideIndex, "ADNNucleotide");
	requireTrue("node validation correct type",
		resolvedNucleotide() == nucleotide(),
		"Expected a serialized nucleotide index to resolve to the nucleotide.");

	SBPointer<ADNNucleotide> nullNucleotide =
		ADNNodeValidation::GetSerializedAdenitaNode<ADNNucleotide>(nodeIndexer, nullIndex, "ADNNucleotide");
	requireTrue("node validation null index",
		nullNucleotide == nullptr,
		"Expected the invalid serialized marker to resolve to null.");

	auto resolveWrongType = [&nodeIndexer, loopIndex]() {
		(void)ADNNodeValidation::GetSerializedAdenitaNode<ADNNucleotide>(nodeIndexer, loopIndex, "ADNNucleotide");
	};
	requireThrowsRuntimeError("node validation wrong type", resolveWrongType);

	auto resolveInvalidIndex = [&nodeIndexer]() {
		(void)ADNNodeValidation::GetSerializedAdenitaNode<ADNNucleotide>(nodeIndexer, nodeIndexer.size(), "ADNNucleotide");
	};
	requireThrowsRuntimeError("node validation invalid index", resolveInvalidIndex);

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

void testPolyhedronMetricsAndIndices() {

	DASPolyhedron polyhedron;
	polyhedron.BuildPolyhedron(makeTetrahedronVertices(), makeTetrahedronFaces());

	const auto minEdge = polyhedron.MinimumEdgeLength();
	const auto maxEdge = polyhedron.MaximumEdgeLength();
	requireTrue("polyhedron minimum edge exists",
		minEdge.first != nullptr,
		"Expected minimum edge lookup to return an edge.");
	requireTrue("polyhedron maximum edge exists",
		maxEdge.first != nullptr,
		"Expected maximum edge lookup to return an edge.");
	requireNear("polyhedron minimum edge length",
		minEdge.second,
		1.0,
		1.0e-8);
	requireNear("polyhedron maximum edge length",
		maxEdge.second,
		std::sqrt(2.0),
		1.0e-8);

	unsigned int* indices = polyhedron.GetIndices();
	requireTrue("polyhedron indices allocated",
		indices != nullptr,
		"Expected triangular face indices to be available.");
	if (indices != nullptr) {
		requireEqual("polyhedron first triangle index 0", indices[0], 0u);
		requireEqual("polyhedron first triangle index 1", indices[1], 1u);
		requireEqual("polyhedron first triangle index 2", indices[2], 2u);
		requireEqual("polyhedron second triangle index 0", indices[3], 0u);
		requireEqual("polyhedron second triangle index 1", indices[4], 3u);
		requireEqual("polyhedron second triangle index 2", indices[5], 1u);
	}

}

void testCanDoExportWritesBasicSections() {

	CircularStrandFixture fixture = createCircularStrandFixture();
	const std::filesystem::path path = temporaryConfigPath("adenita_basic_cando_export.cndo");

	ADNLoader::OutputToCanDo(fixture.part, path.string());
	requireTrue("cando export file exists",
		std::filesystem::exists(path),
		"Expected CanDo export to create an output file.");

	const std::string content = readTextFile(path);
	requireTrue("cando export contains format banner",
		content.find("CanDo (.cndo) file format version 1.0") != std::string::npos,
		"Expected CanDo export banner.");
	requireTrue("cando export contains dna topology header",
		content.find("dnaTop,id,up,down,across,seq") != std::string::npos,
		"Expected dnaTop section header.");
	requireTrue("cando export contains dNode header",
		content.find("dNode,\"e0(1)\",\"e0(2)\",\"e0(3)\"") != std::string::npos,
		"Expected dNode section header.");
	requireTrue("cando export contains triad header",
		content.find("triad,\"e1(1)\",\"e1(2)\",\"e1(3)\",\"e2(1)\",\"e2(2)\",\"e2(3)\",\"e3(1)\",\"e3(2)\",\"e3(3)\"") != std::string::npos,
		"Expected triad section header.");
	requireTrue("cando export contains base pair header",
		content.find("id_nt,id1,id2") != std::string::npos,
		"Expected base-pair section header.");
	requireTrue("cando export contains nucleotide bases",
		content.find(",A") != std::string::npos &&
		content.find(",T") != std::string::npos &&
		content.find(",G") != std::string::npos,
		"Expected exported nucleotide base records.");

}

void writeBasicOxDNAFiles(const std::filesystem::path& topologyPath, const std::filesystem::path& configPath) {

	{
		std::ofstream topology(topologyPath);
		topology << "2 1\n";
		topology << "0 A -1 1\n";
		topology << "0 T 0 -1\n";
	}

	{
		std::ofstream config(configPath);
		config << "t = 0\n";
		config << "b = 0 0 0\n";
		config << "E = 0 0 0\n";
		config << "0 0 0 0 1 0 1 0 0 0 0 0 0 0 0\n";
		config << "1 0 0 0 1 0 1 0 0 0 0 0 0 0 0\n";
	}

}

std::string buildCadnanoLegacyTerminalTubeJson() {

	std::ostringstream scaf;
	std::ostringstream stap;
	std::ostringstream loops;
	std::ostringstream skips;
	for (int i = 0; i < 32; ++i) {
		if (i > 0) {
			scaf << ",";
			stap << ",";
			loops << ",";
			skips << ",";
		}

		if (i == 30) scaf << "[-1,-1,0,31]";
		else if (i == 31) scaf << "[0,30,-1,-1]";
		else scaf << "[-1,-1,-1,-1]";

		stap << "[-1,-1,-1,-1]";
		loops << "0";
		skips << "0";
	}

	std::ostringstream json;
	json << "{"
		<< "\"name\":\"terminal_tube\","
		<< "\"vstrands\":[{"
		<< "\"num\":0,"
		<< "\"col\":0,"
		<< "\"row\":0,"
		<< "\"scaf\":[" << scaf.str() << "],"
		<< "\"stap\":[" << stap.str() << "],"
		<< "\"loop\":[" << loops.str() << "],"
		<< "\"skip\":[" << skips.str() << "]"
		<< "}]"
		<< "}";
	return json.str();

}

void testOxDNAImportResultReportsSuccess() {

	const std::filesystem::path topologyPath = temporaryConfigPath("adenita_basic_oxdna.top");
	const std::filesystem::path configPath = temporaryConfigPath("adenita_basic_oxdna.dat");
	writeBasicOxDNAFiles(topologyPath, configPath);

	const ADNLoader::OxDNAImportResult result = ADNLoader::InputFromOxDNA(topologyPath.string(), configPath.string());
	requireTrue("oxdna import result succeeds",
		result.succeeded(),
		"Expected a valid OxDNA fixture to import successfully.");
	requireTrue("oxdna import result has no error",
		!result.hasError,
		"Expected valid OxDNA import result to have no error.");
	requireTrue("oxdna import result contains part",
		result.part != nullptr,
		"Expected valid OxDNA import result to contain a part.");
	requireEqual("oxdna import nucleotide count",
		result.part->GetNumberOfNucleotides(),
		2u);
	requireEqual("oxdna import strand count",
		result.part->GetNumberOfSingleStrands(),
		1u);

}

void testOxDNAImportResultReportsErrors() {

	const std::filesystem::path topologyPath = temporaryConfigPath("adenita_invalid_oxdna.top");
	const std::filesystem::path configPath = temporaryConfigPath("adenita_invalid_oxdna.dat");
	{
		std::ofstream topology(topologyPath);
		topology << "2 1\n";
		topology << "malformed\n";
	}
	{
		std::ofstream config(configPath);
		config << "t = 0\n";
	}

	const ADNLoader::OxDNAImportResult malformedResult = ADNLoader::InputFromOxDNA(topologyPath.string(), configPath.string());
	requireTrue("oxdna malformed import reports error",
		malformedResult.hasError,
		"Expected malformed topology to report an import error.");
	requireTrue("oxdna malformed import does not succeed",
		!malformedResult.succeeded(),
		"Expected malformed topology import not to succeed.");

	const ADNLoader::OxDNAImportResult missingResult = ADNLoader::InputFromOxDNA("missing_topology.top", "missing_config.dat");
	requireTrue("oxdna missing files report error",
		missingResult.hasError,
		"Expected missing OxDNA files to report an import error.");
	requireTrue("oxdna missing files do not succeed",
		!missingResult.succeeded(),
		"Expected missing OxDNA files import not to succeed.");

}

void testNtthalParserAcceptsValidOutput() {

	const std::string output =
		"Calculated thermodynamical parameters for dimer:\tdS = -68.9269\tdH = -24400\tdG = -3022.33\tt = -37.8822\r\n"
		"SEQ\t    \r\n"
		"SEQ\tTCGG\r\n"
		"STR\tAGCC\r\n"
		"STR\t    \r\n";

	const ThermodynamicParameters result = PIPrimer3::ParseNtthalOutput(output);
	requireTrue("ntthal parser valid output",
		result.isValid,
		"Expected valid ntthal output to parse.");
	requireNear("ntthal parser entropy", result.dS_, -68.9269, 1.0e-6);
	requireNear("ntthal parser enthalpy", result.dH_, -24400.0, 1.0e-6);
	requireNear("ntthal parser gibbs", result.dG_, -3022.33, 1.0e-6);
	requireNear("ntthal parser temperature", result.T_, -37.8822, 1.0e-6);

}

void testNtthalParserRejectsInvalidOutput() {

	const ThermodynamicParameters missingField = PIPrimer3::ParseNtthalOutput(
		"Calculated thermodynamical parameters for dimer:\tdS = -68.9\tdH = -24400\tt = -37.8\n"
		"SEQ\nSEQ\nSTR\nSTR\n");
	requireTrue("ntthal parser rejects missing field",
		!missingField.isValid,
		"Expected missing dG field to be rejected.");

	const ThermodynamicParameters shortOutput = PIPrimer3::ParseNtthalOutput(
		"Calculated thermodynamical parameters for dimer:\tdS = -68.9\tdH = -24400\tdG = -3022\tt = -37.8\n"
		"SEQ\n");
	requireTrue("ntthal parser rejects short output",
		!shortOutput.isValid,
		"Expected short ntthal output to be rejected.");

	const ThermodynamicParameters nonnumericOutput = PIPrimer3::ParseNtthalOutput(
		"Calculated thermodynamical parameters for dimer:\tdS = nope\tdH = -24400\tdG = -3022\tt = -37.8\n"
		"SEQ\nSEQ\nSTR\nSTR\n");
	requireTrue("ntthal parser rejects nonnumeric output",
		!nonnumericOutput.isValid,
		"Expected nonnumeric ntthal output to be rejected.");

	const ThermodynamicParameters malformedNumericOutput = PIPrimer3::ParseNtthalOutput(
		"Calculated thermodynamical parameters for dimer:\tdS = -68.9bad\tdH = -24400\tdG = -3022\tt = -37.8\n"
		"SEQ\nSEQ\nSTR\nSTR\n");
	requireTrue("ntthal parser rejects malformed numeric output",
		!malformedNumericOutput.isValid,
		"Expected malformed numeric ntthal output to be rejected.");

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

void testCadnanoRejectsMalformedLegacyJson() {

	const std::filesystem::path path = temporaryConfigPath("adenita_invalid_cadnano.json");
	writeTextFile(path, R"json({"name":"broken","vstrands":{}})json");

	DASCadnano cadnano;
	SBPointer<ADNPart> part = cadnano.CreateCadnanoPart(path.string());

	requireTrue("cadnano malformed json rejected",
		part == nullptr,
		"Expected malformed cadnano JSON to be rejected.");
	requireTrue("cadnano malformed json reports detail",
		cadnano.GetLastError().find("vstrands") != std::string::npos,
		"Expected malformed cadnano JSON to report the failing field.");

	std::error_code errorCode;
	std::filesystem::remove(path, errorCode);

}

void testCadnanoImportsTerminalTubeAtLastPosition() {

	const std::filesystem::path path = temporaryConfigPath("adenita_terminal_tube_cadnano.json");
	writeTextFile(path, buildCadnanoLegacyTerminalTubeJson());

	DASCadnano cadnano;
	SBPointer<ADNPart> part = cadnano.CreateCadnanoPart(path.string());

	requireTrue("cadnano terminal tube imports",
		part != nullptr,
		"Expected a cadnano tube that reaches the last position to import successfully.");
	if (part != nullptr) {
		requireTrue("cadnano terminal tube creates base segments",
			part->GetNumberOfBaseSegments() > 0,
			"Expected imported cadnano tube to create base segments.");
		requireTrue("cadnano terminal tube creates conformations",
			cadnano.CreateConformations(part),
			"Expected scaffold-only cadnano import to create conformations safely.");
	}

	std::error_code errorCode;
	std::filesystem::remove(path, errorCode);

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
	testFrameUtilsRotateFrameAroundZ();
	testFrameUtilsOrthonormalizationRepairsSmallDrift();
	testFrameUtilsInvalidFrameFallsBack();
	testFrameUtilsRigidRotationPreservesDistances();
	testFrameUtilsDerivesRotatedMockGeometryFrame();
	testFrameAdaptersSanitizeAndRotateOrientable();
	testTemplateFramePreparationRoundTripLeftSide();
	testTemplateFramePreparationRoundTripRightSide();
	testTemplateFramePreparationTargetStateIsStable();
	testTemplateFrameHandednessAcrossPhases();
	testTemplateFrameBasePlaneNormalsStayCoplanar();
	testCanonicalTemplateFrameFromCurrentGeometryTracksBaseSegmentAxis();
	testCanonicalTemplateFrameFromCurrentGeometryDoesNotMutateBaseSegment();
	testNucleotideSetPositionTranslatesBackboneAndSidechain();
	testGeometrySynchronizationDerivesNucleotideFrame();
	testGeometryValidationRejectsStaleNucleotideFrame();
	testGeometryValidationRejectsStaleBaseSegmentFrame();
	testGeometryEditBarrierPreservesRotatedNucleotideDirection();
	testFrameUtilsRotationAroundAxisMatchesZRotation();
	testRotateDoubleStrandGeometryPreservesDistancesAfterRigidTransform();
	testRotateDoubleStrandGeometryFullTurnReturnsToStart();
	testTwisterTemplateReconstructionIsEquivariantAfterRigidTransform();
	testTwisterTemplateReconstructionDoesNotAccumulatePhase();
	testDASReconstructionSideFramesRemainRightHanded();
	testComplementPlacementPreservesExistingNucleotideGeometry();
	testSingleStrandAtomGenerationUsesExistingNucleotideCenter();
	testAllAtomGenerationPreservesSynchronizedNucleotideGeometry();
	testAllAtomGenerationAlignsBasePlanesAndBackboneAfterRigidTransform();
	testCircularSingleStrandWrapsWithoutChangingSequenceOrder();
	testModernJsonValidation();
	testLegacyJsonValidation();
	testCircularSingleStrandJsonRoundTrip();
	testBuildTopScalesHandlesBrokenNucleotideLinks();
	testBuildTopScalesParametrizedHandlesBrokenNucleotideLinks();
	testGenerateSequenceHonorsLengthAlphabetAndMaxGs();
	testDaedalusEdgeSizeQuantizationBoundaries();
	testBaseSegmentSetCellReplacesChild();
	testLoopPairSettersReplaceOnlySelectedChild();
	testSerializedNodeValidation();
	testPolyhedronRebuildClearsPreviousTopology();
	testPolyhedronEdgeLookupDoesNotAllocatePlaceholders();
	testPolyhedronMetricsAndIndices();
	testCanDoExportWritesBasicSections();
	testOxDNAImportResultReportsSuccess();
	testOxDNAImportResultReportsErrors();
	testNtthalParserAcceptsValidOutput();
	testNtthalParserRejectsInvalidOutput();
	testPlyLoaderWeldsDuplicatedAssimpVertices();
	testCadnanoRejectsMalformedLegacyJson();
	testCadnanoImportsTerminalTubeAtLastPosition();
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

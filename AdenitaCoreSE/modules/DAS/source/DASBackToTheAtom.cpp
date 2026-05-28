#include "DASBackToTheAtom.hpp"
#include "ADNBackbone.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNGeometrySynchronization.hpp"
#include "ADNLogger.hpp"
#include "ADNSidechain.hpp"

#include "SBProxy.hpp"
#include "SAMSON.hpp"
#include "SBStructuralModel.hpp"


#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <filesystem>
#include <limits>

namespace {

struct TemplateToWorldTransform {
	ADNFrameUtils::Frame canonicalFrame;
	double phaseRadians{ 0.0 };
	ublas::matrix<double> basisMatrix;
	ADNFrameUtils::Frame leftFrame;
	ADNFrameUtils::Frame rightFrame;
};

[[nodiscard]] ublas::vector<double> frameVectorToUblas(const ADNFrameUtils::Vec3& vector) {

	ublas::vector<double> result(3);
	result[0] = vector.x;
	result[1] = vector.y;
	result[2] = vector.z;
	return result;

}

[[nodiscard]] ublas::matrix<double> frameRowsToUblas(const ADNFrameUtils::Frame& frame) {

	ublas::matrix<double> rows(3, 3);
	ublas::row(rows, 0) = frameVectorToUblas(frame.e1);
	ublas::row(rows, 1) = frameVectorToUblas(frame.e2);
	ublas::row(rows, 2) = frameVectorToUblas(frame.e3);
	return rows;

}

[[nodiscard]] ublas::matrix<double> templateBasisMatrixFromCanonicalFrame(
	const ADNFrameUtils::Frame& canonicalFrame,
	double phaseRadians) {

	// This reproduces the legacy DASBackToTheAtom matrix convention without
	// storing the phase-neutral frame on ADNBaseSegment. ADNVectorMath uses the
	// opposite axis-angle sign from ADNFrameUtils, hence the negative phase.
	ublas::matrix<double> basisRows = frameRowsToUblas(canonicalFrame);
	ublas::matrix<double> rotation =
		ADNVectorMath::MakeRotationMatrix(frameVectorToUblas(canonicalFrame.e3), -phaseRadians);
	basisRows = ADNVectorMath::ApplyTransformation(rotation, basisRows);
	return ublas::trans(basisRows);

}

[[nodiscard]] TemplateToWorldTransform makeTemplateToWorldTransform(
	const ADNBaseSegment& baseSegment,
	const ADNFrameUtils::Frame& canonicalFrame) {

	TemplateToWorldTransform transform;
	transform.canonicalFrame = ADNFrameUtils::orthonormalized(canonicalFrame);
	transform.phaseRadians =
		ADNGeometrySynchronization::baseSegmentReconstructionPhaseRadians(baseSegment);
	transform.basisMatrix =
		templateBasisMatrixFromCanonicalFrame(transform.canonicalFrame, transform.phaseRadians);
	transform.leftFrame =
		ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
			transform.canonicalFrame,
			ADNGeometrySynchronization::TemplateSide::Left,
			transform.phaseRadians);
	transform.rightFrame =
		ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
			transform.canonicalFrame,
			ADNGeometrySynchronization::TemplateSide::Right,
			transform.phaseRadians);
	return transform;

}

[[nodiscard]] TemplateToWorldTransform makeTemplateToWorldTransform(
	const ADNBaseSegment& baseSegment) {

	return makeTemplateToWorldTransform(
		baseSegment,
		ADNGeometrySynchronization::canonicalTemplateFrameFromCurrentGeometry(baseSegment));

}

[[nodiscard]] ADNFrameUtils::Vec3 positionToVec3(const SBPosition3& position) {

	return ADNFrameUtils::Vec3{
		position[0].getValue(),
		position[1].getValue(),
		position[2].getValue()
	};

}

[[nodiscard]] SBPosition3 positionFromVec3(const ADNFrameUtils::Vec3& position) {

	return SBPosition3(
		SBQuantity::picometer(position.x),
		SBQuantity::picometer(position.y),
		SBQuantity::picometer(position.z));

}

[[nodiscard]] ADNFrameUtils::Vec3 baseSegmentTangent(SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment == nullptr) return ADNFrameUtils::Vec3{};

	SBPointer<ADNBaseSegment> previous = baseSegment->GetPrev(true);
	SBPointer<ADNBaseSegment> next = baseSegment->GetNext(true);

	if (previous != nullptr && next != nullptr)
		return positionToVec3(next->GetPosition()) - positionToVec3(previous->GetPosition());
	if (next != nullptr)
		return positionToVec3(next->GetPosition()) - positionToVec3(baseSegment->GetPosition());
	if (previous != nullptr)
		return positionToVec3(baseSegment->GetPosition()) - positionToVec3(previous->GetPosition());

	return ADNFrameUtils::Vec3{};

}

[[nodiscard]] ADNFrameUtils::Vec3 nucleotideTangent(const ADNNucleotide& nucleotide) {

	SBPointer<ADNNucleotide> previous = nucleotide.GetPrev(true);
	SBPointer<ADNNucleotide> next = nucleotide.GetNext(true);

	if (previous != nullptr && next != nullptr)
		return positionToVec3(next->GetPosition()) - positionToVec3(previous->GetPosition());
	if (next != nullptr)
		return positionToVec3(next->GetPosition()) - positionToVec3(nucleotide.GetPosition());
	if (previous != nullptr)
		return positionToVec3(nucleotide.GetPosition()) - positionToVec3(previous->GetPosition());

	return ADNFrameUtils::Vec3{};

}

[[nodiscard]] ADNFrameUtils::Frame nucleotidePlacementFrameFromCoarseGeometry(
	const ADNNucleotide& nucleotide,
	const ADNFrameUtils::Frame& fallback) {

	const ADNFrameUtils::Frame repairedFallback = ADNFrameUtils::orthonormalized(fallback);

	ADNFrameUtils::Vec3 e2 =
		positionToVec3(nucleotide.GetSidechainPosition()) -
		positionToVec3(nucleotide.GetBackbonePosition());
	if (ADNFrameUtils::isNearlyZero(e2))
		e2 = repairedFallback.e2;

	ADNFrameUtils::Vec3 tangent = nucleotideTangent(nucleotide);
	if (ADNFrameUtils::isNearlyZero(tangent))
		tangent = repairedFallback.e3;

	return ADNFrameUtils::frameFromE2AndTangent(e2, tangent, repairedFallback);

}

[[nodiscard]] ADNFrameUtils::Vec3 toLocalCoordinates(
	const ADNFrameUtils::Vec3& worldDelta,
	const ADNFrameUtils::Frame& frame) {

	return ADNFrameUtils::Vec3{
		ADNFrameUtils::dot(worldDelta, frame.e1),
		ADNFrameUtils::dot(worldDelta, frame.e2),
		ADNFrameUtils::dot(worldDelta, frame.e3)
	};

}

[[nodiscard]] ADNFrameUtils::Vec3 fromLocalCoordinates(
	const ADNFrameUtils::Vec3& local,
	const ADNFrameUtils::Frame& frame) {

	return
		frame.e1 * local.x +
		frame.e2 * local.y +
		frame.e3 * local.z;

}

struct AtomPlacementMarkerScore {
	double center{ std::numeric_limits<double>::infinity() };
	double backbone{ std::numeric_limits<double>::infinity() };
	double sidechain{ std::numeric_limits<double>::infinity() };
	double total{ std::numeric_limits<double>::infinity() };
};

[[nodiscard]] ADNFrameUtils::Vec3 matrixRowToVec3(const ublas::matrix<double>& matrix,
	std::size_t row) {

	if (matrix.size1() <= row || matrix.size2() < 3)
		return ADNFrameUtils::Vec3{};

	return ADNFrameUtils::Vec3{
		matrix(row, 0),
		matrix(row, 1),
		matrix(row, 2)
	};

}

[[nodiscard]] ADNFrameUtils::Vec3 mapTemplatePointWithPairLevelTransform(
	const ublas::matrix<double>& pairBasisMatrix,
	const ublas::vector<double>& pairTranslation,
	const ADNFrameUtils::Vec3& templatePoint) {

	ublas::matrix<double> input(1, 3);
	ublas::row(input, 0) = frameVectorToUblas(templatePoint);
	ublas::matrix<double> output =
		ADNVectorMath::ApplyTransformation(pairBasisMatrix, input);
	output = ADNVectorMath::Translate(output, pairTranslation);
	return matrixRowToVec3(output, 0);

}

[[nodiscard]] AtomPlacementMarkerScore scoreTemplateMarkerMapping(
	SBPointer<ADNNucleotide> currentNucleotide,
	SBPointer<ADNNucleotide> templateNucleotide,
	const std::function<ADNFrameUtils::Vec3(const ADNFrameUtils::Vec3&)>& mapPoint) {

	AtomPlacementMarkerScore score;
	if (currentNucleotide == nullptr || templateNucleotide == nullptr)
		return score;

	score.center = ADNFrameUtils::norm(
		mapPoint(positionToVec3(templateNucleotide->GetPosition())) -
		positionToVec3(currentNucleotide->GetPosition()));
	score.backbone = ADNFrameUtils::norm(
		mapPoint(positionToVec3(templateNucleotide->GetBackbonePosition())) -
		positionToVec3(currentNucleotide->GetBackbonePosition()));
	score.sidechain = ADNFrameUtils::norm(
		mapPoint(positionToVec3(templateNucleotide->GetSidechainPosition())) -
		positionToVec3(currentNucleotide->GetSidechainPosition()));
	score.total =
		4.0 * score.center * score.center +
		score.backbone * score.backbone +
		score.sidechain * score.sidechain;
	return score;

}

[[nodiscard]] bool markerScoreSupportsPairLevelPlacement(
	const AtomPlacementMarkerScore& pairScore,
	const AtomPlacementMarkerScore& localScore) {

	constexpr double centerTolerancePm = 150.0;
	if (!std::isfinite(pairScore.total))
		return false;
	if (!std::isfinite(localScore.total))
		return true;

	return
		pairScore.total <= localScore.total * 1.25 ||
		pairScore.center <= centerTolerancePm;

}

[[nodiscard]] bool oneSidedPairLevelPlacementMatchesMarkers(
	SBPointer<ADNNucleotide> currentNucleotide,
	SBPointer<ADNNucleotide> templateNucleotide,
	const ublas::matrix<double>& pairBasisMatrix,
	const ublas::vector<double>& pairTranslation,
	const ADNFrameUtils::Frame& localFallbackFrame,
	AtomPlacementMarkerScore& pairScore,
	AtomPlacementMarkerScore& localScore) {

	const auto pairLevelMapPoint = [&](const ADNFrameUtils::Vec3& templatePoint) {

		return mapTemplatePointWithPairLevelTransform(
			pairBasisMatrix,
			pairTranslation,
			templatePoint);

	};
	pairScore = scoreTemplateMarkerMapping(
		currentNucleotide,
		templateNucleotide,
		pairLevelMapPoint);

	if (currentNucleotide == nullptr || templateNucleotide == nullptr)
		return markerScoreSupportsPairLevelPlacement(pairScore, localScore);

	const ADNFrameUtils::Frame currentFrame =
		nucleotidePlacementFrameFromCoarseGeometry(*currentNucleotide, localFallbackFrame);
	const ADNFrameUtils::Frame templateFrame =
		nucleotidePlacementFrameFromCoarseGeometry(
			*templateNucleotide(),
			ADNFrameAdapters::sanitizedFrame(*templateNucleotide()));
	const ADNFrameUtils::Vec3 worldCenter =
		positionToVec3(currentNucleotide->GetPosition());
	const ADNFrameUtils::Vec3 templateCenter =
		positionToVec3(templateNucleotide->GetPosition());

	const auto localMapPoint = [&](const ADNFrameUtils::Vec3& templatePoint) {

		const ADNFrameUtils::Vec3 templateDelta = templatePoint - templateCenter;
		const ADNFrameUtils::Vec3 templateLocal =
			toLocalCoordinates(templateDelta, templateFrame);
		return worldCenter + fromLocalCoordinates(templateLocal, currentFrame);

	};
	localScore = scoreTemplateMarkerMapping(
		currentNucleotide,
		templateNucleotide,
		localMapPoint);

	return markerScoreSupportsPairLevelPlacement(pairScore, localScore);

}

[[nodiscard]] ADNFrameUtils::Vec3 projectedPerpendicularToAxis(const ADNFrameUtils::Vec3& direction,
	const ADNFrameUtils::Vec3& axis) {

	if (ADNFrameUtils::isNearlyZero(direction) || ADNFrameUtils::isNearlyZero(axis))
		return direction;

	const ADNFrameUtils::Vec3 unitAxis = ADNFrameUtils::normalized(axis);
	return direction - unitAxis * ADNFrameUtils::dot(direction, unitAxis);

}

[[nodiscard]] ADNFrameUtils::Frame reconstructionFrameForBaseSegment(
	SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment == nullptr)
		return ADNFrameUtils::identityFrame();

	const ADNFrameUtils::Frame storedFrame =
		ADNFrameAdapters::frameFromOrientable(*baseSegment);
	if (ADNFrameUtils::isOrthonormalRightHanded(storedFrame))
		return storedFrame;

	ADNFrameUtils::Vec3 axis = storedFrame.e3;
	if (ADNFrameUtils::isNearlyZero(axis))
		axis = baseSegmentTangent(baseSegment);
	if (ADNFrameUtils::isNearlyZero(axis))
		axis = ADNFrameUtils::identityFrame().e3;

	const bool hasPreferredRadial = !ADNFrameUtils::isNearlyZero(storedFrame.e2);
	const ADNFrameUtils::Frame repairedFrame =
		ADNGeometrySynchronization::makeDesignedBaseSegmentFrame(
			axis,
			hasPreferredRadial ? &storedFrame.e2 : nullptr);
	// SetNucleotidePosition is a reconstruction boundary, not atom generation.
	// Persist the repair so creator output with only E3 initialized does not
	// repeatedly depend on sanitized arbitrary radial defaults.
	ADNFrameAdapters::setFrame(*baseSegment, repairedFrame);
	return repairedFrame;

}

[[nodiscard]] ADNFrameUtils::Frame oneSidedCanonicalTemplateFrame(
	SBPointer<ADNBaseSegment> baseSegment,
	SBPointer<ADNNucleotide> anchor) {

	if (baseSegment == nullptr || anchor == nullptr)
		return baseSegment != nullptr ?
			ADNGeometrySynchronization::canonicalTemplateFrameFromCurrentGeometry(*baseSegment) :
			ADNFrameUtils::identityFrame();

	const ADNFrameUtils::Frame fallback = ADNFrameAdapters::sanitizedFrame(*baseSegment);
	ADNFrameUtils::Vec3 axis = baseSegmentTangent(baseSegment);
	if (ADNFrameUtils::isNearlyZero(axis))
		axis = fallback.e3;
	if (ADNFrameUtils::isNearlyZero(axis))
		axis = ADNFrameUtils::identityFrame().e3;
	axis = ADNFrameUtils::normalized(axis);

	const bool anchorIsLeft = baseSegment->IsLeft(anchor);
	const bool anchorIsRight = baseSegment->IsRight(anchor);
	if (!anchorIsLeft && !anchorIsRight)
		return fallback;

	const double phaseRadians =
		ADNGeometrySynchronization::baseSegmentReconstructionPhaseRadians(*baseSegment);
	const ADNFrameUtils::Mat3 phaseRotation =
		ADNFrameUtils::rotationAroundAxis(axis, phaseRadians);
	const ADNFrameUtils::Mat3 undoPhaseRotation =
		ADNFrameUtils::rotationAroundAxis(axis, -phaseRadians);

	const auto currentDirectionFromCanonical = [&](const ADNFrameUtils::Vec3& direction) {

		return projectedPerpendicularToAxis(
			ADNFrameUtils::rotated(phaseRotation, direction),
			axis);

	};

	const auto canonicalDirectionFromCurrent = [&](const ADNFrameUtils::Vec3& direction) {

		return projectedPerpendicularToAxis(
			ADNFrameUtils::rotated(undoPhaseRotation, direction),
			axis);

	};

	const auto anchorSideMatches = [&](const ADNFrameUtils::Vec3& canonicalDirection) {

		const ADNFrameUtils::Vec3 currentDirection =
			currentDirectionFromCanonical(canonicalDirection);
		const ADNFrameUtils::Vec3 centerToAnchor =
			projectedPerpendicularToAxis(
				positionToVec3(anchor->GetPosition()) -
				positionToVec3(baseSegment->GetPosition()),
				axis);
		if (ADNFrameUtils::isNearlyZero(currentDirection) ||
			ADNFrameUtils::isNearlyZero(centerToAnchor))
			return true;

		const double sideSign = ADNFrameUtils::dot(currentDirection, centerToAnchor);
		return (anchorIsLeft && sideSign <= 0.0) ||
			(anchorIsRight && sideSign >= 0.0);

	};

	const ADNFrameUtils::Vec3 centerToAnchor =
		projectedPerpendicularToAxis(
			positionToVec3(anchor->GetPosition()) -
			positionToVec3(baseSegment->GetPosition()),
			axis);
	const ADNFrameUtils::Vec3 currentPairDirectionFromAnchor =
		anchorIsLeft ? -centerToAnchor : centerToAnchor;

	// One-sided complementary placement must use the current anchor nucleotide
	// geometry as the authoritative source of the pair direction. The
	// persistent base-segment frame of a one-sided segment may come from the
	// nucleotide-local sidechain/backbone frame after synchronization,
	// especially after SAMSON move or Rotate DNA. That frame is useful for
	// persistence and editing, but it must not override center-to-anchor
	// geometry when reconstructing the missing complementary nucleotide.
	ADNFrameUtils::Vec3 e2;
	if (!ADNFrameUtils::isNearlyZero(currentPairDirectionFromAnchor))
		e2 = canonicalDirectionFromCurrent(currentPairDirectionFromAnchor);

	if (ADNFrameUtils::isNearlyZero(e2)) {

		const ADNFrameUtils::Frame storedFrame =
			ADNFrameAdapters::frameFromOrientable(*baseSegment);
		if (ADNFrameUtils::isOrthonormalRightHanded(storedFrame)) {

			e2 = projectedPerpendicularToAxis(storedFrame.e2, axis);
			if (!ADNFrameUtils::isNearlyZero(e2) && !anchorSideMatches(e2))
				e2 = -e2;

		}

	}

	// This helper reconstructs a phase-neutral base-segment template frame from
	// one existing nucleotide. The canonical e2 axis is the left-to-right
	// base-pair direction. Do not use sidechain - backbone directly as this
	// axis: that vector is nucleotide-local and usually points between the
	// outer backbone and the inner base. Using it as the pair direction places
	// the complementary nucleotide on the wrong side.
	if (ADNFrameUtils::isNearlyZero(e2)) {

		ADNFrameUtils::Vec3 localRadial =
			positionToVec3(anchor->GetSidechainPosition()) -
			positionToVec3(anchor->GetBackbonePosition());
		localRadial = projectedPerpendicularToAxis(localRadial, axis);
		if (!ADNFrameUtils::isNearlyZero(localRadial)) {

			ADNFrameUtils::Vec3 currentPairDirection =
				anchorIsLeft ? localRadial : -localRadial;
			if (!ADNFrameUtils::isNearlyZero(centerToAnchor)) {

				const double sideSign =
					ADNFrameUtils::dot(currentPairDirection, centerToAnchor);
				if ((anchorIsLeft && sideSign > 0.0) ||
					(anchorIsRight && sideSign < 0.0))
					currentPairDirection = -currentPairDirection;

			}
			e2 = canonicalDirectionFromCurrent(currentPairDirection);

		}

	}

	if (ADNFrameUtils::isNearlyZero(e2))
		e2 = projectedPerpendicularToAxis(fallback.e2, axis);
	if (ADNFrameUtils::isNearlyZero(e2))
		return fallback;

	if (!anchorSideMatches(e2))
		e2 = -e2;

	return ADNFrameUtils::frameFromE2AndTangent(e2, axis, fallback);

}

#ifndef NDEBUG
void logInvalidBaseSegmentFrame(const char* context, SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment != nullptr &&
		!ADNGeometrySynchronization::validateBaseSegmentGeometry(*baseSegment))
		ADNLogger::LogDebug(std::string(context) +
			": reconstructing positions from a stale base-segment frame.");

}

[[nodiscard]] ADNFrameUtils::Vec3 positionToFrameVec3(const SBPosition3& position) {

	return ADNFrameUtils::Vec3{
		position[0].getValue(),
		position[1].getValue(),
		position[2].getValue()
	};

}

[[nodiscard]] ADNFrameUtils::Vec3 sidechainPlaneNormal(SBPointer<ADNNucleotide> nucleotide) {

	if (nucleotide == nullptr) return ADNFrameUtils::Vec3{};

	std::vector<ADNFrameUtils::Vec3> points;
	auto atoms = nucleotide->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> atom, atoms) {

		if (atom != nullptr && !atom->IsInADNBackbone())
			points.push_back(positionToFrameVec3(atom->getPosition()));

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

[[nodiscard]] double frameDeterminant(SBPointer<ADNNucleotide> nucleotide) {

	if (nucleotide == nullptr) return 0.0;
	return ADNFrameUtils::determinant(ADNFrameAdapters::frameFromOrientable(*nucleotide));

}

[[nodiscard]] ADNFrameUtils::Vec3 localBaseSegmentAxis(SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment == nullptr) return ADNFrameUtils::Vec3{};

	SBPointer<ADNBaseSegment> previous = baseSegment->GetPrev(true);
	SBPointer<ADNBaseSegment> next = baseSegment->GetNext(true);

	ADNFrameUtils::Vec3 axis{};
	if (previous != nullptr && next != nullptr)
		axis = positionToFrameVec3(next->GetPosition()) - positionToFrameVec3(previous->GetPosition());
	else if (next != nullptr)
		axis = positionToFrameVec3(next->GetPosition()) - positionToFrameVec3(baseSegment->GetPosition());
	else if (previous != nullptr)
		axis = positionToFrameVec3(baseSegment->GetPosition()) - positionToFrameVec3(previous->GetPosition());

	if (ADNFrameUtils::isNearlyZero(axis))
		axis = ADNFrameAdapters::sanitizedFrame(*baseSegment).e3;

	return ADNFrameUtils::normalized(axis);

}

#ifdef ADN_DEBUG_GEOMETRY
[[nodiscard]] double angularErrorDegrees(const ADNFrameUtils::Vec3& a,
	const ADNFrameUtils::Vec3& b) {

	if (ADNFrameUtils::isNearlyZero(a) || ADNFrameUtils::isNearlyZero(b))
		return 0.0;

	const double dot =
		ADNFrameUtils::dot(ADNFrameUtils::normalized(a), ADNFrameUtils::normalized(b));
	const double clamped = std::max(-1.0, std::min(1.0, dot));
	return std::acos(clamped) * 180.0 / 3.141592653589793238462643383279502884;

}

[[nodiscard]] ADNFrameUtils::Vec3 outputRowToVec3(const ublas::matrix<double>& output,
	std::size_t row) {

	if (output.size1() <= row || output.size2() < 3)
		return ADNFrameUtils::Vec3{};

	return ADNFrameUtils::Vec3{
		output(row, 0),
		output(row, 1),
		output(row, 2)
	};

}

[[nodiscard]] std::string formatVec3(const ADNFrameUtils::Vec3& vector) {

	std::ostringstream stream;
	stream << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
	return stream.str();

}

void logBaseSegmentGeometryDiagnostic(const char* context, SBPointer<ADNBaseSegment> baseSegment) {

	if (baseSegment == nullptr) return;
	const int baseSegmentNumber = baseSegment->GetNumber();
	if (baseSegmentNumber < 0 || baseSegmentNumber >= 5) return;

	const ADNFrameUtils::Frame frame = ADNFrameAdapters::frameFromOrientable(*baseSegment);
	const ADNFrameUtils::Vec3 axis = localBaseSegmentAxis(baseSegment);

	std::ostringstream message;
	message << context << " base segment " << baseSegmentNumber
		<< " |E1|=" << ADNFrameUtils::norm(frame.e1)
		<< " |E2|=" << ADNFrameUtils::norm(frame.e2)
		<< " |E3|=" << ADNFrameUtils::norm(frame.e3)
		<< " dot12=" << ADNFrameUtils::dot(frame.e1, frame.e2)
		<< " dot13=" << ADNFrameUtils::dot(frame.e1, frame.e3)
		<< " dot23=" << ADNFrameUtils::dot(frame.e2, frame.e3)
		<< " det=" << ADNFrameUtils::determinant(frame)
		<< " valid=" << ADNFrameUtils::isOrthonormalRightHanded(frame)
		<< " axis=(" << axis.x << ", " << axis.y << ", " << axis.z << ")";

	SBPointer<ADNCell> cell = baseSegment->GetCell();
	if (cell != nullptr && cell->GetCellType() == CellType::BasePair) {

		SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(cell());
		SBPointer<ADNNucleotide> left = basePair->GetLeftNucleotide();
		SBPointer<ADNNucleotide> right = basePair->GetRightNucleotide();

		const auto logNucleotideDegeneracy = [&message](const char* side, SBPointer<ADNNucleotide> nucleotide) {

			if (nucleotide == nullptr) return;
			const ADNFrameUtils::Vec3 center = positionToFrameVec3(nucleotide->GetPosition());
			const ADNFrameUtils::Vec3 backbone = positionToFrameVec3(nucleotide->GetBackbonePosition());
			const ADNFrameUtils::Vec3 sidechain = positionToFrameVec3(nucleotide->GetSidechainPosition());
			const bool degenerate =
				ADNFrameUtils::norm(center - backbone) < 1.0e-6 &&
				ADNFrameUtils::norm(center - sidechain) < 1.0e-6;
			message << " " << side << "Degenerate=" << degenerate;

		};

		logNucleotideDegeneracy("left", left);
		logNucleotideDegeneracy("right", right);

		const ADNFrameUtils::Vec3 leftNormal = sidechainPlaneNormal(left);
		const ADNFrameUtils::Vec3 rightNormal = sidechainPlaneNormal(right);
		if (!ADNFrameUtils::isNearlyZero(leftNormal) && !ADNFrameUtils::isNearlyZero(axis))
			message << " leftPlaneAxisDot=" << std::abs(ADNFrameUtils::dot(leftNormal, axis));
		if (!ADNFrameUtils::isNearlyZero(rightNormal) && !ADNFrameUtils::isNearlyZero(axis))
			message << " rightPlaneAxisDot=" << std::abs(ADNFrameUtils::dot(rightNormal, axis));

	}

	ADNLogger::LogDebug(message.str());

}

void logOneSidedPlacementDiagnostic(const char* context,
	SBPointer<ADNBaseSegment> baseSegment,
	SBPointer<ADNNucleotide> anchor,
	SBPointer<ADNNucleotide> target,
	const ADNFrameUtils::Frame& canonicalFrame,
	const TemplateToWorldTransform& transform,
	const ublas::matrix<double>& output) {

	if (baseSegment == nullptr || anchor == nullptr || target == nullptr) return;
	const int baseSegmentNumber = baseSegment->GetNumber();
	if (baseSegmentNumber < 0 || baseSegmentNumber >= 5) return;

	const ADNFrameUtils::Vec3 axis = ADNFrameUtils::normalized(canonicalFrame.e3);
	const ADNFrameUtils::Vec3 center = positionToFrameVec3(baseSegment->GetPosition());
	const ADNFrameUtils::Vec3 anchorCenter = positionToFrameVec3(anchor->GetPosition());
	const ADNFrameUtils::Vec3 anchorBackbone = positionToFrameVec3(anchor->GetBackbonePosition());
	const ADNFrameUtils::Vec3 anchorSidechain = positionToFrameVec3(anchor->GetSidechainPosition());
	const ADNFrameUtils::Vec3 targetCenter = outputRowToVec3(output, 0);
	const ADNFrameUtils::Vec3 targetBackbone = outputRowToVec3(output, 1);
	const ADNFrameUtils::Vec3 targetSidechain = outputRowToVec3(output, 2);

	const bool anchorIsLeft = baseSegment->IsLeft(anchor);
	const bool targetIsLeft = baseSegment->IsLeft(target);
	const ADNFrameUtils::Vec3 centerToAnchor =
		projectedPerpendicularToAxis(anchorCenter - center, axis);
	const ADNFrameUtils::Vec3 currentPairDirectionFromAnchor =
		anchorIsLeft ? -centerToAnchor : centerToAnchor;

	const ADNFrameUtils::Frame storedFrame =
		ADNFrameAdapters::frameFromOrientable(*baseSegment);
	const ADNFrameUtils::Mat3 phaseRotation =
		ADNFrameUtils::rotationAroundAxis(axis, transform.phaseRadians);
	const ADNFrameUtils::Vec3 storedCandidate =
		projectedPerpendicularToAxis(
			ADNFrameUtils::rotated(phaseRotation, storedFrame.e2),
			axis);
	const double storedAnchorDot =
		ADNFrameUtils::isNearlyZero(storedCandidate) ||
		ADNFrameUtils::isNearlyZero(currentPairDirectionFromAnchor) ?
		0.0 :
		ADNFrameUtils::dot(
			ADNFrameUtils::normalized(storedCandidate),
			ADNFrameUtils::normalized(currentPairDirectionFromAnchor));

	const ADNFrameUtils::Vec3 anchorRadial =
		projectedPerpendicularToAxis(anchorCenter - center, axis);
	const ADNFrameUtils::Vec3 targetRadial =
		projectedPerpendicularToAxis(targetCenter - center, axis);
	const double radialDot =
		ADNFrameUtils::isNearlyZero(anchorRadial) ||
		ADNFrameUtils::isNearlyZero(targetRadial) ?
		0.0 :
		ADNFrameUtils::dot(
			ADNFrameUtils::normalized(targetRadial),
			ADNFrameUtils::normalized(anchorRadial));

	std::ostringstream message;
	message << context << " base segment " << baseSegmentNumber
		<< " anchorSide=" << (anchorIsLeft ? "left" : "right")
		<< " targetSide=" << (targetIsLeft ? "left" : "right")
		<< " axis=" << formatVec3(axis)
		<< " center=" << formatVec3(center)
		<< " anchorCenter=" << formatVec3(anchorCenter)
		<< " anchorBackbone=" << formatVec3(anchorBackbone)
		<< " anchorSidechain=" << formatVec3(anchorSidechain)
		<< " targetCenter=" << formatVec3(targetCenter)
		<< " targetBackbone=" << formatVec3(targetBackbone)
		<< " targetSidechain=" << formatVec3(targetSidechain)
		<< " storedE2=" << formatVec3(storedFrame.e2)
		<< " centerToAnchor=" << formatVec3(centerToAnchor)
		<< " anchorPairDirection=" << formatVec3(currentPairDirectionFromAnchor)
		<< " storedCandidate=" << formatVec3(storedCandidate)
		<< " storedAnchorDot=" << storedAnchorDot
		<< " storedAnchorAngleDeg="
		<< angularErrorDegrees(storedCandidate, currentPairDirectionFromAnchor)
		<< " targetAnchorRadialDot=" << radialDot;

	ADNLogger::LogDebug(message.str());

}

[[nodiscard]] double averageAtomGroupDistanceToMarker(SBPointer<ADNNucleotide> nucleotide,
	bool backboneGroup,
	const ADNFrameUtils::Vec3& marker,
	std::size_t& count) {

	count = 0;
	double sum = 0.0;

	if (nucleotide == nullptr) return 0.0;
	auto atoms = nucleotide->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> atom, atoms) {

		if (atom == nullptr) continue;
		if (atom->IsInADNBackbone() != backboneGroup) continue;

		sum += ADNFrameUtils::norm(positionToFrameVec3(atom->getPosition()) - marker);
		++count;

	}

	return count == 0 ? 0.0 : sum / static_cast<double>(count);

}

void logUnpairedAtomPlacementDiagnostic(SBPointer<ADNNucleotide> nucleotide,
	SBPointer<ADNNucleotide> templateNucleotide,
	const ADNFrameUtils::Frame& currentFrame,
	const ADNFrameUtils::Frame& templateFrame) {

	static int loggedCount = 0;
	if (loggedCount >= 5) return;
	if (nucleotide == nullptr || templateNucleotide == nullptr) return;

	const ADNFrameUtils::Vec3 currentCenter = positionToFrameVec3(nucleotide->GetPosition());
	const ADNFrameUtils::Vec3 currentBackbone = positionToFrameVec3(nucleotide->GetBackbonePosition());
	const ADNFrameUtils::Vec3 currentSidechain = positionToFrameVec3(nucleotide->GetSidechainPosition());
	const ADNFrameUtils::Vec3 templateCenter = positionToFrameVec3(templateNucleotide->GetPosition());
	const ADNFrameUtils::Vec3 templateBackbone = positionToFrameVec3(templateNucleotide->GetBackbonePosition());
	const ADNFrameUtils::Vec3 templateSidechain = positionToFrameVec3(templateNucleotide->GetSidechainPosition());
	const ADNFrameUtils::Vec3 currentBackboneToSidechain = currentSidechain - currentBackbone;
	const ADNFrameUtils::Vec3 templateBackboneToSidechain = templateSidechain - templateBackbone;

	std::size_t backboneToBackboneCount = 0;
	std::size_t backboneToSidechainCount = 0;
	std::size_t sidechainToBackboneCount = 0;
	std::size_t sidechainToSidechainCount = 0;
	const double backboneToBackbone =
		averageAtomGroupDistanceToMarker(nucleotide, true, currentBackbone, backboneToBackboneCount);
	const double backboneToSidechain =
		averageAtomGroupDistanceToMarker(nucleotide, true, currentSidechain, backboneToSidechainCount);
	const double sidechainToBackbone =
		averageAtomGroupDistanceToMarker(nucleotide, false, currentBackbone, sidechainToBackboneCount);
	const double sidechainToSidechain =
		averageAtomGroupDistanceToMarker(nucleotide, false, currentSidechain, sidechainToSidechainCount);

	std::ostringstream message;
	message << "Unpaired atom placement nucleotide " << nucleotide->getName()
		<< " center=" << formatVec3(currentCenter)
		<< " backbone=" << formatVec3(currentBackbone)
		<< " sidechain=" << formatVec3(currentSidechain)
		<< " templateCenter=" << formatVec3(templateCenter)
		<< " templateBackbone=" << formatVec3(templateBackbone)
		<< " templateSidechain=" << formatVec3(templateSidechain)
		<< " currentE2=" << formatVec3(currentFrame.e2)
		<< " templateE2=" << formatVec3(templateFrame.e2)
		<< " currentE2MarkerDot=" << ADNFrameUtils::dot(
			ADNFrameUtils::normalized(currentFrame.e2),
			ADNFrameUtils::normalized(currentBackboneToSidechain))
		<< " templateE2MarkerDot=" << ADNFrameUtils::dot(
			ADNFrameUtils::normalized(templateFrame.e2),
			ADNFrameUtils::normalized(templateBackboneToSidechain))
		<< " backboneAtoms=" << backboneToBackboneCount
		<< " backboneAvgToBackbone=" << backboneToBackbone
		<< " backboneAvgToSidechain=" << backboneToSidechain
		<< " sidechainAtoms=" << sidechainToSidechainCount
		<< " sidechainAvgToBackbone=" << sidechainToBackbone
		<< " sidechainAvgToSidechain=" << sidechainToSidechain;

	ADNLogger::LogDebug(message.str());
	++loggedCount;

}
#endif

[[nodiscard]] SBPointer<ADNAtom> firstAtomByName(SBPointer<ADNNucleotide> nucleotide,
	const std::string& atomName) {

	if (nucleotide == nullptr) return nullptr;
	auto atoms = nucleotide->GetAtomsByName(atomName);
	if (atoms.size() == 0) return nullptr;
	return *atoms.begin();

}

void logOneSidedPairLevelMarkerFallback(SBPointer<ADNBaseSegment> baseSegment,
	SBPointer<ADNNucleotide> nucleotide,
	const AtomPlacementMarkerScore& pairScore,
	const AtomPlacementMarkerScore& localScore) {

	const int baseSegmentNumber = baseSegment != nullptr ? baseSegment->GetNumber() : -1;
	std::ostringstream message;
	message << "FindAtomsPositions one-sided pair-level placement rejected by coarse markers"
		<< " baseSegment=" << baseSegmentNumber
		<< " nucleotide=" << (nucleotide != nullptr ? nucleotide->getName() : std::string("<null>"))
		<< " pairScore(center/backbone/sidechain/total)="
		<< pairScore.center << "/" << pairScore.backbone << "/"
		<< pairScore.sidechain << "/" << pairScore.total
		<< " localScore(center/backbone/sidechain/total)="
		<< localScore.center << "/" << localScore.backbone << "/"
		<< localScore.sidechain << "/" << localScore.total;
	ADNLogger::LogDebug(message.str());

}

#ifdef ADN_DEBUG_GEOMETRY
void logOneSidedLocalFallbackDiagnostic(SBPointer<ADNBaseSegment> baseSegment,
	SBPointer<ADNNucleotide> nucleotide,
	ADNGeometrySynchronization::TemplateSide side,
	bool hasLeft,
	bool hasRight,
	bool pairBasisInitialized,
	bool pairTranslationInitialized) {

	static int loggedCount = 0;
	if (loggedCount >= 20) return;

	const int baseSegmentNumber = baseSegment != nullptr ? baseSegment->GetNumber() : -1;
	std::ostringstream message;
	message << "FindAtomsPositions one-sided base segment is using local fallback"
		<< " baseSegment=" << baseSegmentNumber
		<< " nucleotide=" << (nucleotide != nullptr ? nucleotide->getName() : std::string("<null>"))
		<< " side=" << (side == ADNGeometrySynchronization::TemplateSide::Right ? "right" : "left")
		<< " hasLeft=" << hasLeft
		<< " hasRight=" << hasRight
		<< " pairBasisInitialized=" << pairBasisInitialized
		<< " pairTranslationInitialized=" << pairTranslationInitialized;
	ADNLogger::LogDebug(message.str());
	++loggedCount;

}

void logOneSidedLocalFallbackGeometryDiagnostic(SBPointer<ADNBaseSegment> baseSegment,
	SBPointer<ADNNucleotide> nucleotide) {

	static int loggedCount = 0;
	if (loggedCount >= 20) return;
	if (nucleotide == nullptr) return;

	const ADNFrameUtils::Vec3 normal = sidechainPlaneNormal(nucleotide);
	SBPointer<ADNNucleotide> previous = nucleotide->GetPrev(true);
	const ADNFrameUtils::Vec3 previousNormal = sidechainPlaneNormal(previous);
	const bool hasNormalAlignment =
		!ADNFrameUtils::isNearlyZero(normal) &&
		!ADNFrameUtils::isNearlyZero(previousNormal);
	const double normalAlignment = hasNormalAlignment ?
		std::abs(ADNFrameUtils::dot(normal, previousNormal)) :
		0.0;

	SBPointer<ADNAtom> phosphate = firstAtomByName(nucleotide, "P");
	SBPointer<ADNAtom> previousO3 = firstAtomByName(previous, "O3'");
	const bool hasBackboneLink = phosphate != nullptr && previousO3 != nullptr;
	const double backboneLinkDistance = hasBackboneLink ?
		(phosphate->getPosition() - previousO3->getPosition()).norm().getValue() :
		0.0;

	const int baseSegmentNumber = baseSegment != nullptr ? baseSegment->GetNumber() : -1;
	std::ostringstream message;
	message << "FindAtomsPositions one-sided local fallback geometry"
		<< " baseSegment=" << baseSegmentNumber
		<< " nucleotide=" << nucleotide->getName()
		<< " ringNormal=" << formatVec3(normal)
		<< " previousRingNormal=" << formatVec3(previousNormal)
		<< " hasPreviousNormal=" << hasNormalAlignment
		<< " normalAbsDot=" << normalAlignment
		<< " hasPToPreviousO3=" << hasBackboneLink
		<< " pToPreviousO3DistancePm=" << backboneLinkDistance;
	ADNLogger::LogDebug(message.str());
	++loggedCount;

}
#endif

[[nodiscard]] bool validateGeneratedBackboneLink(SBPointer<ADNBaseSegment> baseSegment,
	SBPointer<ADNNucleotide> nucleotide) {

	if (nucleotide == nullptr) return true;

	SBPointer<ADNNucleotide> previous = nucleotide->GetPrev(true);
	if (previous == nullptr) return true;

	SBPointer<ADNAtom> phosphate = firstAtomByName(nucleotide, "P");
	SBPointer<ADNAtom> previousO3 = firstAtomByName(previous, "O3'");
	if (phosphate == nullptr || previousO3 == nullptr) return true;

	const double distance = (phosphate->getPosition() - previousO3->getPosition()).norm().getValue();
	constexpr double minPToO3DistancePm = 100.0;
	constexpr double maxPToO3DistancePm = 250.0;
	if (distance >= minPToO3DistancePm && distance <= maxPToO3DistancePm)
		return true;

	const int baseSegmentNumber = baseSegment != nullptr ? baseSegment->GetNumber() : -1;
	ADNLogger::LogDebug("Generated backbone diagnostic failed for base segment " +
		std::to_string(baseSegmentNumber) + " nucleotide " + nucleotide->getName() +
		": P to previous O3' distance " + std::to_string(distance) + " pm.");
	return false;

}

[[nodiscard]] double clampedUnit(double value) {

	if (value < -1.0) return -1.0;
	if (value > 1.0) return 1.0;
	return value;

}
#endif

} // namespace

DASBackToTheAtom::DASBackToTheAtom() {

	//LoadNucleotides();
	LoadNtPairs();

}

DASBackToTheAtom::~DASBackToTheAtom() {

	da_.deleteReferenceTarget();
	dc_.deleteReferenceTarget();
	dg_.deleteReferenceTarget();
	dt_.deleteReferenceTarget();
	da_dt_.first.deleteReferenceTarget();
	da_dt_.second.deleteReferenceTarget();
	dt_da_.first.deleteReferenceTarget();
	dt_da_.second.deleteReferenceTarget();
	dc_dg_.first.deleteReferenceTarget();
	dc_dg_.second.deleteReferenceTarget();
	dg_dc_.first.deleteReferenceTarget();
	dg_dc_.second.deleteReferenceTarget();

}

void DASBackToTheAtom::SetDoubleStrandPositions(SBPointer<ADNDoubleStrand> ds) {

	if (ds == nullptr) return;

	SBPointer<ADNBaseSegment> bs = ds->GetFirstBaseSegment();
	std::vector<SBPointer<ADNBaseSegment>> loops;

	for (size_t i = 0; i < ds->GetLength(); ++i) {

		if (bs == nullptr) {

			std::string msg = "SetDoubleStrandPositions() ERROR => nullptr BaseSegment on position " + std::to_string(i)
				+ " of doubleStrand " + std::to_string(ds->getNodeIndex()) + "(size " + std::to_string(ds->GetLength()) + ")";
			ADNLogger::LogError(msg);
			break;

		}
		const int num = bs->GetNumber();
		SBPointer<ADNCell> cell = bs->GetCell();
		if (cell->GetCellType() == CellType::BasePair) {

			SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(cell());
			SBPointer<ADNNucleotide> left = bp->GetLeftNucleotide();
			SBPointer<ADNNucleotide> right = bp->GetRightNucleotide();
			bool paired = (left != nullptr && right != nullptr);
			SetNucleotidePosition(bs, paired);

		}
		else if (cell->GetCellType() == CellType::LoopPair) {

			loops.push_back(bs);

		}
		bs = bs->GetNext();
		if (bs != nullptr && abs(bs->GetNumber() - num) != 1) {

			std::string msg = "Consecutive base segments have non-consecutive numbers.";
			ADNLogger::LogDebug(msg);

		}

	}

	// Calculate last the loops so we can use already inserted positions
	for (SBPointer<ADNBaseSegment> bs : loops) {
		SetPositionLoopNucleotides(bs);
	}

}

void DASBackToTheAtom::SetNucleotidePosition(SBPointer<ADNBaseSegment> bs, bool set_pair) {

	if (bs == nullptr) return;

#ifndef NDEBUG
	logInvalidBaseSegmentFrame(__func__, bs);
#endif
#if defined(ADN_DEBUG_GEOMETRY) && !defined(NDEBUG)
	logBaseSegmentGeometryDiagnostic("SetNucleotidePosition before", bs);
#endif

	SBPointer<ADNNucleotide> nt_left = nullptr;
	SBPointer<ADNNucleotide> nt_right = nullptr;
	SBPointer<ADNCell> cell = bs->GetCell();
	if (cell->GetCellType() != CellType::BasePair) return;

	SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(cell());
	SBPointer<ADNNucleotide> nt_l = bp->GetLeftNucleotide();
	SBPointer<ADNNucleotide> nt_r = bp->GetRightNucleotide();

	if (nt_r == nullptr && nt_l == nullptr) return;

	NtPair pair = GetIdealBasePairNucleotides(nt_l, nt_r);
	nt_left = pair.first;
	nt_right = pair.second;

	// Place c.o.m. at bs position
	ublas::vector<double> sys_cm = ADNAuxiliary::SBPositionToUblas(bs->GetPosition());
	ublas::vector<double> t_vec = sys_cm - GetIdealPairCenterOfMass(pair);
	ublas::matrix<double> input = ublas::matrix<double>(6, 3);

	ublas::row(input, 0) = ADNAuxiliary::SBPositionToUblas(nt_left->GetPosition());
	ublas::row(input, 1) = ADNAuxiliary::SBPositionToUblas(nt_left->GetBackbonePosition());
	ublas::row(input, 2) = ADNAuxiliary::SBPositionToUblas(nt_left->GetSidechainPosition());
	ublas::row(input, 3) = ADNAuxiliary::SBPositionToUblas(nt_right->GetPosition());
	ublas::row(input, 4) = ADNAuxiliary::SBPositionToUblas(nt_right->GetBackbonePosition());
	ublas::row(input, 5) = ADNAuxiliary::SBPositionToUblas(nt_right->GetSidechainPosition());

	const TemplateToWorldTransform transform =
		makeTemplateToWorldTransform(*bs, reconstructionFrameForBaseSegment(bs));
	ublas::matrix<double> new_pos = ADNVectorMath::ApplyTransformation(transform.basisMatrix, input);
	new_pos = ADNVectorMath::Translate(new_pos, t_vec);

	if (nt_l != nullptr) {

		// Coarse reconstruction and atom placement share this side-frame
		// convention; FindAtomsPositions later consumes the nucleotide frame directly.
		ADNFrameAdapters::setFrame(*nt_l, transform.leftFrame);

#if defined(ADN_DEBUG_GEOMETRY) && !defined(NDEBUG)
		if (nt_l->GetStrand()->IsScaffold()) {

			std::string msg = "Left: " + std::to_string(nt_l->GetE3()[0]) + " " + std::to_string(nt_l->GetE3()[1]) + " " + std::to_string(nt_l->GetE3()[2]);
			ADNLogger::LogDebug(msg);

		}
#endif

		// Set new residue positions
		SBPosition3 p_left = UblasToSBPosition(ublas::row(new_pos, 0));
		nt_l->SetPosition(p_left);
		SBPosition3 p_bb_left = UblasToSBPosition(ublas::row(new_pos, 1));
		nt_l->SetBackbonePosition(p_bb_left);
		SBPosition3 p_sc_left = UblasToSBPosition(ublas::row(new_pos, 2));
		nt_l->SetSidechainPosition(p_sc_left);

	}

	if (nt_r != nullptr) {

		// Right-side nucleotides use the complementary side convention of the
		// same base-pair plane. Keep this centralized for coarse and atomic models.
		ADNFrameAdapters::setFrame(*nt_r, transform.rightFrame);

#if defined(ADN_DEBUG_GEOMETRY) && !defined(NDEBUG)
		if (nt_r->GetStrand()->IsScaffold()) {

			std::string msg = "Right: " + std::to_string(nt_r->GetE3()[0]) + " " + std::to_string(nt_r->GetE3()[1]) + " " + std::to_string(nt_r->GetE3()[2]);
			ADNLogger::LogDebug(msg);

		}
#endif

		// Set positions
		SBPosition3 p_right = UblasToSBPosition(ublas::row(new_pos, 3));
		nt_r->SetPosition(p_right);
		SBPosition3 p_bb_right = UblasToSBPosition(ublas::row(new_pos, 4));
		nt_r->SetBackbonePosition(p_bb_right);
		SBPosition3 p_sc_right = UblasToSBPosition(ublas::row(new_pos, 5));
		nt_r->SetSidechainPosition(p_sc_right);

	}

#if defined(ADN_DEBUG_GEOMETRY) && !defined(NDEBUG)
	logBaseSegmentGeometryDiagnostic("SetNucleotidePosition after", bs);
#endif

}

void DASBackToTheAtom::PlaceNucleotideFromTemplate(SBPointer<ADNBaseSegment> bs, SBPointer<ADNNucleotide> nt) {

	if (bs == nullptr || nt == nullptr) return;

	SBPointer<ADNCell> cell = bs->GetCell();
	if (cell == nullptr || cell->GetCellType() != CellType::BasePair) return;

	SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(cell());
	SBPointer<ADNNucleotide> left = basePair->GetLeftNucleotide();
	SBPointer<ADNNucleotide> right = basePair->GetRightNucleotide();
	if (left == nullptr && right == nullptr) return;

	const NtPair pair = GetIdealBasePairNucleotides(left, right);
	const bool placeRight = bs->IsRight(nt);
	const bool placeLeft = bs->IsLeft(nt);
	if (!placeLeft && !placeRight) return;
	SBPointer<ADNNucleotide> templateNucleotide = placeRight ? pair.second : pair.first;
	if (templateNucleotide == nullptr) return;

	// This path is used by Create Base Pair. It preserves the existing anchor
	// nucleotide and places only the newly-created complementary nucleotide. For
	// moved or rotated single strands, the placement frame is derived from the
	// actual anchor geometry instead of a generic synchronized one-sided frame.
	// Ideal pair coordinates use the pair-level transform; side-specific frames
	// are provisional until the new strand batch is synchronized from the
	// placed center/backbone/sidechain geometry.
	ADNFrameUtils::Frame canonicalFrame =
		oneSidedCanonicalTemplateFrame(bs, nt->GetPair());

	ublas::vector<double> translation =
		ADNAuxiliary::SBPositionToUblas(bs->GetPosition()) -
		GetIdealPairCenterOfMass(pair);

	ublas::matrix<double> input(3, 3);
	ublas::row(input, 0) = ADNAuxiliary::SBPositionToUblas(templateNucleotide->GetPosition());
	ublas::row(input, 1) = ADNAuxiliary::SBPositionToUblas(templateNucleotide->GetBackbonePosition());
	ublas::row(input, 2) = ADNAuxiliary::SBPositionToUblas(templateNucleotide->GetSidechainPosition());

	TemplateToWorldTransform transform;
	ublas::matrix<double> output;
	const auto applyPlacement = [&]() {

		transform = makeTemplateToWorldTransform(*bs, canonicalFrame);
		output = ADNVectorMath::ApplyTransformation(transform.basisMatrix, input);
		output = ADNVectorMath::Translate(output, translation);

	};

	const auto outputPosition = [&]() {

		return ADNFrameUtils::Vec3{
			output(0, 0),
			output(0, 1),
			output(0, 2)
		};

	};

	const auto placementIsOnOppositeSide = [&]() {

		SBPointer<ADNNucleotide> anchor = nt->GetPair();
		if (anchor == nullptr) return true;

		const ADNFrameUtils::Vec3 center = positionToVec3(bs->GetPosition());
		const ADNFrameUtils::Vec3 axis =
			ADNFrameUtils::normalized(canonicalFrame.e3);
		const ADNFrameUtils::Vec3 pairDirection =
			projectedPerpendicularToAxis(
				ADNFrameUtils::rotated(
					ADNFrameUtils::rotationAroundAxis(axis, transform.phaseRadians),
					canonicalFrame.e2),
				axis);
		if (ADNFrameUtils::isNearlyZero(pairDirection)) return true;

		const ADNFrameUtils::Vec3 anchorRadial =
			projectedPerpendicularToAxis(positionToVec3(anchor->GetPosition()) - center, axis);
		const ADNFrameUtils::Vec3 targetRadial =
			projectedPerpendicularToAxis(outputPosition() - center, axis);
		if (ADNFrameUtils::isNearlyZero(anchorRadial)) return true;
		if (ADNFrameUtils::isNearlyZero(targetRadial)) return false;

		const double anchorSign = ADNFrameUtils::dot(anchorRadial, pairDirection);
		const double targetSign = ADNFrameUtils::dot(targetRadial, pairDirection);
		if (placeRight)
			return anchorSign < 0.0 && targetSign > 0.0;
		return anchorSign > 0.0 && targetSign < 0.0;

	};

	applyPlacement();
#ifdef ADN_DEBUG_GEOMETRY
	logOneSidedPlacementDiagnostic(
		"PlaceNucleotideFromTemplate initial",
		bs,
		nt->GetPair(),
		nt,
		canonicalFrame,
		transform,
		output);
#endif
	if (!placementIsOnOppositeSide()) {

		canonicalFrame = ADNFrameUtils::frameFromE2AndTangent(
			-canonicalFrame.e2,
			canonicalFrame.e3,
			canonicalFrame);
		applyPlacement();
#ifdef ADN_DEBUG_GEOMETRY
		logOneSidedPlacementDiagnostic(
			"PlaceNucleotideFromTemplate retry",
			bs,
			nt->GetPair(),
			nt,
			canonicalFrame,
			transform,
			output);
#endif
#ifndef NDEBUG
		if (!placementIsOnOppositeSide())
			ADNLogger::LogDebug(std::string("PlaceNucleotideFromTemplate: complementary nucleotide remains on the anchor side after pair-direction retry."));
#endif

	}

	nt->SetPosition(UblasToSBPosition(ublas::row(output, 0)));
	nt->SetBackbonePosition(UblasToSBPosition(ublas::row(output, 1)));
	nt->SetSidechainPosition(UblasToSBPosition(ublas::row(output, 2)));
	ADNFrameAdapters::setFrame(*nt, placeRight ? transform.rightFrame : transform.leftFrame);

}

void DASBackToTheAtom::SetPositionsForNewNucleotides(SBPointer<ADNPart> part,
	SBPointerIndexer<ADNNucleotide> nts,
	NewNucleotidePlacementMode placementMode) {

	if (part == nullptr) return;

	SBPointerIndexer<ADNBaseSegment> affectedBaseSegments;
	std::vector<SBPointer<ADNNucleotide>> placedNucleotides;

	SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

		if (nt == nullptr) continue;

		// create mock atoms or all atoms
		auto bb = nt->GetBackbone();
		auto sc = nt->GetSidechain();

		auto cBB = bb->GetCenterAtom();
		auto cSC = sc->GetCenterAtom();

		part->RegisterAtom(nt, NucleotideGroup::Backbone, cBB, false);
		part->RegisterAtom(nt, NucleotideGroup::SideChain, cSC, false);
		// hiding atoms here cause when they are created is too slow
		nt->HideCenterAtoms();

		// repeat for pair if there is one
		auto pairNt = nt->GetPair();
		if (pairNt != nullptr) {

			auto bb = pairNt->GetBackbone();
			auto sc = pairNt->GetSidechain();

			auto cBB = bb->GetCenterAtom();
			auto cSC = sc->GetCenterAtom();

			part->RegisterAtom(pairNt, NucleotideGroup::Backbone, cBB, false);
			part->RegisterAtom(pairNt, NucleotideGroup::SideChain, cSC, false);
			pairNt->HideCenterAtoms();

		}

		auto bs = nt->GetBaseSegment();
		if (bs == nullptr) continue;

		auto bsAt = bs->GetCenterAtom();
		if (!bsAt->isCreated()) {

			part->RegisterAtom(bs, bsAt, false);
			bs->HideCenterAtom();

		}

		if (placementMode == NewNucleotidePlacementMode::PositionInputNucleotidesOnly) {

			PlaceNucleotideFromTemplate(bs, nt);
			placedNucleotides.push_back(nt);

		}
		else if (!affectedBaseSegments.hasIndex(bs())) {

			affectedBaseSegments.addReferenceTarget(bs());

		}

	}

	if (placementMode == NewNucleotidePlacementMode::PositionInputNucleotidesOnly) {

		// Newly-created complementary strands are positioned as a batch. Sync
		// their frames only after all centers/backbones/sidechains are placed so
		// tangent reconstruction can see neighboring new nucleotides.
		for (SBPointer<ADNNucleotide> nucleotide : placedNucleotides) {

			if (nucleotide != nullptr)
				ADNGeometrySynchronization::syncNucleotideFrameFromGeometry(*nucleotide);

		}

	}

	if (placementMode == NewNucleotidePlacementMode::ReconstructBaseSegments) {

		SB_FOR(SBPointer<ADNBaseSegment> bs, affectedBaseSegments) {

			if (bs == nullptr) continue;
			// Legacy creation paths rebuild both sides from a temporary template
			// frame. New code should prefer PlaceNucleotideFromTemplate when it
			// must preserve already positioned nucleotides.
			ADNGeometrySynchronization::prepareBaseSegmentFrameForTemplateReconstruction(*bs);
			SetNucleotidePosition(bs, true);

		}

	}

}

void DASBackToTheAtom::UntwistNucleotidesPosition(SBPointer<ADNBaseSegment> bs) {

	if (bs == nullptr) return;

#ifndef NDEBUG
	logInvalidBaseSegmentFrame(__func__, bs);
#endif

	auto nts = bs->GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nt, nts)
		UntwistNucleotidePosition(nt);

}

ublas::matrix<double> DASBackToTheAtom::CreatePositionsMatrix(NtPair pair) {

	SBPointer<ADNNucleotide> nt_left = pair.first;
	SBPointer<ADNNucleotide> nt_right = pair.second;
	const auto ntLeftAtoms = nt_left->GetAtoms();
	const auto ntRightAtoms = nt_right->GetAtoms();
	const size_t cols = 3;
	const size_t rows_left = ntLeftAtoms.size();
	const size_t rows_right = ntRightAtoms.size();
	ublas::matrix<double> positions(rows_left + rows_right, cols);
	int i = 0;

	SB_FOR(const SBPointer<ADNAtom> n, ntLeftAtoms) {

		ublas::vector<double> ac_blas = ADNAuxiliary::SBPositionToUblas(n->getPosition());
		ublas::row(positions, i) = ac_blas;
		++i;

	}

	SB_FOR(const SBPointer<ADNAtom> n, ntRightAtoms) {

		ublas::vector<double> ac_blas = ADNAuxiliary::SBPositionToUblas(n->getPosition());
		ublas::row(positions, i) = ac_blas;
		++i;

	}

	return positions;

}

void DASBackToTheAtom::SetPositionLoopNucleotides(SBPointer<ADNBaseSegment> bs) {

	if (bs == nullptr) return;

	if (bs->GetCell()->GetCellType() == CellType::LoopPair) {

		SBPointer<ADNLoopPair> pair = static_cast<ADNLoopPair*>(bs->GetCell()());
		SBPointer<ADNLoop> left = pair->GetLeftLoop();
		SBPointer<ADNLoop> right = pair->GetRightLoop();

		SBPosition3 posPrevLeft;
		SBPosition3 posPrevRight;
		SBPosition3 posNextLeft;
		SBPosition3 posNextRight;
		SBVector3 prevE2Left;
		SBVector3 prevE2Right;
		SBVector3 nextE2Left;
		SBVector3 nextE2Right;

		if (left != nullptr) {

			SBPointer<ADNNucleotide> ntPrevLeft = nullptr;
			SBPointer<ADNNucleotide> ntNextLeft = nullptr;
			if (left->GetStart() != nullptr) ntPrevLeft = left->GetStart()->GetPrev();
			if (left->GetEnd() != nullptr) ntNextLeft = left->GetEnd()->GetNext();

			if (ntPrevLeft != nullptr) {
				posPrevLeft = ntPrevLeft->GetPosition();
				prevE2Left = ADNAuxiliary::UblasVectorToSBVector(ntPrevLeft->GetE1());
			}
			else if (bs->GetPrev() != nullptr) {
				posPrevLeft = bs->GetPrev()->GetPosition();
				prevE2Left = ADNAuxiliary::UblasVectorToSBVector(bs->GetPrev()->GetE1());
			}
			else {
				posPrevLeft = bs->GetPosition() - SBQuantity::nanometer(ADNConstants::BP_RISE) * ADNAuxiliary::UblasVectorToSBVector(bs->GetE3());
				prevE2Left = ADNAuxiliary::UblasVectorToSBVector(bs->GetE1());
			}

			if (ntNextLeft != nullptr) {
				posNextLeft = ntNextLeft->GetPosition();
				nextE2Left = ADNAuxiliary::UblasVectorToSBVector(ntNextLeft->GetE1());
			}
			else if (bs->GetNext() != nullptr) {
				posNextLeft = bs->GetNext()->GetPosition();
				nextE2Left = ADNAuxiliary::UblasVectorToSBVector(bs->GetNext()->GetE1());
			}
			else {
				posNextLeft = bs->GetPosition() + SBQuantity::nanometer(ADNConstants::BP_RISE) * ADNAuxiliary::UblasVectorToSBVector(bs->GetE3());
				nextE2Left = ADNAuxiliary::UblasVectorToSBVector(bs->GetE1());
			}

			//PositionLoopNucleotides(left, posPrevLeft, posNextLeft);
			PositionLoopNucleotidesQBezier(left, posPrevLeft, posNextLeft, prevE2Left, nextE2Left);

		}

		if (right != nullptr) {

			SBPointer<ADNNucleotide> ntPrevRight = nullptr;
			SBPointer<ADNNucleotide> ntNextRight = nullptr;
			if (right->GetStart() != nullptr) ntPrevRight = right->GetStart()->GetPrev();
			if (right->GetEnd() != nullptr) ntNextRight = right->GetEnd()->GetNext();

			if (ntPrevRight != nullptr) {
				posPrevRight = ntPrevRight->GetPosition();
				prevE2Right = ADNAuxiliary::UblasVectorToSBVector(ntPrevRight->GetE1());
			}
			else if (bs->GetPrev() != nullptr) {
				posPrevRight = bs->GetPrev()->GetPosition();
				prevE2Right = ADNAuxiliary::UblasVectorToSBVector(bs->GetPrev()->GetE1());
			}
			else {
				posPrevRight = bs->GetPosition() - SBQuantity::nanometer(ADNConstants::BP_RISE) * ADNAuxiliary::UblasVectorToSBVector(bs->GetE3());
				prevE2Right = ADNAuxiliary::UblasVectorToSBVector(bs->GetE1());
			}

			if (ntNextRight != nullptr) {
				posNextRight = ntNextRight->GetPosition();
				nextE2Right = ADNAuxiliary::UblasVectorToSBVector(ntNextRight->GetE2());
			}
			else if (bs->GetNext() != nullptr) {
				posNextRight = bs->GetNext()->GetPosition();
				nextE2Right = ADNAuxiliary::UblasVectorToSBVector(bs->GetNext()->GetE2());
			}
			else {
				posNextRight = bs->GetPosition() + SBQuantity::nanometer(ADNConstants::BP_RISE) * ADNAuxiliary::UblasVectorToSBVector(bs->GetE3());
				nextE2Right = ADNAuxiliary::UblasVectorToSBVector(bs->GetE2());
			}

			//PositionLoopNucleotides(right, posPrevRight, posNextRight);
			PositionLoopNucleotidesQBezier(right, posPrevRight, posNextRight, prevE2Right, nextE2Right);

		}

	}

}

void DASBackToTheAtom::CheckDistances(SBPointer<ADNPart> part) const {

	if (part == nullptr) return;

	auto singleStrands = part->GetSingleStrands();
	SBPosition3 prevPos;
	//std::string prevName = "";
	std::string msg = "Checking distances between nucleotides...";
	ADNLogger::LogDebug(msg);
	SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

		int start = 0;
		auto nt = ss->GetFivePrime();
		std::string msg = " string " + ss->getName();
		ADNLogger::LogDebug(msg);
		while (nt != nullptr) {

			if (start != 0) {

				auto distance = (prevPos - nt->GetPosition()).norm();
				if (!ADNVectorMath::IsNearlyZero(distance.getValue() - ADNConstants::BP_RISE * 1000)) {
					//std::string msg = "\tNucleotides " + prevName + " and " + nt->GetName() + " too close or too further away: " + std::to_string(distance.getValue()) + "pm";
					//logger.Log(msg);
				}

			}
			else {
				start = 1;
			}
			prevPos = nt->GetPosition();
			//prevName = nt->getName();
			nt = nt->GetNext();

		}

	}

	auto baseSegments = part->GetBaseSegments();
	msg = "Checking distances between base segments...";
	ADNLogger::LogDebug(msg);
	SB_FOR(SBPointer<ADNBaseSegment> bs, baseSegments) {

		auto bsNext = bs->GetNext();
		if (bsNext != nullptr) {

			const SBPosition3& nextPos = bsNext->GetPosition();
			const std::string nextName = bs->getName();
			const auto distance = (nextPos - bs->GetPosition()).norm();
			if (!ADNVectorMath::IsNearlyZero(distance.getValue() - ADNConstants::BP_RISE * 1000)) {

				//msg = "\tBase Segments " + nextName + " and " + bs->getName() + " too close or too further away: " + std::to_string(distance.getValue()) + "pm";
				//logger.Log(msg);
			}

			if (bs->GetCellType() == CellType::BasePair && bsNext->GetCellType() == CellType::BasePair) {

				SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(bs->GetCell()());
				SBPointer<ADNBasePair> bpNext = static_cast<ADNBasePair*>(bsNext->GetCell()());
				SBPointer<ADNNucleotide> left = bp->GetLeftNucleotide();
				SBPointer<ADNNucleotide> right = bp->GetRightNucleotide();
				SBPointer<ADNNucleotide> leftNext = bpNext->GetLeftNucleotide();
				SBPointer<ADNNucleotide> rightNext = bpNext->GetRightNucleotide();

				if (left != nullptr && right != nullptr && leftNext != nullptr && rightNext != nullptr) {

					auto basePairCenterPosition = (left->GetPosition() + right->GetPosition()) * 0.5;
					auto basePairCenterPositionNext = (leftNext->GetPosition() + rightNext->GetPosition()) * 0.5;
					auto bpDistance = (basePairCenterPosition - basePairCenterPositionNext).norm();
					if (!ADNVectorMath::IsNearlyZero(bpDistance.getValue() - ADNConstants::BP_RISE * 1000)) {

						msg = "\tBase Pairs " + nextName + " and " + bs->getName() + " too close or too further away: " + std::to_string(distance.getValue()) + "pm";
						ADNLogger::LogDebug(msg);

					}

				}

			}

		}

	}

}

void DASBackToTheAtom::UntwistNucleotidePosition(SBPointer<ADNNucleotide> nt) {

	if (nt == nullptr) return;

	SBPointer<ADNBaseSegment> bs = nt->GetBaseSegment();
	if (bs == nullptr) return;

#ifndef NDEBUG
	logInvalidBaseSegmentFrame(__func__, bs);
#endif

	SBPointer<ADNCell> cell = bs->GetCell();
	if (cell->GetCellType() != CellType::BasePair) return;

	SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(cell());
	SBPointer<ADNNucleotide> nt_l = bp->GetLeftNucleotide();
	SBPointer<ADNNucleotide> nt_r = bp->GetRightNucleotide();

	if (nt_r == nullptr && nt_l == nullptr) return;

	NtPair pair = GetIdealBasePairNucleotides(nt_l, nt_r);
	SBPointer<ADNNucleotide> nt_left = pair.first;
	SBPointer<ADNNucleotide> nt_right = pair.second;
	ublas::matrix<double> positions = CreatePositionsMatrix(pair);

	ublas::matrix<double> basisNoTwist = CalculateBaseSegmentBasis(bs);
	const ADNFrameUtils::Frame canonicalFrame = ADNFrameAdapters::sanitizedFrame(*bs);
	const ADNFrameUtils::Frame leftFrame =
		ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
			canonicalFrame,
			ADNGeometrySynchronization::TemplateSide::Left,
			0.0);
	const ADNFrameUtils::Frame rightFrame =
		ADNGeometrySynchronization::canonicalBaseSegmentFrameToNucleotideSideFrame(
			canonicalFrame,
			ADNGeometrySynchronization::TemplateSide::Right,
			0.0);

	// Place c.o.m. at bs position
	ublas::vector<double> sys_cm = ADNAuxiliary::SBPositionToUblas(bs->GetPosition());
	ublas::vector<double> t_vec = sys_cm - ADNVectorMath::CalculateCM(positions);
	ublas::matrix<double> input = ublas::matrix<double>(6, 3);

	ublas::row(input, 0) = ADNAuxiliary::SBPositionToUblas(nt_left->GetPosition());
	ublas::row(input, 1) = ADNAuxiliary::SBPositionToUblas(nt_left->GetBackbonePosition());
	ublas::row(input, 2) = ADNAuxiliary::SBPositionToUblas(nt_left->GetSidechainPosition());
	ublas::row(input, 3) = ADNAuxiliary::SBPositionToUblas(nt_right->GetPosition());
	ublas::row(input, 4) = ADNAuxiliary::SBPositionToUblas(nt_right->GetBackbonePosition());
	ublas::row(input, 5) = ADNAuxiliary::SBPositionToUblas(nt_right->GetSidechainPosition());
	// Apply transformation for non twisted positions
	basisNoTwist = ublas::trans(basisNoTwist);
	ublas::matrix<double> posNoTwist = ADNVectorMath::ApplyTransformation(basisNoTwist, input);
	posNoTwist = ADNVectorMath::Translate(posNoTwist, t_vec);

	if (nt_l != nullptr) {

		// Untwisted reconstruction uses the same side-frame convention with no
		// helical phase applied.
		ADNFrameAdapters::setFrame(*nt_l, leftFrame);

		// Set new residue positions
		SBPosition3 p_left = UblasToSBPosition(ublas::row(posNoTwist, 0));
		nt_l->SetPosition(p_left);
		SBPosition3 p_bb_left = UblasToSBPosition(ublas::row(posNoTwist, 1));
		nt_l->SetBackbonePosition(p_bb_left);
		SBPosition3 p_sc_left = UblasToSBPosition(ublas::row(posNoTwist, 2));
		nt_l->SetSidechainPosition(p_sc_left);

	}

	if (nt_r != nullptr) {

		// Right-side nucleotides stay right-handed by flipping e2 and e3 only.
		ADNFrameAdapters::setFrame(*nt_r, rightFrame);

		// Set positions
		SBPosition3 p_right = UblasToSBPosition(ublas::row(posNoTwist, 3));
		nt_r->SetPosition(p_right);
		SBPosition3 p_bb_right = UblasToSBPosition(ublas::row(posNoTwist, 4));
		nt_r->SetBackbonePosition(p_bb_right);
		SBPosition3 p_sc_right = UblasToSBPosition(ublas::row(posNoTwist, 5));
		nt_r->SetSidechainPosition(p_sc_right);

	}

}

void DASBackToTheAtom::PositionLoopNucleotides(SBPointer<ADNLoop> loop, SBPosition3 bsPositionPrev, SBPosition3 bsPositionNext) {

	if (loop == nullptr) return;

	auto nucleotides = loop->GetNucleotides();
	if (nucleotides.size() == 0) return;

	SBPointer<ADNNucleotide> startNt = loop->GetStart();
	SBPointer<ADNNucleotide> endNt = loop->GetEnd();

	auto order = ADNBasicOperations::OrderNucleotides(startNt, endNt);
	startNt = order.first;
	endNt = order.second;

	if (startNt != nullptr && endNt != nullptr) {

		SBPosition3 start_pos = bsPositionPrev;
		SBPosition3 end_pos = bsPositionNext;
		SBPosition3 shifted = end_pos - start_pos;
		SBPointer<ADNNucleotide> nt = startNt;

		ublas::vector<double> e3 = ADNAuxiliary::SBVectorToUblasVector(shifted.normalizedVersion());
		auto subspace = ADNVectorMath::FindOrthogonalSubspace(e3);
		ublas::vector<double> e1 = ublas::row(subspace, 0);
		ublas::vector<double> e2 = ublas::row(subspace, 1);

		int i = 0;
		auto frac = shifted.norm() / (nucleotides.size() + 1);
		while (nt != endNt->GetNext()) {

			//float frac = float(i) / (nucleotides.size() + 1);
			SBPosition3 shift = (i + 1) * frac * shifted.normalizedVersion();
			shift += start_pos;

			nt->SetPosition(shift);
			nt->SetSidechainPosition(shift);
			nt->SetBackbonePosition(shift);

			nt->SetE1(e1);
			nt->SetE2(e2);
			nt->SetE3(e3);

			nt = nt->GetNext();
			i++;

		}

	}
	else {
		//if loop is at the beginning
	}

}

void DASBackToTheAtom::PositionLoopNucleotidesQBezier(SBPointer<ADNLoop> loop, SBPosition3 bsPositionPrev, SBPosition3 bsPositionNext, SBVector3 bsPrevE3, SBVector3 bsNextE3) {

	if (loop == nullptr) return;

	auto numNts = loop->getNumberOfNucleotides();
	SBPointer<ADNNucleotide> startNt = loop->GetStart();
	SBPointer<ADNNucleotide> endNt = loop->GetEnd();

	auto order = ADNBasicOperations::OrderNucleotides(startNt, endNt);
	startNt = order.first;
	endNt = order.second;

	// height of the curve depends on the number of nucleotides
	// just try and error
	SBPointer<ADNNucleotide> refS = startNt->GetPrev();
	SBPointer<ADNNucleotide> refE = endNt->GetNext();
	SBPosition3 P0;
	SBPosition3 P2;
	SBVector3 startE3;
	SBVector3 endE3;

	if (refS == nullptr) {

		P0 = bsPositionPrev;
		startE3 = bsPrevE3;

	}
	else {

		P0 = refS->GetPosition();
		startE3 = ADNAuxiliary::UblasVectorToSBVector(refS->GetE3());

	}

	if (refE == nullptr) {

		P2 = bsPositionNext;
		endE3 = bsNextE3;

	}
	else {

		P2 = refE->GetPosition();
		endE3 = ADNAuxiliary::UblasVectorToSBVector(refE->GetE3());

	}

	SBVector3 nDir = (startE3 - endE3).normalizedVersion();
	SBQuantity::length step = SBQuantity::nanometer(ADNConstants::BP_RISE) * 0.1;
	SBQuantity::length estLength = (numNts)*SBQuantity::nanometer(ADNConstants::BP_RISE);
	SBPosition3 P1 = (P0 + P2) * 0.5 + SBQuantity::nanometer(0.1) * nDir;

	SBQuantity::length length = ADNVectorMath::LengthQuadraticBezier(P0, P1, P2);

	//P1 += 3*step*nDir;
	while (length < estLength) {
		P1 += step * nDir;
		length = ADNVectorMath::LengthQuadraticBezier(P0, P1, P2);
	}

	// calculate step of the bezier curve
	double deltaT = 1.0 / (numNts + 1);

	if (startNt != nullptr && endNt != nullptr) {

		auto t = deltaT;
		SBPointer<ADNNucleotide> nt = startNt;

		while (nt != endNt->GetNext()) {

			ublas::vector<double> e3 = ADNAuxiliary::SBVectorToUblasVector(ADNVectorMath::DerivativeQuadraticBezier(P0, P1, P2, t));
			auto subspace = ADNVectorMath::FindOrthogonalSubspace(e3);
			ublas::vector<double> e1 = ublas::row(subspace, 0);
			ublas::vector<double> e2 = ublas::row(subspace, 1);
			//float frac = float(i) / (nucleotides.size() + 1);
			SBPosition3 shift = ADNVectorMath::QuadraticBezierPoint(P0, P1, P2, t);

			nt->SetPosition(shift);
			nt->SetSidechainPosition(shift);
			nt->SetBackbonePosition(shift);

			nt->SetE1(e1);
			nt->SetE2(e2);
			nt->SetE3(e3);

			nt = nt->GetNext();
			t += deltaT;

		}

	}

}

void DASBackToTheAtom::CreateBonds(SBPointer<ADNPart> origami, bool createFlag) {

	if (origami == nullptr) return;

	auto nts = origami->GetNucleotides();

	SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

		if (nt == nullptr) continue;

		auto atoms = nt->GetAtoms();
		auto bb = nt->GetBackbone();
		auto sc = nt->GetSidechain();
		if (bb == nullptr || sc == nullptr) continue;
		auto connections = ADNModel::GetNucleotideBonds(nt->getNucleotideType());

		SB_FOR(SBPointer<ADNAtom> at, atoms) {

			if (at == nullptr) continue;

			SBPointer<ADNAtom> atC = nullptr;
			const std::string atName = at->getName();
			if (connections.find(atName) != connections.end()) {

				const auto& conns = connections.at(atName);
				for (const std::string& name : conns) {

					auto lst = nt->GetAtomsByName(name);
					if (lst.size() == 1) {

						atC = *lst.begin();

						// ensure that the bond is not created twice
						if (at->getBondTo(atC())) continue;

						SBPointer<SBBond> bond = new SBBond(at(), atC());
						if (createFlag) {

							if (SAMSON::isHolding()) SAMSON::hold(bond());
							bond->create();

						}
						if (at->IsInADNBackbone()) bb->addChild(bond());
						else sc->addChild(bond());
						//bond->setVisibilityFlag(false);

					}

				}

			}

		}

		// create connection with previous nucleotide
		if (nt->getEndType() != ADNNucleotide::EndType::FivePrime && nt->getEndType() != ADNNucleotide::EndType::FiveAndThreePrime) {

			auto prevNt = nt->GetPrev(true);
			if (prevNt != nullptr) {

				SBPointer<ADNAtom> atP = *nt->GetAtomsByName("P").begin();
				SBPointer<ADNAtom> atO3p = *prevNt->GetAtomsByName("O3'").begin();
				if (atP != nullptr && atO3p != nullptr) {

					// ensure that the bond is not created twice
					if (atP->getBondTo(atO3p())) continue;

					SBPointer<SBBond> bond = new SBBond(atP(), atO3p());
					if (createFlag) {

						if (SAMSON::isHolding()) SAMSON::hold(bond());
						bond->create();

					}
					bb->addChild(bond());
					//bond->setVisibilityFlag(false);

				}

			}

		}

	}

}

DASBackToTheAtom::AtomTemplateSelection DASBackToTheAtom::SelectAtomTemplateForNucleotide(
	SBPointer<ADNNucleotide> nt) const {

	AtomTemplateSelection selection;
	if (nt == nullptr) return selection;

	bool rightSide = false;
	SBPointer<ADNBaseSegment> baseSegment = nt->GetBaseSegment();
	if (baseSegment != nullptr) {

		if (baseSegment->IsLeft(nt)) {

			selection.sideKnown = true;
			rightSide = false;

		}
		else if (baseSegment->IsRight(nt)) {

			selection.sideKnown = true;
			rightSide = true;

		}

	}

	selection.side = rightSide ?
		ADNGeometrySynchronization::TemplateSide::Right :
		ADNGeometrySynchronization::TemplateSide::Left;

	DNABlocks nucleotideType = nt->getNucleotideType();
	if (nucleotideType == DNABlocks::DI)
		nucleotideType = DNABlocks::DA;

	if (rightSide) {

		// Right-side residues must come from the right nucleotide of an ideal
		// pair. Reusing left-side templates shifts backbone atoms even when the
		// coarse nucleotide frame is correct.
		selection.pair = dt_da_;
		selection.nucleotide = dt_da_.second;
		if (nucleotideType == DNABlocks::DT) {

			selection.pair = da_dt_;
			selection.nucleotide = da_dt_.second;

		}
		else if (nucleotideType == DNABlocks::DC) {

			selection.pair = dg_dc_;
			selection.nucleotide = dg_dc_.second;

		}
		else if (nucleotideType == DNABlocks::DG) {

			selection.pair = dc_dg_;
			selection.nucleotide = dc_dg_.second;

		}

	}
	else {

		selection.pair = da_dt_;
		selection.nucleotide = da_dt_.first;
		if (nucleotideType == DNABlocks::DT) {

			selection.pair = dt_da_;
			selection.nucleotide = dt_da_.first;

		}
		else if (nucleotideType == DNABlocks::DC) {

			selection.pair = dc_dg_;
			selection.nucleotide = dc_dg_.first;

		}
		else if (nucleotideType == DNABlocks::DG) {

			selection.pair = dg_dc_;
			selection.nucleotide = dg_dc_.first;

		}

	}

	return selection;

}

void DASBackToTheAtom::FindAtomsPositions(SBPointer<ADNNucleotide> nt,
	const AtomTemplateSelection& selection,
	std::map<ADNBaseSegment*, BaseSegmentAtomPlacementCache>& placementCache) {

	if (nt == nullptr) return;
	if (selection.nucleotide == nullptr) return;

	ADNFrameUtils::Frame frame = ADNFrameAdapters::sanitizedFrame(*nt);
	SBPointer<ADNBaseSegment> baseSegment = nt->GetBaseSegment();
	bool usePairLevelTransform = false;
	bool localFallbackFromOneSidedBaseSegment = false;
	BaseSegmentAtomPlacementCache placement;
	if (baseSegment != nullptr && selection.sideKnown) {

		SBPointer<ADNCell> cell = baseSegment->GetCell();
		if (cell != nullptr && cell->GetCellType() == CellType::BasePair) {

			auto cachedPlacement = placementCache.find(baseSegment());
			if (cachedPlacement == placementCache.end()) {

				SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(cell());
				SBPointer<ADNNucleotide> left = basePair->GetLeftNucleotide();
				SBPointer<ADNNucleotide> right = basePair->GetRightNucleotide();
				BaseSegmentAtomPlacementCache cacheEntry;
				const bool hasLeft = left != nullptr;
				const bool hasRight = right != nullptr;
				cacheEntry.paired = hasLeft && hasRight;
				// Atom generation consumes coarse geometry but must not repair
				// or rewrite it. Cache local placement frames for this call so
				// both nucleotide sides share the same non-mutating canonical
				// frame.
				const ADNFrameUtils::Frame canonicalFrame = cacheEntry.paired ?
					ADNGeometrySynchronization::canonicalTemplateFrameFromCurrentGeometry(*baseSegment) :
					oneSidedCanonicalTemplateFrame(baseSegment, nt);
				const TemplateToWorldTransform transform =
					makeTemplateToWorldTransform(*baseSegment, canonicalFrame);
				cacheEntry.leftFrame = transform.leftFrame;
				cacheEntry.rightFrame = transform.rightFrame;

				if (ADNFrameUtils::isOrthonormalRightHanded(canonicalFrame)) {

					// One-sided atom templates still live in ideal base-pair
					// coordinates. Use the selected ideal pair as the center
					// reference when the real complement is absent.
					const NtPair idealPairForTransform = cacheEntry.paired ?
						GetIdealBasePairNucleotides(left, right) :
						selection.pair;
					cacheEntry.pairBasisMatrix = transform.basisMatrix;
					cacheEntry.pairTranslation =
						ADNAuxiliary::SBPositionToUblas(baseSegment->GetPosition()) -
						GetIdealPairCenterOfMass(idealPairForTransform);
					cacheEntry.hasPairLevelTransform = true;

				}
				cachedPlacement = placementCache.emplace(baseSegment(), cacheEntry).first;

			}

			placement = cachedPlacement->second;
			if (placement.hasPairLevelTransform) {

				if (placement.paired) {

					usePairLevelTransform = true;

				}
				else {

					const ADNFrameUtils::Frame localFallbackFrame =
						selection.side == ADNGeometrySynchronization::TemplateSide::Right ?
						placement.rightFrame :
						placement.leftFrame;
					if (ADNGeometrySynchronization::validateBaseSegmentGeometry(*baseSegment)) {

						// When the base-segment frame agrees with current
						// geometry, the one-sided nucleotide was reconstructed
						// in base-segment space. Trust that context directly:
						// ADNNucleotide::GetPosition() is derived from the
						// backbone/side-chain markers and is not the same
						// quantity as the ideal atom center of mass.
						usePairLevelTransform = true;

					}
					else {

						AtomPlacementMarkerScore pairScore;
						AtomPlacementMarkerScore localScore;
						// Pair-level placement preserves stacking, but
						// one-sided segments can be malformed or manually
						// edited. For stale geometry context, compare the
						// mapped ideal markers against the current coarse
						// center/backbone/side-chain markers before committing
						// to the base-segment transform.
						usePairLevelTransform = oneSidedPairLevelPlacementMatchesMarkers(
							nt,
							selection.nucleotide,
							placement.pairBasisMatrix,
							placement.pairTranslation,
							localFallbackFrame,
							pairScore,
							localScore);
						if (!usePairLevelTransform) {

							localFallbackFromOneSidedBaseSegment = true;
							frame = localFallbackFrame;
#ifndef NDEBUG
							logOneSidedPairLevelMarkerFallback(
								baseSegment,
								nt,
								pairScore,
								localScore);
#endif

						}

					}

				}

			}
			else {

				localFallbackFromOneSidedBaseSegment = !placement.paired;
				frame = selection.side == ADNGeometrySynchronization::TemplateSide::Right ?
					placement.rightFrame :
					placement.leftFrame;
#if defined(ADN_DEBUG_GEOMETRY) && !defined(NDEBUG)
				if (localFallbackFromOneSidedBaseSegment) {

					SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(cell());
					logOneSidedLocalFallbackDiagnostic(
						baseSegment,
						nt,
						selection.side,
						basePair->GetLeftNucleotide() != nullptr,
						basePair->GetRightNucleotide() != nullptr,
						placement.pairBasisMatrix.size1() != 0 && placement.pairBasisMatrix.size2() != 0,
						placement.pairTranslation.size() != 0);

				}
#endif

			}

		}

	}

	if (usePairLevelTransform) {

		const auto generatedAtoms = nt->GetAtoms();
		ublas::matrix<double> input(generatedAtoms.size(), 3);
		int row = 0;
		SB_FOR(SBPointer<ADNAtom> atom, generatedAtoms) {

			ublas::row(input, row) = ADNAuxiliary::SBPositionToUblas(atom->getPosition());
			++row;

		}

		// Paired and one-sided base-segment nucleotides both use ideal base-pair
		// coordinates. A one-sided strand generates atoms only for the existing
		// nucleotide, but the selected ideal nucleotide still belongs to an
		// ideal pair coordinate system. Use the pair-level transform to preserve
		// helical stacking and backbone continuity. The nucleotide-local
		// transform below is only a fallback for isolated nucleotides without
		// usable base-segment context.
		ublas::matrix<double> output =
			ADNVectorMath::ApplyTransformation(placement.pairBasisMatrix, input);
		output = ADNVectorMath::Translate(output, placement.pairTranslation);

		row = 0;
		SB_FOR(SBPointer<ADNAtom> atom, generatedAtoms) {

			atom->setPosition(UblasToSBPosition(ublas::row(output, row)));
			++row;

		}

		return;

	}

	// Fallback for isolated or malformed nucleotides. This preserves the local
	// backbone/side-chain side assignment by mapping through nucleotide-local
	// frames, but it does not enforce ideal helical stacking.
	const ADNFrameUtils::Frame currentFrame =
		nucleotidePlacementFrameFromCoarseGeometry(*nt, frame);
	const ADNFrameUtils::Frame templateFrame =
		nucleotidePlacementFrameFromCoarseGeometry(
			*selection.nucleotide(),
			ADNFrameAdapters::sanitizedFrame(*selection.nucleotide()));
	const ADNFrameUtils::Vec3 worldCenter = positionToVec3(nt->GetPosition());
	const ADNFrameUtils::Vec3 templateCenter = positionToVec3(selection.nucleotide->GetPosition());

	auto generatedAtoms = nt->GetAtoms();
	auto templateAtoms = selection.nucleotide->GetAtoms();

	auto generatedIt = generatedAtoms.begin();
	auto templateIt = templateAtoms.begin();
	for (; generatedIt != generatedAtoms.end() && templateIt != templateAtoms.end();
		++generatedIt, ++templateIt) {

		SBPointer<ADNAtom> generatedAtom = *generatedIt;
		SBPointer<ADNAtom> templateAtom = *templateIt;
		if (generatedAtom == nullptr || templateAtom == nullptr) continue;

		const ADNFrameUtils::Vec3 templateDelta =
			positionToVec3(templateAtom->getPosition()) - templateCenter;
		const ADNFrameUtils::Vec3 templateLocal =
			toLocalCoordinates(templateDelta, templateFrame);
		const ADNFrameUtils::Vec3 world =
			worldCenter + fromLocalCoordinates(templateLocal, currentFrame);
		generatedAtom->setPosition(positionFromVec3(world));

	}

#if defined(ADN_DEBUG_GEOMETRY) && !defined(NDEBUG)
	logUnpairedAtomPlacementDiagnostic(nt, selection.nucleotide, currentFrame, templateFrame);
	if (localFallbackFromOneSidedBaseSegment)
		logOneSidedLocalFallbackGeometryDiagnostic(baseSegment, nt);
#endif

}

void DASBackToTheAtom::PopulateWithMockAtoms(SBPointer<ADNPart> origami, bool positionsFromNucleotide, bool createAtoms) {

	if (origami == nullptr) return;

	auto nts = origami->GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

		auto bb = nt->GetBackbone();
		auto sc = nt->GetSidechain();

		auto cBB = bb->GetCenterAtom();
		auto cSC = sc->GetCenterAtom();
		if (positionsFromNucleotide) {
			cBB->setPosition(nt->GetBackbonePosition());
			cSC->setPosition(nt->GetSidechainPosition());
		}

		origami->RegisterAtom(nt, NucleotideGroup::Backbone, cBB, createAtoms);
		origami->RegisterAtom(nt, NucleotideGroup::SideChain, cSC, createAtoms);
		// hiding atoms here cause when they are created is too slow
		nt->HideCenterAtoms();

	}

	auto bss = origami->GetBaseSegments();
	SB_FOR(SBPointer<ADNBaseSegment> bs, bss) {

		auto at = bs->GetCenterAtom();
		at->setElementType(SBElement::Unknown);// Meitnerium);

		origami->RegisterAtom(bs, at, createAtoms);
		// hiding atoms here cause when they are created is too slow
		bs->HideCenterAtom();

	}

}

void DASBackToTheAtom::PopulateNucleotideWithAllAtoms(SBPointer<ADNPart> origami,
	SBPointer<ADNNucleotide> nt,
	const AtomTemplateSelection& selection,
	bool createFlag) {

	if (origami == nullptr) return;
	if (nt == nullptr) return;
	if (selection.nucleotide == nullptr) return;

	auto atoms = selection.nucleotide->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> atom, atoms) {

		NucleotideGroup g = NucleotideGroup::SideChain;
		if (atom->IsInADNBackbone()) g = NucleotideGroup::Backbone;

		// clone the atom
		SBPointer<ADNAtom> newAtom = new ADNAtom(atom->getElementType(), atom->getPosition());
		newAtom->setName(atom->getName());
		newAtom->setRecordType((char*)"ATOM", 4);

		origami->RegisterAtom(nt, g, newAtom, createFlag);

	}

}

void DASBackToTheAtom::GenerateAllAtomModel(SBPointer<ADNPart> origami, bool createFlag) {

	if (origami == nullptr) return;

	// Atom generation is a non-mutating placement operation on existing coarse
	// geometry. It uses local cached template frames below instead of globally
	// synchronizing or rewriting base-segment frames.
	std::map<ADNBaseSegment*, BaseSegmentAtomPlacementCache> placementCache;

	auto nts = origami->GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

		if (nt == nullptr) continue;

		// delete existing bonds
		SBNodeIndexer bondIndexer;
		nt->getNodes(bondIndexer, SBNode::Bond);
		SB_FOR(SBNode * bond, bondIndexer) {

			SBPointer<SBBond> ptr = static_cast<SBBond*>(bond);
			ptr->getParent()->removeChild(ptr());
			ptr->erase();
			ptr.deleteReferenceTarget();

		}

		// delete previous atoms if they have been created
		auto atoms = nt->GetAtoms();
		SB_FOR(SBPointer<ADNAtom> a, atoms) {

			if (a != nullptr) {

				// Erase previously generated DNA atoms except for the dummy center atoms used for visualization
				// Note: the dummy atom is only deleted from data graph but reference is not destroyed such that it will be possible to use it during the visualization 

				origami->DeregisterAtom(a);

				if (a != nt->GetBackboneCenterAtom() && a != nt->GetSidechainCenterAtom()) {

					a->erase();
					a.deleteReferenceTarget();

				}

			}

		}

		const AtomTemplateSelection selection = SelectAtomTemplateForNucleotide(nt);

		// Populate and place atoms with the same side-aware template. This keeps
		// single-strand atoms centered on the existing nucleotide and prevents
		// right-side residues from reusing left-side local coordinates.
		PopulateNucleotideWithAllAtoms(origami, nt, selection, createFlag);
		FindAtomsPositions(nt, selection, placementCache);

	}

	// NB: In some origami, e.g. in wireframes, Adenita populates nucleotides with atoms in such a way that they are placed farther from each other than their covalent radii
	// which means that we cannot use only SBStructuralModel::createCovalentBonds since it checks for covalent radii within some margin.
	// So, we still use the original CreateBonds function from Adenita to create the bonds, while SBStructuralModel::createCovalentBonds is used to set their order and type.
	// Example of issues: create a Tetrahedron using the Wireframe editor and Generate atomic model - this will lead to atoms, e.g. in vertices, placed quite strangely.

	CreateBonds(origami, createFlag);

	if (createFlag) {

		// since for nucleotides of unknown/not-specified type Adenita sets their type to DI while populating them with atoms for DA
		// we set types of such nucleotides to DA to create covalent bonds and then restore them back to DI

		SBIndexer<ADNNucleotide*> nucleotidesDI;
		SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

			if (nt->getNucleotideType() == DNABlocks::DI) {

				nucleotidesDI.push_back(nt());
				nt->setNucleotideType(DNABlocks::DA);

			}

		}

		origami->createCovalentBonds();

		SB_FOR(ADNNucleotide * nt, nucleotidesDI)
			nt->setNucleotideType(DNABlocks::DI);

	}

#ifndef NDEBUG
	ValidateGeneratedBasePairPlanes(origami);
	ValidateGeneratedSingleStrandAtomGeometry(origami);
#endif

}

#ifndef NDEBUG
bool DASBackToTheAtom::ValidateGeneratedBasePairPlanes(SBPointer<ADNPart> part) const {

	if (part == nullptr) return true;

	bool valid = true;
	auto baseSegments = part->GetBaseSegments();
	SB_FOR(SBPointer<ADNBaseSegment> baseSegment, baseSegments) {

		if (baseSegment == nullptr) continue;
		SBPointer<ADNCell> cell = baseSegment->GetCell();
		if (cell == nullptr || cell->GetCellType() != CellType::BasePair) continue;

		SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(cell());
		SBPointer<ADNNucleotide> left = basePair->GetLeftNucleotide();
		SBPointer<ADNNucleotide> right = basePair->GetRightNucleotide();
		if (left == nullptr || right == nullptr) continue;

		const ADNFrameUtils::Vec3 leftNormal = sidechainPlaneNormal(left);
		const ADNFrameUtils::Vec3 rightNormal = sidechainPlaneNormal(right);
		if (ADNFrameUtils::isNearlyZero(leftNormal) ||
			ADNFrameUtils::isNearlyZero(rightNormal))
			continue;

		const double normalAbsDot = std::abs(ADNFrameUtils::dot(leftNormal, rightNormal));
		const ADNFrameUtils::Vec3 pairDirection = ADNFrameUtils::normalized(
			positionToFrameVec3(right->GetPosition()) - positionToFrameVec3(left->GetPosition()));
		const double pairDirectionPlaneAbsDot =
			ADNFrameUtils::isNearlyZero(pairDirection) ? 0.0 :
			std::max(
				std::abs(ADNFrameUtils::dot(pairDirection, leftNormal)),
				std::abs(ADNFrameUtils::dot(pairDirection, rightNormal)));
		const ADNFrameUtils::Vec3 baseSegmentAxis = localBaseSegmentAxis(baseSegment);
		const double axisNormalAbsDot =
			ADNFrameUtils::isNearlyZero(baseSegmentAxis) ? 1.0 :
			std::min(
				std::abs(ADNFrameUtils::dot(baseSegmentAxis, leftNormal)),
				std::abs(ADNFrameUtils::dot(baseSegmentAxis, rightNormal)));
		const double leftDeterminant = frameDeterminant(left);
		const double rightDeterminant = frameDeterminant(right);

		// Base-ring normals should follow the local double-strand axis. Pair
		// coplanarity alone is not enough: both bases can remain coplanar while
		// the whole generated base-pair plane is tilted by a stale template axis.
		if (normalAbsDot <= 0.95 ||
			pairDirectionPlaneAbsDot > 0.35 ||
			axisNormalAbsDot < 0.85 ||
			leftDeterminant <= 0.0 ||
			rightDeterminant <= 0.0) {

			valid = false;
			const double angleDegrees =
				std::acos(clampedUnit(normalAbsDot)) * 180.0 / 3.141592653589793238462643383279502884;
			ADNLogger::LogDebug("Generated base-pair plane diagnostic failed for base segment " +
				std::to_string(baseSegment->GetNumber()) +
				" (" + left->getName() + ", " + right->getName() + "): normal angle " +
				std::to_string(angleDegrees) + " degrees, pair-normal dot " +
				std::to_string(pairDirectionPlaneAbsDot) + ", axis-normal dot " +
				std::to_string(axisNormalAbsDot) + ", determinants " +
				std::to_string(leftDeterminant) + " / " + std::to_string(rightDeterminant) + ".");

		}

		if (!validateGeneratedBackboneLink(baseSegment, left))
			valid = false;
		if (!validateGeneratedBackboneLink(baseSegment, right))
			valid = false;

	}

	return valid;

}

bool DASBackToTheAtom::ValidateGeneratedSingleStrandAtomGeometry(SBPointer<ADNPart> part) const {

	if (part == nullptr) return true;

	bool valid = true;
	auto baseSegments = part->GetBaseSegments();
	SB_FOR(SBPointer<ADNBaseSegment> baseSegment, baseSegments) {

		if (baseSegment == nullptr) continue;
		SBPointer<ADNCell> cell = baseSegment->GetCell();
		if (cell == nullptr || cell->GetCellType() != CellType::BasePair) continue;

		SBPointer<ADNBasePair> basePair = static_cast<ADNBasePair*>(cell());
		SBPointer<ADNNucleotide> left = basePair->GetLeftNucleotide();
		SBPointer<ADNNucleotide> right = basePair->GetRightNucleotide();
		if ((left == nullptr) == (right == nullptr)) continue;

		SBPointer<ADNNucleotide> nucleotide = left != nullptr ? left : right;
		SBPointer<ADNNucleotide> previous = nucleotide->GetPrev(true);
		const ADNFrameUtils::Vec3 normal = sidechainPlaneNormal(nucleotide);
		const ADNFrameUtils::Vec3 previousNormal = sidechainPlaneNormal(previous);

		// One-sided strands do not have a paired residue for coplanarity checks.
		// Compare neighboring generated base-ring normals instead, ignoring
		// normal sign because plane fitting may choose either orientation.
		if (!ADNFrameUtils::isNearlyZero(normal) &&
			!ADNFrameUtils::isNearlyZero(previousNormal)) {

			const double normalAlignment =
				std::abs(ADNFrameUtils::dot(normal, previousNormal));
			if (normalAlignment < 0.5) {

				valid = false;
				ADNLogger::LogDebug(
					"Generated single-strand diagnostic failed for base segment " +
					std::to_string(baseSegment->GetNumber()) + " nucleotide " +
					nucleotide->getName() + ": ring-normal alignment " +
					std::to_string(normalAlignment) + ".");

			}

		}

		if (!validateGeneratedBackboneLink(baseSegment, nucleotide))
			valid = false;

	}

	return valid;

}
#endif

std::tuple<SBPosition3, SBPosition3, SBPosition3> DASBackToTheAtom::CalculateCenters(SBPointer<ADNNucleotide> nt) {

	SBPosition3 cm_bb = SBPosition3();
	SBPosition3 cm_sc = SBPosition3();
	SBPosition3 cm = SBPosition3();
	ublas::matrix<double> positions_bb = ublas::matrix<double>(0, 3);
	ublas::matrix<double> positions_sc = ublas::matrix<double>(0, 3);
	ublas::matrix<double> positions = ublas::matrix<double>(0, 3);

	auto ntAtoms = nt->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> n, ntAtoms) {

		ublas::vector<double> ac_blas = ADNAuxiliary::SBPositionToUblas(n->getPosition());
		ADNVectorMath::AddRowToMatrix(positions, ac_blas);

		if (n->IsInADNBackbone())
			ADNVectorMath::AddRowToMatrix(positions_bb, ac_blas);
		else
			ADNVectorMath::AddRowToMatrix(positions_sc, ac_blas);

	}

	ublas::vector<double> cm_bb_vec = ADNVectorMath::CalculateCM(positions_bb);
	ublas::vector<double> cm_sc_vec = ADNVectorMath::CalculateCM(positions_sc);
	ublas::vector<double> cm_vec = ADNVectorMath::CalculateCM(positions);
	cm_bb = UblasToSBPosition(cm_bb_vec);
	cm_sc = UblasToSBPosition(cm_sc_vec);
	cm = UblasToSBPosition(cm_vec);
	return std::make_tuple(cm, cm_bb, cm_sc);

}

std::tuple<SBPosition3, SBPosition3, SBPosition3> DASBackToTheAtom::CalculateCentersOfMass(SBPointer<ADNNucleotide> nt) {

	SBPosition3 cm_bb = SBPosition3();
	SBPosition3 cm_sc = SBPosition3();
	SBPosition3 cm = SBPosition3();
	ublas::matrix<double> positions_bb = ublas::matrix<double>(0, 3);
	ublas::matrix<double> positions_sc = ublas::matrix<double>(0, 3);
	ublas::matrix<double> positions = ublas::matrix<double>(0, 3);
	auto ntAtoms = nt->GetAtoms();
	double totalMass(0.0);
	double totalBBMass(0.0);
	double totalSCMass(0.0);
	SB_FOR(SBPointer<ADNAtom> n, ntAtoms) {

		const double mass = n->getAtomicWeight().getValue();
		ublas::vector<double> ac_blas = mass * ADNAuxiliary::SBPositionToUblas(n->getPosition());
		totalMass += mass;
		ADNVectorMath::AddRowToMatrix(positions, ac_blas);

		if (n->IsInADNBackbone()) {

			ADNVectorMath::AddRowToMatrix(positions_bb, ac_blas);
			totalBBMass += mass;

		}
		else {

			ADNVectorMath::AddRowToMatrix(positions_sc, ac_blas);
			totalSCMass += mass;

		}

	}

	ublas::vector<double> cm_bb_vec = ADNVectorMath::CalculateCM(positions_bb, totalBBMass);
	ublas::vector<double> cm_sc_vec = ADNVectorMath::CalculateCM(positions_sc, totalSCMass);
	ublas::vector<double> cm_vec = ADNVectorMath::CalculateCM(positions, totalMass);
	cm_bb = UblasToSBPosition(cm_bb_vec);
	cm_sc = UblasToSBPosition(cm_sc_vec);
	cm = UblasToSBPosition(cm_vec);

	return std::make_tuple(cm, cm_bb, cm_sc);

}

SBPosition3 DASBackToTheAtom::UblasToSBPosition(ublas::vector<double> vec) {

	// we assume vec is in picometers!
	std::vector<double> pos = ADNVectorMath::CreateStdVector(vec);
	SBPosition3 res = SBPosition3();
	res[0] = SBQuantity::angstrom(pos[0] * 0.01);
	res[1] = SBQuantity::angstrom(pos[1] * 0.01);
	res[2] = SBQuantity::angstrom(pos[2] * 0.01);

	return res;

}

void DASBackToTheAtom::SetReferenceFrame(NtPair pair) {

	SBPointer<ADNNucleotide> nt_left = pair.first;
	SBPointer<ADNNucleotide> nt_right = pair.second;
	// Fetch needed points
	std::vector<double> c1_prime_left_std;
	std::vector<double> c1_prime_right_std;
	std::vector<double> c5_prime_left_std;
	std::vector<double> c5_prime_right_std;
	std::vector<double> c3_prime_left_std;
	std::vector<double> c3_prime_right_std;

	auto atoms = nt_left->GetAtoms();
	std::vector<std::vector<double>> positions;
	std::vector<int> sidechain_indices;
	int idx = 0;
	SB_FOR(SBPointer<ADNAtom> a, atoms) {

		positions.push_back(ADNAuxiliary::SBPositionToVector(a->getPosition()));
		if (a->getName() == "C1'") {
			c1_prime_left_std = ADNAuxiliary::SBPositionToVector(a->getPosition());
		}
		if (a->getName() == "C5'") {
			c5_prime_left_std = ADNAuxiliary::SBPositionToVector(a->getPosition());
		}
		if (a->getName() == "C3'") {
			c3_prime_left_std = ADNAuxiliary::SBPositionToVector(a->getPosition());
		}
		if (!a->IsInADNBackbone()) {
			sidechain_indices.push_back(idx);
		}
		++idx;

	}

	atoms = nt_right->GetAtoms();
	SB_FOR(SBPointer<ADNAtom> a, atoms) {

		positions.push_back(ADNAuxiliary::SBPositionToVector(a->getPosition()));
		if (a->getName() == "C1'") {
			c1_prime_right_std = ADNAuxiliary::SBPositionToVector(a->getPosition());
		}
		if (a->getName() == "C5'") {
			c5_prime_right_std = ADNAuxiliary::SBPositionToVector(a->getPosition());
		}
		if (a->getName() == "C3'") {
			c3_prime_right_std = ADNAuxiliary::SBPositionToVector(a->getPosition());
		}
		if (!a->IsInADNBackbone()) {
			sidechain_indices.push_back(idx);
		}
		++idx;

	}

	ublas::vector<double> c5_prime_left = ADNVectorMath::CreateBoostVector(c5_prime_left_std);
	ublas::vector<double> c3_prime_left = ADNVectorMath::CreateBoostVector(c3_prime_left_std);
	ublas::vector<double> c1_prime_left = ADNVectorMath::CreateBoostVector(c1_prime_left_std);
	ublas::vector<double> c1_prime_right = ADNVectorMath::CreateBoostVector(c1_prime_right_std);

	// Center system
	ublas::matrix<double> positions_matrix = ADNVectorMath::CreateBoostMatrix(positions);
	ublas::matrix<double> new_positions = ADNVectorMath::CenterSystem(positions_matrix);

	// Fit sidechain to plane to calculate z (we get directions for left nt)
	ublas::matrix<double> sidechain_positions(0, 3);
	for (auto& it : sidechain_indices) {
		ADNVectorMath::AddRowToMatrix(sidechain_positions, ublas::row(new_positions, it));
	}
	ublas::vector<double> z = ADNVectorMath::CalculatePlane(sidechain_positions);
	// z has to go 5' -> 3'
	ublas::vector<double> d_5p3p = c3_prime_left - c5_prime_left;
	double chk = ublas::inner_prod(z, d_5p3p);
	if (chk < 0) {
		z *= -1.0;
	}
	// y is on the calculated plane c1' -- c1' direction
	ublas::vector<double> y_prime = c1_prime_left - c1_prime_right;
	// we need to make sure it is contained in the plane
	ublas::vector<double> y = y_prime - (ublas::inner_prod(y_prime, z)) * z;
	y /= ublas::norm_2(y);
	// third component
	ublas::vector<double> x = ADNVectorMath::CrossProduct(y, z);
	x /= ublas::norm_2(x);

	// we want to transform all positions so local base is the standard basis
	ublas::matrix<double> transform(3, 3);
	ublas::column(transform, 0) = x;
	ublas::column(transform, 1) = y;
	ublas::column(transform, 2) = z;
	ublas::matrix<double> inv_transform = ADNVectorMath::InvertMatrix(transform);

	ublas::matrix<double> coords = ADNVectorMath::ApplyTransformation(inv_transform, new_positions);

	atoms = nt_left->GetAtoms();
	int r_id = SetAtomsPositions(atoms, coords, 0);
	atoms = nt_right->GetAtoms();
	r_id = SetAtomsPositions(atoms, coords, r_id);

}

int DASBackToTheAtom::SetAtomsPositions(SBPointerIndexer<ADNAtom> atoms, ublas::matrix<double> new_positions, int r_id) {

	SB_FOR(SBPointer<ADNAtom> a, atoms) {

		ublas::vector<double> new_pos = ublas::row(new_positions, r_id);
		std::vector<double> np = ADNVectorMath::CreateStdVector(new_pos);
		SBPosition3 pos = SBPosition3();
		pos[0] = SBQuantity::angstrom(np[0] * 0.01); // we need to convert from picometers
		pos[1] = SBQuantity::angstrom(np[1] * 0.01);
		pos[2] = SBQuantity::angstrom(np[2] * 0.01);
		a->setPosition(pos);
		++r_id;

	}

	return r_id;

}

void DASBackToTheAtom::SetNucleotidesPositions(SBPointer<ADNPart> part) {

	if (part == nullptr) return;

	auto doubleStrands = part->GetDoubleStrands();
	SB_FOR(SBPointer<ADNDoubleStrand> ds, doubleStrands)
		SetDoubleStrandPositions(ds);

}

//void DASBackToTheAtom::RotateNucleotide(SBPointer<ADNNucleotide> nt, double angle, bool set_pair = false) {
//
//  ublas::matrix<double> subspace(3, 3);
//  ublas::row(subspace, 0) = nt->GetE1();
//  ublas::row(subspace, 1) = nt->GetE2();
//  ublas::row(subspace, 2) = nt->GetE3();
//
//  ublas::matrix<double> rot_mat = ADNVectorMath::MakeRotationMatrix(nt->GetE3(), angle);
//  ublas::matrix<double> new_basis = ADNVectorMath::ApplyTransformation(rot_mat, subspace);
//
//  nt->SetE1(ublas::row(new_basis, 0));
//  nt->SetE2(ublas::row(new_basis, 1));
//  nt->SetE3(ublas::row(new_basis, 2));
//
//  // rotation matrix is defined with respect to origin
//  // we need first to translate coordinates
//  ublas::vector<double> sys_cm = ADNAuxiliary::SBPositionToUblas(nt->GetPosition());
//  ublas::vector<double> sys_bb = ADNAuxiliary::SBPositionToUblas(nt->GetBackbonePosition());
//  ublas::vector<double> sys_sc = ADNAuxiliary::SBPositionToUblas(nt->GetSidechainPosition());
//
//  ublas::matrix<double> positions(0, 3);
//  ADNVectorMath::AddRowToMatrix(positions, sys_cm);
//  ADNVectorMath::AddRowToMatrix(positions, sys_bb);
//  ADNVectorMath::AddRowToMatrix(positions, sys_sc);
//
//  positions = ADNVectorMath::Translate(positions, -sys_cm);
//  auto transformation = nt->GetGlobalBasisTransformation();
//  ublas::matrix<double> new_pos = ADNVectorMath::ApplyTransformation(transformation, positions);
//  new_pos = ADNVectorMath::Translate(new_pos, sys_cm);
//
//  if (set_pair && nt->GetPair() != nullptr) {
//    RotateNucleotide(nt->GetPair(), angle, false);
//  }
//}

//void DASBackToTheAtom::DisplayDNABlock(std::string block) {
//  SBPointer<SBStructuralModel> structuralModel = new SBMStructuralModel();
//  structuralModel->setName("test");
//  SBPointer<SBChain> chain = new SBChain();
//  chain->setName("test chain");
//  if (block == "A") {
//    SBPointer<SBResidue> res = da_->CreateSBResidue();
//    chain->addChild(res());
//  }
//  else if (block == "C") {
//    SBPointer<SBResidue> res = dc_->CreateSBResidue();
//    chain->addChild(res());
//  }
//  else if (block == "G") {
//    SBPointer<SBResidue> res = dg_->CreateSBResidue();
//    chain->addChild(res());
//  }
//  else if (block == "T") {
//    SBPointer<SBResidue> res = dt_->CreateSBResidue();
//    chain->addChild(res());
//  }
//  else if (block == "AT") {
//    SBPointer<SBResidue> res1 = da_dt_.first->CreateSBResidue();
//    SBPointer<SBResidue> res2 = da_dt_.second->CreateSBResidue();
//    chain->addChild(res1());
//    SBPointer<SBChain> chain2 = new SBChain();
//    chain2->setName("test chain 2");
//    chain2->addChild(res2());
//    structuralModel->getStructuralRoot()->addChild(chain2());
//  }
//  else if (block == "CG") {
//    SBPointer<SBResidue> res1 = dc_dg_.first->CreateSBResidue();
//    SBPointer<SBResidue> res2 = dc_dg_.second->CreateSBResidue();
//    chain->addChild(res1());
//    SBPointer<SBChain> chain2 = new SBChain();
//    chain2->setName("test chain 2");
//    chain2->addChild(res2());
//    structuralModel->getStructuralRoot()->addChild(chain2());
//  }
//  else if (block == "GC") {
//    SBPointer<SBResidue> res1 = dg_dc_.first->CreateSBResidue();
//    SBPointer<SBResidue> res2 = dg_dc_.second->CreateSBResidue();
//    chain->addChild(res1());
//    SBPointer<SBChain> chain2 = new SBChain();
//    chain2->setName("test chain 2");
//    chain2->addChild(res2());
//    structuralModel->getStructuralRoot()->addChild(chain2());
//  }
//  else if (block == "TA") {
//    SBPointer<SBResidue> res1 = dt_da_.first->CreateSBResidue();
//    SBPointer<SBResidue> res2 = dt_da_.second->CreateSBResidue();
//    chain->addChild(res1());
//    SBPointer<SBChain> chain2 = new SBChain();
//    chain2->setName("test chain 2");
//    chain2->addChild(res2());
//    structuralModel->getStructuralRoot()->addChild(chain2());
//  }
//  structuralModel->getStructuralRoot()->addChild(chain());
//  SAMSON::beginHolding("Add model");
//  structuralModel->create();
//  SAMSON::getActiveLayer()->addChild(structuralModel());
//  SAMSON::endHolding();
//}

void DASBackToTheAtom::LoadNucleotides() {

	std::map<DNABlocks, char> residueNames = { { DNABlocks::DA, 'A' },{ DNABlocks::DT, 'T' },{ DNABlocks::DC, 'C' },{ DNABlocks::DG, 'G' } };

	for (auto it = residueNames.begin(); it != residueNames.end(); ++it) {

		if (it->second == 'N') continue;

		std::string name(1, it->second);
		const std::string nt_source = SB_ELEMENT_PATH + "/Data/" + name + ".pdb";

		try {

			if (!std::filesystem::exists(std::filesystem::u8path(nt_source))) {

				ADNLogger::LogError("Could not find the file " + nt_source);
				return;

			}

		}
		catch (...) {

			ADNLogger::LogError("Caught an exception when checking the file " + nt_source);
			return;

		}

		SBPointer<ADNNucleotide> nt = ParsePDB(nt_source);

		nt->setNucleotideType(it->first);
		auto atoms = nt->GetAtoms();
		std::vector<std::vector<double>> positions;
		std::vector<std::vector<double>> base_plane;
		SB_FOR(SBPointer<ADNAtom> a, atoms) {

			auto coords = ADNAuxiliary::SBPositionToVector(a->getPosition());
			positions.push_back(coords);

		}

		// Set same coordinate system for all nucleotides
		ublas::matrix<double> positions_matrix = ADNVectorMath::CreateBoostMatrix(positions);
		ublas::matrix<double> new_positions = ADNVectorMath::CenterSystem(positions_matrix);
		int r_id = SetAtomsPositions(atoms, new_positions, 0);
		// nt->SetReferenceFrame();
		auto nt_cms = CalculateCentersOfMass(nt);
		nt->SetPosition(std::get<0>(nt_cms));
		nt->SetBackbonePosition(std::get<1>(nt_cms));
		nt->SetSidechainPosition(std::get<2>(nt_cms));

		if (it->first == DNABlocks::DA) da_ = nt;
		else if (it->first == DNABlocks::DC) dc_ = nt;
		else if (it->first == DNABlocks::DG) dg_ = nt;
		else if (it->first == DNABlocks::DT) dt_ = nt;

	}

}

void DASBackToTheAtom::LoadNtPairs() {

	for (auto it = nt_pairs_names_.begin(); it != nt_pairs_names_.end(); ++it) {

		std::string name = it->right;
		if (name == "NN") continue;

		const std::string nt_source = SB_ELEMENT_PATH + "/Data/" + name + ".pdb";
		try {

			if (!std::filesystem::exists(std::filesystem::u8path(nt_source))) {

				ADNLogger::LogError("Could not find the file " + nt_source);
				return;

			}

		}
		catch (...) {

			ADNLogger::LogError("Caught an exception when checking the file " + nt_source);
			return;

		}

		NtPair nt_pair = ParseBasePairPDB(nt_source);

		SBPointer<ADNNucleotide> nt_left = nt_pair.first;
		SBPointer<ADNNucleotide> nt_right = nt_pair.second;
		nt_left->setNucleotideType(it->left.first);
		nt_right->setNucleotideType(it->left.second);

		SetReferenceFrame(nt_pair);

		// Set positions
		auto nt_right_cms = CalculateCentersOfMass(nt_right);
		auto nt_left_cms = CalculateCentersOfMass(nt_left);
		// center pair
		auto total_cms = (std::get<0>(nt_right_cms) + std::get<0>(nt_left_cms)) * 0.5;

		nt_right->SetPosition(std::get<0>(nt_right_cms) - total_cms);
		nt_left->SetPosition(std::get<0>(nt_left_cms) - total_cms);
		nt_right->SetBackbonePosition(std::get<1>(nt_right_cms) - total_cms);
		nt_left->SetBackbonePosition(std::get<1>(nt_left_cms) - total_cms);
		nt_right->SetSidechainPosition(std::get<2>(nt_right_cms) - total_cms);
		nt_left->SetSidechainPosition(std::get<2>(nt_left_cms) - total_cms);

		if (it->left == std::make_pair(DNABlocks::DA, DNABlocks::DT)) da_dt_ = nt_pair;
		else if (it->left == std::make_pair(DNABlocks::DC, DNABlocks::DG)) dc_dg_ = nt_pair;
		else if (it->left == std::make_pair(DNABlocks::DG, DNABlocks::DC)) dg_dc_ = nt_pair;
		else if (it->left == std::make_pair(DNABlocks::DT, DNABlocks::DA)) dt_da_ = nt_pair;

	}

}

NtPair DASBackToTheAtom::ParseBasePairPDB(const std::string& source) {

	std::ifstream file(std::filesystem::u8path(source), std::ios::in);

	if (!file) {

		ADNLogger::LogError("Could not open the file " + source);

	}

	SBPointer<ADNNucleotide> nt_left = new ADNNucleotide();
	nt_left->Init();
	SBPointer<ADNNucleotide> nt_right = new ADNNucleotide();
	nt_right->Init();

	char line[1024];
	int atom_id_left = 0;
	int atom_id_right = 0;
	std::map<int, SBPointer<ADNAtom>> atoms_by_pdb_id;
	int r_num_f = -1;
	std::string prev_residue_chain = "";

	while (file.good()) {

		file.getline(line, 1023);
		std::string s = line;
		const std::string record_name = s.substr(0, 6);
		if (record_name == "ATOM  ") {

			auto residue_chain = s.substr(21, 1);
			if (prev_residue_chain != "" && residue_chain != prev_residue_chain) {
				r_num_f *= -1;
			}

			std::string pdb_id = s.substr(6, 5);
			int p_id = std::stoi(pdb_id);

			SBPointer<ADNAtom> atom = new ADNAtom();
			atom->setRecordType((char*)"ATOM", 4);

			std::string name = s.substr(12, 4);
			boost::trim(name);
			atom->setName(name);
			atom->setElementType(ADNModel::GetElementType(name));
			std::string x = s.substr(30, 8);
			std::string y = s.substr(38, 8);
			std::string z = s.substr(46, 8);
			SBPosition3 pos = SBPosition3();
			pos[0] = SBQuantity::angstrom(std::stod(x));
			pos[1] = SBQuantity::angstrom(std::stod(y));
			pos[2] = SBQuantity::angstrom(std::stod(z));
			atom->setPosition(pos);

			if (r_num_f == -1) {

				NucleotideGroup g = SBNode::SideChain;
				if (ADNModel::IsAtomInBackboneByName(atom->getName())) g = SBNode::Backbone;
				nt_left->addAtom(g, atom);

			}
			else {

				NucleotideGroup g = SBNode::SideChain;
				if (ADNModel::IsAtomInBackboneByName(atom->getName())) g = SBNode::Backbone;
				nt_right->addAtom(g, atom);

			}

			atoms_by_pdb_id.insert(std::make_pair(p_id, atom));
			prev_residue_chain = residue_chain;

		}
#if 0
		else if (record_name == "CONECT") {

			// Check length of connect field
			const size_t l = s.size();
			std::string a_id = s.substr(7, 5);
			boost::trim(a_id);
			int aid = std::stoi(a_id);
			std::string c_id1 = s.substr(12, 5);
			boost::trim(c_id1);
			int cid = std::stoi(c_id1);
			int cid2 = -1;
			if (l >= 17) {
				std::string c_id2 = s.substr(17, 5);
				boost::trim(c_id2);
				if (c_id2.size() > 0) {
					cid2 = std::stoi(c_id2);
				}
			}
			int cid3 = -1;
			if (l >= 22) {
				std::string c_id3 = s.substr(22, 5);
				boost::trim(c_id3);
				if (c_id3.size() > 0) {
					cid3 = std::stoi(c_id3);
				}
			}
			int cid4 = -1;
			if (l >= 27) {
				std::string c_id4 = s.substr(27, 5);
				boost::trim(c_id4);
				if (c_id4.size() > 0) {
					cid4 = std::stoi(c_id4);
				}
			}

			//std::vector<int> neighbors{ cid, cid2, cid3, cid4 };
			//SBPointer<ADNAtom> atom = atoms_by_pdb_id.at(aid);
			/*for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
			  if (*it > 0) {
				SBPointer<ADNAtom> at = atoms_by_pdb_id.at(*it);
				atom->connections_.push_back(at);
			  }
			}*/

		}
#endif

	}

	return std::make_pair(nt_left, nt_right);

}

NtPair DASBackToTheAtom::GetIdealBasePairNucleotides(SBPointer<ADNNucleotide> nt_l, SBPointer<ADNNucleotide> nt_r) const {

	// scaffold nucleotide is always on the left

	std::pair<DNABlocks, DNABlocks> pair_type;
	if (nt_l == nullptr)
		pair_type = std::make_pair(ADNModel::GetComplementaryBase(nt_r->getNucleotideType()), nt_r->getNucleotideType());
	else if (nt_r == nullptr)
		pair_type = std::make_pair(nt_l->getNucleotideType(), ADNModel::GetComplementaryBase(nt_l->getNucleotideType()));
	else
		pair_type = std::make_pair(nt_l->getNucleotideType(), nt_r->getNucleotideType());

	return GetIdealBasePairNucleotides(pair_type.first, pair_type.second);

}

NtPair DASBackToTheAtom::GetIdealBasePairNucleotides(DNABlocks nt_l, DNABlocks nt_r) const {

	// scaffold nucleotide is always on the left

	std::pair<DNABlocks, DNABlocks> pair_type = std::make_pair(nt_l, nt_r);

	NtPair pair = da_dt_;

	if (pair_type == std::make_pair(DNABlocks::DA, DNABlocks::DT)) pair = da_dt_;
	else if (pair_type == std::make_pair(DNABlocks::DC, DNABlocks::DG)) pair = dc_dg_;
	else if (pair_type == std::make_pair(DNABlocks::DG, DNABlocks::DC)) pair = dg_dc_;
	else if (pair_type == std::make_pair(DNABlocks::DT, DNABlocks::DA)) pair = dt_da_;

	return pair;

}

ublas::vector<double> DASBackToTheAtom::GetIdealPairCenterOfMass(NtPair pair) const {

	ADNNucleotide* key = pair.first();
	auto it = idealPairCenterCache_.find(key);
	if (it != idealPairCenterCache_.end())
		return it->second;

	const ublas::matrix<double> positions = CreatePositionsMatrix(pair);
	const ublas::vector<double> center = ADNVectorMath::CalculateCM(positions);
	idealPairCenterCache_[key] = center;
	return center;

}

ublas::matrix<double> DASBackToTheAtom::CalculateBaseSegmentBasis(SBPointer<ADNBaseSegment> bs) {

	ublas::vector<double> new_z(3);
	const auto& direction = bs->GetE3();
	new_z[0] = direction[0];
	new_z[1] = direction[1];
	new_z[2] = direction[2];
	new_z /= ublas::norm_2(new_z);
	bs->SetE3(new_z);
	// if normal_ or u_ are defined we already have the subspace
	ublas::matrix<double> subspace(0, 3);
	if (ublas::norm_2(bs->GetE2()) > 0) {

		ublas::vector<double> normal = bs->GetE2();
		ublas::vector<double> u(3);
		if (ublas::norm_2(bs->GetE1()) > 0) {

			u = bs->GetE1();

		}
		else {

			u = ADNVectorMath::CrossProduct(new_z, normal);
			bs->SetE1(u);

		}

		ADNVectorMath::AddRowToMatrix(subspace, u);
		ADNVectorMath::AddRowToMatrix(subspace, normal);

	}
	else {

		subspace = ADNVectorMath::FindOrthogonalSubspace(new_z);
		bs->SetE1(ublas::row(subspace, 0));
		bs->SetE2(ublas::row(subspace, 1));

	}
	ADNVectorMath::AddRowToMatrix(subspace, new_z);

	return subspace;

}

SBPointer<ADNNucleotide> DASBackToTheAtom::ParsePDB(const std::string& source) {

	std::ifstream file(std::filesystem::u8path(source), std::ios::in);

	if (!file) {

		ADNLogger::LogError("Could not open the file " + source);

	}

	SBPointer<ADNNucleotide> nt = new ADNNucleotide();
	nt->Init();

	char line[1024];
	int atom_id = 1;

	while (file.good()) {

		file.getline(line, 1023);
		std::string s = line;
		const std::string record_name = s.substr(0, 6);
		if (record_name == "ATOM  ") {

			SBPointer<ADNAtom> atom = new ADNAtom();
			atom->setRecordType((char*)"ATOM", 4);

			std::string name = s.substr(12, 4);
			boost::trim(name);
			atom->setName(name);
			std::string x = s.substr(30, 8);
			std::string y = s.substr(38, 8);
			std::string z = s.substr(46, 8);
			SBPosition3 pos = SBPosition3();
			pos[0] = SBQuantity::angstrom(std::stod(x));
			pos[1] = SBQuantity::angstrom(std::stod(y));
			pos[2] = SBQuantity::angstrom(std::stod(z));
			atom->setPosition(pos);
			NucleotideGroup g = SBNode::SideChain;
			if (ADNModel::IsAtomInBackboneByName(atom->getName())) g = SBNode::Backbone;
			nt->addAtom(g, atom);

		}
#if 0
		else if (record_name == "CONECT") {

			// Check length of connect field
			const size_t l = s.size();
			std::string a_id = s.substr(7, 5);
			boost::trim(a_id);
			int aid = std::stoi(a_id);
			std::string c_id1 = s.substr(12, 5);
			boost::trim(c_id1);
			int cid = std::stoi(c_id1);
			int cid2 = -1;
			if (l >= 17) {
				std::string c_id2 = s.substr(17, 5);
				boost::trim(c_id2);
				if (c_id2.size() > 0) {
					cid2 = std::stoi(c_id2);
				}
			}
			int cid3 = -1;
			if (l >= 22) {
				std::string c_id3 = s.substr(22, 5);
				boost::trim(c_id3);
				if (c_id3.size() > 0) {
					cid3 = std::stoi(c_id3);
				}
			}
			int cid4 = -1;
			if (l >= 27) {
				std::string c_id4 = s.substr(27, 5);
				boost::trim(c_id4);
				if (c_id4.size() > 0) {
					cid4 = std::stoi(c_id4);
				}
			}

			//std::vector<int> neighbors{ cid, cid2, cid3, cid4 };
			/*SBPointer<ADNAtom> atom = nt->GetAtom(aid);
			for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
			  if (*it > 0) {
				SBPointer<ADNAtom> at = nt->GetAtom(*it);
				atom->connections_.push_back(at);
			  }
			}*/

		}
#endif

	}

	return nt;

}

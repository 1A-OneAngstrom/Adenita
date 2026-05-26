#include "ADNGeometrySynchronization.hpp"

#include "ADNBaseSegment.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNNucleotide.hpp"
#include "ADNPart.hpp"
#include "ADNSingleStrand.hpp"

#include <cmath>

namespace ADNGeometrySynchronization {

namespace {

constexpr double geometryEps = 1.0e-10;

template <typename Vector3>
[[nodiscard]] ADNFrameUtils::Vec3 toVec3(const Vector3& vector) {

	return ADNFrameUtils::Vec3{
		vector[0].getValue(),
		vector[1].getValue(),
		vector[2].getValue()
	};

}

[[nodiscard]] ADNFrameUtils::Vec3 nucleotideTangent(const ADNNucleotide& nucleotide) {

	const SBPointer<ADNNucleotide> prev = nucleotide.GetPrev(true);
	const SBPointer<ADNNucleotide> next = nucleotide.GetNext(true);

	if (prev != nullptr && next != nullptr)
		return toVec3(next->GetPosition() - prev->GetPosition());

	if (next != nullptr)
		return toVec3(next->GetPosition() - nucleotide.GetPosition());

	if (prev != nullptr)
		return toVec3(nucleotide.GetPosition() - prev->GetPosition());

	return ADNFrameUtils::Vec3{};

}

[[nodiscard]] ADNFrameUtils::Vec3 baseSegmentTangent(const ADNBaseSegment& baseSegment) {

	const SBPointer<ADNBaseSegment> prev = baseSegment.GetPrev(true);
	const SBPointer<ADNBaseSegment> next = baseSegment.GetNext(true);

	if (prev != nullptr && next != nullptr)
		return toVec3(next->GetPosition() - prev->GetPosition());

	if (next != nullptr)
		return toVec3(next->GetPosition() - baseSegment.GetPosition());

	if (prev != nullptr)
		return toVec3(baseSegment.GetPosition() - prev->GetPosition());

	return ADNFrameUtils::Vec3{};

}

[[nodiscard]] ADNFrameUtils::Frame repairedFallback(const Orientable& orientable) {

	return ADNFrameUtils::orthonormalized(ADNFrameAdapters::frameFromOrientable(orientable));

}

[[nodiscard]] bool frameIsValid(const Orientable& orientable) {

	return ADNFrameUtils::isOrthonormalRightHanded(ADNFrameAdapters::frameFromOrientable(orientable));

}

[[nodiscard]] double alignedAbsDot(const ADNFrameUtils::Vec3& frameAxis,
	const ADNFrameUtils::Vec3& geometryDirection,
	double eps = geometryEps) {

	if (ADNFrameUtils::isNearlyZero(frameAxis, eps) ||
		ADNFrameUtils::isNearlyZero(geometryDirection, eps))
		return 1.0;

	return std::abs(ADNFrameUtils::dot(
		ADNFrameUtils::normalized(frameAxis, eps),
		ADNFrameUtils::normalized(geometryDirection, eps)));

}

struct BaseSegmentNucleotideSides {
	SBPointer<ADNNucleotide> left{ nullptr };
	SBPointer<ADNNucleotide> right{ nullptr };
	SBPointer<ADNNucleotide> first{ nullptr };
};

[[nodiscard]] BaseSegmentNucleotideSides baseSegmentNucleotideSides(const ADNBaseSegment& baseSegment) {

	BaseSegmentNucleotideSides sides;

	auto nucleotides = baseSegment.GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide == nullptr) continue;
		if (sides.first == nullptr) sides.first = nucleotide;
		if (baseSegment.IsLeft(nucleotide)) sides.left = nucleotide;
		else if (baseSegment.IsRight(nucleotide)) sides.right = nucleotide;

	}

	return sides;

}

} // namespace

void syncNucleotideFrameFromGeometry(ADNNucleotide& nucleotide) {

	const ADNFrameUtils::Frame fallback = repairedFallback(nucleotide);
	const ADNFrameUtils::Vec3 e2 = toVec3(nucleotide.GetSidechainPosition() - nucleotide.GetBackbonePosition());
	if (ADNFrameUtils::isNearlyZero(e2, geometryEps)) {

		ADNFrameAdapters::setFrame(nucleotide, fallback);
		return;

	}

	ADNFrameUtils::Vec3 tangent = nucleotideTangent(nucleotide);
	if (ADNFrameUtils::isNearlyZero(tangent, geometryEps))
		tangent = fallback.e3;

	ADNFrameAdapters::setFrame(nucleotide,
		ADNFrameUtils::frameFromE2AndTangent(e2, tangent, fallback));

}

void syncBaseSegmentFrameFromGeometry(ADNBaseSegment& baseSegment) {

	const ADNFrameUtils::Frame fallback = repairedFallback(baseSegment);

	const BaseSegmentNucleotideSides sides = baseSegmentNucleotideSides(baseSegment);

	ADNFrameUtils::Vec3 e2{};
	if (sides.left != nullptr && sides.right != nullptr) {

		e2 = toVec3(sides.right->GetPosition() - sides.left->GetPosition());

	}
	else if (sides.first != nullptr && frameIsValid(*sides.first)) {

		e2 = ADNFrameAdapters::frameFromOrientable(*sides.first).e2;

	}
	else {

		e2 = fallback.e2;

	}

	if (ADNFrameUtils::isNearlyZero(e2, geometryEps)) {

		ADNFrameAdapters::setFrame(baseSegment, fallback);
		return;

	}

	ADNFrameUtils::Vec3 tangent = baseSegmentTangent(baseSegment);
	if (ADNFrameUtils::isNearlyZero(tangent, geometryEps) && sides.first != nullptr && frameIsValid(*sides.first))
		tangent = ADNFrameAdapters::frameFromOrientable(*sides.first).e3;
	if (ADNFrameUtils::isNearlyZero(tangent, geometryEps))
		tangent = fallback.e3;

	ADNFrameAdapters::setFrame(baseSegment,
		ADNFrameUtils::frameFromE2AndTangent(e2, tangent, fallback));

}

void syncSingleStrandFramesFromGeometry(ADNSingleStrand& strand) {

	auto nucleotides = strand.GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide != nullptr)
			syncNucleotideFrameFromGeometry(*nucleotide);

	}

}

void syncDoubleStrandFramesFromGeometry(ADNDoubleStrand& strand) {

	auto baseSegments = strand.GetBaseSegments();
	SB_FOR(SBPointer<ADNBaseSegment> baseSegment, baseSegments) {

		if (baseSegment != nullptr)
			syncBaseSegmentFrameFromGeometry(*baseSegment);

	}

}

void syncPartFramesFromGeometry(ADNPart& part, SyncReason reason) {

	(void)reason;

	auto nucleotides = part.GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide != nullptr)
			syncNucleotideFrameFromGeometry(*nucleotide);

	}

	auto baseSegments = part.GetBaseSegments();
	SB_FOR(SBPointer<ADNBaseSegment> baseSegment, baseSegments) {

		if (baseSegment != nullptr)
			syncBaseSegmentFrameFromGeometry(*baseSegment);

	}

}

FrameGeometryAlignment analyzeNucleotideFrameAlignment(const ADNNucleotide& nucleotide,
	double minBackboneSidechainAbsDot,
	double minTangentAbsDot) {

	FrameGeometryAlignment alignment;
	const ADNFrameUtils::Frame frame = ADNFrameAdapters::frameFromOrientable(nucleotide);
	alignment.frameValid = ADNFrameUtils::isOrthonormalRightHanded(frame);

	const ADNFrameUtils::Vec3 backboneSidechain =
		toVec3(nucleotide.GetSidechainPosition() - nucleotide.GetBackbonePosition());
	if (!ADNFrameUtils::isNearlyZero(backboneSidechain, geometryEps)) {

		alignment.primaryDirectionAvailable = true;
		alignment.primaryDirectionAbsDot = alignedAbsDot(frame.e2, backboneSidechain);
		alignment.primaryDirectionAligned =
			alignment.primaryDirectionAbsDot >= minBackboneSidechainAbsDot;

	}

	const ADNFrameUtils::Vec3 tangent = nucleotideTangent(nucleotide);
	if (alignment.primaryDirectionAvailable &&
		!ADNFrameUtils::isNearlyZero(tangent, geometryEps)) {

		alignment.tangentDirectionAvailable = true;
		const ADNFrameUtils::Frame geometryFrame =
			ADNFrameUtils::frameFromE2AndTangent(backboneSidechain, tangent, frame);
		alignment.tangentDirectionAbsDot = alignedAbsDot(frame.e3, geometryFrame.e3);
		alignment.tangentDirectionAligned =
			alignment.tangentDirectionAbsDot >= minTangentAbsDot;

	}

	return alignment;

}

FrameGeometryAlignment analyzeBaseSegmentFrameAlignment(const ADNBaseSegment& baseSegment,
	double minPairAbsDot,
	double minTangentAbsDot) {

	FrameGeometryAlignment alignment;
	const ADNFrameUtils::Frame frame = ADNFrameAdapters::frameFromOrientable(baseSegment);
	alignment.frameValid = ADNFrameUtils::isOrthonormalRightHanded(frame);

	const BaseSegmentNucleotideSides sides = baseSegmentNucleotideSides(baseSegment);
	ADNFrameUtils::Vec3 pairDirection{};
	if (sides.left != nullptr && sides.right != nullptr)
		pairDirection = toVec3(sides.right->GetPosition() - sides.left->GetPosition());

	if (!ADNFrameUtils::isNearlyZero(pairDirection, geometryEps)) {

		alignment.primaryDirectionAvailable = true;
		alignment.primaryDirectionAbsDot = alignedAbsDot(frame.e2, pairDirection);
		alignment.primaryDirectionAligned =
			alignment.primaryDirectionAbsDot >= minPairAbsDot;

	}

	const ADNFrameUtils::Vec3 tangent = baseSegmentTangent(baseSegment);
	if (alignment.primaryDirectionAvailable &&
		!ADNFrameUtils::isNearlyZero(tangent, geometryEps)) {

		alignment.tangentDirectionAvailable = true;
		const ADNFrameUtils::Frame geometryFrame =
			ADNFrameUtils::frameFromE2AndTangent(pairDirection, tangent, frame);
		alignment.tangentDirectionAbsDot = alignedAbsDot(frame.e3, geometryFrame.e3);
		alignment.tangentDirectionAligned =
			alignment.tangentDirectionAbsDot >= minTangentAbsDot;

	}

	return alignment;

}

bool validateNucleotideGeometry(const ADNNucleotide& nucleotide) {

	const FrameGeometryAlignment alignment = analyzeNucleotideFrameAlignment(nucleotide);
	return alignment.frameValid &&
		alignment.primaryDirectionAligned &&
		alignment.tangentDirectionAligned;

}

bool validateBaseSegmentGeometry(const ADNBaseSegment& baseSegment) {

	const FrameGeometryAlignment alignment = analyzeBaseSegmentFrameAlignment(baseSegment);
	return alignment.frameValid &&
		alignment.primaryDirectionAligned &&
		alignment.tangentDirectionAligned;

}

} // namespace ADNGeometrySynchronization

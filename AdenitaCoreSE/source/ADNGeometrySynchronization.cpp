#include "ADNGeometrySynchronization.hpp"

#include "ADNBaseSegment.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNNucleotide.hpp"
#include "ADNPart.hpp"
#include "ADNSingleStrand.hpp"

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

	SBPointer<ADNNucleotide> left = nullptr;
	SBPointer<ADNNucleotide> right = nullptr;
	SBPointer<ADNNucleotide> first = nullptr;

	auto nucleotides = baseSegment.GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide == nullptr) continue;
		if (first == nullptr) first = nucleotide;
		if (baseSegment.IsLeft(nucleotide)) left = nucleotide;
		else if (baseSegment.IsRight(nucleotide)) right = nucleotide;

	}

	ADNFrameUtils::Vec3 e2{};
	if (left != nullptr && right != nullptr) {

		e2 = toVec3(right->GetPosition() - left->GetPosition());

	}
	else if (first != nullptr && frameIsValid(*first)) {

		e2 = ADNFrameAdapters::frameFromOrientable(*first).e2;

	}
	else {

		e2 = fallback.e2;

	}

	if (ADNFrameUtils::isNearlyZero(e2, geometryEps)) {

		ADNFrameAdapters::setFrame(baseSegment, fallback);
		return;

	}

	ADNFrameUtils::Vec3 tangent = baseSegmentTangent(baseSegment);
	if (ADNFrameUtils::isNearlyZero(tangent, geometryEps) && first != nullptr && frameIsValid(*first))
		tangent = ADNFrameAdapters::frameFromOrientable(*first).e3;
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

bool validateNucleotideGeometry(const ADNNucleotide& nucleotide) {

	return frameIsValid(nucleotide);

}

bool validateBaseSegmentGeometry(const ADNBaseSegment& baseSegment) {

	return frameIsValid(baseSegment);

}

} // namespace ADNGeometrySynchronization

#include "ADNGeometrySynchronization.hpp"

#include "ADNAtom.hpp"
#include "ADNBaseSegment.hpp"
#include "ADNConstants.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNNucleotide.hpp"
#include "ADNPart.hpp"
#include "ADNSingleStrand.hpp"

#include <cmath>

namespace ADNGeometrySynchronization {

namespace {

constexpr double geometryEps = 1.0e-10;
constexpr double pi = 3.141592653589793238462643383279502884;

[[nodiscard]] double degreesToRadians(double degrees) {

	return degrees * pi / 180.0;

}

[[nodiscard]] double baseSegmentTemplatePhaseRadians(const ADNBaseSegment& baseSegment) {

	double initialTwistAngle = 0.0;
	SBPointer<ADNDoubleStrand> doubleStrand = baseSegment.GetDoubleStrand();
	if (doubleStrand != nullptr)
		initialTwistAngle = doubleStrand->GetInitialTwistAngle();

	return degreesToRadians(initialTwistAngle + baseSegment.GetNumber() * ADNConstants::BP_ROT);

}

template <typename Vector3>
[[nodiscard]] ADNFrameUtils::Vec3 toVec3(const Vector3& vector) {

	return ADNFrameUtils::Vec3{
		vector[0].getValue(),
		vector[1].getValue(),
		vector[2].getValue()
	};

}

[[nodiscard]] SBPosition3 toPosition(const ADNFrameUtils::Vec3& vector) {

	return SBPosition3(
		SBQuantity::picometer(vector.x),
		SBQuantity::picometer(vector.y),
		SBQuantity::picometer(vector.z));

}

[[nodiscard]] ADNFrameUtils::Vec3 rotatedAroundPoint(const ADNFrameUtils::Vec3& position,
	const ADNFrameUtils::Vec3& center,
	const ADNFrameUtils::Mat3& rotation) {

	return center + ADNFrameUtils::rotated(rotation, position - center);

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

[[nodiscard]] bool nucleotideBackboneSidechainGeometryAvailable(const ADNNucleotide& nucleotide) {

	return !ADNFrameUtils::isNearlyZero(
		toVec3(nucleotide.GetSidechainPosition() - nucleotide.GetBackbonePosition()),
		geometryEps);

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

[[nodiscard]] SBPointer<ADNNucleotide> selectTemplateSourceNucleotide(const ADNBaseSegment& baseSegment,
	const BaseSegmentNucleotideSides& sides,
	TemplateSide& side) {

	if (sides.left != nullptr && nucleotideBackboneSidechainGeometryAvailable(*sides.left)) {

		side = TemplateSide::Left;
		return sides.left;

	}
	if (sides.right != nullptr && nucleotideBackboneSidechainGeometryAvailable(*sides.right)) {

		side = TemplateSide::Right;
		return sides.right;

	}
	if (sides.left != nullptr && frameIsValid(*sides.left)) {

		side = TemplateSide::Left;
		return sides.left;

	}
	if (sides.right != nullptr && frameIsValid(*sides.right)) {

		side = TemplateSide::Right;
		return sides.right;

	}
	if (sides.first != nullptr) {

		side = baseSegment.IsRight(sides.first) ? TemplateSide::Right : TemplateSide::Left;
		return sides.first;

	}

	return nullptr;

}

void addPartIfMissing(SBPointerIndexer<ADNPart>& parts, SBPointer<ADNPart> part) {

	if (part != nullptr && !parts.hasIndex(part()))
		parts.addReferenceTarget(part());

}

void rotateNucleotideGeometry(ADNNucleotide& nucleotide,
	const ADNFrameUtils::Vec3& center,
	const ADNFrameUtils::Mat3& rotation) {

	const ADNFrameUtils::Vec3 newCenter =
		rotatedAroundPoint(toVec3(nucleotide.GetPosition()), center, rotation);
	const ADNFrameUtils::Vec3 newBackbone =
		rotatedAroundPoint(toVec3(nucleotide.GetBackbonePosition()), center, rotation);
	const ADNFrameUtils::Vec3 newSidechain =
		rotatedAroundPoint(toVec3(nucleotide.GetSidechainPosition()), center, rotation);

	SBPointer<ADNAtom> backboneCenter = nucleotide.GetBackboneCenterAtom();
	SBPointer<ADNAtom> sidechainCenter = nucleotide.GetSidechainCenterAtom();
	auto atoms = nucleotide.GetAtoms();
	SB_FOR(SBPointer<ADNAtom> atom, atoms) {

		if (atom == nullptr) continue;
		if (backboneCenter != nullptr && atom() == backboneCenter()) continue;
		if (sidechainCenter != nullptr && atom() == sidechainCenter()) continue;

		atom->setPosition(toPosition(
			rotatedAroundPoint(toVec3(atom->getPosition()), center, rotation)));

	}

	nucleotide.SetPosition(toPosition(newCenter));
	nucleotide.SetBackbonePosition(toPosition(newBackbone));
	nucleotide.SetSidechainPosition(toPosition(newSidechain));
	ADNFrameAdapters::rotateFrame(nucleotide, rotation);

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

	BaseSegmentNucleotideSides sides = baseSegmentNucleotideSides(baseSegment);

	// Geometry synchronization deliberately stores a frame aligned with the
	// current pair/tangent geometry. Template reconstruction uses a different,
	// phase-neutral convention prepared by prepareBaseSegmentFrameForTemplateReconstruction.
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

void syncPartFramesBeforeGeometryEdit(ADNPart& part) {

	syncPartFramesFromGeometry(part, SyncReason::BeforeGeometryEdit);

}

void syncPartFramesAfterGeometryEdit(ADNPart& part) {

	syncPartFramesFromGeometry(part, SyncReason::AfterGeometryEdit);

}

void syncSingleStrandFramesBeforeGeometryEdit(ADNSingleStrand& strand) {

	SBPointer<ADNPart> part = strand.GetPart();
	if (part != nullptr) syncPartFramesBeforeGeometryEdit(*part);
	else syncSingleStrandFramesFromGeometry(strand);

}

void syncSingleStrandFramesAfterGeometryEdit(ADNSingleStrand& strand) {

	SBPointer<ADNPart> part = strand.GetPart();
	if (part != nullptr) syncPartFramesAfterGeometryEdit(*part);
	else syncSingleStrandFramesFromGeometry(strand);

}

void syncDoubleStrandFramesBeforeGeometryEdit(ADNDoubleStrand& strand) {

	SBPointer<ADNPart> part = strand.GetPart();
	if (part != nullptr) syncPartFramesBeforeGeometryEdit(*part);
	else syncDoubleStrandFramesFromGeometry(strand);

}

void syncDoubleStrandFramesAfterGeometryEdit(ADNDoubleStrand& strand) {

	SBPointer<ADNPart> part = strand.GetPart();
	if (part != nullptr) syncPartFramesAfterGeometryEdit(*part);
	else syncDoubleStrandFramesFromGeometry(strand);

}

void rotateBaseSegmentGeometry(ADNBaseSegment& baseSegment, double radians) {

	ADNFrameUtils::Frame frame = ADNFrameAdapters::sanitizedFrame(baseSegment);
	ADNFrameUtils::Vec3 axis = ADNFrameUtils::normalized(frame.e3);
	if (ADNFrameUtils::isNearlyZero(axis, geometryEps)) {

		syncBaseSegmentFrameFromGeometry(baseSegment);
		frame = ADNFrameAdapters::sanitizedFrame(baseSegment);
		axis = ADNFrameUtils::normalized(frame.e3);

	}
	if (ADNFrameUtils::isNearlyZero(axis, geometryEps)) return;

	const ADNFrameUtils::Vec3 center = toVec3(baseSegment.GetPosition());
	const ADNFrameUtils::Mat3 rotation = ADNFrameUtils::rotationAroundAxis(axis, radians);

	ADNFrameAdapters::rotateFrame(baseSegment, rotation);

	auto nucleotides = baseSegment.GetNucleotides();
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide != nullptr)
			rotateNucleotideGeometry(*nucleotide, center, rotation);

	}

	ADNFrameAdapters::sanitizeFrame(baseSegment);

}

void rotateDoubleStrandGeometry(ADNDoubleStrand& strand, double radians) {

	auto baseSegments = strand.GetBaseSegments();
	SB_FOR(SBPointer<ADNBaseSegment> baseSegment, baseSegments) {

		if (baseSegment != nullptr)
			rotateBaseSegmentGeometry(*baseSegment, radians);

	}

}

ADNFrameUtils::Frame canonicalBaseSegmentFrameToNucleotideSideFrame(
	const ADNFrameUtils::Frame& canonicalFrame,
	TemplateSide side,
	double phaseRadians) {

	const ADNFrameUtils::Frame sanitizedCanonical = ADNFrameUtils::orthonormalized(canonicalFrame);
	const ADNFrameUtils::Mat3 helicalPhase =
		ADNFrameUtils::rotationAroundAxis(sanitizedCanonical.e3, phaseRadians);
	ADNFrameUtils::Frame sideFrame = ADNFrameUtils::rotated(helicalPhase, sanitizedCanonical);

	if (side == TemplateSide::Right) {

		sideFrame.e2 = -sideFrame.e2;
		sideFrame.e3 = -sideFrame.e3;

	}

	return ADNFrameUtils::orthonormalized(sideFrame);

}

ADNFrameUtils::Frame nucleotideSideFrameToCanonicalBaseSegmentFrame(
	const ADNFrameUtils::Frame& nucleotideFrame,
	TemplateSide side,
	double phaseRadians) {

	ADNFrameUtils::Frame leftSideFrame = ADNFrameUtils::orthonormalized(nucleotideFrame);
	if (side == TemplateSide::Right) {

		leftSideFrame.e2 = -leftSideFrame.e2;
		leftSideFrame.e3 = -leftSideFrame.e3;

	}

	const ADNFrameUtils::Mat3 undoHelicalPhase =
		ADNFrameUtils::rotationAroundAxis(leftSideFrame.e3, -phaseRadians);
	return ADNFrameUtils::orthonormalized(
		ADNFrameUtils::rotated(undoHelicalPhase, leftSideFrame));

}

void prepareBaseSegmentFrameForTemplateReconstruction(ADNBaseSegment& baseSegment) {

	const BaseSegmentNucleotideSides sides = baseSegmentNucleotideSides(baseSegment);
	TemplateSide side = TemplateSide::Left;
	SBPointer<ADNNucleotide> source = selectTemplateSourceNucleotide(baseSegment, sides, side);
	if (source == nullptr) {

		ADNFrameAdapters::sanitizeFrame(baseSegment);
		return;

	}

	// DASBackToTheAtom applies the helical phase itself. Derive the base-segment
	// template frame by removing that phase from a current nucleotide-side frame.
	syncNucleotideFrameFromGeometry(*source);
	const ADNFrameUtils::Frame sourceFrame = ADNFrameAdapters::sanitizedFrame(*source);
	const ADNFrameUtils::Frame canonicalFrame =
		nucleotideSideFrameToCanonicalBaseSegmentFrame(
			sourceFrame,
			side,
			baseSegmentTemplatePhaseRadians(baseSegment));

	ADNFrameAdapters::setFrame(baseSegment, canonicalFrame);

}

void prepareBaseSegmentFramesForTemplateReconstruction(
	const SBPointerIndexer<ADNNucleotide>& nucleotides) {

	SBPointerIndexer<ADNBaseSegment> baseSegments;
	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide == nullptr) continue;
		SBPointer<ADNBaseSegment> baseSegment = nucleotide->GetBaseSegment();
		if (baseSegment != nullptr && !baseSegments.hasIndex(baseSegment()))
			baseSegments.addReferenceTarget(baseSegment());

	}

	prepareBaseSegmentFramesForTemplateReconstruction(baseSegments);

}

void prepareBaseSegmentFramesForTemplateReconstruction(
	const SBPointerIndexer<ADNBaseSegment>& baseSegments) {

	SB_FOR(SBPointer<ADNBaseSegment> baseSegment, baseSegments) {

		if (baseSegment != nullptr)
			prepareBaseSegmentFrameForTemplateReconstruction(*baseSegment);

	}

}

SBPointerIndexer<ADNPart> collectPartsFromDoubleStrands(
	const SBPointerIndexer<ADNDoubleStrand>& doubleStrands) {

	SBPointerIndexer<ADNPart> parts;

	SB_FOR(SBPointer<ADNDoubleStrand> doubleStrand, doubleStrands) {

		if (doubleStrand != nullptr)
			addPartIfMissing(parts, doubleStrand->GetPart());

	}

	return parts;

}

SBPointerIndexer<ADNPart> collectPartsFromNucleotides(
	const SBPointerIndexer<ADNNucleotide>& nucleotides) {

	SBPointerIndexer<ADNPart> parts;

	SB_FOR(SBPointer<ADNNucleotide> nucleotide, nucleotides) {

		if (nucleotide != nullptr)
			addPartIfMissing(parts, findOwningPart(nucleotide()));

	}

	return parts;

}

SBPointer<ADNPart> findOwningPart(SBNode* node) {

	for (SBNode* current = node; current != nullptr; current = current->getParent()) {

		if (ADNPart* part = dynamic_cast<ADNPart*>(current))
			return part;

		if (ADNSingleStrand* strand = dynamic_cast<ADNSingleStrand*>(current))
			return strand->GetPart();

		if (ADNDoubleStrand* strand = dynamic_cast<ADNDoubleStrand*>(current))
			return strand->GetPart();

		if (ADNBaseSegment* baseSegment = dynamic_cast<ADNBaseSegment*>(current)) {

			SBPointer<ADNDoubleStrand> strand = baseSegment->GetDoubleStrand();
			if (strand != nullptr) return strand->GetPart();
			return nullptr;

		}

		if (ADNNucleotide* nucleotide = dynamic_cast<ADNNucleotide*>(current)) {

			SBPointer<ADNSingleStrand> strand = nucleotide->GetStrand();
			if (strand != nullptr) return strand->GetPart();

			SBPointer<ADNDoubleStrand> doubleStrand = nucleotide->GetDoubleStrand();
			if (doubleStrand != nullptr) return doubleStrand->GetPart();
			return nullptr;

		}

		if (ADNAtom* atom = dynamic_cast<ADNAtom*>(current)) {

			SBPointer<ADNPart> part = findOwningPart(atom->getNucleotide());
			if (part != nullptr)
				return part;

		}

	}

	return nullptr;

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

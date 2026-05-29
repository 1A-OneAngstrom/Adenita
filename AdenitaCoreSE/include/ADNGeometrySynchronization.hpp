#pragma once

#include "ADNFrameUtils.hpp"
#include "SBCHeapExport.hpp"
#include "SBDDataGraphNode.hpp"
#include "SBPointer.hpp"
#include "SBPointerIndexer.hpp"

/*! \file ADNGeometrySynchronization.hpp */

class ADNBaseSegment;
class ADNDoubleStrand;
class ADNNucleotide;
class ADNPart;
class ADNSingleStrand;

namespace ADNGeometrySynchronization {

/// \brief Reason why frame synchronization is being triggered.
enum class SyncReason {
	AfterStructuralPositionChange, ///< Synchronize after atom or object positions changed.
	BeforeGeometryEdit, ///< Synchronize before Adenita reconstructs geometry.
	AfterGeometryEdit, ///< Synchronize after Adenita reconstructs geometry.
	BeforeSerialization, ///< Synchronize before writing the part to a persistent format.
	AfterUnserialization, ///< Synchronize after links and geometry were restored from storage.
	ManualRepair ///< Synchronize from an explicit repair call.
};

/// \brief Result of comparing a cached frame to reconstructible geometry.
struct SB_EXPORT FrameGeometryAlignment {
	bool frameValid{ false }; ///< True when the cached frame is orthonormal and right-handed.
	bool primaryDirectionAvailable{ false }; ///< True when the geometry provides a primary direction.
	bool tangentDirectionAvailable{ false }; ///< True when the geometry provides a tangent direction.
	bool primaryDirectionAligned{ true }; ///< True when the frame agrees with the primary direction.
	bool tangentDirectionAligned{ true }; ///< True when the frame agrees with the tangent direction.
	double primaryDirectionAbsDot{ 1.0 }; ///< Absolute dot product against the primary direction.
	double tangentDirectionAbsDot{ 1.0 }; ///< Absolute dot product against the tangent direction.
};

/// \brief Side of a base pair used as a template-frame source.
enum class TemplateSide {
	Left, ///< Left nucleotide frame convention.
	Right ///< Right nucleotide frame convention.
};

/// \brief Return the helical phase applied during base-segment reconstruction.
///
/// The returned angle is positive in the right-handed convention used by
/// ADNFrameUtils. Legacy DASBackToTheAtom code stores the same operation as a
/// negative angle in ADNVectorMath::MakeRotationMatrix, whose sign convention
/// is opposite to ADNFrameUtils::rotationAroundAxis.
SB_EXPORT [[nodiscard]] double baseSegmentReconstructionPhaseRadians(const ADNBaseSegment& baseSegment);

/// \brief Build a persistent frame for newly designed base segments.
///
/// Designed creator output starts with a requested axis but degenerate
/// nucleotide positions, so geometry-derived synchronization cannot recover a
/// radial direction yet. This helper builds a stable right-handed frame from
/// the axis and an optional radial seed. It is safe to persist on the base
/// segment before nucleotide reconstruction.
SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame makeDesignedBaseSegmentFrame(
	const ADNFrameUtils::Vec3& axis,
	const ADNFrameUtils::Vec3* preferredRadial = nullptr);
/// \brief Store a designed construction frame on a newly-created base segment.
SB_EXPORT void initializeDesignedBaseSegmentFrame(ADNBaseSegment& baseSegment,
	const ADNFrameUtils::Vec3& axis,
	const ADNFrameUtils::Vec3* preferredRadial = nullptr);

/// \brief Reconstruct a nucleotide frame from its current geometry.
SB_EXPORT void syncNucleotideFrameFromGeometry(ADNNucleotide& nucleotide);
/// \brief Reconstruct a base-segment frame from its paired nucleotide geometry.
SB_EXPORT void syncBaseSegmentFrameFromGeometry(ADNBaseSegment& baseSegment);

/// \brief Synchronize all nucleotide frames in a single strand.
SB_EXPORT void syncSingleStrandFramesFromGeometry(ADNSingleStrand& strand);
/// \brief Synchronize all nucleotide and base-segment frames in a double strand.
SB_EXPORT void syncDoubleStrandFramesFromGeometry(ADNDoubleStrand& strand);
/// \brief Synchronize every reconstructible frame contained in a part.
///
/// The \p reason allows callers to describe the synchronization boundary so
/// the implementation can preserve stable sign conventions where needed.
SB_EXPORT void syncPartFramesFromGeometry(ADNPart& part, SyncReason reason);

/// \brief Synchronize a part before an Adenita geometry edit reconstructs positions.
SB_EXPORT void syncPartFramesBeforeGeometryEdit(ADNPart& part);
/// \brief Synchronize a part after an Adenita geometry edit reconstructs positions.
SB_EXPORT void syncPartFramesAfterGeometryEdit(ADNPart& part);
/// \brief Synchronize a single strand before an Adenita geometry edit reconstructs positions.
SB_EXPORT void syncSingleStrandFramesBeforeGeometryEdit(ADNSingleStrand& strand);
/// \brief Synchronize a single strand after an Adenita geometry edit reconstructs positions.
SB_EXPORT void syncSingleStrandFramesAfterGeometryEdit(ADNSingleStrand& strand);
/// \brief Synchronize a double strand before an Adenita geometry edit reconstructs positions.
SB_EXPORT void syncDoubleStrandFramesBeforeGeometryEdit(ADNDoubleStrand& strand);
/// \brief Synchronize a double strand after an Adenita geometry edit reconstructs positions.
SB_EXPORT void syncDoubleStrandFramesAfterGeometryEdit(ADNDoubleStrand& strand);

/// \brief Apply a delta rotation to current nucleotide geometry around its local axis.
///
/// This helper preserves the current geometry and composes incremental editor
/// rotations. It is not a DASBackToTheAtom template reconstruction boundary.
SB_EXPORT void rotateBaseSegmentGeometry(ADNBaseSegment& baseSegment, double radians);
/// \brief Apply a delta rotation to every base segment in a double strand.
SB_EXPORT void rotateDoubleStrandGeometry(ADNDoubleStrand& strand, double radians);

/// \brief Convert a canonical base-segment template frame to a nucleotide-side frame.
///
/// The canonical frame is the phase-neutral frame consumed by
/// DASBackToTheAtom. The returned side frame includes the helical phase and the
/// right-side sign convention when \p side is \c TemplateSide::Right. Atom
/// placement later consumes these nucleotide-side frames directly through
/// ADNNucleotide::GetGlobalBasisTransformation.
SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame canonicalBaseSegmentFrameToNucleotideSideFrame(
	const ADNFrameUtils::Frame& canonicalFrame,
	TemplateSide side,
	double phaseRadians);
/// \brief Convert a nucleotide-side frame to the canonical base-segment template frame.
///
/// Use this before template reconstruction paths that call
/// DASBackToTheAtom::SetNucleotidePosition or
/// DASBackToTheAtom::UntwistNucleotidesPosition. Geometry-aligned frames should
/// not be passed directly to these paths because DASBackToTheAtom applies the
/// helical phase itself.
SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame nucleotideSideFrameToCanonicalBaseSegmentFrame(
	const ADNFrameUtils::Frame& nucleotideFrame,
	TemplateSide side,
	double phaseRadians);

/// \brief Derive the canonical DAS template frame from current coarse geometry.
///
/// The returned frame is phase-neutral and follows the base-segment centerline
/// and base-pair radial direction. This helper does not mutate the base
/// segment; use it when atom placement needs fresh frames without repositioning
/// or rewriting coarse DNA geometry.
SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame canonicalTemplateFrameFromCurrentGeometry(
	const ADNBaseSegment& baseSegment);

/// \brief Prepare a base-segment frame for DASBackToTheAtom template reconstruction.
///
/// This mutating compatibility helper stores a phase-neutral template frame on
/// the base segment. Prefer canonicalTemplateFrameFromCurrentGeometry() in new
/// code so persistent base-segment frames remain aligned with visible geometry.
/// Callers that use this helper should reconstruct geometry immediately and
/// then resynchronize persistent frames from the resulting positions.
SB_EXPORT void prepareBaseSegmentFrameForTemplateReconstruction(ADNBaseSegment& baseSegment);
/// \brief Prepare base-segment frames touched by a set of nucleotides.
SB_EXPORT void prepareBaseSegmentFramesForTemplateReconstruction(
	const SBPointerIndexer<ADNNucleotide>& nucleotides);
/// \brief Prepare the given base-segment frames for DASBackToTheAtom template reconstruction.
SB_EXPORT void prepareBaseSegmentFramesForTemplateReconstruction(
	const SBPointerIndexer<ADNBaseSegment>& baseSegments);

/// \brief Return unique owning parts for the given double strands.
SB_EXPORT [[nodiscard]] SBPointerIndexer<ADNPart> collectPartsFromDoubleStrands(
	const SBPointerIndexer<ADNDoubleStrand>& doubleStrands);
/// \brief Return unique owning parts for the given nucleotides.
SB_EXPORT [[nodiscard]] SBPointerIndexer<ADNPart> collectPartsFromNucleotides(
	const SBPointerIndexer<ADNNucleotide>& nucleotides);
/// \brief Return the owning part for an Adenita node when it can be resolved.
SB_EXPORT [[nodiscard]] SBPointer<ADNPart> findOwningPart(SBNode* node);

/// \brief Compare a nucleotide frame with its current geometry.
SB_EXPORT [[nodiscard]] FrameGeometryAlignment analyzeNucleotideFrameAlignment(const ADNNucleotide& nucleotide,
	double minBackboneSidechainAbsDot = 0.85,
	double minTangentAbsDot = 0.70);
/// \brief Compare a base-segment frame with its current geometry.
SB_EXPORT [[nodiscard]] FrameGeometryAlignment analyzeBaseSegmentFrameAlignment(const ADNBaseSegment& baseSegment,
	double minPairAbsDot = 0.85,
	double minTangentAbsDot = 0.70);

/// \brief Test whether a nucleotide carries enough geometric information to reconstruct its frame.
SB_EXPORT [[nodiscard]] bool validateNucleotideGeometry(const ADNNucleotide& nucleotide);
/// \brief Test whether a base segment carries enough geometric information to reconstruct its frame.
SB_EXPORT [[nodiscard]] bool validateBaseSegmentGeometry(const ADNBaseSegment& baseSegment);

} // namespace ADNGeometrySynchronization

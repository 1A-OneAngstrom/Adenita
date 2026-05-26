#pragma once

#include "SBCHeapExport.hpp"
#include "SBPointer.hpp"
#include "SBPointerIndexer.hpp"

/*! \file ADNGeometrySynchronization.hpp */

class ADNBaseSegment;
class ADNDoubleStrand;
class ADNNucleotide;
class ADNPart;
class ADNSingleStrand;
class SBNode;

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

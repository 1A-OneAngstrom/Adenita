#pragma once

#include "SBCHeapExport.hpp"

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
	BeforeSerialization, ///< Synchronize before writing the part to a persistent format.
	AfterUnserialization, ///< Synchronize after links and geometry were restored from storage.
	ManualRepair ///< Synchronize from an explicit repair call.
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

/// \brief Test whether a nucleotide carries enough geometric information to reconstruct its frame.
SB_EXPORT [[nodiscard]] bool validateNucleotideGeometry(const ADNNucleotide& nucleotide);
/// \brief Test whether a base segment carries enough geometric information to reconstruct its frame.
SB_EXPORT [[nodiscard]] bool validateBaseSegmentGeometry(const ADNBaseSegment& baseSegment);

} // namespace ADNGeometrySynchronization

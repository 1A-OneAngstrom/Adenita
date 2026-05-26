#pragma once

#include "SBCHeapExport.hpp"

class ADNBaseSegment;
class ADNDoubleStrand;
class ADNNucleotide;
class ADNPart;
class ADNSingleStrand;

namespace ADNGeometrySynchronization {

enum class SyncReason {
	AfterStructuralPositionChange,
	BeforeSerialization,
	AfterUnserialization,
	ManualRepair
};

SB_EXPORT void syncNucleotideFrameFromGeometry(ADNNucleotide& nucleotide);
SB_EXPORT void syncBaseSegmentFrameFromGeometry(ADNBaseSegment& baseSegment);

SB_EXPORT void syncSingleStrandFramesFromGeometry(ADNSingleStrand& strand);
SB_EXPORT void syncDoubleStrandFramesFromGeometry(ADNDoubleStrand& strand);
SB_EXPORT void syncPartFramesFromGeometry(ADNPart& part, SyncReason reason);

SB_EXPORT [[nodiscard]] bool validateNucleotideGeometry(const ADNNucleotide& nucleotide);
SB_EXPORT [[nodiscard]] bool validateBaseSegmentGeometry(const ADNBaseSegment& baseSegment);

} // namespace ADNGeometrySynchronization

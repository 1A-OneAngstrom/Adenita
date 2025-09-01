#pragma once

#include <SBVector3.hpp>

#include "ADNAuxiliary.hpp"
#include "ADNMixins.hpp"
#include "ADNVectorMath.hpp"
#include "ADNArray.hpp"
#include "ADNConfig.hpp"

#include <iostream>
#include <iomanip>

class ADNPart;
class ADNNucleotide;
class DASPolyhedron;

namespace ADNDisplayHelper {

    SB_EXPORT void displayLine(const SBPosition3& start, const SBPosition3& end, const std::string& text = "");
    SB_EXPORT void displayCylinder(const SBPosition3& start, const SBPosition3& end, const std::string& text = "");
    SB_EXPORT void displayLine(ublas::vector<double> center, ublas::vector<double> dir, int length);
    SB_EXPORT void displayVector(SBVector3 vec, const SBPosition3& shift);
    SB_EXPORT void displayVector(SBVector3 vec, const SBPosition3& shift, float* color, int length);
    SB_EXPORT void displayArrow(SBVector3 vec, const SBPosition3& shift);
    SB_EXPORT void displayArrow(const SBPosition3& start, const SBPosition3& end, unsigned int nodeIndex, float* color, bool selectable = false);
    SB_EXPORT void displayLengthText(const SBPosition3& start, const SBPosition3& end, std::string text = "");
    SB_EXPORT void displayDirectedCylinder(const SBPosition3& start, const SBPosition3& end);
    SB_EXPORT void displayDirectedCylinder(const SBPosition3& start, const SBPosition3& end, float* color, int radius);
    SB_EXPORT void displayDirectedCylinder(float* start, float* end, float* color, int radius);
    SB_EXPORT void displayPlane(/*const SBVector3& vec, const SBPosition3& shift*/);
    SB_EXPORT void displayOrthoPlane(const SBVector3& vec, const SBPosition3& shift);
    SB_EXPORT void displaySphere(const SBPosition3& pos, float radius, ADNArray<float> color);
    SB_EXPORT void displayBasePairConnection(ADNPointer<ADNNucleotide> nt);
    SB_EXPORT void displayBaseVectors(ADNPointer<ADNNucleotide> nt, const SBPosition3& pos);
    SB_EXPORT void displayText(const SBPosition3& pos, const std::string& text = "");
    SB_EXPORT void displayTextBottomLeft(const std::string& text = "");
    SB_EXPORT void displayTriangleMesh(DASPolyhedron* polyhedron);

    SB_EXPORT void displayPart(ADNPointer<ADNPart> part, float basePairRadius = 1000.0f, float opaqueness = 0.5f);        ///< Display only the top scales of a part
    SB_EXPORT void displayGoldSphere(const SBNodeIndexer& goldAtoms);

};

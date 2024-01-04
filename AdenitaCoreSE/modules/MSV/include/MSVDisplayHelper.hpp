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

    void displayLine(const SBPosition3& start, const SBPosition3& end, const std::string& text = "");
    void displayCylinder(const SBPosition3& start, const SBPosition3& end, const std::string& text = "");
    void displayLine(ublas::vector<double> center, ublas::vector<double> dir, int length);
    void displayVector(SBVector3 vec, const SBPosition3& shift);
    void displayVector(SBVector3 vec, const SBPosition3& shift, float* color, int length);
    void displayArrow(SBVector3 vec, const SBPosition3& shift);
    void displayArrow(const SBPosition3& start, const SBPosition3& end, unsigned int nodeIndex, float* color, bool selectable = false);
    void displayLengthText(const SBPosition3& start, const SBPosition3& end, std::string text = "");
    void displayDirectedCylinder(const SBPosition3& start, const SBPosition3& end);
    void displayDirectedCylinder(const SBPosition3& start, const SBPosition3& end, float* color, int radius);
    void displayDirectedCylinder(float* start, float* end, float* color, int radius);
    void displayPlane(/*const SBVector3& vec, const SBPosition3& shift*/);
    void displayOrthoPlane(const SBVector3& vec, const SBPosition3& shift);
    void displaySphere(const SBPosition3& pos, float radius, ADNArray<float> color);
    void displayBasePairConnection(ADNPointer<ADNNucleotide> nt);
    void displayBaseVectors(ADNPointer<ADNNucleotide> nt, const SBPosition3& pos);
    void displayText(const SBPosition3& pos, const std::string& text = "");
    void displayTextBottomLeft(const std::string& text = "");
    void displayTriangleMesh(DASPolyhedron* polyhedron);

    void displayPart(ADNPointer<ADNPart> part, float basePairRadius = 1000.0f, float opaqueness = 0.5f);        ///< Display only the top scales of a part
    void displayGoldSphere(const SBNodeIndexer& goldAtoms);

};

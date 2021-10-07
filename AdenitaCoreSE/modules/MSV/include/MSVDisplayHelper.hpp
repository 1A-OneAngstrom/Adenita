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

    void displayLine(SBPosition3 start, SBPosition3 end, std::string text = "");
    void displayCylinder(SBPosition3 start, SBPosition3 end, std::string text = "");
    void displayLine(ublas::vector<double> center, ublas::vector<double> dir, int length);
    void displayVector(SBVector3 vec, SBPosition3 shift);
    void displayVector(SBVector3 vec, SBPosition3 shift, float* color, int length);
    void displayArrow(SBVector3 vec, SBPosition3 shift);
    void displayArrow(SBPosition3 start, SBPosition3 end, unsigned int nodeIndex, float* color, bool selectable = false);
    void displayLengthText(SBPosition3 start, SBPosition3 end, std::string text = "");
    void displayDirectedCylinder(SBPosition3 start, SBPosition3 end);
    void displayDirectedCylinder(SBPosition3 start, SBPosition3 end, float* color, int radius);
    void displayDirectedCylinder(float* start, float* end, float* color, int radius);
    void displayPlane(SBVector3 vec, SBPosition3 shift);
    void displayOrthoPlane(SBVector3 vec, SBPosition3 shift);
    void displaySphere(SBPosition3 pos, float radius, ADNArray<float> color);
    void displayBasePairConnection(ADNPointer<ADNNucleotide> nt);
    void displayBaseVectors(ADNPointer<ADNNucleotide> nt, SBPosition3 pos);
    void displayText(SBPosition3 pos, std::string text = "");
    void displayTextBottomLeft(std::string text = "");
    void displayTriangleMesh(DASPolyhedron* polyhedron);

    void displayPart(ADNPointer<ADNPart> part, float basePairRadius = 1000.0f, float opaqueness = 0.5f);        ///< Display only the top scales of a part
    void displayGoldSphere(SBNodeIndexer goldAtoms);

};

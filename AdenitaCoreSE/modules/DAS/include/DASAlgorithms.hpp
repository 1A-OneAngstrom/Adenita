#pragma once

#include "ADNNanorobot.hpp"
#include "ADNConstants.hpp"
#include "DASBackToTheAtom.hpp"
#include "ADNBasicOperations.hpp"


class SB_EXPORT DASCrossover {

public:

    DASCrossover() = default;
    ~DASCrossover() = default;

    SBPointer<ADNNucleotide> firstNt_ = nullptr;
    SBPointer<ADNNucleotide> secondNt_ = nullptr;

    void CreateCrossover(SBPointer<ADNPart> part);
    bool IsScaffoldCrossover();

};

class SB_EXPORT DASAlgorithms {

public:

    DASAlgorithms() = default;
    ~DASAlgorithms() = default;

    static bool CheckCrossoverBetweenNucleotides(SBPointer<ADNNucleotide> ntFirst, SBPointer<ADNNucleotide> ntSecond, double angle_threshold, double dist_threshold);
    static std::vector<std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>>> DetectPossibleCrossovers(SBPointer<ADNPart> nanorobot, double angle_threshold, double dist_threshold);
    static std::vector<DASCrossover*> DetectPossibleCrossovers(SBPointer<ADNDoubleStrand> dsF, SBPointer<ADNDoubleStrand> dsS, double angle_threshold, double dist_threshold);
    static std::vector<std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>>> DetectCrossovers(SBPointer<ADNNanorobot> nanorobot);
    static void TraceScaffoldOrigami(SBPointer<ADNPart> part);
    //static void MinimizeDSconnections(SBPointer<ADNDoubleStrand> ds);
    static double CalculateTwistAngle(SBPointer<ADNDoubleStrand> ds);
    static std::vector<std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>>> GetLinkingNucleotides(SBPointer<ADNBaseSegment> bs);
    static SBQuantity::length CalculateAverageDistance(std::vector<std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>>> ntPairsList);

    //! Routes the scaffold through a DNA nanotube created with the nanotube editor
    /*!
      \param number of tubes
      \param length of the tubes
      \param a pointer to the part containing the nanotube
    */
    static void RouteNanotube(int numTubes, int lengthTubes, SBPointer<ADNPart> nanotube);

    //! Generates a sequence with a specific GC content and a maximum number of contiguous Gs
    static std::string GenerateSequence(double gcCont, int maxContGs, int sz);

};

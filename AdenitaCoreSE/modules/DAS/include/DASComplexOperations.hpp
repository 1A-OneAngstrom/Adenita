#pragma once

#include "ADNBasicOperations.hpp"
#include "DASCreator.hpp"


namespace DASOperations {

    struct MergePair {
        SBPointer<ADNSingleStrand> first = nullptr;
        SBPointer<ADNSingleStrand> second = nullptr;
        SBPointer<ADNPart> firstPart = nullptr;
        SBPointer<ADNPart> secondPart = nullptr;
    };

    struct Connections {
        MergePair stringPair;
        MergePair compStringPair;
    };

    SB_EXPORT Connections PrepareStrandsForConnection(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2, SBPointer<ADNNucleotide> nt1, SBPointer<ADNNucleotide> nt2);

    SB_EXPORT void CreateCrossover(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2, SBPointer<ADNNucleotide> nt1,
        SBPointer<ADNNucleotide> nt2, bool two = false, std::string seq = "");

    SB_EXPORT void AddComplementaryStrands(ADNNanorobot* nanorobot, SBPointerIndexer<ADNNucleotide> selectedNucleotides);

}

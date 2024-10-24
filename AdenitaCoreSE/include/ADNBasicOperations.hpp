#pragma once

#include "ADNPart.hpp"

// return types for operations
struct RTDoubleStrand {

    ADNPointer<ADNDoubleStrand> ds = nullptr;
    ADNPointer<ADNSingleStrand> ss1 = nullptr;
    ADNPointer<ADNSingleStrand> ss2 = nullptr;

};

namespace ADNBasicOperations {

    // Concatenate
    SB_EXPORT ADNPointer<ADNSingleStrand> MergeSingleStrands(ADNPointer<ADNPart> part1, ADNPointer<ADNPart> part2, ADNPointer<ADNSingleStrand> first_strand, ADNPointer<ADNSingleStrand> second_strand);
    SB_EXPORT ADNPointer<ADNDoubleStrand> MergeDoubleStrand(ADNPointer<ADNPart> part, ADNPointer<ADNDoubleStrand> first_strand, ADNPointer<ADNDoubleStrand> second_strand);
    SB_EXPORT ADNPointer<ADNPart> MergeParts(ADNPointer<ADNPart> part1, ADNPointer<ADNPart> part2);   ///< Merges ADNPart \p part2 into ADNPart \p part1

    // Extend
    SB_EXPORT CollectionMap<ADNNucleotide> AddNucleotidesThreePrime(ADNPointer<ADNPart> part, ADNPointer<ADNSingleStrand> ss, int number, SBVector3 dir);

    // Break
    SB_EXPORT std::pair<ADNPointer<ADNSingleStrand>, ADNPointer<ADNSingleStrand>> BreakSingleStrand(ADNPointer<ADNPart> part, ADNPointer<ADNNucleotide> nt);
    SB_EXPORT std::pair<ADNPointer<ADNDoubleStrand>, ADNPointer<ADNDoubleStrand>> BreakDoubleStrand(ADNPointer<ADNPart> part, ADNPointer<ADNBaseSegment> bs);

    // Delete
    SB_EXPORT std::pair<ADNPointer<ADNSingleStrand>, ADNPointer<ADNSingleStrand>> DeleteNucleotide(ADNPointer<ADNPart> part, ADNPointer<ADNNucleotide> nt);
    SB_EXPORT void DeleteNucleotideWithoutBreak(ADNPointer<ADNPart> part, ADNPointer<ADNNucleotide> nt);
    SB_EXPORT std::pair<ADNPointer<ADNDoubleStrand>, ADNPointer<ADNDoubleStrand>> DeleteBaseSegment(ADNPointer<ADNPart> part, ADNPointer<ADNBaseSegment> bs);
    //SB_EXPORT void DeleteBaseSegmentWithoutBreak(ADNPointer<ADNBaseSegment> bs);
    SB_EXPORT void DeleteSingleStrand(ADNPointer<ADNSingleStrand> ss);
    SB_EXPORT void DeleteDoubleStrand(ADNPointer<ADNDoubleStrand> ds);

    // Mutations
    SB_EXPORT void MutateNucleotide(ADNPointer<ADNNucleotide> nt, DNABlocks newType, bool changePair = true);
    SB_EXPORT void SetSingleStrandSequence(ADNPointer<ADNSingleStrand> ss, std::string seq, bool changePair = true, bool overwrite = true);
    SB_EXPORT void MutateBasePairIntoLoopPair(ADNPointer<ADNBaseSegment> bs);
    //! Shift start of the strand to the selected nucleotide and sequence.
    /*!
      \param a ADNPointer to the ADNNucleotide which should be the new 5' of its strand
      \param whether to keep the sequence as it was (reset it from new 5' on)
    */
    SB_EXPORT void SetStart(ADNPointer<ADNNucleotide> nucleotide, bool resetSequence = false);
    //! Moves an ADNDoubleStrand to another ADNPart
    /*!
      \param ADNPointer to the part that contains the double strand
      \param ADNPointer to the new part
      \param ADNPointer to the ADNDoubleStrand we want to move
    */
    SB_EXPORT void MoveStrand(ADNPointer<ADNPart> oldPart, ADNPointer<ADNPart> part, ADNPointer<ADNDoubleStrand> ds);
    //! Moves an ADNDoubleStrand to another ADNPart
    /*!
      \param ADNPointer to the part that contains the double strand
      \param ADNPointer to the new part
      \param ADNPointer to the ADNDoubleStrand we want to move
    */
    SB_EXPORT void MoveStrand(ADNPointer<ADNPart> oldPart, ADNPointer<ADNPart> part, ADNPointer<ADNSingleStrand> ss);

    // Geometric operations
    SB_EXPORT void TwistDoubleHelix(ADNPointer<ADNDoubleStrand> ds, double deg);
    SB_EXPORT void CenterPart(ADNPointer<ADNPart> part);
    SB_EXPORT SBPosition3 CalculateCenterOfMass(ADNPointer<ADNPart> part);

    // Ordering and helpers
    SB_EXPORT std::pair<ADNPointer<ADNNucleotide>, ADNPointer<ADNNucleotide>> OrderNucleotides(ADNPointer<ADNNucleotide> nt1, ADNPointer<ADNNucleotide> nt2);
    SB_EXPORT std::pair<ADNNucleotide::EndType, ADNPointer<ADNBaseSegment>> GetNextBaseSegment(ADNPointer<ADNNucleotide> nt);
    SB_EXPORT std::tuple<ADNPointer<ADNBaseSegment>, bool, bool, bool> GetBaseSegmentInfo(ADNPointer<ADNNucleotide> nt);
    SB_EXPORT void SetBackNucleotideIntoBaseSegment(ADNPointer<ADNNucleotide> nt, std::tuple<ADNPointer<ADNBaseSegment>, bool, bool, bool> info);

}

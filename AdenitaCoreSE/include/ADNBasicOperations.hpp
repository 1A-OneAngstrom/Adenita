#pragma once

#include "ADNPart.hpp"

// return types for operations
struct RTDoubleStrand {

    SBPointer<ADNDoubleStrand> ds = nullptr;
    SBPointer<ADNSingleStrand> ss1 = nullptr;
    SBPointer<ADNSingleStrand> ss2 = nullptr;

};

namespace ADNBasicOperations {

    // Concatenate
    SB_EXPORT SBPointer<ADNSingleStrand> MergeSingleStrands(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2, SBPointer<ADNSingleStrand> first_strand, SBPointer<ADNSingleStrand> second_strand);
    SB_EXPORT [[nodiscard]] SBPointer<ADNDoubleStrand> MergeDoubleStrand(SBPointer<ADNPart> part, SBPointer<ADNDoubleStrand> first_strand, SBPointer<ADNDoubleStrand> second_strand);
    SB_EXPORT [[nodiscard]] SBPointer<ADNPart> MergeParts(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2);   ///< Merges ADNPart \p part2 into ADNPart \p part1

    // Extend
    SB_EXPORT SBPointerIndexer<ADNNucleotide> AddNucleotidesThreePrime(SBPointer<ADNPart> part, SBPointer<ADNSingleStrand> ss, int number, SBVector3 dir);

    // Break
    SB_EXPORT std::pair<SBPointer<ADNSingleStrand>, SBPointer<ADNSingleStrand>> BreakSingleStrand(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt);
    SB_EXPORT std::pair<SBPointer<ADNDoubleStrand>, SBPointer<ADNDoubleStrand>> BreakDoubleStrand(SBPointer<ADNPart> part, SBPointer<ADNBaseSegment> bs);

    // Delete
    SB_EXPORT std::pair<SBPointer<ADNSingleStrand>, SBPointer<ADNSingleStrand>> DeleteNucleotide(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt);
    SB_EXPORT void DeleteNucleotideWithoutBreak(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt);
    SB_EXPORT std::pair<SBPointer<ADNDoubleStrand>, SBPointer<ADNDoubleStrand>> DeleteBaseSegment(SBPointer<ADNPart> part, SBPointer<ADNBaseSegment> bs);
    //SB_EXPORT void DeleteBaseSegmentWithoutBreak(SBPointer<ADNBaseSegment> bs);
    SB_EXPORT void DeleteSingleStrand(SBPointer<ADNSingleStrand> ss);
    SB_EXPORT void DeleteDoubleStrand(SBPointer<ADNDoubleStrand> ds);

    // Mutations
    SB_EXPORT void MutateNucleotide(SBPointer<ADNNucleotide> nt, DNABlocks newType, bool changePair = true);
    SB_EXPORT void SetSingleStrandSequence(SBPointer<ADNSingleStrand> ss, const std::string& seq, bool changePair = true, bool overwrite = true);
    SB_EXPORT void MutateBasePairIntoLoopPair(SBPointer<ADNBaseSegment> bs);
    //! Shift start of the strand to the selected nucleotide and sequence.
    /*!
      \param a SBPointer to the ADNNucleotide which should be the new 5' of its strand
      \param whether to keep the sequence as it was (reset it from new 5' on)
    */
    SB_EXPORT void SetStart(SBPointer<ADNNucleotide> nucleotide, bool resetSequence = false);
    //! Moves an ADNDoubleStrand to another ADNPart
    /*!
      \param SBPointer to the part that contains the double strand
      \param SBPointer to the new part
      \param SBPointer to the ADNDoubleStrand we want to move
    */
    SB_EXPORT void MoveStrand(SBPointer<ADNPart> oldPart, SBPointer<ADNPart> part, SBPointer<ADNDoubleStrand> ds);
    //! Moves an ADNDoubleStrand to another ADNPart
    /*!
      \param SBPointer to the part that contains the double strand
      \param SBPointer to the new part
      \param SBPointer to the ADNDoubleStrand we want to move
    */
    SB_EXPORT void MoveStrand(SBPointer<ADNPart> oldPart, SBPointer<ADNPart> part, SBPointer<ADNSingleStrand> ss);

    // Geometric operations
    SB_EXPORT void TwistDoubleHelix(SBPointer<ADNDoubleStrand> ds, double deg);
    SB_EXPORT void CenterPart(SBPointer<ADNPart> part);
    SB_EXPORT [[nodiscard]] SBPosition3 CalculateCenterOfMass(SBPointer<ADNPart> part);

    // Ordering and helpers
    SB_EXPORT [[nodiscard]] std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>> OrderNucleotides(SBPointer<ADNNucleotide> nt1, SBPointer<ADNNucleotide> nt2);
    SB_EXPORT [[nodiscard]] std::pair<ADNNucleotide::EndType, SBPointer<ADNBaseSegment>> GetNextBaseSegment(SBPointer<ADNNucleotide> nt);
    SB_EXPORT [[nodiscard]] std::tuple<SBPointer<ADNBaseSegment>, bool, bool, bool> GetBaseSegmentInfo(SBPointer<ADNNucleotide> nt);
    SB_EXPORT void SetBackNucleotideIntoBaseSegment(SBPointer<ADNNucleotide> nt, std::tuple<SBPointer<ADNBaseSegment>, bool, bool, bool> info);

}

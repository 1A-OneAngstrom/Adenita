#pragma once

#include "ADNConstants.hpp"
#include "ADNMixins.hpp"
#include "ADNPart.hpp"
#include "ADNConformations.hpp"

#define ADENITA_NANOROBOT_REGISTER_PARTS 0
#define ADENITA_NANOROBOT_REGISTER_CONFORMATIONS 0

/// ADNNanorobot: main interface to Adenita logical model.

class SB_EXPORT ADNNanorobot {//: public Nameable, public Positionable, public Orientable {

public:
  
    ADNNanorobot();                                                                                                                     ///< Default constructor
	ADNNanorobot(const ADNNanorobot &n);                                                                                                ///< Copy constructor
	~ADNNanorobot() = default;                                                                                                          ///< Default destructor

    ADNNanorobot&                                               operator=(const ADNNanorobot& other);                                   ///< Operator =

	void                                                        RegisterPart(SBPointer<ADNPart> part);                                 ///< Adds the ADNPart \p part to the internal part indexer
	void                                                        DeregisterPart(SBPointer<ADNPart> part);                               ///< Deletes the ADNPart \p part from the internal part indexer

    [[nodiscard]] int                                           GetNumberOfParts() const;                                               ///< Returns the number of parts
    [[nodiscard]] int                                           GetNumberOfDoubleStrands() const;                                       ///< Returns the number of double strands
    [[nodiscard]] int                                           GetNumberOfBaseSegments() const;                                        ///< Returns the number of base segments
    [[nodiscard]] int                                           GetNumberOfSingleStrands() const;                                       ///< Returns the number of single strands
    [[nodiscard]] int                                           GetNumberOfNucleotides() const;                                         ///< Returns the number of nucleotides

    [[nodiscard]] SBPointerIndexer<ADNPart>                        GetParts() const;                                                       ///< Returns all the registered ADNPart
    
    [[nodiscard]] SBPointerIndexer<ADNSingleStrand>                GetSingleStrands() const;                                               ///< Returns all the registered ADNSingleStrand

    [[nodiscard]] SBPointerIndexer<ADNPart>                        GetSelectedParts() const;                                               ///< Returns all currently selected ADNPart
    [[nodiscard]] SBPointerIndexer<ADNSingleStrand>                GetSelectedSingleStrands() const;                                       ///< Returns all currently selected ADNSingleStrand
    [[nodiscard]] SBPointerIndexer<ADNDoubleStrand>                GetSelectedDoubleStrands() const;                                       ///< Returns all currently selected ADNDoubleStrand
    [[nodiscard]] SBPointerIndexer<ADNBaseSegment>                 GetSelectedBaseSegmentsFromNucleotides() const;                         ///< Returns all currently selected ADNBaseSegment 
    [[nodiscard]] SBPointerIndexer<ADNNucleotide>                  GetSelectedNucleotides() const;                                         ///< Returns all currently selected ADNNucleotides
    
    [[nodiscard]] SBPointerIndexer<SBAtom>                         GetHighlightedAtoms() const;                                            ///< Returns all currently highlighted SBAtoms
    [[nodiscard]] SBPointerIndexer<ADNNucleotide>                  GetHighlightedNucleotides() const;                                      ///< Returns all currently highlighted ADNNucleotides
    [[nodiscard]] SBPointerIndexer<ADNBaseSegment>                 GetHighlightedBaseSegmentsFromNucleotides() const;                      ///< Returns all currently highlighted ADNBaseSegments from Nucleotides
    [[nodiscard]] SBPointerIndexer<ADNBaseSegment>                 GetHighlightedBaseSegments() const;                                     ///< Returns all currently highlighted ADNBaseSegments
    [[nodiscard]] SBPointerIndexer<ADNDoubleStrand>                GetHighlightedDoubleStrands() const;                                    ///< Returns all currently highlighted ADNDoubleStrand
    
    [[nodiscard]] SBPointerIndexer<ADNConformation>                GetConformations() const;                                               ///< Return all conformations
    
    void                                                        RegisterConformation(SBPointer<ADNConformation> conformation);         ///< Register a conformation

    [[nodiscard]] SBPosition3                                   GetNucleotideBackbonePosition(SBPointer<ADNConformation> conformation, SBPointer<ADNNucleotide> nucleotide) const;  ///< Return the position of the backbone of the nucleotide \p nucleotide in the conformation \p conformation
    [[nodiscard]] SBPosition3                                   GetNucleotideSideChainPosition(SBPointer<ADNConformation> conformation, SBPointer<ADNNucleotide> nucleotide) const; ///< Return the position of the side chain of the nucleotide \p nucleotide in the conformation \p conformation

    [[nodiscard]] SBIAPosition3                                 GetBoundingBox(SBPointerIndexer<ADNPart> parts) const;                     ///< bounding box

private:

#if ADENITA_NANOROBOT_REGISTER_PARTS
    SBPointerIndexer<ADNPart>                                      partsIndex_;
#endif
#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    SBPointerIndexer<ADNConformation>                              conformationsIndex_;
#endif

    unsigned int                                                partId_{ 1 };                                                            ///< part id only for naming

};

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

	void                                                        RegisterPart(ADNPointer<ADNPart> part);                                 ///< Adds the ADNPart \p part to the internal part indexer
	void                                                        DeregisterPart(ADNPointer<ADNPart> part);                               ///< Deletes the ADNPart \p part from the internal part indexer

    int                                                         GetNumberOfParts() const;                                               ///< Returns the number of parts
    int                                                         GetNumberOfDoubleStrands() const;                                       ///< Returns the number of double strands
    int                                                         GetNumberOfBaseSegments() const;                                        ///< Returns the number of base segments
    int                                                         GetNumberOfSingleStrands() const;                                       ///< Returns the number of single strands
    int                                                         GetNumberOfNucleotides() const;                                         ///< Returns the number of nucleotides

    CollectionMap<ADNPart>                                      GetParts() const;                                                       ///< Returns all the registered ADNPart
    
    CollectionMap<ADNSingleStrand>                              GetSingleStrands() const;                                               ///< Returns all the registered ADNSingleStrand

    CollectionMap<ADNPart>                                      GetSelectedParts() const;                                               ///< Returns all currently selected ADNPart
    CollectionMap<ADNSingleStrand>                              GetSelectedSingleStrands() const;                                       ///< Returns all currently selected ADNSingleStrand
    CollectionMap<ADNDoubleStrand>                              GetSelectedDoubleStrands() const;                                       ///< Returns all currently selected ADNDoubleStrand
    CollectionMap<ADNBaseSegment>                               GetSelectedBaseSegmentsFromNucleotides() const;                         ///< Returns all currently selected ADNBaseSegment 
    CollectionMap<ADNNucleotide>                                GetSelectedNucleotides() const;                                         ///< Returns all currently selected ADNNucleotides
    
    CollectionMap<SBAtom>                                       GetHighlightedAtoms() const;                                            ///< Returns all currently highlighted SBAtoms
    CollectionMap<ADNNucleotide>                                GetHighlightedNucleotides() const;                                      ///< Returns all currently highlighted ADNNucleotides
    CollectionMap<ADNBaseSegment>                               GetHighlightedBaseSegmentsFromNucleotides() const;                      ///< Returns all currently highlighted ADNBaseSegments from Nucleotides
    CollectionMap<ADNBaseSegment>                               GetHighlightedBaseSegments() const;                                     ///< Returns all currently highlighted ADNBaseSegments
    CollectionMap<ADNDoubleStrand>                              GetHighlightedDoubleStrands() const;                                    ///< Returns all currently highlighted ADNDoubleStrand
    
    CollectionMap<ADNConformation>                              GetConformations() const;                                               ///< Return all conformations
    
    void                                                        RegisterConformation(ADNPointer<ADNConformation> conformation);         ///< Register a conformation

    SBPosition3                                                 GetNucleotideBackbonePosition(ADNPointer<ADNConformation> conformation, ADNPointer<ADNNucleotide> nucleotide) const;  ///< Return the position of the backbone of the nucleotide \p nucleotide in the conformation \p conformation
    SBPosition3                                                 GetNucleotideSideChainPosition(ADNPointer<ADNConformation> conformation, ADNPointer<ADNNucleotide> nucleotide) const; ///< Return the position of the side chain of the nucleotide \p nucleotide in the conformation \p conformation

    std::pair<SBPosition3, SBPosition3>                         GetBoundingBox(CollectionMap<ADNPart> parts) const;                     ///< bounding box

private:

#if ADENITA_NANOROBOT_REGISTER_PARTS
    CollectionMap<ADNPart>                                      partsIndex_;
#endif
#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    CollectionMap<ADNConformation>                              conformationsIndex_;
#endif

    unsigned int                                                partId_ = 1;                                                            ///< part id only for naming

};

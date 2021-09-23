#pragma once

#include "ADNConstants.hpp"
#include "ADNMixins.hpp"
#include "ADNPart.hpp"
#include "ADNConformations.hpp"

#define ADENITA_NANOROBOT_REGISTER_PARTS 0
#define ADENITA_NANOROBOT_REGISTER_CONFORMATIONS 1

/// ADNNanorobot: main interface to Adenita logical model.

class ADNNanorobot {//: public Nameable, public Positionable, public Orientable {

public:
  
    ADNNanorobot() {};// : Nameable(), Positionable(), Orientable() {};                                                                       ///< Default constructor
	ADNNanorobot(const ADNNanorobot &n);                                                                                                ///< Copy constructor
	~ADNNanorobot() = default;                                                                                                          ///< Default destructor

    ADNNanorobot&                                               operator=(const ADNNanorobot& other);                                   ///< Operator =

	void                                                        RegisterPart(ADNPointer<ADNPart> part);                                 ///< Adds the ADNPart \p part to the internal part indexer
	void                                                        DeregisterPart(ADNPointer<ADNPart> part);                               ///< Deletes the ADNPart \p part from the internal part indexer

    int                                                         GetNumberOfParts();                                                     ///< Returns the number of parts
    int                                                         GetNumberOfDoubleStrands();                                             ///< Returns the number of double strands
    int                                                         GetNumberOfBaseSegments();                                              ///< Returns the number of base segments
    int                                                         GetNumberOfSingleStrands();                                             ///< Returns the number of single strands
    int                                                         GetNumberOfNucleotides();                                               ///< Returns the number of nucleotides

    CollectionMap<ADNPart>                                      GetParts() const;                                                       ///< Returns all the registered ADNPart
    
    CollectionMap<ADNSingleStrand>                              GetSingleStrands() const;                                               ///< Returns all the registered ADNSingleStrand

    CollectionMap<ADNPart>                                      GetSelectedParts();                                                     ///< Returns all currently selected ADNPart
    CollectionMap<ADNSingleStrand>                              GetSelectedSingleStrands();                                             ///< Returns all currently selected ADNSingleStrand
    CollectionMap<ADNDoubleStrand>                              GetSelectedDoubleStrands();                                             ///< Returns all currently selected ADNDoubleStrand
    CollectionMap<ADNBaseSegment>                               GetSelectedBaseSegmentsFromNucleotides();                               ///< Returns all currently selected ADNBaseSegment 
    CollectionMap<ADNNucleotide>                                GetSelectedNucleotides();                                               ///< Returns all currently selected ADNNucleotides
    
    CollectionMap<SBAtom>                                       GetHighlightedAtoms();                                                  ///< Returns all currently highlighted SBAtoms
    CollectionMap<ADNNucleotide>                                GetHighlightedNucleotides();                                            ///< Returns all currently highlighted ADNNucleotides
    CollectionMap<ADNBaseSegment>                               GetHighlightedBaseSegmentsFromNucleotides();                            ///< Returns all currently highlighted ADNBaseSegments from Nucleotides
    CollectionMap<ADNBaseSegment>                               GetHighlightedBaseSegments();                                           ///< Returns all currently highlighted ADNBaseSegments
    CollectionMap<ADNDoubleStrand>                              GetHighlightedDoubleStrands();                                          ///< Returns all currently highlighted ADNDoubleStrand
    
    CollectionMap<ADNConformation>                              GetConformations();                                                     ///< Return all conformations
    
    void                                                        RegisterConformation(ADNPointer<ADNConformation> conformation);         ///< Register a conformation

    SBPosition3                                                 GetNucleotideBackbonePosition(ADNConformation conformation, ADNPointer<ADNNucleotide> nucleotide);  ///< Return the position of the backbone of the nucleotide \p nucleotide in the conformation \p conformation
    SBPosition3                                                 GetNucleotideSidechainPosition(ADNConformation conformation, ADNPointer<ADNNucleotide> nucleotide); ///< Return the position of the side chain of the nucleotide \p nucleotide in the conformation \p conformation

    std::pair<SBPosition3, SBPosition3>                         GetBoundingBox(CollectionMap<ADNPart> parts);                           ///< bounding box

private:

#if ADENITA_NANOROBOT_REGISTER_PARTS
    CollectionMap<ADNPart>                                      partsIndex_;
#endif
#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    CollectionMap<ADNConformation>                              conformationsIndex_;
#endif

    unsigned int                                                partId_ = 1;                                                            ///< part id only for naming

};

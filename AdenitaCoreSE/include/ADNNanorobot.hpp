#pragma once

#include "ADNConstants.hpp"
#include "ADNMixins.hpp"
#include "ADNPart.hpp"
#include "ADNConformations.hpp"

#define ADENITA_NANOROBOT_REGISTER_PARTS 0
#define ADENITA_NANOROBOT_REGISTER_CONFORMATIONS 0

/// ADNNanorobot: main interface to Adenita logical model.

class ADNNanorobot : public Nameable, public Positionable, public Orientable {

public:
  
    ADNNanorobot() : Nameable(), Positionable(), Orientable() {};                                                                       ///< Default constructor
	ADNNanorobot(const ADNNanorobot &n);                                                                                                ///< Copy constructor
	~ADNNanorobot() = default;                                                                                                          ///< Default destructor

    ADNNanorobot&                                               operator=(const ADNNanorobot& other);                                   ///< Operator =

	void                                                        RegisterPart(ADNPointer<ADNPart> part);                                 ///< Adds the ADNPart \p part to the internal part indexer
	void                                                        DeregisterPart(ADNPointer<ADNPart> part);                               ///< Deletes the ADNPart \p part from the internal part indexer

    int                                                         GetNumberOfDoubleStrands();                                             ///< Returns the number of double strands
    int                                                         GetNumberOfBaseSegments();                                              ///< Returns the number of base segments
    int                                                         GetNumberOfSingleStrands();                                             ///< Returns the number of single strands
    int                                                         GetNumberOfNucleotides();                                               ///< Returns the number of nucleotides

    CollectionMap<ADNPart>                                      GetParts() const;                                                       ///< Returns all the registered ADNPart
    
    ADNPointer<ADNPart>                                         GetPart(ADNPointer<ADNSingleStrand> singleStrand);                      ///< Returns a pointer to the part to which an ADNSingleStrand \p singleStrand belongs
    ADNPointer<ADNPart>                                         GetPart(ADNPointer<ADNDoubleStrand> doubleStrand);                      ///< Returns a pointer to the part to which an ADNDoubleStrand \p doubleStrand belongs
    
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

    CollectionMap<ADNSingleStrand>                              GetSingleStrands(ADNPointer<ADNPart> part);                             ///< Return the single strands of the ADNPart \p part
    
    void                                                        RemoveSingleStrand(ADNPointer<ADNSingleStrand> singleStrand);           ///< Removes the single strand \p singleStrand from the ADNPart to which it belongs
    void                                                        RemoveDoubleStrand(ADNPointer<ADNDoubleStrand> doubleStrand);           ///< Removes the double strand \p doubleStrand from the ADNPart to which it belongs
    
    void                                                        AddSingleStrand(ADNPointer<ADNSingleStrand> singleStrand, ADNPointer<ADNPart> part); ///< Adds a single strand to the ADNPart \p part
    
    CollectionMap<ADNSingleStrand>                              GetScaffolds(ADNPointer<ADNPart> part);                                 ///< Return the scaffolds of the ADNPart \p part
    CollectionMap<ADNDoubleStrand>                              GetDoubleStrands(ADNPointer<ADNPart> part);                             ///< Return the double strands of the ADNPart \p part
    
    ADNPointer<ADNDoubleStrand>                                 GetDoubleStrand(ADNPointer<ADNNucleotide> nucleotide);                  ///< Return the double strands to which the nucleotide belongs

    CollectionMap<ADNNucleotide>                                GetSingleStrandNucleotides(ADNPointer<ADNSingleStrand> singleStrand);   ///< Return the nucleotides of the single strand \p singleStrand
    ADNPointer<ADNNucleotide>                                   GetSingleStrandFivePrime(ADNPointer<ADNSingleStrand> singleStrand);     ///< Return the five prime nucleotide of the single strand \p singleStrand 

    bool                                                        IsScaffold(ADNPointer<ADNSingleStrand> singleStrand);                   ///< Return whether a single strand is a scaffold

    End                                                         GetNucleotideEnd(ADNPointer<ADNNucleotide> nucleotide);                 ///< Return if the nucleotide is 5', 3', neither or both
    
    ADNPointer<ADNNucleotide>                                   GetNucleotideNext(ADNPointer<ADNNucleotide> nucleotide, bool circular = false);   ///< Return the nucleotide next on the single strand
    ADNPointer<ADNNucleotide>                                   GetNucleotidePair(ADNPointer<ADNNucleotide> nucleotide);                ///< Return a nucleotide's pair
    
    SBPosition3                                                 GetNucleotidePosition(ADNPointer<ADNNucleotide> nucleotide);            ///< Return the position of a nucleotide
    SBPosition3                                                 GetNucleotideBackbonePosition(ADNPointer<ADNNucleotide> nucleotide);    ///< Return the position of the backbone of a nucleotide
    SBPosition3                                                 GetNucleotideSidechainPosition(ADNPointer<ADNNucleotide> nucleotide);   ///< Return the position of the sidechain of a nucleotide

    void                                                        HideCenterAtoms(ADNPointer<ADNNucleotide> nucleotide);                  ///< Hides center "mock" atom

    
    CollectionMap<ADNConformation>                              GetConformations();                                                     ///< Return all conformations
    
    void                                                        RegisterConformation(ADNPointer<ADNConformation> conformation);         ///< Register a conformation

    SBPosition3                                                 GetNucleotideBackbonePosition(ADNConformation conformation, ADNPointer<ADNNucleotide> nucleotide);  ///< Return the position of the backbone of the nucleotide \p nucleotide in the conformation \p conformation
    SBPosition3                                                 GetNucleotideSidechainPosition(ADNConformation conformation, ADNPointer<ADNNucleotide> nucleotide); ///< Return the position of the side chain of the nucleotide \p nucleotide in the conformation \p conformation

    unsigned int                                                UseSingleStrandId();
    unsigned int                                                UseDoubleStrandId();
    unsigned int                                                UsePartId();

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

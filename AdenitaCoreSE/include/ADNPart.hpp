#pragma once

#include "ADNConstants.hpp"
#include "ADNModel.hpp"
#include "ADNCell.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNSingleStrand.hpp"
#include "ADNNucleotide.hpp"
#include "ADNAtom.hpp"
#include "ADNBaseSegment.hpp"

#include "SBStructuralModel.hpp"
#include "SBMStructuralModelNodeRoot.hpp"
#include "SBIAVector3.hpp"

#define ADENITA_ADNPART_REGISTER_STRANDS 0
// not checked for Base segments since they are used in Daedalus and in other places by making pairs of base segments from different strands
#define ADENITA_ADNPART_REGISTER_BASESEGMENTS 1
#define ADENITA_ADNPART_REGISTER_NUCLEOTIDES 0
#define ADENITA_ADNPART_REGISTER_ATOMS 0

class SB_EXPORT ADNPart : public SBStructuralModel {

  SB_CLASS

public:

    ADNPart();
    ADNPart(const ADNPart &n);
    ~ADNPart() = default;

    ADNPart&                                                    operator=(const ADNPart& other);

    virtual void                                                serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
    virtual void                                                unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

    void                                                        RegisterDoubleStrand(ADNPointer<ADNDoubleStrand> ds);   ///< Adds a double strand to the part
    void                                                        RegisterBaseSegmentEnd(ADNPointer<ADNDoubleStrand> ds, ADNPointer<ADNBaseSegment> bs, bool addToDs = true);
    void                                                        RegisterSingleStrand(ADNPointer<ADNSingleStrand> ss);   ///< Adds a single strand to the part
    void                                                        RegisterNucleotideThreePrime(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNNucleotide> nt, bool addToSs = true);
    void                                                        RegisterNucleotideFivePrime(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNNucleotide> nt, bool addToSs = true);
    void                                                        RegisterNucleotide(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNNucleotide> nt, ADNPointer<ADNNucleotide> ntNext, bool addToSs = true);
    void                                                        RegisterAtom(ADNPointer<ADNNucleotide> nt, NucleotideGroup g, ADNPointer<ADNAtom> at, bool create = false);
    void                                                        RegisterAtom(ADNPointer<ADNBaseSegment> bs, ADNPointer<ADNAtom> at, bool create = false);

    unsigned int                                                GetBaseSegmentIndex(ADNPointer<ADNBaseSegment> bs) const;

    CollectionMap<ADNSingleStrand>                              GetSingleStrands() const;                                               ///< Return a pointer indexer of single strands of the ADNPart part
    CollectionMap<ADNDoubleStrand>                              GetDoubleStrands() const;                                               ///< Return a pointer indexer of double strands of the ADNPart part
    CollectionMap<ADNBaseSegment>                               GetBaseSegments(CellType celltype = CellType::ALL) const;
    CollectionMap<ADNSingleStrand>                              GetScaffolds() const;                                                   ///< Return the scaffolds of the ADNPart part
    CollectionMap<ADNNucleotide>                                GetNucleotides(CellType celltype = CellType::ALL) const;
    CollectionMap<ADNAtom>                                      GetAtoms() const;

    int                                                         GetNumberOfDoubleStrands() const;
    int                                                         getNumberOfDoubleStrands() const;
    int                                                         GetNumberOfSingleStrands() const;
    int                                                         getNumberOfSingleStrands() const;
    int                                                         GetNumberOfNucleotides() const;
    int                                                         getNumberOfNucleotides() const;
    int                                                         GetNumberOfAtoms() const;
    int                                                         getNumberOfAtoms() const;
    int                                                         GetNumberOfBaseSegments() const;
    int                                                         getNumberOfBaseSegments() const;

    void                                                        DeregisterSingleStrand(ADNPointer<ADNSingleStrand> ss, bool removeFromParent = true, bool removeFromIndex = true);
    void                                                        DeregisterNucleotide(ADNPointer<ADNNucleotide> nt, bool removeFromSs = true, bool removeFromBs = true, bool removeFromIndex = true);
    void                                                        DeregisterDoubleStrand(ADNPointer<ADNDoubleStrand> ds, bool removeFromParent = true, bool removeFromIndex = true);
    void                                                        DeregisterBaseSegment(ADNPointer<ADNBaseSegment> bs, bool removeFromDs = true, bool removeFromIndex = true);
    void                                                        DeregisterAtom(ADNPointer<ADNAtom> atom, bool removeFromAtom = true);

    bool                                                        isLoadedViaSAMSON() const noexcept;
    void                                                        setLoadedViaSAMSON(bool l);

    const SBIAPosition3&                                        GetBoundingBox() const;
    void                                                        ResetBoundingBox();

protected:

private:

    // inside these pointers ids are unique
#if ADENITA_ADNPART_REGISTER_ATOMS
    CollectionMap<ADNAtom>                                      atomsIndex_;
#endif
#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    CollectionMap<ADNNucleotide>                                nucleotidesIndex_;
#endif
    CollectionMap<ADNBaseSegment>                               baseSegmentsIndex_;
#if ADENITA_ADNPART_REGISTER_STRANDS
    CollectionMap<ADNSingleStrand>                              singleStrandsIndex_;
    CollectionMap<ADNDoubleStrand>                              doubleStrandsIndex_;
#endif

    bool                                                        loadedViaSAMSONFlag{ false };

    // ids are just for naming
    unsigned int                                                nucleotideId_{ 1 };
    unsigned int                                                singleStrandId_{ 1 };
    unsigned int                                                doubleStrandId_{ 1 };

    void                                                        SetBoundingBox(ADNPointer<ADNNucleotide> newNt);
    void                                                        SetBoundingBox(ADNPointer<ADNBaseSegment> newBs);
    void                                                        InitBoundingBox();

    SBIAPosition3                                               boundingBox;

};

SB_REGISTER_TARGET_TYPE(ADNPart, "ADNPart", "D3809709-A2EA-DDC1-9753-A40B2B9DE57E");
SB_DECLARE_BASE_TYPE(ADNPart, SBStructuralModel);

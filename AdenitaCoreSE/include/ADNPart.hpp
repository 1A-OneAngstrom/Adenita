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

    void                                                        RegisterDoubleStrand(SBPointer<ADNDoubleStrand> ds);   ///< Adds a double strand to the part
    void                                                        RegisterBaseSegmentEnd(SBPointer<ADNDoubleStrand> ds, SBPointer<ADNBaseSegment> bs, bool addToDs = true);
    void                                                        RegisterSingleStrand(SBPointer<ADNSingleStrand> ss);   ///< Adds a single strand to the part
    void                                                        RegisterNucleotideThreePrime(SBPointer<ADNSingleStrand> ss, SBPointer<ADNNucleotide> nt, bool addToSs = true);
    void                                                        RegisterNucleotideFivePrime(SBPointer<ADNSingleStrand> ss, SBPointer<ADNNucleotide> nt, bool addToSs = true);
    void                                                        RegisterNucleotide(SBPointer<ADNSingleStrand> ss, SBPointer<ADNNucleotide> nt, SBPointer<ADNNucleotide> ntNext, bool addToSs = true);
    void                                                        RegisterAtom(SBPointer<ADNNucleotide> nt, NucleotideGroup g, SBPointer<ADNAtom> at, bool create = false);
    void                                                        RegisterAtom(SBPointer<ADNBaseSegment> bs, SBPointer<ADNAtom> at, bool create = false);

    [[nodiscard]] unsigned int                                  GetBaseSegmentIndex(SBPointer<ADNBaseSegment> bs) const;

    [[nodiscard]] SBPointerIndexer<ADNSingleStrand>             GetSingleStrands() const;                                               ///< Return a pointer indexer of single strands of the ADNPart part
    [[nodiscard]] SBPointerIndexer<ADNDoubleStrand>             GetDoubleStrands() const;                                               ///< Return a pointer indexer of double strands of the ADNPart part
    [[nodiscard]] SBPointerIndexer<ADNBaseSegment>              GetBaseSegments(CellType celltype = CellType::ALL) const;
    [[nodiscard]] SBPointerIndexer<ADNSingleStrand>             GetScaffolds() const;                                                   ///< Return the scaffolds of the ADNPart part
    [[nodiscard]] SBPointerIndexer<ADNNucleotide>               GetNucleotides(CellType celltype = CellType::ALL) const;
    [[nodiscard]] SBPointerIndexer<ADNAtom>                     GetAtoms() const;

    [[nodiscard]] unsigned int                                  GetNumberOfDoubleStrands() const;
    [[nodiscard]] unsigned int                                  getNumberOfDoubleStrands() const;
    [[nodiscard]] unsigned int                                  GetNumberOfSingleStrands() const;
    [[nodiscard]] unsigned int                                  getNumberOfSingleStrands() const;
    [[nodiscard]] unsigned int                                  GetNumberOfNucleotides() const;
    [[nodiscard]] unsigned int                                  getNumberOfNucleotides() const;
    [[nodiscard]] unsigned int                                  GetNumberOfAtoms() const;
    [[nodiscard]] unsigned int                                  getNumberOfAtoms() const;
    [[nodiscard]] unsigned int                                  GetNumberOfBaseSegments() const;
    [[nodiscard]] unsigned int                                  getNumberOfBaseSegments() const;

    void                                                        DeregisterSingleStrand(SBPointer<ADNSingleStrand> ss, bool removeFromParent = true, bool removeFromIndex = true);
    void                                                        DeregisterNucleotide(SBPointer<ADNNucleotide> nt, bool removeFromSs = true, bool removeFromBs = true, bool removeFromIndex = true);
    void                                                        DeregisterDoubleStrand(SBPointer<ADNDoubleStrand> ds, bool removeFromParent = true, bool removeFromIndex = true);
    void                                                        DeregisterBaseSegment(SBPointer<ADNBaseSegment> bs, bool removeFromDs = true, bool removeFromIndex = true);
    void                                                        DeregisterAtom(SBPointer<ADNAtom> atom, bool removeFromAtom = true);

    bool                                                        isLoadedViaSAMSON() const noexcept;
    void                                                        setLoadedViaSAMSON(bool l);

    [[nodiscard]] const SBIAPosition3&                          GetBoundingBox() const;
    void                                                        ResetBoundingBox();

private:

    // inside these pointers ids are unique
#if ADENITA_ADNPART_REGISTER_ATOMS
    SBPointerIndexer<ADNAtom>                                      atomsIndex_;
#endif
#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    SBPointerIndexer<ADNNucleotide>                                nucleotidesIndex_;
#endif
    SBPointerIndexer<ADNBaseSegment>                               baseSegmentsIndex_;
#if ADENITA_ADNPART_REGISTER_STRANDS
    SBPointerIndexer<ADNSingleStrand>                              singleStrandsIndex_;
    SBPointerIndexer<ADNDoubleStrand>                              doubleStrandsIndex_;
#endif

    bool                                                        loadedViaSAMSONFlag{ false };

    // ids are just for naming
    unsigned int                                                nucleotideId_{ 1 };
    unsigned int                                                singleStrandId_{ 1 };
    unsigned int                                                doubleStrandId_{ 1 };

    void                                                        SetBoundingBox(SBPointer<ADNNucleotide> newNt);
    void                                                        SetBoundingBox(SBPointer<ADNBaseSegment> newBs);
    void                                                        InitBoundingBox();

    SBIAPosition3                                               boundingBox;

};

SB_REGISTER_TARGET_TYPE(ADNPart, "ADNPart", "D3809709-A2EA-DDC1-9753-A40B2B9DE57E");
SB_DECLARE_BASE_TYPE(ADNPart, SBStructuralModel);

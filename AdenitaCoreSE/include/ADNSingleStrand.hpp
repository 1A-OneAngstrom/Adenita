#pragma once

#include "SBChain.hpp"

#include "ADNMixins.hpp"

class ADNNucleotide;
class ADNPart;

class ADNSingleStrand : public SBChain {

    SB_CLASS

public:

    ADNSingleStrand() : SBChain() {}
    //ADNSingleStrand(int numNts);
    //ADNSingleStrand(std::vector<ADNPointer<ADNNucleotide>> nts);
    ADNSingleStrand(const ADNSingleStrand& other);
    ~ADNSingleStrand() = default;

    ADNSingleStrand&                                            operator=(const ADNSingleStrand& other);

    void                                                        serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
    void                                                        unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

    ADNPointer<ADNPart>                                         GetPart();                                                              ///< Returns a pointer to the part to which this single strand belongs

    ADNPointer<ADNNucleotide>                                   GetFivePrime();                                                         ///< Returns the five prime nucleotide of the single strand
    SBNode*                                                     getFivePrime() const;                                                   ///< Returns the five prime nucleotide of the single strand
    ADNPointer<ADNNucleotide>                                   GetThreePrime();                                                        ///< Returns the three prime nucleotide of the single strand
    SBNode*                                                     getThreePrime() const;                                                  ///< Returns the three prime nucleotide of the single strand

    // if using these functions, make sure nucleotides are properly added
    void                                                        SetFivePrime(ADNPointer<ADNNucleotide> nucleotide);
    void                                                        SetThreePrime(ADNPointer<ADNNucleotide> nucleotide);

    void                                                        IsScaffold(bool b);
    bool                                                        IsScaffold() const;                                                     ///< Returns whether a single strand is a scaffold
    bool                                                        getIsScaffold() const;
    void                                                        setIsScaffold(bool b);

    void                                                        IsCircular(bool c);
    bool                                                        IsCircular() const;
    bool                                                        getIsCircular() const;
    void                                                        setIsCircular(bool b);

    CollectionMap<ADNNucleotide>                                GetNucleotides() const;                                                 ///< Returns the nucleotides of the single strand
    int                                                         getNumberOfNucleotides() const;

    void                                                        AddNucleotideThreePrime(ADNPointer<ADNNucleotide> nucleotide);          ///< Adds the nucleotide to the three prime end
    void                                                        AddNucleotideFivePrime(ADNPointer<ADNNucleotide> nucleotide);           ///< Adds the nucleotide to the five prime end
    void                                                        AddNucleotide(ADNPointer<ADNNucleotide> nucleotide, ADNPointer<ADNNucleotide> nextNucleotide);  ///< add nucleotide at any position

    void                                                        ShiftStart(ADNPointer<ADNNucleotide> nucleotide, bool shiftSeq = false);///< Shift start of the strand to the selected nucleotide and sequence.

    std::string                                                 GetSequence() const;
    std::string                                                 getSequence() const;
    std::string                                                 GetSequenceWithTags() const;

    double                                                      GetGCContent() const;                                                   ///< Returns GC content as a % in this strand
    double                                                      getGCContent() const;                                                   ///< Returns GC content as a % in this strand

    void                                                        SetSequence(std::string seq);
    void                                                        setSequence(std::string seq);
    void                                                        SetDefaultName();

private:

    bool                                                        scaffoldFlag = false;
    bool                                                        circularFlag = false;

    ADNPointer<ADNNucleotide>                                   fivePrimeNucleotide = nullptr;
    ADNPointer<ADNNucleotide>                                   threePrimeNucleotide = nullptr;

};

SB_REGISTER_TARGET_TYPE(ADNSingleStrand, "ADNSingleStrand", "8EB118A4-A8BF-19F5-5171-C68582AC6262");
SB_DECLARE_BASE_TYPE(ADNSingleStrand, SBChain);

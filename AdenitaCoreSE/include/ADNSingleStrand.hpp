#pragma once

#include "SBChain.hpp"

#include "ADNMixins.hpp"

class ADNNucleotide;
class ADNPart;

class SB_EXPORT ADNSingleStrand : public SBChain {

    SB_CLASS

public:

    ADNSingleStrand() : SBChain() {}
    //ADNSingleStrand(int numNts);
    //ADNSingleStrand(std::vector<ADNPointer<ADNNucleotide>> nts);
    ADNSingleStrand(const ADNSingleStrand& other);
    ~ADNSingleStrand() = default;

    ADNSingleStrand&                                            operator=(const ADNSingleStrand& other);

    virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
    virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

    [[nodiscard]] ADNPointer<ADNPart>                           GetPart() const;                                                        ///< Returns a pointer to the part to which this single strand belongs

    [[nodiscard]] ADNPointer<ADNNucleotide>                     GetFivePrime() const;                                                   ///< Returns the five prime nucleotide of the single strand
    [[nodiscard]] SBNode*                                       getFivePrime() const;                                                   ///< Returns the five prime nucleotide of the single strand
    [[nodiscard]] ADNPointer<ADNNucleotide>                     GetThreePrime() const;                                                  ///< Returns the three prime nucleotide of the single strand
    [[nodiscard]] SBNode*                                       getThreePrime() const;                                                  ///< Returns the three prime nucleotide of the single strand

    // if using these functions, make sure nucleotides are properly added
    void                                                        SetFivePrime(ADNPointer<ADNNucleotide> nucleotide);
    void                                                        SetThreePrime(ADNPointer<ADNNucleotide> nucleotide);

    [[nodiscard]] bool                                          IsScaffold() const;                                                     ///< Returns whether a single strand is a scaffold
    [[nodiscard]] bool                                          getScaffoldFlag() const;
    void                                                        setScaffoldFlag(bool b);

    [[nodiscard]] bool                                          IsCircular() const;
    [[nodiscard]] bool                                          getCircularFlag() const;
    void                                                        setCircularFlag(bool b);

    [[nodiscard]] CollectionMap<ADNNucleotide>                  GetNucleotides() const;                                                 ///< Returns the nucleotides of the single strand
    [[nodiscard]] int                                           getNumberOfNucleotides() const;

    void                                                        AddNucleotideThreePrime(ADNPointer<ADNNucleotide> nucleotide);          ///< Adds the nucleotide to the three prime end
    void                                                        AddNucleotideFivePrime(ADNPointer<ADNNucleotide> nucleotide);           ///< Adds the nucleotide to the five prime end
    void                                                        AddNucleotide(ADNPointer<ADNNucleotide> nucleotide, ADNPointer<ADNNucleotide> nextNucleotide);  ///< add nucleotide at any position

    void                                                        ShiftStart(ADNPointer<ADNNucleotide> nucleotide, bool shiftSeq = false);///< Shift start of the strand to the selected nucleotide and sequence.

    [[nodiscard]] std::string                                   GetSequence() const;
    [[nodiscard]] std::string                                   getSequence() const;
    [[nodiscard]] std::string                                   GetSequenceWithTags() const;

    [[nodiscard]] double                                        GetGCContent() const;                                                   ///< Returns GC content as a % in this strand
    [[nodiscard]] double                                        getGCContent() const;                                                   ///< Returns GC content as a % in this strand

    void                                                        SetSequence(std::string seq);
    void                                                        setSequence(std::string seq);
    void                                                        SetDefaultName();

private:

    bool                                                        scaffoldFlag{ false };
    bool                                                        circularFlag{ false };

    ADNPointer<ADNNucleotide>                                   fivePrimeNucleotide{ nullptr };
    ADNPointer<ADNNucleotide>                                   threePrimeNucleotide{ nullptr };

};

SB_REGISTER_TARGET_TYPE(ADNSingleStrand, "ADNSingleStrand", "8EB118A4-A8BF-19F5-5171-C68582AC6262");
SB_DECLARE_BASE_TYPE(ADNSingleStrand, SBChain);

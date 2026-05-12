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
    //ADNSingleStrand(std::vector<SBPointer<ADNNucleotide>> nts);
    ADNSingleStrand(const ADNSingleStrand& other);
    ~ADNSingleStrand() = default;

    ADNSingleStrand&                                            operator=(const ADNSingleStrand& other);

    virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
    virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

    [[nodiscard]] SBPointer<ADNPart>                           GetPart() const;                                                        ///< Returns a pointer to the part to which this single strand belongs

    [[nodiscard]] SBPointer<ADNNucleotide>                     GetFivePrime() const;                                                   ///< Returns the five prime nucleotide of the single strand
    [[nodiscard]] SBNode*                                       getFivePrime() const;                                                   ///< Returns the five prime nucleotide of the single strand
    [[nodiscard]] SBPointer<ADNNucleotide>                     GetThreePrime() const;                                                  ///< Returns the three prime nucleotide of the single strand
    [[nodiscard]] SBNode*                                       getThreePrime() const;                                                  ///< Returns the three prime nucleotide of the single strand

    // if using these functions, make sure nucleotides are properly added
    void                                                        SetFivePrime(SBPointer<ADNNucleotide> nucleotide);
    void                                                        SetThreePrime(SBPointer<ADNNucleotide> nucleotide);

    [[nodiscard]] bool                                          IsScaffold() const;                                                     ///< Returns whether a single strand is a scaffold
    [[nodiscard]] bool                                          getScaffoldFlag() const;
    void                                                        setScaffoldFlag(bool b);

    // Circular single strands keep a linear 5' -> 3' child order with an
    // arbitrary break. Use ADNNucleotide::GetNext(true) / GetPrev(true) to
    // traverse across the metadata-only closure.
    [[nodiscard]] bool                                          IsCircular() const;
    [[nodiscard]] bool                                          getCircularFlag() const;
    void                                                        setCircularFlag(bool b);

    [[nodiscard]] SBPointerIndexer<ADNNucleotide>                  GetNucleotides() const;                                                 ///< Returns the nucleotides of the single strand
    [[nodiscard]] int                                           getNumberOfNucleotides() const;

    void                                                        AddNucleotideThreePrime(SBPointer<ADNNucleotide> nucleotide);          ///< Adds the nucleotide to the three prime end
    void                                                        AddNucleotideFivePrime(SBPointer<ADNNucleotide> nucleotide);           ///< Adds the nucleotide to the five prime end
    void                                                        AddNucleotide(SBPointer<ADNNucleotide> nucleotide, SBPointer<ADNNucleotide> nextNucleotide);  ///< add nucleotide at any position

    void                                                        ShiftStart(SBPointer<ADNNucleotide> nucleotide, bool shiftSeq = false);///< Shift start of the strand to the selected nucleotide and sequence.

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

    SBPointer<ADNNucleotide>                                   fivePrimeNucleotide{ nullptr };
    SBPointer<ADNNucleotide>                                   threePrimeNucleotide{ nullptr };

};

SB_REGISTER_TARGET_TYPE(ADNSingleStrand, "ADNSingleStrand", "8EB118A4-A8BF-19F5-5171-C68582AC6262");
SB_DECLARE_BASE_TYPE(ADNSingleStrand, SBChain);

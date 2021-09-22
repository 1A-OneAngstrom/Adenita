#pragma once

#include "SBChain.hpp"

#include "ADNMixins.hpp"

class ADNNucleotide;

class ADNSingleStrand : public SBChain {

    SB_CLASS

public:

    ADNSingleStrand() : SBChain() {}
    //ADNSingleStrand(int numNts);
    //ADNSingleStrand(std::vector<ADNPointer<ADNNucleotide>> nts);
    ADNSingleStrand(const ADNSingleStrand& other);
    ~ADNSingleStrand() = default;

    ADNSingleStrand& operator=(const ADNSingleStrand& other);

    void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
    void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

    std::string const& GetName() const;
    void SetName(const std::string& name);

    ADNPointer<ADNNucleotide> GetFivePrime();
    SBNode* getFivePrime() const;
    ADNPointer<ADNNucleotide> GetThreePrime();
    SBNode* getThreePrime() const;

    ADNPointer<ADNNucleotide> GetNthNucleotide(int n);

    // if using these functions, make sure nucleotides are properly added
    void SetFivePrime(ADNPointer<ADNNucleotide> nt);
    void SetThreePrime(ADNPointer<ADNNucleotide> nt);

    void IsScaffold(bool b);
    bool IsScaffold() const;
    bool getIsScaffold() const;
    void setIsScaffold(bool b);
    void IsCircular(bool c);
    bool IsCircular() const;
    bool getIsCircular() const;
    void setIsCircular(bool b);
    int getNumberOfNucleotides() const;
    CollectionMap<ADNNucleotide> GetNucleotides() const;
    //ADNPointer<ADNNucleotide> GetNucleotide(unsigned int id) const;
    void AddNucleotideThreePrime(ADNPointer<ADNNucleotide> nt);  // add nucleotide to the three prime end
    void AddNucleotideFivePrime(ADNPointer<ADNNucleotide> nt);  // add nucleotide to the five prime end
    void AddNucleotide(ADNPointer<ADNNucleotide> nt, ADNPointer<ADNNucleotide> nextNt);  // add nucleotide at any position

    //! Shift start of the strand to the selected nucleotide and sequence.
    /*!
      \param a ADNPointer to the ADNNucleotide which should be the new 5' of its strand
      \param whether to keep the sequence as it was (reset it from new 5' on)
    */
    void ShiftStart(ADNPointer<ADNNucleotide> nucleotide, bool shiftSeq = false);
    /**
    * Returns the sequence of the strand
    * \param sequence from 5' to 3'
    */
    std::string GetSequence() const;
    std::string getSequence() const;
    std::string GetSequenceWithTags() const;
    /**
    * Returns GC content as a % in this strand
    */
    double GetGCContent() const;
    double getGCContent() const;

    void SetSequence(std::string seq);
    void setSequence(std::string seq);
    void SetDefaultName();

private:

    bool isScaffold_ = false;
    bool isCircular_ = false;
    ADNPointer<ADNNucleotide> fivePrime_;
    ADNPointer<ADNNucleotide> threePrime_;

};

SB_REGISTER_TARGET_TYPE(ADNSingleStrand, "ADNSingleStrand", "8EB118A4-A8BF-19F5-5171-C68582AC6262");
SB_DECLARE_BASE_TYPE(ADNSingleStrand, SBChain);

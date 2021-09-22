#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNMixins.hpp"

class ADNBaseSegment;
class ADNNucleotide;

class ADNLoop : public SBStructuralGroup {

	SB_CLASS

public:

	ADNLoop() : SBStructuralGroup() {}
	/**
	* Destructor for ANTLoop.
	* Deletes references in other ANTLoop, but not on the ANTSingleStrand.
	*/
	~ADNLoop() = default;

	void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	void SetStart(ADNPointer<ADNNucleotide> nt);
	ADNPointer<ADNNucleotide> GetStart();
	SBNode* getStartNucleotide() const;
	void SetEnd(ADNPointer<ADNNucleotide> nt);
	ADNPointer<ADNNucleotide> GetEnd();
	SBNode* getEndNucleotide() const;
	void SetBaseSegment(ADNPointer<ADNBaseSegment> bs, bool setPositions = false);
	int getNumberOfNucleotides() const;
	std::string getLoopSequence() const;

	CollectionMap<ADNNucleotide> GetNucleotides() const;

	void AddNucleotide(ADNPointer<ADNNucleotide> nt);
	void RemoveNucleotide(ADNPointer<ADNNucleotide> nt);
	bool IsEmpty() const;

private:

	ADNPointer<ADNNucleotide> startNt_ = nullptr;
	ADNPointer<ADNNucleotide> endNt_ = nullptr;
	CollectionMap<ADNNucleotide> nucleotides_;

};

SB_REGISTER_TARGET_TYPE(ADNLoop, "ADNLoop", "8531205A-01B2-C438-1E26-A50699CA6678");
SB_DECLARE_BASE_TYPE(ADNLoop, SBStructuralGroup);

#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNNucleotide.hpp"
#include "ADNMixins.hpp"

class ADNBaseSegment;

class ADNLoop : public SBStructuralGroup {

	SB_CLASS

public:

	ADNLoop() : SBStructuralGroup() {}
	/**
	* Destructor for ANTLoop.
	* Deletes references in other ANTLoop, but not on the ANTSingleStrand.
	*/
	~ADNLoop() = default;

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	void														SetStart(ADNPointer<ADNNucleotide> nucleotide);							///< Set the start nucleotide
	ADNPointer<ADNNucleotide>									GetStart() const;
	SBNode*														getStartNucleotide() const;
	void														SetEnd(ADNPointer<ADNNucleotide> nucleotide);							///< Set the end nucleotide
	ADNPointer<ADNNucleotide>									GetEnd() const;
	SBNode*														getEndNucleotide() const;

	void														SetBaseSegment(ADNPointer<ADNBaseSegment> baseSegment, bool setPositions = false);

	std::string													getLoopSequence() const;

	int															getNumberOfNucleotides() const;
	CollectionMap<ADNNucleotide>								GetNucleotides() const;

	void														AddNucleotide(ADNPointer<ADNNucleotide> nucleotide);
	void														RemoveNucleotide(ADNPointer<ADNNucleotide> nucleotide);

	bool														IsEmpty() const;

private:

	ADNPointer<ADNNucleotide>									startNucleotide = nullptr;
	ADNPointer<ADNNucleotide>									endNucleotide = nullptr;

	CollectionMap<ADNNucleotide>								nucleotides_;

};

SB_REGISTER_TARGET_TYPE(ADNLoop, "ADNLoop", "8531205A-01B2-C438-1E26-A50699CA6678");
SB_DECLARE_BASE_TYPE(ADNLoop, SBStructuralGroup);

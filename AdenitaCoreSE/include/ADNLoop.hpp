#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNNucleotide.hpp"
#include "ADNMixins.hpp"

class ADNBaseSegment;

class SB_EXPORT ADNLoop : public SBStructuralGroup {

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

	void														SetStart(SBPointer<ADNNucleotide> nucleotide);							///< Set the start nucleotide
	[[nodiscard]] SBPointer<ADNNucleotide>						GetStart() const;
	[[nodiscard]] SBNode*										getStartNucleotide() const;
	void														SetEnd(SBPointer<ADNNucleotide> nucleotide);							///< Set the end nucleotide
	[[nodiscard]] SBPointer<ADNNucleotide>						GetEnd() const;
	[[nodiscard]] SBNode*										getEndNucleotide() const;

	void														SetBaseSegment(SBPointer<ADNBaseSegment> baseSegment, bool setPositions = false);

	[[nodiscard]] std::string									getLoopSequence() const;

	[[nodiscard]] int											getNumberOfNucleotides() const;
	[[nodiscard]] SBPointerIndexer<ADNNucleotide>					GetNucleotides() const;

	void														AddNucleotide(SBPointer<ADNNucleotide> nucleotide);
	void														RemoveNucleotide(SBPointer<ADNNucleotide> nucleotide);

	[[nodiscard]] bool											IsEmpty() const;

private:

	SBPointer<ADNNucleotide>									startNucleotide = nullptr;
	SBPointer<ADNNucleotide>									endNucleotide = nullptr;

	SBPointerIndexer<ADNNucleotide>								nucleotides_;

};

SB_REGISTER_TARGET_TYPE(ADNLoop, "ADNLoop", "8531205A-01B2-C438-1E26-A50699CA6678");
SB_DECLARE_BASE_TYPE(ADNLoop, SBStructuralGroup);

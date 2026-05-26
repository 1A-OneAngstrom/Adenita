#pragma once

#include "SBResidue.hpp"

#include "ADNMixins.hpp"

class ADNAtom;
class ADNBaseSegment;
class ADNBackbone;
class ADNSidechain;
class ADNSingleStrand;
class ADNDoubleStrand;

class SB_EXPORT ADNNucleotide : public SBResidue, public PositionableSB, public Orientable {

	SB_CLASS

public:

	// the End type of the nucleotide: 5', 3', both, none
	enum class EndType {
		NotEnd = 0,
		ThreePrime = 3,
		FivePrime = 5,
		FiveAndThreePrime = 8,
	};

	ADNNucleotide() : PositionableSB(), SBResidue(), Orientable() {}
	ADNNucleotide(const ADNNucleotide& other);
	~ADNNucleotide() = default;

	ADNNucleotide&												operator=(const ADNNucleotide& other);

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	void														setNucleotideType(DNABlocks t);
	[[nodiscard]] DNABlocks										getNucleotideType() const;

	[[nodiscard]] std::string									getNucleotideTypeString() const;
	[[nodiscard]] std::string									getOneLetterNucleotideTypeString() const;

	void														SetPair(SBPointer<ADNNucleotide> nucleotide);
	[[nodiscard]] SBPointer<ADNNucleotide>						GetPair() const;														///< Return a nucleotide's pair
	[[nodiscard]] SBNode*										getPair() const;
	void														disconnectPair(SBPointer<ADNNucleotide> nucleotide);					///< Disconnects a pairing to the nucleotide \p nucleotide
	void														disconnectPair();														///< Disconnects a pairing between this nucleotide and its pair nucleotide

	[[nodiscard]] SBPointer<ADNNucleotide>						GetPrev(bool checkCircular = false) const;								///< Return the nucleotide previous on the single strand
	[[nodiscard]] SBNode*										getPrev() const;
	[[nodiscard]] SBPointer<ADNNucleotide>						GetNext(bool checkCircular = false) const;								///< Return the nucleotide next on the single strand
	[[nodiscard]] SBNode*										getNext() const;
	[[nodiscard]] SBPointer<ADNSingleStrand>					GetStrand() const;
	[[nodiscard]] SBNode*										getSingleStrand() const;

	void														SetBaseSegment(SBPointer<ADNBaseSegment> bs);
	[[nodiscard]] SBPointer<ADNBaseSegment>					GetBaseSegment() const;
	[[nodiscard]] SBNode*										getBaseSegment() const;
	[[nodiscard]] std::string									getBaseSegmentTypeString() const;

	[[nodiscard]] SBPointer<ADNDoubleStrand>					GetDoubleStrand() const;												///< Return the double strands to which the nucleotide belongs

	[[nodiscard]] std::string									getEndTypeString() const;
	[[nodiscard]] ADNNucleotide::EndType						getEndType() const;														///< Return if the nucleotide is 5', 3', neither or both
	void														setEndType(ADNNucleotide::EndType type);
	[[nodiscard]] bool											isEndTypeNucleotide() const;

	void														Init();
	[[nodiscard]] SBPointer<ADNBackbone>						GetBackbone() const;
	[[nodiscard]] SBPointer<ADNSidechain>						GetSidechain() const;

	void														SetSidechainPosition(const SBPosition3& pos);
	[[nodiscard]] const SBPosition3&								GetSidechainPosition() const;											///< Return the position of the sidechain of a nucleotide
	void														SetBackbonePosition(const SBPosition3& pos);
	[[nodiscard]] const SBPosition3&								GetBackbonePosition() const;											///< Return the position of the backbone of a nucleotide

	// overload position to retrieve it from bb and sc
	void														SetPosition(const SBPosition3& pos);									///< Set the center of a nucleotide by translating its backbone and sidechain centers
	[[nodiscard]] SBPosition3									GetPosition() const;													///< Return the position of a nucleotide

	void														addAtom(NucleotideGroup g, SBPointer<ADNAtom> a);
	void														deleteAtom(NucleotideGroup g, SBPointer<ADNAtom> a);
	[[nodiscard]] SBPointerIndexer<ADNAtom>						GetAtoms() const;
	int															getNumberOfAtoms() const;
	[[nodiscard]] SBPointerIndexer<ADNAtom>						GetAtomsByName(const std::string& name) const;
	void														HideCenterAtoms();														///< Hides center "mock" atom
	[[nodiscard]] SBPointer<ADNAtom>							GetBackboneCenterAtom() const;
	[[nodiscard]] SBPointer<ADNAtom>							GetSidechainCenterAtom() const;

	// Local base is always the standard basis
	[[nodiscard]] ublas::matrix<double>							GetGlobalBasisTransformation() const;
	[[nodiscard]] bool											GlobalBaseIsSet() const;

	//! check where in the base segment is the nucleotide located
	[[nodiscard]] bool											IsLeft();
	[[nodiscard]] bool											IsRight();

	//! tagging
	[[nodiscard]] std::string									getTag() const;
	void														setTag(std::string t);
	bool														hasTag() const;

	/// \name Debugging
	//@{

	virtual void												print(unsigned int offset = 0) const override;							///< Prints debugging information

	//@}

private:

	SBPointer<ADNNucleotide>								pairNucleotide;
	SBPointer<ADNBaseSegment>								baseSegment;  // base segment to which the nucleotide belongs to

	ADNNucleotide::EndType										endType{ ADNNucleotide::EndType::NotEnd };

	std::string													tag;

};

SB_REGISTER_TARGET_TYPE(ADNNucleotide, "ADNNucleotide", "26603E7A-7792-0C83-B1D5-6C1D222B3379");
SB_DECLARE_BASE_TYPE(ADNNucleotide, SBResidue);

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

	void														serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void														unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	void														setNucleotideType(DNABlocks t);
	DNABlocks													getNucleotideType() const;

	std::string													getNucleotideTypeString() const;

	void														SetPair(ADNPointer<ADNNucleotide> nucleotide);
	ADNPointer<ADNNucleotide>									GetPair() const;														///< Return a nucleotide's pair
	SBNode*														getPair() const;
	void														disconnectPair(ADNPointer<ADNNucleotide> nucleotide);					///< Disconnects a pairing to the nucleotide \p nucleotide

	ADNPointer<ADNNucleotide>									GetPrev(bool checkCircular = false) const;								///< Return the nucleotide previous on the single strand
	SBNode*														getPrev() const;
	ADNPointer<ADNNucleotide>									GetNext(bool checkCircular = false) const;								///< Return the nucleotide next on the single strand
	SBNode*														getNext() const;
	ADNPointer<ADNSingleStrand>									GetStrand() const;
	SBNode*														getSingleStrand() const;

	void														SetBaseSegment(ADNPointer<ADNBaseSegment> bs);
	ADNPointer<ADNBaseSegment>									GetBaseSegment();
	SBNode*														getBaseSegment() const;
	std::string													getBaseSegmentTypeString() const;

	ADNPointer<ADNDoubleStrand>                                 GetDoubleStrand();														///< Return the double strands to which the nucleotide belongs

	std::string													getEndTypeString() const;
	ADNNucleotide::EndType										getEndType();																///< Return if the nucleotide is 5', 3', neither or both
	void														setEndType(ADNNucleotide::EndType type);
	bool														isEndTypeNucleotide();

	void														Init();
	ADNPointer<ADNBackbone>										GetBackbone() const;
	ADNPointer<ADNSidechain>									GetSidechain() const;

	void														SetSidechainPosition(Position3D pos);
	Position3D													GetSidechainPosition() const;											///< Return the position of the sidechain of a nucleotide
	void														SetBackbonePosition(Position3D pos);
	Position3D													GetBackbonePosition() const;											///< Return the position of the backbone of a nucleotide

	// overload position to retrieve it from bb and sc
	Position3D													GetPosition() const;													///< Return the position of a nucleotide

	void														AddAtom(NucleotideGroup g, ADNPointer<ADNAtom> a);
	void														DeleteAtom(NucleotideGroup g, ADNPointer<ADNAtom> a);
	CollectionMap<ADNAtom>										GetAtoms();
	int															getNumberOfAtoms() const;
	CollectionMap<ADNAtom>										GetAtomsByName(std::string name);
	void														HideCenterAtoms();														///< Hides center "mock" atom
	ADNPointer<ADNAtom>											GetBackboneCenterAtom();
	ADNPointer<ADNAtom>											GetSidechainCenterAtom();

	// Local base is always the standard basis
	ublas::matrix<double>										GetGlobalBasisTransformation();
	bool														GlobalBaseIsSet();

	//! check where in the base segment is the nucleotide located
	bool														IsLeft();
	bool														IsRight();

	//! tagging
	std::string													getTag() const;
	void														setTag(std::string t);
	bool														hasTag();

private:

	ADNWeakPointer<ADNNucleotide>								pairNucleotide;
	ADNWeakPointer<ADNBaseSegment>								baseSegment;  // base segment to which the nucleotide belongs to

	ADNNucleotide::EndType										endType = ADNNucleotide::EndType::NotEnd;

	std::string													tag;

};

SB_REGISTER_TARGET_TYPE(ADNNucleotide, "ADNNucleotide", "26603E7A-7792-0C83-B1D5-6C1D222B3379");
SB_DECLARE_BASE_TYPE(ADNNucleotide, SBResidue);

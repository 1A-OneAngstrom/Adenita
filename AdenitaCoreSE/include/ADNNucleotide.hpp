#pragma once

#include "SBResidue.hpp"

#include "ADNMixins.hpp"

class ADNAtom;
class ADNBaseSegment;
class ADNBackbone;
class ADNSidechain;
class ADNSingleStrand;

// move inside the class and rename to EndType
enum class End {
	ThreePrime = 3,
	FivePrime = 5,
	NotEnd = 0,
	FiveAndThreePrime = 8,
};

class ADNNucleotide : public PositionableSB, public SBResidue, public Orientable {

	SB_CLASS

public:

	ADNNucleotide() : PositionableSB(), SBResidue(), Orientable() {}
	ADNNucleotide(const ADNNucleotide& other);
	~ADNNucleotide() = default;

	ADNNucleotide& operator=(const ADNNucleotide& other);

	void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	void SetType(DNABlocks t);
	DNABlocks GetType();
	DNABlocks getNucleotideType() const;

	void SetPair(ADNPointer<ADNNucleotide> nt);
	ADNPointer<ADNNucleotide> GetPair() const;
	SBNode* getPair() const;

	ADNPointer<ADNNucleotide> GetPrev(bool checkCircular = false) const;
	SBNode* getPrev() const;
	ADNPointer<ADNNucleotide> GetNext(bool checkCircular = false) const;
	SBNode* getNext() const;
	ADNPointer<ADNSingleStrand> GetStrand() const;
	SBNode* getSingleStrand() const;

	void SetBaseSegment(ADNPointer<ADNBaseSegment> bs);
	ADNPointer<ADNBaseSegment> GetBaseSegment();
	SBNode* getBaseSegment() const;
	std::string getBaseSegmentType() const;
	std::string getEndType() const;

	End GetEnd();
	void SetEnd(End e);
	bool IsEnd();

	void Init();
	ADNPointer<ADNBackbone> GetBackbone() const;
	void SetBackbone(ADNPointer<ADNBackbone> bb);
	ADNPointer<ADNSidechain> GetSidechain() const;
	void SetSidechain(ADNPointer<ADNSidechain> sc);

	void SetSidechainPosition(Position3D pos);
	Position3D GetSidechainPosition() const;
	void SetBackbonePosition(Position3D pos);
	Position3D GetBackbonePosition() const;

	// overload position to retrieve it from bb and sc
	Position3D GetPosition() const;

	void AddAtom(NucleotideGroup g, ADNPointer<ADNAtom> a);
	void DeleteAtom(NucleotideGroup g, ADNPointer<ADNAtom> a);
	CollectionMap<ADNAtom> GetAtoms();
	CollectionMap<ADNAtom> GetAtomsByName(std::string name);
	void HideCenterAtoms();
	ADNPointer<ADNAtom> GetBackboneCenterAtom();
	ADNPointer<ADNAtom> GetSidechainCenterAtom();

	// Local base is always the standard basis */
	ublas::matrix<double> GetGlobalBasisTransformation();
	bool GlobalBaseIsSet();

	//! check where in the base segment is the nucleotide located
	bool IsLeft();
	bool IsRight();

	//! tagging
	std::string getTag() const;
	void setTag(std::string t);
	bool hasTag();

private:

	ADNWeakPointer<ADNNucleotide> pair_;
	ADNWeakPointer<ADNBaseSegment> bs_;  // base segment to which the nucleotide belongs to

	End end_;

	std::string tag_;

};

SB_REGISTER_TARGET_TYPE(ADNNucleotide, "ADNNucleotide", "26603E7A-7792-0C83-B1D5-6C1D222B3379");
SB_DECLARE_BASE_TYPE(ADNNucleotide, SBResidue);

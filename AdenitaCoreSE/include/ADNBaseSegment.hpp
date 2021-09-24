#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNCell.hpp"

class ADNDoubleStrand;

class ADNBaseSegment : public SBStructuralGroup, public PositionableSB, public Orientable {

	SB_CLASS

public:

	ADNBaseSegment() : PositionableSB(), SBStructuralGroup(), Orientable() {}
	ADNBaseSegment(CellType cellType);
	ADNBaseSegment(const ADNBaseSegment& other);
	~ADNBaseSegment() = default;

	ADNBaseSegment&												operator=(const ADNBaseSegment& other);

	void														serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void														unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	void														SetNumber(int n);
	int															GetNumber() const;
	void														setNumber(int n);
	int															getNumber() const;

	ADNPointer<ADNBaseSegment>									GetPrev(bool checkCircular = false) const;
	ADNPointer<ADNBaseSegment>									GetNext(bool checkCircular = false) const;
	
	bool														IsEnd() const;															///< True if it's the first or last base segment
	bool														IsFirst() const;														///< True if it's the first base segment
	bool														IsLast() const;															///< True if it's the first base segment

	ADNPointer<ADNDoubleStrand>									GetDoubleStrand() const;
	SBNode*														getDoubleStrand() const;
	unsigned int												getNumberOfNucleotides() const;
	CollectionMap<ADNNucleotide>								GetNucleotides() const;

	void														SetCell(ADNCell* c);  // we use raw pointers so subclassing will work
	ADNPointer<ADNCell>											GetCell() const;
	CellType													GetCellType() const;
	std::string													getCellTypeString() const;
	void														RemoveNucleotide(ADNPointer<ADNNucleotide> nt);

	//! check if left or right in the base segment
	bool														IsLeft(ADNPointer<ADNNucleotide> nt);
	bool														IsRight(ADNPointer<ADNNucleotide> nt);

private:

	ADNPointer<ADNCell>											cell_ = nullptr;
	int															number_ = -1;  // number of the base in the double strand

};

SB_REGISTER_TARGET_TYPE(ADNBaseSegment, "ADNBaseSegment", "114D0E73-D768-0DF5-3C1A-11569CB91F25");
SB_DECLARE_BASE_TYPE(ADNBaseSegment, SBStructuralGroup);

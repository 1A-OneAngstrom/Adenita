#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNCell.hpp"

class ADNDoubleStrand;

class SB_EXPORT ADNBaseSegment : public SBStructuralGroup, public PositionableSB, public Orientable {

	SB_CLASS

public:

	ADNBaseSegment() : PositionableSB(), SBStructuralGroup(), Orientable() {}
	ADNBaseSegment(CellType cellType);
	ADNBaseSegment(const ADNBaseSegment& other);
	~ADNBaseSegment() = default;

	ADNBaseSegment&												operator=(const ADNBaseSegment& other);

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	void														SetNumber(int n);
	[[nodiscard]] int											GetNumber() const;
	void														setNumber(int n);
	[[nodiscard]] int											getNumber() const;

	[[nodiscard]] ADNPointer<ADNBaseSegment>					GetPrev(bool checkCircular = false) const;
	[[nodiscard]] ADNPointer<ADNBaseSegment>					GetNext(bool checkCircular = false) const;
	
	[[nodiscard]] bool											IsEnd() const;															///< True if it's the first or last base segment
	[[nodiscard]] bool											IsFirst() const;														///< True if it's the first base segment
	[[nodiscard]] bool											IsLast() const;															///< True if it's the first base segment

	[[nodiscard]] ADNPointer<ADNDoubleStrand>					GetDoubleStrand() const;
	[[nodiscard]] SBNode*										getDoubleStrand() const;
	[[nodiscard]] unsigned int									getNumberOfNucleotides() const;
	[[nodiscard]] CollectionMap<ADNNucleotide>					GetNucleotides() const;
	void														RemoveNucleotide(ADNPointer<ADNNucleotide> nt);

	[[nodiscard]] void											SetCell(ADNCell* c);  // we use raw pointers so subclassing will work
	[[nodiscard]] ADNPointer<ADNCell>							GetCell() const;
	[[nodiscard]] CellType										GetCellType() const;
	[[nodiscard]] std::string									getCellTypeString() const;

	//! check if left or right in the base segment
	[[nodiscard]] bool											IsLeft(ADNPointer<ADNNucleotide> nt) const;
	[[nodiscard]] bool											IsRight(ADNPointer<ADNNucleotide> nt) const;

private:

	ADNPointer<ADNCell>											cell_{ nullptr };
	int															number_{ -1 };  // number of the base in the double strand

};

SB_REGISTER_TARGET_TYPE(ADNBaseSegment, "ADNBaseSegment", "114D0E73-D768-0DF5-3C1A-11569CB91F25");
SB_DECLARE_BASE_TYPE(ADNBaseSegment, SBStructuralGroup);

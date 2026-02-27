#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNMixins.hpp"
#include "ADNLoop.hpp"

class ADNNucleotide;

enum class CellType {
	Undefined = -1,
	BasePair = 0,
	SkipPair = 1,
	LoopPair = 2,
	ALL = 99,
};

class SB_EXPORT ADNCell : public SBStructuralGroup {

	SB_CLASS

public:

	ADNCell() : SBStructuralGroup() {}
	virtual ~ADNCell() {}

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	[[nodiscard]] static std::string							getCellTypeString(CellType type);
	[[nodiscard]] std::string									getCellTypeString() const;

	virtual CellType											GetCellType() const { return CellType::Undefined; }						///< Returns the cell type

	virtual void												RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {}
	[[nodiscard]] virtual CollectionMap<ADNNucleotide>			GetNucleotides() const { return CollectionMap<ADNNucleotide>(); }
	[[nodiscard]] unsigned int									getNumberOfNucleotides() const { return GetNucleotides().size(); }
	virtual bool												IsLeft(ADNPointer<ADNNucleotide> nt) const { return false; }
	virtual bool												IsRight(ADNPointer<ADNNucleotide> nt) const { return false; }
	
};

SB_REGISTER_TARGET_TYPE(ADNCell, "ADNCell", "E6BFD315-2734-B4A6-5808-E784AA4102EF");
SB_DECLARE_BASE_TYPE(ADNCell, SBStructuralGroup);

class SB_EXPORT ADNBasePair : public ADNCell {

	SB_CLASS

public:

	ADNBasePair() = default;
	~ADNBasePair() = default;

	void														serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;			///< Serializes the node
	void														unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;				///< Unserializes the node

	[[nodiscard]] CellType										GetCellType() const override { return CellType::BasePair; }

	[[nodiscard]] ADNPointer<ADNNucleotide>						GetLeftNucleotide() const;
	[[nodiscard]] SBNode*										getLeftNucleotide() const;
	void														SetLeftNucleotide(ADNPointer<ADNNucleotide> nt);
	[[nodiscard]] ADNPointer<ADNNucleotide>						GetRightNucleotide() const;
	[[nodiscard]] SBNode*										getRightNucleotide() const;
	void														SetRightNucleotide(ADNPointer<ADNNucleotide> nt);
	void														SetRemainingNucleotide(ADNPointer<ADNNucleotide> nt);
	void														AddPair(ADNPointer<ADNNucleotide> left, ADNPointer<ADNNucleotide> right);
	void														PairNucleotides();
	void														RemoveNucleotide(ADNPointer<ADNNucleotide> nt) override;
	[[nodiscard]] CollectionMap<ADNNucleotide>					GetNucleotides() const override;
	bool														IsLeft(ADNPointer<ADNNucleotide> nt) const override;
	bool														IsRight(ADNPointer<ADNNucleotide> nt) const override;

private:

	ADNPointer<ADNNucleotide>									leftNucleotide = nullptr;
	ADNPointer<ADNNucleotide>									rightNucleotide = nullptr;

};

SB_REGISTER_TARGET_TYPE(ADNBasePair, "ADNBasePair", "71C5049C-EC51-8DC5-15EF-1525E4DBAB42");
SB_DECLARE_BASE_TYPE(ADNBasePair, ADNCell);

class SB_EXPORT ADNSkipPair : public ADNCell {

	SB_CLASS

public:

	ADNSkipPair() = default;
	~ADNSkipPair() = default;

	void														serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;			///< Serializes the node
	void														unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;				///< Unserializes the node

	[[nodiscard]] CellType										GetCellType() const override { return CellType::SkipPair; }

	void														RemoveNucleotide(ADNPointer<ADNNucleotide> nt) override;

};

SB_REGISTER_TARGET_TYPE(ADNSkipPair, "ADNSkipPair", "65441545-3022-773B-49A5-FF39A89AE754");
SB_DECLARE_BASE_TYPE(ADNSkipPair, ADNCell);


class SB_EXPORT ADNLoopPair : public ADNCell {

	SB_CLASS

public:

	ADNLoopPair() = default;
	~ADNLoopPair() = default;

	void														serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;			///< Serializes the node
	void														unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;				///< Unserializes the node

	[[nodiscard]] CellType										GetCellType() const override { return CellType::LoopPair; }

	[[nodiscard]] ADNPointer<ADNLoop>							GetLeftLoop() const;
	[[nodiscard]] SBNode*										getLeftLoop() const;
	void														SetLeftLoop(ADNPointer<ADNLoop> lp);
	[[nodiscard]] ADNPointer<ADNLoop>							GetRightLoop() const;
	[[nodiscard]] SBNode*										getRightLoop() const;
	void														SetRightLoop(ADNPointer<ADNLoop> lp);

	void														RemoveNucleotide(ADNPointer<ADNNucleotide> nt) override;
	[[nodiscard]] CollectionMap<ADNNucleotide>					GetNucleotides() const override;

	[[nodiscard]] bool											IsLeft(ADNPointer<ADNNucleotide> nt) const override;
	[[nodiscard]] bool											IsRight(ADNPointer<ADNNucleotide> nt) const override;

private:

	ADNPointer<ADNLoop>											leftLoop = nullptr;
	ADNPointer<ADNLoop>											rightLoop = nullptr;

};

SB_REGISTER_TARGET_TYPE(ADNLoopPair, "ADNLoopPair", "F9CB2D19-D635-F494-D87D-EC619763E577");
SB_DECLARE_BASE_TYPE(ADNLoopPair, ADNCell);

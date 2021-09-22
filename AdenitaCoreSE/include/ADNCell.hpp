#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNMixins.hpp"

class ADNLoop;
class ADNNucleotide;

enum class CellType {
	Undefined = -1,
	BasePair = 0,
	SkipPair = 1,
	LoopPair = 2,
	ALL = 99,
};

class ADNCell : public SBStructuralGroup {

	SB_CLASS

public:

	ADNCell() : SBStructuralGroup() {}
	virtual ~ADNCell() {}

	virtual void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	virtual void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node
	
	virtual CellType GetType() const { return CellType::Undefined; }
	virtual void RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {}
	virtual CollectionMap<ADNNucleotide> GetNucleotides() const { return CollectionMap<ADNNucleotide>(); }
	virtual unsigned int getNumberOfNucleotides() const { return GetNucleotides().size(); }
	virtual bool IsLeft(ADNPointer<ADNNucleotide> nt) const { return false; }  // samson doesn't like abstract classes
	virtual bool IsRight(ADNPointer<ADNNucleotide> nt) const { return false; }
	
};

SB_REGISTER_TARGET_TYPE(ADNCell, "ADNCell", "E6BFD315-2734-B4A6-5808-E784AA4102EF");
SB_DECLARE_BASE_TYPE(ADNCell, SBStructuralGroup);

class ADNBasePair : public ADNCell {

	SB_CLASS

public:

	ADNBasePair() = default;
	~ADNBasePair() = default;

	void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	CellType GetType() const override { return CellType::BasePair; }

	ADNPointer<ADNNucleotide> GetLeftNucleotide();
	SBNode* getLeft() const;
	void SetLeftNucleotide(ADNPointer<ADNNucleotide> nt);
	ADNPointer<ADNNucleotide> GetRightNucleotide();
	SBNode* getRight() const;
	void SetRightNucleotide(ADNPointer<ADNNucleotide> nt);
	void SetRemainingNucleotide(ADNPointer<ADNNucleotide> nt);
	void AddPair(ADNPointer<ADNNucleotide> left, ADNPointer<ADNNucleotide> right);
	void PairNucleotides();
	void RemoveNucleotide(ADNPointer<ADNNucleotide> nt) override;
	CollectionMap<ADNNucleotide> GetNucleotides() const override;
	bool IsLeft(ADNPointer<ADNNucleotide> nt) const override;
	bool IsRight(ADNPointer<ADNNucleotide> nt) const override;

private:

	ADNPointer<ADNNucleotide> left_ = nullptr;
	ADNPointer<ADNNucleotide> right_ = nullptr;

};

SB_REGISTER_TARGET_TYPE(ADNBasePair, "ADNBasePair", "71C5049C-EC51-8DC5-15EF-1525E4DBAB42");
SB_DECLARE_BASE_TYPE(ADNBasePair, ADNCell);

class ADNSkipPair : public ADNCell {

	SB_CLASS

public:

	ADNSkipPair() = default;
	~ADNSkipPair() = default;

	void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	CellType GetType() const override { return CellType::SkipPair; }

	void RemoveNucleotide(ADNPointer<ADNNucleotide> nt) override;

	bool IsLeft(ADNPointer<ADNNucleotide> nt) const override { return false; }
	bool IsRight(ADNPointer<ADNNucleotide> nt) const override { return false; }

};

SB_REGISTER_TARGET_TYPE(ADNSkipPair, "ADNSkipPair", "65441545-3022-773B-49A5-FF39A89AE754");
SB_DECLARE_BASE_TYPE(ADNSkipPair, ADNCell);


class ADNLoopPair : public ADNCell {

	SB_CLASS

public:

	ADNLoopPair() = default;
	~ADNLoopPair() = default;

	void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	CellType GetType() const override { return CellType::LoopPair; }

	ADNPointer<ADNLoop> GetLeftLoop();
	SBNode* getLeft() const;
	void SetLeftLoop(ADNPointer<ADNLoop> lp);
	ADNPointer<ADNLoop> GetRightLoop();
	SBNode* getRight() const;
	void SetRightLoop(ADNPointer<ADNLoop> lp);

	void RemoveNucleotide(ADNPointer<ADNNucleotide> nt) override;
	CollectionMap<ADNNucleotide> GetNucleotides() const override;

	bool IsLeft(ADNPointer<ADNNucleotide> nt) const override;
	bool IsRight(ADNPointer<ADNNucleotide> nt) const override;

private:

	ADNPointer<ADNLoop> left_ = nullptr;
	ADNPointer<ADNLoop> right_ = nullptr;

};

SB_REGISTER_TARGET_TYPE(ADNLoopPair, "ADNLoopPair", "F9CB2D19-D635-F494-D87D-EC619763E577");
SB_DECLARE_BASE_TYPE(ADNLoopPair, ADNCell);

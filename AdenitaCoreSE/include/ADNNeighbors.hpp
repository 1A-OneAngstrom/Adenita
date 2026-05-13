#pragma once

#include "ADNMixins.hpp"

#include "SBQuantity.hpp"

class ADNPart;
class ADNNucleotide;

class SB_EXPORT ADNNeighborNt {

public:

	ADNNeighborNt(unsigned int idx, SBPointer<ADNNucleotide> nt) : id_(idx), nt_(nt) {};

	unsigned int GetId() const noexcept { return id_; };
	SBPointer<ADNNucleotide> GetNucleotide() const { return nt_; };

private:

	unsigned int id_{ 0 };
	SBPointer<ADNNucleotide> nt_;

};

class SB_EXPORT ADNNeighbors {

public:

	ADNNeighbors();
	~ADNNeighbors() = default;

	[[nodiscard]] ADNNeighborNt* GetPINucleotide(SBPointer<ADNNucleotide> nt) const;
	[[nodiscard]] std::vector<ADNNeighborNt*> GetNeighbors(ADNNeighborNt* nt) const;
	[[nodiscard]] SBPointerIndexer<ADNNucleotide> GetNeighbors(SBPointer<ADNNucleotide> nt) const;

	void SetFromOwnSingleStrand(bool b);
	void SetIncludePairs(bool b);
	void SetMaxCutOff(SBQuantity::length cutOff);
	void SetMinCutOff(SBQuantity::length cutOff);

	void InitializeNeighbors(SBPointer<ADNPart> part);

private:

	SBQuantity::length maxCutOff_;
	SBQuantity::length minCutOff_ = SBQuantity::nanometer(0.0);

	std::map<unsigned int, ADNNeighborNt*> ntIndices_;
	std::vector<unsigned int> neighborList_;
	std::vector<unsigned int> headList_;
	std::vector<unsigned int> numNeighborsList_;

	bool fromOwnSingleStrand_{ false };
	bool includePairs_{ false };

};

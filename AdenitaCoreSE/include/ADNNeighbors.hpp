#pragma once

#include "ADNMixins.hpp"

#include "SBQuantity.hpp"

class ADNPart;
class ADNNucleotide;

class SB_EXPORT ADNNeighborNt {

public:

	ADNNeighborNt(unsigned int idx, ADNPointer<ADNNucleotide> nt) : id_(idx), nt_(nt) {};

	unsigned int GetId() const noexcept { return id_; };
	ADNPointer<ADNNucleotide> GetNucleotide() const { return nt_; };

private:

	unsigned int id_{ 0 };
	ADNPointer<ADNNucleotide> nt_;

};

class SB_EXPORT ADNNeighbors {

public:

	ADNNeighbors();
	~ADNNeighbors() = default;

	ADNNeighborNt* GetPINucleotide(ADNPointer<ADNNucleotide> nt) const;
	std::vector<ADNNeighborNt*> GetNeighbors(ADNNeighborNt* nt) const;
	CollectionMap<ADNNucleotide> GetNeighbors(ADNPointer<ADNNucleotide> nt) const;

	void SetFromOwnSingleStrand(bool b);
	void SetIncludePairs(bool b);
	void SetMaxCutOff(SBQuantity::length cutOff);
	void SetMinCutOff(SBQuantity::length cutOff);

	void InitializeNeighbors(ADNPointer<ADNPart> part);

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

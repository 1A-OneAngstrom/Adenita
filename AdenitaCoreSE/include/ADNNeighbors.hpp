#pragma once

#include "ADNMixins.hpp"

#include "SBQuantity.hpp"
#include <memory>

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

/// \brief Owns a snapshot of nucleotide neighbors for the configured distance filters.
/// Returned wrapper pointers are borrowed until the next initialization or destruction.
class SB_EXPORT ADNNeighbors {

public:

	ADNNeighbors();
	~ADNNeighbors() = default;
	ADNNeighbors(const ADNNeighbors&) = delete; ///< Wrapper identities belong to one index.
	ADNNeighbors& operator=(const ADNNeighbors&) = delete;

	/// \brief Returns the indexed wrapper, or null for an absent or deleted nucleotide.
	[[nodiscard]] ADNNeighborNt* GetPINucleotide(SBPointer<ADNNucleotide> nt) const;
	/// \brief Returns neighbors only for a live wrapper owned by this index.
	[[nodiscard]] std::vector<ADNNeighborNt*> GetNeighbors(ADNNeighborNt* nt) const;
	/// \brief Returns an empty indexer when the queried nucleotide is not indexed.
	[[nodiscard]] SBPointerIndexer<ADNNucleotide> GetNeighbors(SBPointer<ADNNucleotide> nt) const;

	void SetFromOwnSingleStrand(bool b);
	void SetIncludePairs(bool b);
	void SetMaxCutOff(SBQuantity::length cutOff);
	void SetMinCutOff(SBQuantity::length cutOff);

	/// \brief Replaces every index and adjacency record using the current geometry and filters.
	/// \param part The part to index; null or empty input clears the snapshot.
	/// Allocation failure leaves the previous snapshot intact. Rebuild after editing geometry.
	void InitializeNeighbors(SBPointer<ADNPart> part);

private:

	SBQuantity::length maxCutOff_ = SBQuantity::nanometer(0.0);
	SBQuantity::length minCutOff_ = SBQuantity::nanometer(0.0);

	std::map<unsigned int, std::unique_ptr<ADNNeighborNt>> ntIndices_;
	std::vector<unsigned int> neighborList_;
	std::vector<size_t> headList_;
	std::vector<size_t> numNeighborsList_;

	bool fromOwnSingleStrand_{ false };
	bool includePairs_{ false };

};

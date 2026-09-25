#include "ADNNeighbors.hpp"
#include "ADNPart.hpp"

#include <limits>
#include <stdexcept>

ADNNeighbors::ADNNeighbors() {}

ADNNeighborNt* ADNNeighbors::GetPINucleotide(SBPointer<ADNNucleotide> nt) const {
    if (nt == nullptr) return nullptr;
    for (const auto& entry : ntIndices_)
        if (entry.second->GetNucleotide() == nt) return entry.second.get();
    return nullptr;
}

std::vector<ADNNeighborNt*> ADNNeighbors::GetNeighbors(ADNNeighborNt* nt) const {
    std::vector<ADNNeighborNt*> neighbors;
    if (nt == nullptr) return neighbors;

    // Compare addresses before dereferencing a borrowed pointer: it may belong
    // to another index, or have been invalidated by rebuilding this one.
    auto found = ntIndices_.end();
    for (auto it = ntIndices_.begin(); it != ntIndices_.end(); ++it)
        if (it->second.get() == nt) { found = it; break; }
    if (found == ntIndices_.end() || nt->GetNucleotide() == nullptr) return neighbors;
    const size_t index = found->first;
    const size_t begin = headList_.at(index);
    const size_t count = numNeighborsList_.at(index);
    for (size_t i = 0; i < count; ++i) {
        auto* neighbor = ntIndices_.at(neighborList_.at(begin + i)).get();
        if (neighbor->GetNucleotide() != nullptr) neighbors.push_back(neighbor);
    }
    return neighbors;
}

SBPointerIndexer<ADNNucleotide> ADNNeighbors::GetNeighbors(SBPointer<ADNNucleotide> nt) const {
    SBPointerIndexer<ADNNucleotide> neighbors;
    for (auto* neighbor : GetNeighbors(GetPINucleotide(nt)))
        neighbors.addReferenceTarget(neighbor->GetNucleotide()());
    return neighbors;
}

void ADNNeighbors::SetFromOwnSingleStrand(bool b) { fromOwnSingleStrand_ = b; }
void ADNNeighbors::SetIncludePairs(bool b) { includePairs_ = b; }
void ADNNeighbors::SetMaxCutOff(SBQuantity::length cutOff) { maxCutOff_ = cutOff; }
void ADNNeighbors::SetMinCutOff(SBQuantity::length cutOff) { minCutOff_ = cutOff; }

void ADNNeighbors::InitializeNeighbors(SBPointer<ADNPart> part) {
    // Build one coherent replacement. RAII also frees rejected/partial builds,
    // including their nucleotide references, if an allocation throws.
    decltype(ntIndices_) indices;
    decltype(neighborList_) adjacency;
    decltype(headList_) heads;
    decltype(numNeighborsList_) counts;
    if (part != nullptr) {
        const auto nts = part->GetNucleotides();
        if (nts.size() > (std::numeric_limits<unsigned int>::max)())
            throw std::length_error("Too many nucleotides for a neighbor index");
        const unsigned int size = static_cast<unsigned int>(nts.size());
        heads.resize(size);
        counts.resize(size);
        for (unsigned int i = 0; i < size; ++i)
            indices.emplace(i, std::make_unique<ADNNeighborNt>(i, nts[i]));
        for (unsigned int i = 0; i < size; ++i) {
            heads[i] = adjacency.size();
            const auto nt1 = nts[i];
            if (nt1 == nullptr) continue;
            const auto pos1 = nt1->GetPosition();
            for (unsigned int j = 0; j < size; ++j) {
                const auto nt2 = nts[j];
                if (nt2 == nullptr || nt1 == nt2) continue;
                if (!fromOwnSingleStrand_ && nt1->GetStrand() == nt2->GetStrand()) continue;
                if (!includePairs_ && nt2->GetPair() == nt1) continue;
                const auto distance = (nt2->GetPosition() - pos1).norm();
                if (distance > minCutOff_ && distance < maxCutOff_) adjacency.push_back(j);
            }
            counts[i] = adjacency.size() - heads[i];
        }
    }
    ntIndices_.swap(indices);
    neighborList_.swap(adjacency);
    headList_.swap(heads);
    numNeighborsList_.swap(counts);
}

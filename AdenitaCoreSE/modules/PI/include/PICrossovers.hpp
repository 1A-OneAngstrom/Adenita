#pragma once

#include "ADNConfig.hpp"
#include "ADNMixins.hpp"

class ADNPart;
class ADNNucleotide;
class ADNNeighbors;

using XOPair = std::pair < SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide> >;

namespace PICrossovers {

	SB_EXPORT std::vector<XOPair> GetCrossovers(SBPointer<ADNPart> part);
	SB_EXPORT std::vector<XOPair> GetPossibleCrossovers(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt, ADNNeighbors* neigh);
	SB_EXPORT std::vector<XOPair> GetPossibleCrossovers(SBPointer<ADNPart> part, ADNNeighbors* neigh = nullptr);

};

#pragma once

#include "ADNConfig.hpp"
#include "ADNMixins.hpp"

class ADNPart;
class ADNNucleotide;
class ADNNeighbors;

using XOPair = std::pair < ADNPointer<ADNNucleotide>, ADNPointer<ADNNucleotide> >;

namespace PICrossovers {

	std::vector<XOPair> GetCrossovers(ADNPointer<ADNPart> part);
	std::vector<XOPair> GetPossibleCrossovers(ADNPointer<ADNPart> part, ADNPointer<ADNNucleotide> nt, ADNNeighbors* neigh);
	std::vector<XOPair> GetPossibleCrossovers(ADNPointer<ADNPart> part, ADNNeighbors* neigh = nullptr);

};

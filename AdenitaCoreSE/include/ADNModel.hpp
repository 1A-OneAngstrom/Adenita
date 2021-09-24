#pragma once

#define _USE_MATH_DEFINES

#include <boost/bimap.hpp>
#include <boost/assign/list_of.hpp>

#include "ADNAuxiliary.hpp"
#include "ADNVectorMath.hpp"
#include "ADNLogger.hpp"
#include "ADNMixins.hpp"

#include "ADNAtom.hpp"
#include "ADNBackbone.hpp"
#include "ADNBaseSegment.hpp"
#include "ADNCell.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNLoop.hpp"
#include "ADNNucleotide.hpp"
#include "ADNSidechain.hpp"
#include "ADNSingleStrand.hpp"

#include <cmath>

/* Physical info and maps */

static std::vector<std::string> backbone_names_ = std::vector<std::string>{ "P", "OP1", "OP2", "O5'", "C5'", "C4'",
"O4'", "C3'", "O3'", "C2'", "C1'" };

using DNAPairsToString = boost::bimap<std::pair<DNABlocks, DNABlocks>, std::string>;
const DNAPairsToString nt_pairs_names_ = boost::assign::list_of<DNAPairsToString::relation>
(std::make_pair(DNABlocks::DA, DNABlocks::DT), "AT")(std::make_pair(DNABlocks::DT, DNABlocks::DA), "TA")
(std::make_pair(DNABlocks::DC, DNABlocks::DG), "CG")(std::make_pair(DNABlocks::DG, DNABlocks::DC), "GC")(std::make_pair(DNABlocks::DI, DNABlocks::DI), "NN");

/* Namespace with static functions */
namespace ADNModel {

	DNABlocks GetComplementaryBase(DNABlocks base);

	char GetResidueName(DNABlocks t);
	DNABlocks ResidueNameToType(char n);

	bool IsAtomInBackboneByName(std::string name);

	SBElement::Type GetElementType(std::string atomName);

	std::map<std::string, std::vector<std::string>> GetNucleotideBonds(DNABlocks t);

}

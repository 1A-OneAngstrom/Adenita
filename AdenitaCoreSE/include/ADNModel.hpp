#pragma once

#define _USE_MATH_DEFINES

#include "ADNVectorMath.hpp"
#include "ADNMixins.hpp"
#include "ADNAuxiliary.hpp"
#include "ADNLogger.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/bimap.hpp>
#include <boost/assign/list_of.hpp>

#include <cmath>

/* Physical info and maps */

using DNAPairsToString = boost::bimap<std::pair<DNABlocks, DNABlocks>, std::string>;
const DNAPairsToString nt_pairs_names_ = boost::assign::list_of<DNAPairsToString::relation>
(std::make_pair(DNABlocks::DA, DNABlocks::DT), "AT")
(std::make_pair(DNABlocks::DT, DNABlocks::DA), "TA")
(std::make_pair(DNABlocks::DC, DNABlocks::DG), "CG")
(std::make_pair(DNABlocks::DG, DNABlocks::DC), "GC")
(std::make_pair(DNABlocks::DI, DNABlocks::DI), "NN");

/* Namespace with static functions */
namespace ADNModel {

    SB_EXPORT DNABlocks											GetComplementaryBase(DNABlocks base);

	SB_EXPORT std::string										GetResidueName(DNABlocks t, bool removePrefixD = true);
	SB_EXPORT DNABlocks								            ResidueNameToType(const std::string& n);
	SB_EXPORT DNABlocks								            ResidueNameToType(char n);

    SB_EXPORT bool												IsAtomInBackboneByName(std::string_view name);

    SB_EXPORT SBElement::Type									GetElementType(const std::string& atomName);

    SB_EXPORT std::map<std::string, std::vector<std::string>>	GetNucleotideBonds(DNABlocks t);

    const std::vector<std::string>                              backbone_names_ = std::vector<std::string>{ "P", "OP1", "OP2", "O5'", "C5'", "C4'",
    "O4'", "C3'", "O3'", "C2'", "C1'" };

	const std::map<std::string, SBElement::Type>				atomType = {
      { "P",   SBElement::Phosphorus },
      { "OP1", SBElement::Oxygen },
      { "O1P", SBElement::Oxygen },
      { "OP2", SBElement::Oxygen },
      { "O2P", SBElement::Oxygen },
      { "O5'", SBElement::Oxygen },
      { "O4'", SBElement::Oxygen },
      { "O3'", SBElement::Oxygen },
      { "O6",  SBElement::Oxygen },
      { "O4",  SBElement::Oxygen },
      { "O2",  SBElement::Oxygen },
      { "C5'", SBElement::Carbon },
      { "C4'", SBElement::Carbon },
      { "C3'", SBElement::Carbon },
      { "C2'", SBElement::Carbon },
      { "C1'", SBElement::Carbon },
      { "C8",  SBElement::Carbon },
      { "C7",  SBElement::Carbon },
      { "C6",  SBElement::Carbon },
      { "C5",  SBElement::Carbon },
      { "C4",  SBElement::Carbon },
      { "C2",  SBElement::Carbon },
      { "N9",  SBElement::Nitrogen },
      { "N7",  SBElement::Nitrogen },
      { "N6",  SBElement::Nitrogen },
      { "N4",  SBElement::Nitrogen },
      { "N3",  SBElement::Nitrogen },
      { "N2",  SBElement::Nitrogen },
      { "N1",  SBElement::Nitrogen }
    };

}

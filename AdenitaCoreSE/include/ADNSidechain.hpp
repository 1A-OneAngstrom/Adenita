#pragma once

#include "SBSideChain.hpp"

#include "ADNMixins.hpp"

class ADNNucleotide;

class SB_EXPORT ADNSidechain : public SBSideChain, public PositionableSB {

	SB_CLASS

public:

	ADNSidechain();
	ADNSidechain(const ADNSidechain& other);
	~ADNSidechain() = default;

	ADNSidechain&												operator=(const ADNSidechain& other);

	void														serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;														///< Serializes the node
	void														unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	bool														AddAtom(ADNPointer<ADNAtom> atom);
	bool														DeleteAtom(ADNPointer<ADNAtom> atom);
	CollectionMap<ADNAtom>										GetAtoms() const;
	int															getNumberOfAtoms() const;

	ADNPointer<ADNNucleotide>									GetNucleotide() const;

};

SB_REGISTER_TARGET_TYPE(ADNSidechain, "ADNSidechain", "CD6919A2-5B4C-7723-AAD7-804157EA51EA");
SB_DECLARE_BASE_TYPE(ADNSidechain, SBSideChain);

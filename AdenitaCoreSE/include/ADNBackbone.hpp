#pragma once

#include "SBBackbone.hpp"

#include "ADNMixins.hpp"

class ADNNucleotide;

class SB_EXPORT ADNBackbone : public SBBackbone, public PositionableSB {

	SB_CLASS

public:

	ADNBackbone();
	ADNBackbone(const ADNBackbone& other);
	~ADNBackbone() = default;

	ADNBackbone&												operator=(const ADNBackbone& other);

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	bool														AddAtom(ADNPointer<ADNAtom> atom);
	bool														DeleteAtom(ADNPointer<ADNAtom> atom);
	CollectionMap<ADNAtom>										GetAtoms() const;
	int															getNumberOfAtoms() const;

	ADNPointer<ADNNucleotide>									GetNucleotide() const;

};

SB_REGISTER_TARGET_TYPE(ADNBackbone, "ADNBackbone", "CFA95C6A-7686-3029-F93C-2F1FD1988C33");
SB_DECLARE_BASE_TYPE(ADNBackbone, SBBackbone);

#pragma once

#include "SBAtom.hpp"

#include "ADNAuxiliary.hpp"

class SB_EXPORT ADNAtom : public SBAtom {

	SB_CLASS

public:

	ADNAtom() : SBAtom() {}
	ADNAtom(const ADNAtom& other);
	ADNAtom(SBElement::Type element, const SBPosition3& position);
	~ADNAtom() = default;

	ADNAtom&													operator=(const ADNAtom& other);

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	SBNode*														getNucleotide() const;

	bool														IsInADNBackbone() const;

};

SB_REGISTER_TARGET_TYPE(ADNAtom, "ADNAtom", "292377CD-F926-56E9-52AB-D6B623C3A104");
SB_DECLARE_BASE_TYPE(ADNAtom, SBAtom);

#include "ADNAtom.hpp"
#include "ADNModel.hpp"


ADNAtom::ADNAtom(SBElement::Type element, const SBPosition3& position) : SBAtom(element, position) {}

void ADNAtom::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

	SBAtom::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

void ADNAtom::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

	SBAtom::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

SBNode* ADNAtom::getNucleotide() const {

#if 1
	return getResidue();
#else
	if (getParent()) if (getParent()->getParent()) if (getParent()->getParent()->getType() == SBNode::Residue)
		return getParent()->getParent();

	return nullptr;
#endif

}

bool ADNAtom::IsInADNBackbone() const {

	return isFromNucleicAcidBackbone();

}

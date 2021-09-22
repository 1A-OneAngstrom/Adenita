#include "ADNAtom.hpp"
#include "ADNModel.hpp"


ADNAtom::ADNAtom(const ADNAtom& other) : SBAtom(other) {

	*this = other;

}

ADNAtom& ADNAtom::operator=(const ADNAtom& other) {

	SBAtom::operator =(other);

	return *this;

}

void ADNAtom::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

	SBAtom::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

void ADNAtom::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

	SBAtom::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

std::string const& ADNAtom::GetName() const {
	return getName();
}

void ADNAtom::SetName(const std::string& name) {
	setName(name);
}

Position3D const& ADNAtom::GetPosition() const {
	return getPosition();
}

void ADNAtom::SetPosition(Position3D const& newPosition) {
	setPosition(newPosition);
}

SBNode* ADNAtom::getNt() const {
	return getParent()->getParent();
}

SBNode* ADNAtom::getNtGroup() const {
	return getParent();
}

bool ADNAtom::IsInADNBackbone() {
	return isFromNucleicAcidBackbone();
}

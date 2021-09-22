//#include "ADNBackbone.hpp"
#include "ADNModel.hpp"

ADNBackbone::ADNBackbone() : PositionableSB(), SBBackbone() {

    auto cA = GetCenterAtom();
    cA->setElementType(SBElement::Einsteinium);

}

ADNBackbone::ADNBackbone(const ADNBackbone& other) {

    *this = other;

}

ADNBackbone& ADNBackbone::operator=(const ADNBackbone& other) {

    PositionableSB::operator =(other);
    SBBackbone::operator =(other);

    return *this;

}

void ADNBackbone::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBBackbone::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    ADNPointer<ADNAtom> at = GetCenterAtom();
    serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(at()));

}

void ADNBackbone::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBBackbone::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int idx = serializer->readUnsignedIntElement();
    ADNPointer<ADNAtom> at = (ADNAtom*)nodeIndexer.getNode(idx);
    SetCenterAtom(at);

}

bool ADNBackbone::AddAtom(ADNPointer<ADNAtom> atom) {

    return addChild(atom());

}

bool ADNBackbone::DeleteAtom(ADNPointer<ADNAtom> atom) {

    return removeChild(atom());

}

int ADNBackbone::getNumberOfAtoms() const {

    return static_cast<int>(GetAtoms().size());

}

CollectionMap<ADNAtom> ADNBackbone::GetAtoms() const {

    CollectionMap<ADNAtom> atomList;

#if 1
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, SBNode::IsType(SBNode::Atom) && (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    SB_FOR(SBNode * n, nodeIndexer)
        atomList.addReferenceTarget(static_cast<ADNAtom*>(n));
#else
    auto children = *getChildren();
    SB_FOR(SBStructuralNode * n, children) {

        if (n->getType() == SBNode::Atom /*&& n->getProxy()->getElementUUID() == SBUUID(SB_ELEMENT_UUID)*/) {

            ADNAtom* a = static_cast<ADNAtom*>(n);
            atomList.addReferenceTarget(a);

        }

    }
#endif

    return atomList;

}

ADNPointer<ADNNucleotide> ADNBackbone::GetNucleotide() const {

    if (getParent()) if (getParent()->getType() == SBNode::Residue)
        return ADNPointer<ADNNucleotide>(static_cast<ADNNucleotide*>(getParent()));

    return nullptr;

}

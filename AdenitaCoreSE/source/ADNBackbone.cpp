#include "ADNBackbone.hpp"
#include "ADNAtom.hpp"
#include "ADNNucleotide.hpp"
#include "ADNNodeValidation.hpp"

ADNBackbone::ADNBackbone() : PositionableSB(), SBBackbone() {

    auto cA = GetCenterAtom();
    cA->setElementType(SBElement::Unknown); // Einsteinium);

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

    SBPointer<ADNAtom> at = GetCenterAtom();
    serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(at()));

}

void ADNBackbone::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBBackbone::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int idx = serializer->readUnsignedIntElement();
    SBPointer<ADNAtom> at = ADNNodeValidation::GetSerializedAdenitaNode<ADNAtom>(nodeIndexer, idx, "ADNAtom");
    SetCenterAtom(at);

}

bool ADNBackbone::addAtom(SBPointer<ADNAtom> atom) {

    return addChild(atom());

}

bool ADNBackbone::deleteAtom(SBPointer<ADNAtom> atom) {

    return removeChild(atom());

}

int ADNBackbone::getNumberOfAtoms() const {

#if 1
    return countNodes(SBNode::IsType(SBNode::Atom) && (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#else
    return static_cast<int>(GetAtoms().size());
#endif

}

SBPointerIndexer<ADNAtom> ADNBackbone::GetAtoms() const {

    SBPointerIndexer<ADNAtom> atomList;

#if 1
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, SBNode::IsType(SBNode::Atom) && (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    SB_FOR(SBNode * n, nodeIndexer)
        atomList.addReferenceTarget(static_cast<ADNAtom*>(n));
#else
    const SBPointerList<SBStructuralNode>* children = getChildren();
    SB_FOR(SBStructuralNode * n, *children) {

        if (n->getType() == SBNode::Atom /*&& n->getProxy()->getElementUUID() == SBUUID(SB_ELEMENT_UUID)*/) {

            ADNAtom* a = static_cast<ADNAtom*>(n);
            atomList.addReferenceTarget(a);

        }

    }
#endif

    return atomList;

}

SBPointer<ADNNucleotide> ADNBackbone::GetNucleotide() const {

    if (SBNode* parent = getParent())
        if (ADNNodeValidation::IsAdenitaNode(parent, "ADNNucleotide"))
            return SBPointer<ADNNucleotide>(dynamic_cast<ADNNucleotide*>(parent));

    return nullptr;

}

//#include "ADNSidechain.hpp"
//#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"


ADNSidechain::ADNSidechain() : PositionableSB(), SBSideChain() {

    auto cA = GetCenterAtom();
    cA->setElementType(SBElement::Fermium);

}

ADNSidechain::ADNSidechain(const ADNSidechain& other) {

    *this = other;

}

ADNSidechain& ADNSidechain::operator=(const ADNSidechain& other) {

    PositionableSB::operator =(other);
    SBSideChain::operator =(other);

    return *this;

}

void ADNSidechain::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBSideChain::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    ADNPointer<ADNAtom> at = GetCenterAtom();
    serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(at()));

}

void ADNSidechain::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBSideChain::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int idx = serializer->readUnsignedIntElement();
    ADNPointer<ADNAtom> at = (ADNAtom*)nodeIndexer.getNode(idx);
    SetCenterAtom(at);

}

bool ADNSidechain::AddAtom(ADNPointer<ADNAtom> atom) {

    return addChild(atom());

}

bool ADNSidechain::DeleteAtom(ADNPointer<ADNAtom> atom) {

    return removeChild(atom());

}

int ADNSidechain::getNumberOfAtoms() const {

    return static_cast<int>(GetAtoms().size());

}

CollectionMap<ADNAtom> ADNSidechain::GetAtoms() const {

    CollectionMap<ADNAtom> atomList;

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

ADNPointer<ADNNucleotide> ADNSidechain::GetNucleotide() const {

    SBNode* parent = getParent();
    if (parent) if (parent->getType() == SBNode::Residue) //if (parent->getProxy()->getName() == "ADNNucleotide")
        return ADNPointer<ADNNucleotide>(static_cast<ADNNucleotide*>(getParent()));

    return nullptr;

}

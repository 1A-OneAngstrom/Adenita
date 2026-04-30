#include "ADNSidechain.hpp"
#include "ADNNucleotide.hpp"
#include "ADNAtom.hpp"
#include "ADNModel.hpp"

#include <stdexcept>
#include <string>

namespace {

constexpr unsigned int InvalidNodeIndex = static_cast<unsigned int>(-1);

bool isAdenitaNode(SBNode* node, const std::string& className) {

    if (node == nullptr) return false;
    auto proxy = node->getProxy();
    return proxy != nullptr &&
        proxy->getName() == className &&
        proxy->getElementUUID() == SBUUID(SB_ELEMENT_UUID);

}

SBPointer<ADNAtom> getSerializedAtom(const SBNodeIndexer& nodeIndexer, unsigned int index) {

    if (index == InvalidNodeIndex) return nullptr;
    if (index >= nodeIndexer.size())
        throw std::runtime_error("Invalid Adenita serialized ADNAtom node index.");

    SBNode* node = nodeIndexer.getNode(index);
    if (node == nullptr)
        throw std::runtime_error("Missing Adenita serialized ADNAtom node.");

    if (!isAdenitaNode(node, "ADNAtom"))
        throw std::runtime_error("Unexpected Adenita serialized node type for ADNAtom.");

    ADNAtom* atom = dynamic_cast<ADNAtom*>(node);
    if (atom == nullptr)
        throw std::runtime_error("Failed to cast Adenita serialized node to ADNAtom.");

    return atom;

}

}

ADNSidechain::ADNSidechain() : PositionableSB(), SBSideChain() {

    auto cA = GetCenterAtom();
    cA->setElementType(SBElement::Unknown);// Fermium);

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

    SBPointer<ADNAtom> at = GetCenterAtom();
    serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(at()));

}

void ADNSidechain::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBSideChain::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int idx = serializer->readUnsignedIntElement();
    SBPointer<ADNAtom> at = getSerializedAtom(nodeIndexer, idx);
    SetCenterAtom(at);

}

bool ADNSidechain::addAtom(SBPointer<ADNAtom> atom) {

    return addChild(atom());

}

bool ADNSidechain::deleteAtom(SBPointer<ADNAtom> atom) {

    return removeChild(atom());

}

int ADNSidechain::getNumberOfAtoms() const {

#if 1
    return countNodes(SBNode::IsType(SBNode::Atom) && (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#else
    return static_cast<int>(GetAtoms().size());
#endif

}

SBPointerIndexer<ADNAtom> ADNSidechain::GetAtoms() const {

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

SBPointer<ADNNucleotide> ADNSidechain::GetNucleotide() const {

    if (SBNode* parent = getParent())
        if (isAdenitaNode(parent, "ADNNucleotide"))
            return SBPointer<ADNNucleotide>(dynamic_cast<ADNNucleotide*>(parent));

    return nullptr;

}

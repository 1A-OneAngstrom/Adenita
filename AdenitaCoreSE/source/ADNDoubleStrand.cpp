//#include "ADNDoubleStrand.hpp"
//#include "ADNBaseSegment.hpp"
#include "ADNModel.hpp"
#include "ADNPart.hpp"


ADNDoubleStrand::ADNDoubleStrand(const ADNDoubleStrand& other) : SBStructuralGroup(other) {

    *this = other;

}

ADNDoubleStrand& ADNDoubleStrand::operator=(const ADNDoubleStrand& other) {

    SBStructuralGroup::operator =(other);

    if (this != &other) {

        startBaseSegment = other.startBaseSegment;
        endBaseSegment = other.endBaseSegment;

    }

    return *this;

}

void ADNDoubleStrand::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeBoolElement("isCircular", IsCircular());
    serializer->writeDoubleElement("twistAngle", GetInitialTwistAngle());

    serializer->writeUnsignedIntElement("start", nodeIndexer.getIndex(startBaseSegment()));
    serializer->writeUnsignedIntElement("end", nodeIndexer.getIndex(endBaseSegment()));

}

void ADNDoubleStrand::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    setCircularFlag(serializer->readBoolElement());
    SetInitialTwistAngle(serializer->readDoubleElement());

    unsigned int sIdx = serializer->readUnsignedIntElement();
    unsigned int eIdx = serializer->readUnsignedIntElement();
    SBNode* sNode = nodeIndexer.getNode(sIdx);
    SBNode* eNode = nodeIndexer.getNode(eIdx);
    if (sNode) startBaseSegment = static_cast<ADNBaseSegment*>(sNode);
    if (eNode) endBaseSegment = static_cast<ADNBaseSegment*>(eNode);

}

ADNPointer<ADNPart> ADNDoubleStrand::GetPart() {

    // the ADNPart is a structural model
    SBNode* model = getModel();
    if (!model) return nullptr;
    ADNPointer<ADNPart> part = static_cast<ADNPart*>(model);
    return part;

}

void ADNDoubleStrand::SetInitialTwistAngle(double angle) {
    this->initialTwistAngle = angle;
}

double ADNDoubleStrand::GetInitialTwistAngle() const {
    return initialTwistAngle;
}

double ADNDoubleStrand::getInitialTwistAngle() const {
    return GetInitialTwistAngle();
}

int ADNDoubleStrand::GetLength() const {
    return boost::numeric_cast<int>(GetBaseSegments().size());
}

int ADNDoubleStrand::getLength() const {
    return GetLength();
}

bool ADNDoubleStrand::IsCircular() const {
    return circularFlag;
}

bool ADNDoubleStrand::getCircularFlag() const {
    return circularFlag;
}

void ADNDoubleStrand::setCircularFlag(bool b) {
    this->circularFlag = b;
}

CollectionMap<ADNBaseSegment> ADNDoubleStrand::GetBaseSegments() const {

    CollectionMap<ADNBaseSegment> baseSegmentList;

#if 0
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, SBNode::IsType(SBNode::StructuralGroup) && (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    SB_FOR(SBNode * n, nodeIndexer)
        baseSegmentList.addReferenceTarget(static_cast<ADNBaseSegment*>(n));
#else
    const SBPointerList<SBStructuralNode>* children = getChildren();
    SB_FOR(SBStructuralNode * n, *children) {

        if (n->getProxy()->getName() == "ADNBaseSegment" && n->getProxy()->getElementUUID() == SBUUID(SB_ELEMENT_UUID)) {

            ADNBaseSegment* a = static_cast<ADNBaseSegment*>(n);
            baseSegmentList.addReferenceTarget(a);

        }

    }
#endif

    return baseSegmentList;

}

ADNPointer<ADNBaseSegment> ADNDoubleStrand::GetNthBaseSegment(int n) {

    ADNPointer<ADNBaseSegment> bs = startBaseSegment;

    for (int i = 0; i < n; ++i) {

        if (bs != nullptr) bs = bs->GetNext();
        else break;

    }

    return bs;

}

ADNPointer<ADNBaseSegment> ADNDoubleStrand::GetFirstBaseSegment() const {
    return startBaseSegment;
}

SBNode* ADNDoubleStrand::getFirstBaseSegment() const {
    return GetFirstBaseSegment()();
}

void ADNDoubleStrand::SetStart(ADNPointer<ADNBaseSegment> baseSegment) {
    this->startBaseSegment = baseSegment;
}

ADNPointer<ADNBaseSegment> ADNDoubleStrand::GetLastBaseSegment() const {
    return endBaseSegment;
}

SBNode* ADNDoubleStrand::getLastBaseSegment() const {
    return GetLastBaseSegment()();
}

void ADNDoubleStrand::SetEnd(ADNPointer<ADNBaseSegment> baseSegment) {
    this->endBaseSegment = baseSegment;
}

void ADNDoubleStrand::AddBaseSegmentBeginning(ADNPointer<ADNBaseSegment> baseSegment) {

    if (baseSegment == nullptr) return;

    baseSegment->setName("Base segment " + std::to_string(baseSegment->getNodeIndex()));
    int number = baseSegment->GetNumber();
    if (number == -1) {
        // number not defined
        number = 0;
    }

    if (startBaseSegment != nullptr) {
        number = startBaseSegment->GetNumber() - 1;
    }
    else {
        // nt is also fivePrime_
        endBaseSegment = baseSegment;
    }

    baseSegment->SetNumber(number);
    addChild(baseSegment(), startBaseSegment());
    startBaseSegment = baseSegment;

}

void ADNDoubleStrand::AddBaseSegmentEnd(ADNPointer<ADNBaseSegment> baseSegment) {

    if (baseSegment == nullptr) return;

    baseSegment->setName("Base segment " + std::to_string(baseSegment->getNodeIndex()));
    int number = baseSegment->GetNumber();
    if (number == -1) {
        // number not defined
        number = 0;
    }

    if (endBaseSegment == nullptr) {
        // bs is also first
        startBaseSegment = baseSegment;
    }
    else {
        number = endBaseSegment->GetNumber() + 1;
    }

    addChild(baseSegment());
    endBaseSegment = baseSegment;
    baseSegment->SetNumber(number);

}

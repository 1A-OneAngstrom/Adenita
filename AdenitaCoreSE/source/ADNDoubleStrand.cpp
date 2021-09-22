//#include "ADNDoubleStrand.hpp"
//#include "ADNBaseSegment.hpp"
#include "ADNModel.hpp"


ADNDoubleStrand::ADNDoubleStrand(const ADNDoubleStrand& other) : SBStructuralGroup(other) {

    *this = other;

}

ADNDoubleStrand& ADNDoubleStrand::operator=(const ADNDoubleStrand& other) {

    SBStructuralGroup::operator =(other);

    if (this != &other) {

        start_ = other.start_;
        end_ = other.end_;

    }

    return *this;

}

void ADNDoubleStrand::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeBoolElement("isCircular", IsCircular());
    serializer->writeDoubleElement("twistAngle", GetInitialTwistAngle());
    serializer->writeUnsignedIntElement("start", nodeIndexer.getIndex(start_()));
    serializer->writeUnsignedIntElement("end", nodeIndexer.getIndex(end_()));

}

void ADNDoubleStrand::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    IsCircular(serializer->readBoolElement());
    SetInitialTwistAngle(serializer->readDoubleElement());
    unsigned int sIdx = serializer->readUnsignedIntElement();
    unsigned int eIdx = serializer->readUnsignedIntElement();
    SBNode* sNode = nodeIndexer.getNode(sIdx);
    SBNode* eNode = nodeIndexer.getNode(eIdx);
    start_ = static_cast<ADNBaseSegment*>(sNode);
    end_ = static_cast<ADNBaseSegment*>(eNode);

}

void ADNDoubleStrand::SetInitialTwistAngle(double angle) {
    initialTwistAngle_ = angle;
}

double ADNDoubleStrand::GetInitialTwistAngle() const {
    return initialTwistAngle_;
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

void ADNDoubleStrand::IsCircular(bool c) {
    isCircular_ = c;
}

bool ADNDoubleStrand::IsCircular() const {
    return isCircular_;
}

bool ADNDoubleStrand::getIsCircular() const {
    return IsCircular();
}

void ADNDoubleStrand::setIsCircular(bool b) {
    IsCircular(b);
}

CollectionMap<ADNBaseSegment> ADNDoubleStrand::GetBaseSegments() const {

    CollectionMap<ADNBaseSegment> bsList;

    auto children = *getChildren();
    SB_FOR(SBStructuralNode * n, children) {

        ADNBaseSegment* a = static_cast<ADNBaseSegment*>(n);
        bsList.addReferenceTarget(a);

    }

    return bsList;

}

ADNPointer<ADNBaseSegment> ADNDoubleStrand::GetNthBaseSegment(int n) {

    ADNPointer<ADNBaseSegment> bs = start_;

    for (int i = 0; i < n; ++i)
        bs = bs->GetNext();

    return bs;

}

ADNPointer<ADNBaseSegment> ADNDoubleStrand::GetFirstBaseSegment() const {
    return start_;
}

SBNode* ADNDoubleStrand::getFirstBaseSegment() const {
    return GetFirstBaseSegment()();
}

void ADNDoubleStrand::SetStart(ADNPointer<ADNBaseSegment> bs) {
    start_ = bs;
}

ADNPointer<ADNBaseSegment> ADNDoubleStrand::GetLastBaseSegment() const {
    return end_;
}

SBNode* ADNDoubleStrand::getLastBaseSegment() const {
    return GetLastBaseSegment()();
}

void ADNDoubleStrand::SetEnd(ADNPointer<ADNBaseSegment> bs) {
    end_ = bs;
}

void ADNDoubleStrand::AddBaseSegmentBeginning(ADNPointer<ADNBaseSegment> bs) {

    bs->setName("Base Segment " + std::to_string(bs->getNodeIndex()));
    int number = bs->GetNumber();
    if (number == -1) {
        // number not defined
        number = 0;
    }

    if (start_ != nullptr) {
        number = start_->GetNumber() - 1;
    }
    else {
        // nt is also fivePrime_
        end_ = bs;
    }
    bs->SetNumber(number);
    addChild(bs(), start_());
    start_ = bs;

}

void ADNDoubleStrand::AddBaseSegmentEnd(ADNPointer<ADNBaseSegment> bs) {

    bs->setName("Base Segment " + std::to_string(bs->getNodeIndex()));
    int number = bs->GetNumber();
    if (number == -1) {
        // number not defined
        number = 0;
    }

    if (end_ == nullptr) {
        // bs is also first
        start_ = bs;
    }
    else {
        number = end_->GetNumber() + 1;
    }

    addChild(bs());
    end_ = bs;
    bs->SetNumber(number);

}

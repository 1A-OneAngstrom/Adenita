//#include "ADNCell.hpp"
#include "ADNModel.hpp"


void ADNCell::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

void ADNCell::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

void ADNBasePair::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    ADNCell::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("left", nodeIndexer.getIndex(left_()));
    serializer->writeUnsignedIntElement("right", nodeIndexer.getIndex(right_()));

}

void ADNBasePair::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    ADNCell::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int lIdx = serializer->readUnsignedIntElement();
    unsigned int rIdx = serializer->readUnsignedIntElement();

    SBNode* lNode = nodeIndexer.getNode(lIdx);
    SetLeftNucleotide(static_cast<ADNNucleotide*>(lNode));

    SBNode* rNode = nodeIndexer.getNode(rIdx);
    SetRightNucleotide(static_cast<ADNNucleotide*>(rNode));

}

ADNPointer<ADNNucleotide> ADNBasePair::GetLeftNucleotide() {
    return left_;
}

SBNode* ADNBasePair::getLeft() const {
    return left_();
}

void ADNBasePair::SetLeftNucleotide(ADNPointer<ADNNucleotide> nt) {
    left_ = nt;
}

ADNPointer<ADNNucleotide> ADNBasePair::GetRightNucleotide() {
    return right_;
}

SBNode* ADNBasePair::getRight() const {
    return right_();
}

void ADNBasePair::SetRightNucleotide(ADNPointer<ADNNucleotide> nt) {
    right_ = nt;
}

void ADNBasePair::SetRemainingNucleotide(ADNPointer<ADNNucleotide> nt) {

    if (left_ != nullptr && right_ == nullptr) {
        SetRightNucleotide(nt);
    }
    else if (left_ == nullptr && right_ != nullptr) {
        SetLeftNucleotide(nt);
    }
    PairNucleotides();

}

void ADNBasePair::AddPair(ADNPointer<ADNNucleotide> left, ADNPointer<ADNNucleotide> right) {

    SetLeftNucleotide(left);
    SetRightNucleotide(right);
    if (left != nullptr) left->SetPair(right);
    if (right_ != nullptr) right->SetPair(left);

}

void ADNBasePair::PairNucleotides() {

    if (left_ != nullptr && right_ != nullptr) {

        left_->SetPair(right_);
        right_->SetPair(left_);

    }

}

void ADNBasePair::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {

    if (left_ == nt) {
        left_ = nullptr;
    }
    else if (right_ == nt) {
        right_ = nullptr;
    }

}

CollectionMap<ADNNucleotide> ADNBasePair::GetNucleotides() const {

    CollectionMap<ADNNucleotide> nts;
    if (left_ != nullptr) nts.addReferenceTarget(left_());
    if (right_ != nullptr) nts.addReferenceTarget(right_());
    return nts;

}

bool ADNBasePair::IsLeft(ADNPointer<ADNNucleotide> nt) const {

    return bool(left_ == nt);

}

bool ADNBasePair::IsRight(ADNPointer<ADNNucleotide> nt) const {

    return bool(right_ == nt);

}


void ADNSkipPair::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {
    ADNCell::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);
}

void ADNSkipPair::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {
    ADNCell::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);
}

void ADNSkipPair::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {
}


void ADNLoopPair::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    ADNCell::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("left", nodeIndexer.getIndex(left_()));
    serializer->writeUnsignedIntElement("right", nodeIndexer.getIndex(right_()));

}

void ADNLoopPair::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    ADNCell::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int lIdx = serializer->readUnsignedIntElement();
    unsigned int rIdx = serializer->readUnsignedIntElement();
    ADNPointer<ADNLoop> lp = static_cast<ADNLoop*>(nodeIndexer.getNode(lIdx));
    ADNPointer<ADNLoop> rp = static_cast<ADNLoop*>(nodeIndexer.getNode(rIdx));
    if (lp != nullptr) SetLeftLoop(lp);
    if (rp != nullptr) SetRightLoop(rp);

}

ADNPointer<ADNLoop> ADNLoopPair::GetLeftLoop() {
    return left_;
}

SBNode* ADNLoopPair::getLeft() const {
    return left_();
}

void ADNLoopPair::SetLeftLoop(ADNPointer<ADNLoop> lp) {

    left_ = lp;
    addChild(lp());
    lp->setName("Left Loop " + std::to_string(lp->getNodeIndex()));

}

ADNPointer<ADNLoop> ADNLoopPair::GetRightLoop() {
    return right_;
}

SBNode* ADNLoopPair::getRight() const {
    return right_();
}

void ADNLoopPair::SetRightLoop(ADNPointer<ADNLoop> lp) {

    right_ = lp;
    addChild(lp());
    lp->setName("Right Loop " + std::to_string(lp->getNodeIndex()));

}

void ADNLoopPair::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {

    if (left_ != nullptr) {
        left_->RemoveNucleotide(nt);
    }
    if (right_ != nullptr) {
        right_->RemoveNucleotide(nt);
    }

}

bool ADNLoopPair::IsRight(ADNPointer<ADNNucleotide> nt) const {

    bool s = false;
    if (right_ != nullptr) {

        auto nts = right_->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> c, nts) {

            if (c == nt) {

                s = true;
                break;

            }

        }

    }

    return s;

}

bool ADNLoopPair::IsLeft(ADNPointer<ADNNucleotide> nt) const {

    bool s = false;
    if (left_ != nullptr) {

        auto nts = left_->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> c, nts) {

            if (c == nt) {

                s = true;
                break;

            }

        }

    }

    return s;

}

CollectionMap<ADNNucleotide> ADNLoopPair::GetNucleotides() const {

    CollectionMap<ADNNucleotide> nts;

    if (left_ != nullptr) {

        auto leftNts = left_->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> n, leftNts) {
            nts.addReferenceTarget(n());
        }

    }

    if (right_ != nullptr) {

        auto rightNts = right_->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> n, rightNts) {
            nts.addReferenceTarget(n());
        }

    }

    return nts;

}

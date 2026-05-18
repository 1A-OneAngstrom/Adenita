#include "ADNCell.hpp"
#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"
#include "ADNNodeValidation.hpp"

void ADNCell::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

void ADNCell::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

std::string ADNCell::getCellTypeString(CellType type) {

    switch (type) {
    case CellType::BasePair: return "Base Pair";
    case CellType::LoopPair: return "Loop Pair";
    case CellType::SkipPair: return "Skip Pair";
    case CellType::ALL: return "All";
    default: return "Unknown";
    }

}

std::string ADNCell::getCellTypeString() const {

    return ADNCell::getCellTypeString(GetCellType());

}

void ADNBasePair::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    ADNCell::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("left", nodeIndexer.getIndex(leftNucleotide()));
    serializer->writeUnsignedIntElement("right", nodeIndexer.getIndex(rightNucleotide()));

}

void ADNBasePair::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    ADNCell::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    const unsigned int lIdx = serializer->readUnsignedIntElement();
    const unsigned int rIdx = serializer->readUnsignedIntElement();

    SetLeftNucleotide(ADNNodeValidation::GetSerializedAdenitaNode<ADNNucleotide>(nodeIndexer, lIdx, "ADNNucleotide"));
    SetRightNucleotide(ADNNodeValidation::GetSerializedAdenitaNode<ADNNucleotide>(nodeIndexer, rIdx, "ADNNucleotide"));

}

SBPointer<ADNNucleotide> ADNBasePair::GetLeftNucleotide() const {
    return leftNucleotide;
}

SBNode* ADNBasePair::getLeftNucleotide() const {
    return leftNucleotide();
}

void ADNBasePair::SetLeftNucleotide(SBPointer<ADNNucleotide> nt) {
    this->leftNucleotide = nt;
}

SBPointer<ADNNucleotide> ADNBasePair::GetRightNucleotide() const {
    return rightNucleotide;
}

SBNode* ADNBasePair::getRightNucleotide() const {
    return rightNucleotide();
}

void ADNBasePair::SetRightNucleotide(SBPointer<ADNNucleotide> nt) {
    this->rightNucleotide = nt;
}

void ADNBasePair::SetRemainingNucleotide(SBPointer<ADNNucleotide> nt) {

    if (leftNucleotide != nullptr && rightNucleotide == nullptr) {
        SetRightNucleotide(nt);
    }
    else if (leftNucleotide == nullptr && rightNucleotide != nullptr) {
        SetLeftNucleotide(nt);
    }

    PairNucleotides();

}

void ADNBasePair::AddPair(SBPointer<ADNNucleotide> left, SBPointer<ADNNucleotide> right) {

    SetLeftNucleotide(left);
    SetRightNucleotide(right);
    if (left != nullptr) left->SetPair(right);
    if (right != nullptr) right->SetPair(left);

}

void ADNBasePair::PairNucleotides() {

    if (leftNucleotide != nullptr && rightNucleotide != nullptr) {

        leftNucleotide->SetPair(rightNucleotide);
        rightNucleotide->SetPair(leftNucleotide);

    }

}

void ADNBasePair::RemoveNucleotide(SBPointer<ADNNucleotide> nt) {

    if (leftNucleotide == nt)
        leftNucleotide = nullptr;
    else if (rightNucleotide == nt)
        rightNucleotide = nullptr;

}

SBPointerIndexer<ADNNucleotide> ADNBasePair::GetNucleotides() const {

    SBPointerIndexer<ADNNucleotide> nts;
    if (leftNucleotide != nullptr) nts.addReferenceTarget(leftNucleotide());
    if (rightNucleotide != nullptr) nts.addReferenceTarget(rightNucleotide());
    return nts;

}

bool ADNBasePair::IsLeft(SBPointer<ADNNucleotide> nt) const {

    return bool(leftNucleotide == nt);

}

bool ADNBasePair::IsRight(SBPointer<ADNNucleotide> nt) const {

    return bool(rightNucleotide == nt);

}

void ADNSkipPair::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {
    ADNCell::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);
}

void ADNSkipPair::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {
    ADNCell::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);
}

void ADNSkipPair::RemoveNucleotide(SBPointer<ADNNucleotide> nt) {
}

void ADNLoopPair::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    ADNCell::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("left", nodeIndexer.getIndex(leftLoop()));
    serializer->writeUnsignedIntElement("right", nodeIndexer.getIndex(rightLoop()));

}

void ADNLoopPair::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    ADNCell::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int lIdx = serializer->readUnsignedIntElement();
    unsigned int rIdx = serializer->readUnsignedIntElement();
    SBPointer<ADNLoop> lp = ADNNodeValidation::GetSerializedAdenitaNode<ADNLoop>(nodeIndexer, lIdx, "ADNLoop");
    SBPointer<ADNLoop> rp = ADNNodeValidation::GetSerializedAdenitaNode<ADNLoop>(nodeIndexer, rIdx, "ADNLoop");
    if (lp != nullptr) SetLeftLoop(lp);
    if (rp != nullptr) SetRightLoop(rp);

}

SBPointer<ADNLoop> ADNLoopPair::GetLeftLoop() const {
    return leftLoop;
}

SBNode* ADNLoopPair::getLeftLoop() const {
    return leftLoop();
}

void ADNLoopPair::SetLeftLoop(SBPointer<ADNLoop> lp) {

    SBPointer<ADNLoop> previousLoop = this->leftLoop;
    const bool replacingLoop = previousLoop() != lp();
    if (previousLoop != nullptr && replacingLoop && previousLoop() != rightLoop()) {
        removeChild(previousLoop());
    }

    this->leftLoop = lp;
    if (this->leftLoop != nullptr) {

        if (replacingLoop) addChild(this->leftLoop());
        this->leftLoop->setName("Left loop " + std::to_string(this->leftLoop->getNodeIndex()));
        this->leftLoop->setStructuralID(this->leftLoop->getNodeIndex());

    }

}

SBPointer<ADNLoop> ADNLoopPair::GetRightLoop() const {
    return rightLoop;
}

SBNode* ADNLoopPair::getRightLoop() const {
    return rightLoop();
}

void ADNLoopPair::SetRightLoop(SBPointer<ADNLoop> lp) {

    SBPointer<ADNLoop> previousLoop = this->rightLoop;
    const bool replacingLoop = previousLoop() != lp();
    if (previousLoop != nullptr && replacingLoop && previousLoop() != leftLoop()) {
        removeChild(previousLoop());
    }

    this->rightLoop = lp;
    if (this->rightLoop != nullptr) {

        if (replacingLoop) addChild(this->rightLoop());
        this->rightLoop->setName("Right loop " + std::to_string(this->rightLoop->getNodeIndex()));
        this->rightLoop->setStructuralID(this->rightLoop->getNodeIndex());

    }

}

void ADNLoopPair::RemoveNucleotide(SBPointer<ADNNucleotide> nt) {

    if (leftLoop != nullptr)
        leftLoop->RemoveNucleotide(nt);

    if (rightLoop != nullptr)
        rightLoop->RemoveNucleotide(nt);

}

bool ADNLoopPair::IsRight(SBPointer<ADNNucleotide> nt) const {

    if (rightLoop != nullptr) {

        auto nts = rightLoop->GetNucleotides();
        SB_FOR(SBPointer<ADNNucleotide> c, nts) {

            if (c == nt)
                return true;

        }

    }

    return false;

}

bool ADNLoopPair::IsLeft(SBPointer<ADNNucleotide> nt) const {

    if (leftLoop != nullptr) {

        auto nts = leftLoop->GetNucleotides();
        SB_FOR(SBPointer<ADNNucleotide> c, nts) {

            if (c == nt)
                return true;

        }

    }

    return false;

}

SBPointerIndexer<ADNNucleotide> ADNLoopPair::GetNucleotides() const {

    SBPointerIndexer<ADNNucleotide> nts;

    if (leftLoop != nullptr) {

        auto leftNts = leftLoop->GetNucleotides();
        SB_FOR(SBPointer<ADNNucleotide> n, leftNts) {
            nts.addReferenceTarget(n());
        }

    }

    if (rightLoop != nullptr) {

        auto rightNts = rightLoop->GetNucleotides();
        SB_FOR(SBPointer<ADNNucleotide> n, rightNts) {
            nts.addReferenceTarget(n());
        }

    }

    return nts;

}

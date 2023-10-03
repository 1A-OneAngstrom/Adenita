#include "ADNCell.hpp"
#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"

void ADNCell::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

void ADNCell::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

}

std::string ADNCell::getCellTypeString(CellType type) {

    std::string typeStr = "Unknown";
    if (type == CellType::BasePair) {
        typeStr = "Base Pair";
    }
    else if (type == CellType::LoopPair) {
        typeStr = "Loop Pair";
    }
    else if (type == CellType::SkipPair) {
        typeStr = "Skip Pair";
    }
    else if (type == CellType::ALL) {
        typeStr = "All";
    }

    return typeStr;

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

    SBNode* lNode = nodeIndexer.getNode(lIdx);
    SetLeftNucleotide(static_cast<ADNNucleotide*>(lNode));

    SBNode* rNode = nodeIndexer.getNode(rIdx);
    SetRightNucleotide(static_cast<ADNNucleotide*>(rNode));

}

ADNPointer<ADNNucleotide> ADNBasePair::GetLeftNucleotide() const {
    return leftNucleotide;
}

SBNode* ADNBasePair::getLeft() const {
    return leftNucleotide();
}

void ADNBasePair::SetLeftNucleotide(ADNPointer<ADNNucleotide> nt) {
    this->leftNucleotide = nt;
}

ADNPointer<ADNNucleotide> ADNBasePair::GetRightNucleotide() const {
    return rightNucleotide;
}

SBNode* ADNBasePair::getRight() const {
    return rightNucleotide();
}

void ADNBasePair::SetRightNucleotide(ADNPointer<ADNNucleotide> nt) {
    this->rightNucleotide = nt;
}

void ADNBasePair::SetRemainingNucleotide(ADNPointer<ADNNucleotide> nt) {

    if (leftNucleotide != nullptr && rightNucleotide == nullptr) {
        SetRightNucleotide(nt);
    }
    else if (leftNucleotide == nullptr && rightNucleotide != nullptr) {
        SetLeftNucleotide(nt);
    }
    PairNucleotides();

}

void ADNBasePair::AddPair(ADNPointer<ADNNucleotide> left, ADNPointer<ADNNucleotide> right) {

    SetLeftNucleotide(left);
    SetRightNucleotide(right);
    if (left != nullptr) left->SetPair(right);
    if (rightNucleotide != nullptr) right->SetPair(left);

}

void ADNBasePair::PairNucleotides() {

    if (leftNucleotide != nullptr && rightNucleotide != nullptr) {

        leftNucleotide->SetPair(rightNucleotide);
        rightNucleotide->SetPair(leftNucleotide);

    }

}

void ADNBasePair::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {

    if (leftNucleotide == nt) {
        leftNucleotide = nullptr;
    }
    else if (rightNucleotide == nt) {
        rightNucleotide = nullptr;
    }

}

CollectionMap<ADNNucleotide> ADNBasePair::GetNucleotides() const {

    CollectionMap<ADNNucleotide> nts;
    if (leftNucleotide != nullptr) nts.addReferenceTarget(leftNucleotide());
    if (rightNucleotide != nullptr) nts.addReferenceTarget(rightNucleotide());
    return nts;

}

bool ADNBasePair::IsLeft(ADNPointer<ADNNucleotide> nt) const {

    return bool(leftNucleotide == nt);

}

bool ADNBasePair::IsRight(ADNPointer<ADNNucleotide> nt) const {

    return bool(rightNucleotide == nt);

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

    serializer->writeUnsignedIntElement("left", nodeIndexer.getIndex(leftLoop()));
    serializer->writeUnsignedIntElement("right", nodeIndexer.getIndex(rightLoop()));

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

ADNPointer<ADNLoop> ADNLoopPair::GetLeftLoop() const {
    return leftLoop;
}

SBNode* ADNLoopPair::getLeft() const {
    return leftLoop();
}

void ADNLoopPair::SetLeftLoop(ADNPointer<ADNLoop> lp) {

    this->leftLoop = lp;
    addChild(lp());
    lp->setName("Left loop " + std::to_string(lp->getNodeIndex()));

}

ADNPointer<ADNLoop> ADNLoopPair::GetRightLoop() const {
    return rightLoop;
}

SBNode* ADNLoopPair::getRight() const {
    return rightLoop();
}

void ADNLoopPair::SetRightLoop(ADNPointer<ADNLoop> lp) {

    this->rightLoop = lp;
    addChild(lp());
    lp->setName("Right loop " + std::to_string(lp->getNodeIndex()));

}

void ADNLoopPair::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {

    if (leftLoop != nullptr) {
        leftLoop->RemoveNucleotide(nt);
    }
    if (rightLoop != nullptr) {
        rightLoop->RemoveNucleotide(nt);
    }

}

bool ADNLoopPair::IsRight(ADNPointer<ADNNucleotide> nt) const {

    if (rightLoop != nullptr) {

        auto nts = rightLoop->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> c, nts) {

            if (c == nt)
                return true;

        }

    }

    return false;

}

bool ADNLoopPair::IsLeft(ADNPointer<ADNNucleotide> nt) const {

    if (leftLoop != nullptr) {

        auto nts = leftLoop->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> c, nts) {

            if (c == nt)
                return true;

        }

    }

    return false;

}

CollectionMap<ADNNucleotide> ADNLoopPair::GetNucleotides() const {

    CollectionMap<ADNNucleotide> nts;

    if (leftLoop != nullptr) {

        auto leftNts = leftLoop->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> n, leftNts) {
            nts.addReferenceTarget(n());
        }

    }

    if (rightLoop != nullptr) {

        auto rightNts = rightLoop->GetNucleotides();
        SB_FOR(ADNPointer<ADNNucleotide> n, rightNts) {
            nts.addReferenceTarget(n());
        }

    }

    return nts;

}

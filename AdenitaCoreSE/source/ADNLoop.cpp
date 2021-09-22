//#include "ADNLoop.hpp"
//#include "ADNBaseSegment.hpp"
//#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"


void ADNLoop::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("startNt", nodeIndexer.getIndex(startNt_()));
    serializer->writeUnsignedIntElement("endNt", nodeIndexer.getIndex(endNt_()));
    serializer->writeUnsignedIntElement("numNt", getNumberOfNucleotides());
    serializer->writeStartElement("nucleotides");

    SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides_) {
        serializer->writeUnsignedIntElement("id", nodeIndexer.getIndex(nt()));
    }

    serializer->writeEndElement();

}

void ADNLoop::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int sIdx = serializer->readUnsignedIntElement();
    unsigned int eIdx = serializer->readUnsignedIntElement();
    unsigned int numNt = serializer->readUnsignedIntElement();
    serializer->readStartElement();

    for (unsigned int i = 0; i < numNt; ++i) {

        unsigned int idx = serializer->readUnsignedIntElement();
        SBNode* node = nodeIndexer.getNode(idx);
        ADNPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(node);
        AddNucleotide(nt);

    }

    serializer->readEndElement();

    SBNode* sNode = nodeIndexer.getNode(sIdx);
    SetStart(static_cast<ADNNucleotide*>(sNode));
    SBNode* eNode = nodeIndexer.getNode(eIdx);
    SetEnd(static_cast<ADNNucleotide*>(eNode));

}

void ADNLoop::SetStart(ADNPointer<ADNNucleotide> nt) {
    startNt_ = nt;
}

ADNPointer<ADNNucleotide> ADNLoop::GetStart() {
    return startNt_;
}

SBNode* ADNLoop::getStartNucleotide() const {
    return startNt_();
}

void ADNLoop::SetEnd(ADNPointer<ADNNucleotide> nt) {
    endNt_ = nt;
}

ADNPointer<ADNNucleotide> ADNLoop::GetEnd() {
    return endNt_;
}

SBNode* ADNLoop::getEndNucleotide() const {
    return endNt_();
}

int ADNLoop::getNumberOfNucleotides() const {
    return static_cast<int>(GetNucleotides().size());
}

std::string ADNLoop::getLoopSequence() const {

    std::string seq = "";
    ADNPointer<ADNNucleotide> nt = startNt_;
    while (nt != endNt_->GetNext()) {

        DNABlocks t = nt->GetType();
        seq += ADNModel::GetResidueName(t);
        nt = nt->GetNext();

    }

    return seq;

}

void ADNLoop::SetBaseSegment(ADNPointer<ADNBaseSegment> bs, bool setPositions) {

    auto nts = GetNucleotides();
    SB_FOR(ADNPointer<ADNNucleotide> nt, nts) {

        nt->SetBaseSegment(bs);
        if (setPositions) {

            nt->SetPosition(bs->GetPosition());
            nt->SetBackbonePosition(bs->GetPosition());
            nt->SetSidechainPosition(bs->GetPosition());

        }

    }

}

CollectionMap<ADNNucleotide> ADNLoop::GetNucleotides() const {
    return nucleotides_;
}

void ADNLoop::AddNucleotide(ADNPointer<ADNNucleotide> nt) {
    nucleotides_.addReferenceTarget(nt());
}

void ADNLoop::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {
    nucleotides_.removeReferenceTarget(nt());
}

bool ADNLoop::IsEmpty() const {
    return bool(getNumberOfNucleotides() == 0);
}

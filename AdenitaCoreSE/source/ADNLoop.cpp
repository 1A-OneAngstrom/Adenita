#include "ADNLoop.hpp"
#include "ADNBaseSegment.hpp"
#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"


void ADNLoop::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("startNt", nodeIndexer.getIndex(startNucleotide()));
    serializer->writeUnsignedIntElement("endNt", nodeIndexer.getIndex(endNucleotide()));
    serializer->writeUnsignedIntElement("numNt", getNumberOfNucleotides());
    serializer->writeStartElement("nucleotides");

    SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides_) {
        serializer->writeUnsignedIntElement("id", nodeIndexer.getIndex(nt()));
    }

    serializer->writeEndElement();

}

void ADNLoop::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    const unsigned int sIdx = serializer->readUnsignedIntElement();
    const unsigned int eIdx = serializer->readUnsignedIntElement();
    const unsigned int numNt = serializer->readUnsignedIntElement();
    serializer->readStartElement();

    for (unsigned int i = 0; i < numNt; ++i) {

        const unsigned int idx = serializer->readUnsignedIntElement();
        SBNode* node = nodeIndexer.getNode(idx);
        if (node) {
            
            ADNPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(node);
            AddNucleotide(nt);

        }

    }

    serializer->readEndElement();

    SBNode* sNode = nodeIndexer.getNode(sIdx);
    if (sNode) SetStart(static_cast<ADNNucleotide*>(sNode));
    SBNode* eNode = nodeIndexer.getNode(eIdx);
    if (eNode) SetEnd(static_cast<ADNNucleotide*>(eNode));

}

void ADNLoop::SetStart(ADNPointer<ADNNucleotide> nucleotide) {

    this->startNucleotide = nucleotide;

}

ADNPointer<ADNNucleotide> ADNLoop::GetStart() const {

    return startNucleotide;

}

SBNode* ADNLoop::getStartNucleotide() const {

    return startNucleotide();

}

void ADNLoop::SetEnd(ADNPointer<ADNNucleotide> nucleotide) {

    this->endNucleotide = nucleotide;

}

ADNPointer<ADNNucleotide> ADNLoop::GetEnd() const {

    return endNucleotide;

}

SBNode* ADNLoop::getEndNucleotide() const {

    return endNucleotide();

}

std::string ADNLoop::getLoopSequence() const {

    std::string seq = "";
    ADNPointer<ADNNucleotide> currentNucleotide = startNucleotide;
    while (currentNucleotide != nullptr && currentNucleotide != endNucleotide->GetNext()) {

        seq += currentNucleotide->getNucleotideTypeString();
        currentNucleotide = currentNucleotide->GetNext();

    }

    return seq;

}

void ADNLoop::SetBaseSegment(ADNPointer<ADNBaseSegment> baseSegment, bool setPositions) {

    SB_FOR(ADNPointer<ADNNucleotide> nucleotide, nucleotides_) {

        if (nucleotide == nullptr) continue;

        nucleotide->SetBaseSegment(baseSegment);
        if (setPositions) {

            const Position3D baseSegmentPosition = baseSegment->GetPosition();

            nucleotide->SetPosition(baseSegmentPosition);
            nucleotide->SetBackbonePosition(baseSegmentPosition);
            nucleotide->SetSidechainPosition(baseSegmentPosition);

        }

    }

}

int ADNLoop::getNumberOfNucleotides() const {

    return static_cast<int>(GetNucleotides().size());

}

CollectionMap<ADNNucleotide> ADNLoop::GetNucleotides() const {

    return nucleotides_;

}

void ADNLoop::AddNucleotide(ADNPointer<ADNNucleotide> nucleotide) {

    nucleotides_.addReferenceTarget(nucleotide());

}

void ADNLoop::RemoveNucleotide(ADNPointer<ADNNucleotide> nucleotide) {

    if (startNucleotide == nucleotide) startNucleotide = nullptr;
    if (endNucleotide == nucleotide) endNucleotide = nullptr;

    nucleotides_.removeReferenceTarget(nucleotide());

}

bool ADNLoop::IsEmpty() const {

    return bool(getNumberOfNucleotides() == 0);

}

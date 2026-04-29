#include "ADNPart.hpp"
#include "SEAdenitaCoreSEApp.hpp"

ADNPart::ADNPart() : SBStructuralModel() {
    
    InitBoundingBox();

}

ADNPart::ADNPart(const ADNPart & n) : SBStructuralModel(n) {

    *this = n;

}

ADNPart & ADNPart::operator=(const ADNPart& other) {

    SBStructuralModel::operator =(other);

    if (this != &other) {
    
    }

    return *this;

}

void ADNPart::serialize(SBCSerializer * serializer, const SBNodeIndexer & nodeIndexer, const SBVersionNumber & sdkVersionNumber, const SBVersionNumber & classVersionNumber) const {

    SBStructuralModel::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeUnsignedIntElement("ntId", nucleotideId_);
    serializer->writeUnsignedIntElement("ssId", singleStrandId_);
    serializer->writeUnsignedIntElement("dsId", doubleStrandId_);

    // atoms
    auto atomIndexer = GetAtoms();
    serializer->writeUnsignedIntElement("numAtoms", atomIndexer.size());
    serializer->writeStartElement("atoms");
    SB_FOR(SBPointer<ADNAtom> n, atomIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("atomIndex", idx);

    }
    serializer->writeEndElement();
    //end atoms

    // nucleotides
    auto nucleotideIndexer = GetNucleotides();
    serializer->writeUnsignedIntElement("numNucleotides", nucleotideIndexer.size());
    serializer->writeStartElement("nucleotides");
    SB_FOR(SBPointer<ADNNucleotide> n, nucleotideIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("ntIndex", idx);

    }
    serializer->writeEndElement();
    //end nucleotides

    // single strands
    auto singleStrandIndexer = GetSingleStrands();
    serializer->writeUnsignedIntElement("numSingleStrands", singleStrandIndexer.size());
    serializer->writeStartElement("singleStrands");
    SB_FOR(SBPointer<ADNSingleStrand> n, singleStrandIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("ssIndex", idx);

    }
    serializer->writeEndElement();
    //end single strands

    // base segments
    auto baseSegmentIndexer = GetBaseSegments();
    serializer->writeUnsignedIntElement("numBaseSegments", baseSegmentIndexer.size());
    serializer->writeStartElement("baseSegments");
    SB_FOR(SBPointer<ADNBaseSegment> n, baseSegmentIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("bsIndex", idx);

    }
    serializer->writeEndElement();
    //end base segments

    // double strands
    auto doubleStrandIndexer = GetDoubleStrands();
    serializer->writeUnsignedIntElement("numDoubleStrands", doubleStrandIndexer.size());
    serializer->writeStartElement("double strands");
    SB_FOR(SBPointer<ADNDoubleStrand> n, doubleStrandIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("dsIndex", idx);

    }
    serializer->writeEndElement();
    //end double strands

}

void ADNPart::unserialize(SBCSerializer * serializer, const SBNodeIndexer & nodeIndexer, const SBVersionNumber & sdkVersionNumber, const SBVersionNumber & classVersionNumber) {

    SBStructuralModel::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    nucleotideId_ = serializer->readUnsignedIntElement();
    singleStrandId_ = serializer->readUnsignedIntElement();
    doubleStrandId_ = serializer->readUnsignedIntElement();

    // atoms index
    unsigned int numAtoms = serializer->readUnsignedIntElement();
    serializer->readStartElement();
    for (unsigned int i = 0; i < numAtoms; ++i) {

        unsigned int idx = serializer->readUnsignedIntElement();
#if ADENITA_ADNPART_REGISTER_ATOMS
        atomsIndex_.addReferenceTarget((ADNAtom*)nodeIndexer.getNode(idx));
#endif

    }
    serializer->readEndElement();

    // nucleotides index
    unsigned int numNucleotides = serializer->readUnsignedIntElement();
    serializer->readStartElement();
    for (unsigned int i = 0; i < numNucleotides; ++i) {

        unsigned int idx = serializer->readUnsignedIntElement();
#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
        nucleotidesIndex_.addReferenceTarget((ADNNucleotide*)nodeIndexer.getNode(idx));
#endif

    }
    serializer->readEndElement();

    // single strands index
    unsigned int numSingleStrands = serializer->readUnsignedIntElement();
    serializer->readStartElement();
    for (unsigned int i = 0; i < numSingleStrands; ++i) {

        unsigned int idx = serializer->readUnsignedIntElement();
#if ADENITA_ADNPART_REGISTER_STRANDS
        singleStrandsIndex_.addReferenceTarget((ADNSingleStrand*)nodeIndexer.getNode(idx));
#else
        // TODO
#endif

    }
    serializer->readEndElement();

    // base segments
    const unsigned int numBaseSegments = serializer->readUnsignedIntElement();
    serializer->readStartElement();
    for (unsigned int i = 0; i < numBaseSegments; ++i) {

        unsigned int idx = serializer->readUnsignedIntElement();
#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
        baseSegmentsIndex_.addReferenceTarget((ADNBaseSegment*)nodeIndexer.getNode(idx));
#else
        // TODO
#endif

    }
    serializer->readEndElement();

    // double strands
    const unsigned int numDoubleStrands = serializer->readUnsignedIntElement();
    serializer->readStartElement();
    for (unsigned int i = 0; i < numDoubleStrands; ++i) {

        unsigned int idx = serializer->readUnsignedIntElement();
#if ADENITA_ADNPART_REGISTER_STRANDS
        doubleStrandsIndex_.addReferenceTarget((ADNDoubleStrand*)nodeIndexer.getNode(idx));
#else
        // TODO
#endif

    }
    serializer->readEndElement();

    setLoadedViaSAMSON(true);

    // does not fully work: the VM is created but is not visible - needs a request for update after all the nodes were created
    //if (this->getParent()) {
    //
    //    SEAdenitaCoreSEApp::resetVisualModel(this->getParent());
    //    SAMSON::requestViewportUpdate();
    //    SAMSON::getActiveCamera()->center();
    //
    //}

}

SBPointerIndexer<ADNBaseSegment> ADNPart::GetBaseSegments(CellType cellType) const {

#if 0//ADENITA_ADNPART_REGISTER_BASESEGMENTS
    auto baseSegmentIndexer = baseSegmentsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SBPointerIndexer<ADNBaseSegment> baseSegmentIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        baseSegmentIndexer.addReferenceTarget(static_cast<ADNBaseSegment*>(node));
#endif

    SBPointerIndexer<ADNBaseSegment> bsList;
    if (cellType == CellType::ALL) {

        bsList = baseSegmentIndexer;

    }
    else {

        SB_FOR(SBPointer<ADNBaseSegment> bs, baseSegmentIndexer) {

            if (bs->GetCellType() == cellType)
                bsList.addReferenceTarget(bs());

        }

    }

    return bsList;

}

SBPointerIndexer<ADNSingleStrand> ADNPart::GetScaffolds() const {

    SBPointerIndexer<ADNSingleStrand> chainList;

    auto singleStrandIndexer = GetSingleStrands();

    SB_FOR(SBPointer<ADNSingleStrand> singleStrand, singleStrandIndexer) {

        if (singleStrand->IsScaffold())
            chainList.addReferenceTarget(singleStrand());

    }

    return chainList;

}

SBPointerIndexer<ADNNucleotide> ADNPart::GetNucleotides(CellType cellType) const {

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    return nucleotidesIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SBPointerIndexer<ADNNucleotide> nucleotideIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        nucleotideIndexer.addReferenceTarget(static_cast<ADNNucleotide*>(node));

    return nucleotideIndexer;
#endif

}

int ADNPart::GetNumberOfNucleotides() const {
    
#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    return static_cast<int>(GetNucleotides().size());
#else
    return countNodes((SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#endif

}

int ADNPart::getNumberOfNucleotides() const {
    return GetNumberOfNucleotides();
}

int ADNPart::GetNumberOfAtoms() const {

#if ADENITA_ADNPART_REGISTER_ATOMS
    return static_cast<int>(GetAtoms().size());
#else
    return countNodes((SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#endif

}

int ADNPart::getNumberOfAtoms() const {
    return GetNumberOfAtoms();
}

int ADNPart::GetNumberOfBaseSegments() const {

#if 0//ADENITA_ADNPART_REGISTER_BASESEGMENTS
    return static_cast<int>(GetBaseSegments().size());
#else
    return countNodes((SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#endif

}

int ADNPart::getNumberOfBaseSegments() const {
    return GetNumberOfBaseSegments();
}

SBPointerIndexer<ADNSingleStrand> ADNPart::GetSingleStrands() const {

#if ADENITA_ADNPART_REGISTER_STRANDS
    return singleStrandsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SBPointerIndexer<ADNSingleStrand> strandIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        strandIndexer.addReferenceTarget(static_cast<ADNSingleStrand*>(node));

    return strandIndexer;
#endif

}

SBPointerIndexer<ADNDoubleStrand> ADNPart::GetDoubleStrands() const {

#if ADENITA_ADNPART_REGISTER_STRANDS
    return doubleStrandsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SBPointerIndexer<ADNDoubleStrand> strandIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        strandIndexer.addReferenceTarget(static_cast<ADNDoubleStrand*>(node));

    return strandIndexer;
#endif

}

SBPointerIndexer<ADNAtom> ADNPart::GetAtoms() const {

#if ADENITA_ADNPART_REGISTER_ATOMS
    return atomsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SBPointerIndexer<ADNAtom> atomIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        atomIndexer.addReferenceTarget(static_cast<ADNAtom*>(node));

    return atomIndexer;
#endif

}

int ADNPart::GetNumberOfDoubleStrands() const {

#if ADENITA_ADNPART_REGISTER_STRANDS
    return static_cast<int>(GetDoubleStrands().size());
#else
    return countNodes((SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#endif

}

int ADNPart::getNumberOfDoubleStrands() const {
    return GetNumberOfDoubleStrands();
}

int ADNPart::GetNumberOfSingleStrands() const {

#if ADENITA_ADNPART_REGISTER_STRANDS
    return static_cast<int>(GetSingleStrands().size());
#else
    return countNodes((SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#endif

}

int ADNPart::getNumberOfSingleStrands() const {
    return GetNumberOfSingleStrands();
}

void ADNPart::DeregisterSingleStrand(SBPointer<ADNSingleStrand> ss, bool removeFromParent, bool removeFromIndex) {

    if (ss == nullptr) return;

    if (removeFromParent)
        removeChild(ss());

#if ADENITA_ADNPART_REGISTER_STRANDS
    if (removeFromIndex) singleStrandsIndex_.removeReferenceTarget(ss());
#endif

}

void ADNPart::DeregisterNucleotide(SBPointer<ADNNucleotide> nt, bool removeFromSs, bool removeFromBs, bool removeFromIndex) {

    if (nt == nullptr) return;

    if (removeFromSs) {

        SBPointer<ADNSingleStrand> ss = nt->GetStrand();
        if (ss != nullptr) ss->removeChild(nt());

    }

    if (removeFromBs) {

        auto bs = nt->GetBaseSegment();
        if (bs != nullptr) bs->RemoveNucleotide(nt);

    }
  
#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    if (removeFromIndex) nucleotidesIndex_.removeReferenceTarget(nt());
#endif

}

void ADNPart::DeregisterDoubleStrand(SBPointer<ADNDoubleStrand> ds, bool removeFromParent, bool removeFromIndex) {

    if (ds == nullptr) return;

    if (removeFromParent)
        removeChild(ds());

#if ADENITA_ADNPART_REGISTER_STRANDS
    if (removeFromIndex) doubleStrandsIndex_.removeReferenceTarget(ds());
#endif

}

void ADNPart::DeregisterBaseSegment(SBPointer<ADNBaseSegment> bs, bool removeFromDs, bool removeFromIndex) {

    if (bs == nullptr) return;

    if (removeFromDs) 
        if (bs->getParent())
            bs->getParent()->removeChild(bs());

#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
    if (removeFromIndex) baseSegmentsIndex_.removeReferenceTarget(bs());
#endif

}

void ADNPart::DeregisterAtom(SBPointer<ADNAtom> atom, bool removeFromAtom) {

    if (atom == nullptr) return;

    if (removeFromAtom) 
        if (atom->getParent())
            atom->getParent()->removeChild(atom());

#if ADENITA_ADNPART_REGISTER_ATOMS
    atomsIndex_.removeReferenceTarget(atom());
#endif

}

bool ADNPart::isLoadedViaSAMSON() const noexcept {
    return loadedViaSAMSONFlag;
}

void ADNPart::setLoadedViaSAMSON(bool l) {
    loadedViaSAMSONFlag = l;
}

const SBIAPosition3& ADNPart::GetBoundingBox() const {
    return boundingBox;
}

void ADNPart::ResetBoundingBox() {

    auto nts = GetNucleotides();
    InitBoundingBox();
    SB_FOR(SBPointer<ADNNucleotide> nt, nts)
        SetBoundingBox(nt);

}

void ADNPart::SetBoundingBox(SBPointer<ADNNucleotide> newNt) {

    if (newNt == nullptr) return;

    const SBPosition3 pos = newNt->GetBackbonePosition();
    boundingBox.bound(pos);

}

void ADNPart::SetBoundingBox(SBPointer<ADNBaseSegment> newBs) {

    if (newBs == nullptr) return;

    const SBPosition3& pos = newBs->GetPosition();
    boundingBox.bound(pos);

}

void ADNPart::InitBoundingBox() {

    const SBQuantity::picometer maxVal = SBQuantity::picometer(std::numeric_limits<double>::max());
    boundingBox = SBIAPosition3(maxVal, -maxVal, maxVal, -maxVal, maxVal, -maxVal);

}

void ADNPart::RegisterSingleStrand(SBPointer<ADNSingleStrand> ss) {

    if (ss == nullptr) return;

    if (ss->getName().empty()) {

        ss->setName("Single strand " + std::to_string(singleStrandId_));
        ss->setStructuralID(singleStrandId_);
        ++singleStrandId_;

    }

    addChild(ss());

#if ADENITA_ADNPART_REGISTER_STRANDS
    singleStrandsIndex_.addReferenceTarget(ss());
#endif

}

void ADNPart::RegisterNucleotideThreePrime(SBPointer<ADNSingleStrand> ss, SBPointer<ADNNucleotide> nt, bool addToSs) {

    if (nt == nullptr) return;

    if (nt->getName().empty()) {

        nt->setName(nt->getNucleotideTypeString() + " " + std::to_string(nucleotideId_));
        nt->setStructuralID(nucleotideId_);
        ++nucleotideId_;

    }
    if (addToSs && ss != nullptr) ss->AddNucleotideThreePrime(nt);

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    nucleotidesIndex_.addReferenceTarget(nt());
#endif
    SetBoundingBox(nt);

}

void ADNPart::RegisterNucleotideFivePrime(SBPointer<ADNSingleStrand> ss, SBPointer<ADNNucleotide> nt, bool addToSs) {

    if (nt == nullptr) return;

    if (nt->getName().empty()) {

        nt->setName(nt->getNucleotideTypeString() + " " + std::to_string(nucleotideId_));
        nt->setStructuralID(nucleotideId_);
        ++nucleotideId_;

    }

    if (addToSs && ss != nullptr) ss->AddNucleotideFivePrime(nt);

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    nucleotidesIndex_.addReferenceTarget(nt());
#endif
    SetBoundingBox(nt);

}

void ADNPart::RegisterNucleotide(SBPointer<ADNSingleStrand> ss, SBPointer<ADNNucleotide> nt, SBPointer<ADNNucleotide> ntNext, bool addToSs) {

    if (nt == nullptr) return;

    if (nt->getName().empty()) {

        nt->setName(nt->getNucleotideTypeString() + " " + std::to_string(nucleotideId_));
        nt->setStructuralID(nucleotideId_);
        ++nucleotideId_;

    }

    if (addToSs && ss != nullptr) ss->AddNucleotide(nt, ntNext);

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    nucleotidesIndex_.addReferenceTarget(nt());
#endif
    SetBoundingBox(nt);

}

void ADNPart::RegisterAtom(SBPointer<ADNNucleotide> nt, NucleotideGroup g, SBPointer<ADNAtom> at, bool create) {

    if (!at.isValid()) return;

    if (create) {

        if (SAMSON::isHolding()) SAMSON::hold(at());
        at->create();

    }

    if (nt.isValid()) {

        if (at->getNucleotide() != nt())
            nt->addAtom(g, at);

    }

#if ADENITA_ADNPART_REGISTER_ATOMS
    atomsIndex_.addReferenceTarget(at());
#endif

}

void ADNPart::RegisterAtom(SBPointer<ADNBaseSegment> bs, SBPointer<ADNAtom> at, bool create) {

    if (at == nullptr) return;

    if (create) {

        if (SAMSON::isHolding()) SAMSON::hold(at());
        at->create();

    }

    if (bs != nullptr) bs->addChild(at());

#if ADENITA_ADNPART_REGISTER_ATOMS
    atomsIndex_.addReferenceTarget(at());
#endif

}

void ADNPart::RegisterBaseSegmentEnd(SBPointer<ADNDoubleStrand> ds, SBPointer<ADNBaseSegment> bs, bool addToDs) {

    if (bs == nullptr) return;

    if (addToDs && ds != nullptr) ds->AddBaseSegmentEnd(bs);

#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
    baseSegmentsIndex_.addReferenceTarget(bs());
#endif
    //SetBoundingBox(bs);

}

unsigned int ADNPart::GetBaseSegmentIndex(SBPointer<ADNBaseSegment> bs) const {

    return baseSegmentsIndex_.getIndex(bs());

}

void ADNPart::RegisterDoubleStrand(SBPointer<ADNDoubleStrand> ds) {

    if (ds == nullptr) return;

    if (ds->getName().empty()) {

        ds->setName("Double strand " + std::to_string(doubleStrandId_));
        ds->setStructuralID(doubleStrandId_);
        ++doubleStrandId_;

    }
    
    addChild(ds());

#if ADENITA_ADNPART_REGISTER_STRANDS
    doubleStrandsIndex_.addReferenceTarget(ds());
#endif

}

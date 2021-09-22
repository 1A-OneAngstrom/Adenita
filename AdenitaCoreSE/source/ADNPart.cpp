#include "ADNPart.hpp"

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
    SB_FOR(ADNPointer<ADNAtom> n, atomIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("atomIndex", idx);

    }
    serializer->writeEndElement();
    //end atoms

    // nucleotides
    auto nucleotideIndexer = GetNucleotides();
    serializer->writeUnsignedIntElement("numNucleotides", nucleotideIndexer.size());
    serializer->writeStartElement("nucleotides");
    SB_FOR(ADNPointer<ADNNucleotide> n, nucleotideIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("ntIndex", idx);

    }
    serializer->writeEndElement();
    //end nucleotides

    // single strands
    auto singleStrandIndexer = GetSingleStrands();
    serializer->writeUnsignedIntElement("numSingleStrands", singleStrandIndexer.size());
    serializer->writeStartElement("singleStrands");
    SB_FOR(ADNPointer<ADNSingleStrand> n, singleStrandIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("ssIndex", idx);

    }
    serializer->writeEndElement();
    //end single strands

    // base segments
    auto baseSegmentIndexer = GetBaseSegments();
    serializer->writeUnsignedIntElement("numBaseSegments", baseSegmentIndexer.size());
    serializer->writeStartElement("baseSegments");
    SB_FOR(ADNPointer<ADNBaseSegment> n, baseSegmentIndexer) {

        unsigned int idx = nodeIndexer.getIndex(n());
        serializer->writeUnsignedIntElement("bsIndex", idx);

    }
    serializer->writeEndElement();
    //end base segments

    // double strands
    auto doubleStrandIndexer = GetDoubleStrands();
    serializer->writeUnsignedIntElement("numDoubleStrands", doubleStrandIndexer.size());
    serializer->writeStartElement("double strands");
    SB_FOR(ADNPointer<ADNDoubleStrand> n, doubleStrandIndexer) {

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
    unsigned int numBaseSegments = serializer->readUnsignedIntElement();
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
    unsigned int numDoubleStrands = serializer->readUnsignedIntElement();
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

}

CollectionMap<ADNBaseSegment> ADNPart::GetBaseSegments(CellType celltype) const {

#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
    auto baseSegmentIndexer = baseSegmentsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    CollectionMap<ADNBaseSegment> baseSegmentIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        baseSegmentIndexer.addReferenceTarget(static_cast<ADNBaseSegment*>(node));
#endif

    CollectionMap<ADNBaseSegment> bsList;
    if (celltype == CellType::ALL) {

        bsList = baseSegmentIndexer;

    }
    else {

        SB_FOR(ADNPointer<ADNBaseSegment> bs, baseSegmentIndexer) {

            if (bs->GetCellType() == celltype)
                bsList.addReferenceTarget(bs());

        }

    }

    return bsList;

}

CollectionMap<ADNSingleStrand> ADNPart::GetScaffolds() const {

    CollectionMap<ADNSingleStrand> chainList;

    auto singleStrandIndexer = GetSingleStrands();

    SB_FOR(ADNPointer<ADNSingleStrand> singleStrand, singleStrandIndexer) {

        if (singleStrand->IsScaffold())
            chainList.addReferenceTarget(singleStrand());

    }

    return chainList;

}

CollectionMap<ADNNucleotide> ADNPart::GetNucleotides(CellType celltype) const {

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    return nucleotidesIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    CollectionMap<ADNNucleotide> nucleotideIndexer;
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

#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
    return static_cast<int>(GetBaseSegments().size());
#else
    return countNodes((SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#endif

}

int ADNPart::getNumberOfBaseSegments() const {
    return GetNumberOfBaseSegments();
}

CollectionMap<ADNSingleStrand> ADNPart::GetSingleStrands() const {

#if ADENITA_ADNPART_REGISTER_STRANDS
    return singleStrandsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    CollectionMap<ADNSingleStrand> strandIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        strandIndexer.addReferenceTarget(static_cast<ADNSingleStrand*>(node));

    return strandIndexer;
#endif

}

CollectionMap<ADNDoubleStrand> ADNPart::GetDoubleStrands() const {

#if ADENITA_ADNPART_REGISTER_STRANDS
    return doubleStrandsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    CollectionMap<ADNDoubleStrand> strandIndexer;
    SB_FOR(SBNode * node, nodeIndexer)
        strandIndexer.addReferenceTarget(static_cast<ADNDoubleStrand*>(node));

    return strandIndexer;
#endif

}

CollectionMap<ADNAtom> ADNPart::GetAtoms() const {

#if ADENITA_ADNPART_REGISTER_ATOMS
    return atomsIndex_;
#else
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    CollectionMap<ADNAtom> atomIndexer;
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

void ADNPart::DeregisterSingleStrand(ADNPointer<ADNSingleStrand> ss, bool removeFromParent, bool removeFromIndex) {

    if (removeFromParent)
        removeChild(ss());

#if ADENITA_ADNPART_REGISTER_STRANDS
    if (removeFromIndex) singleStrandsIndex_.removeReferenceTarget(ss());
#endif

}

void ADNPart::DeregisterNucleotide(ADNPointer<ADNNucleotide> nt, bool removeFromSs, bool removeFromBs, bool removeFromIndex) {

    if (removeFromSs) {

        ADNPointer<ADNSingleStrand> ss = nt->GetStrand();
        ss->removeChild(nt());

    }
    if (removeFromBs) {

        auto bs = nt->GetBaseSegment();
        bs->RemoveNucleotide(nt);

    }
  
#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    if (removeFromIndex) nucleotidesIndex_.removeReferenceTarget(nt());
#endif

}

void ADNPart::DeregisterDoubleStrand(ADNPointer<ADNDoubleStrand> ds, bool removeFromParent, bool removeFromIndex) {

    if (removeFromParent)
        removeChild(ds());

#if ADENITA_ADNPART_REGISTER_STRANDS
    if (removeFromIndex) doubleStrandsIndex_.removeReferenceTarget(ds());
#endif

}

void ADNPart::DeregisterBaseSegment(ADNPointer<ADNBaseSegment> bs, bool removeFromDs, bool removeFromIndex) {

    if (removeFromDs) 
        if (bs->getParent())
            bs->getParent()->removeChild(bs());

#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
    if (removeFromIndex) baseSegmentsIndex_.removeReferenceTarget(bs());
#endif

}

void ADNPart::DeregisterAtom(ADNPointer<ADNAtom> atom, bool removeFromAtom) {

    if (removeFromAtom) 
        if (atom->getParent())
            atom->getParent()->removeChild(atom());

#if ADENITA_ADNPART_REGISTER_ATOMS
    atomsIndex_.removeReferenceTarget(atom());
#endif

}

bool ADNPart::isLoadedViaSAMSON() {
    return loadedViaSAMSONFlag;
}

void ADNPart::setLoadedViaSAMSON(bool l) {
    loadedViaSAMSONFlag = l;
}

std::pair<SBPosition3, SBPosition3> ADNPart::GetBoundingBox() {
    return std::pair<SBPosition3, SBPosition3>(minBox_, maxBox_);
}

void ADNPart::ResetBoundingBox() {

    auto nts = GetNucleotides();
    InitBoundingBox();
    SB_FOR(ADNPointer<ADNNucleotide> nt, nts)
        SetBoundingBox(nt);

}

void ADNPart::SetBoundingBox(ADNPointer<ADNNucleotide> newNt) {

    const SBPosition3 pos = newNt->GetBackbonePosition();
    //boundingBox.bound(pos);
    if (pos[0] < minBox_[0]) minBox_[0] = pos[0];
    if (pos[1] < minBox_[1]) minBox_[1] = pos[1];
    if (pos[2] < minBox_[2]) minBox_[2] = pos[2];
    if (pos[0] > maxBox_[0]) maxBox_[0] = pos[0];
    if (pos[1] > maxBox_[1]) maxBox_[1] = pos[1];
    if (pos[2] > maxBox_[2]) maxBox_[2] = pos[2];

}

void ADNPart::SetBoundingBox(ADNPointer<ADNBaseSegment> newBs) {

    const SBPosition3 pos = newBs->GetPosition();
    //boundingBox.bound(pos);
    if (pos[0] < minBox_[0]) minBox_[0] = pos[0];
    if (pos[1] < minBox_[1]) minBox_[1] = pos[1];
    if (pos[2] < minBox_[2]) minBox_[2] = pos[2];
    if (pos[0] > maxBox_[0]) maxBox_[0] = pos[0];
    if (pos[1] > maxBox_[1]) maxBox_[1] = pos[1];
    if (pos[2] > maxBox_[2]) maxBox_[2] = pos[2];

}

void ADNPart::InitBoundingBox() {

    const SBQuantity::picometer maxVal = SBQuantity::picometer(std::numeric_limits<double>::max());
    minBox_ = SBPosition3(maxVal, maxVal, maxVal);
    maxBox_ = SBPosition3(-maxVal, -maxVal, -maxVal);
    //boundingBox = SBIAPosition3(maxVal, -maxVal, maxVal, -maxVal, maxVal, -maxVal);

}

void ADNPart::RegisterSingleStrand(ADNPointer<ADNSingleStrand> ss) {

    if (ss->getName().empty()) {

        ss->setName("Single Strand " + std::to_string(singleStrandId_));
        ++singleStrandId_;

    }

    addChild(ss());

#if ADENITA_ADNPART_REGISTER_STRANDS
    singleStrandsIndex_.addReferenceTarget(ss());
#endif

}

void ADNPart::RegisterNucleotideThreePrime(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNNucleotide> nt, bool addToSs) {

    if (nt->getName().empty()) {

        nt->setName(ADNModel::GetResidueName(nt->GetType()) + " " + std::to_string(nucleotideId_));
        ++nucleotideId_;

    }
    if (addToSs) ss->AddNucleotideThreePrime(nt);

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    nucleotidesIndex_.addReferenceTarget(nt());
#endif
    SetBoundingBox(nt);

}

void ADNPart::RegisterNucleotideFivePrime(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNNucleotide> nt, bool addToSs) {

    if (nt->getName().empty()) {

        nt->setName(ADNModel::GetResidueName(nt->GetType()) + " " + std::to_string(nucleotideId_));
        ++nucleotideId_;

    }

    if (addToSs) ss->AddNucleotideFivePrime(nt);

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    nucleotidesIndex_.addReferenceTarget(nt());
#endif
    SetBoundingBox(nt);

}

void ADNPart::RegisterNucleotide(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNNucleotide> nt, ADNPointer<ADNNucleotide> ntNext, bool addToSs) {

    if (nt->getName().empty()) {

        nt->setName(ADNModel::GetResidueName(nt->GetType()) + " " + std::to_string(nucleotideId_));
        ++nucleotideId_;

    }

    if (addToSs) ss->AddNucleotide(nt, ntNext);

#if ADENITA_ADNPART_REGISTER_NUCLEOTIDES
    nucleotidesIndex_.addReferenceTarget(nt());
#endif
    SetBoundingBox(nt);

}

void ADNPart::RegisterAtom(ADNPointer<ADNNucleotide> nt, NucleotideGroup g, ADNPointer<ADNAtom> at, bool create) {

    if (create)
        at->create();

    nt->AddAtom(g, at);

#if ADENITA_ADNPART_REGISTER_ATOMS
    atomsIndex_.addReferenceTarget(at());
#endif

}

void ADNPart::RegisterAtom(ADNPointer<ADNBaseSegment> bs, ADNPointer<ADNAtom> at, bool create) {

    if (create)
        at->create();

    bs->addChild(at());

#if ADENITA_ADNPART_REGISTER_ATOMS
    atomsIndex_.addReferenceTarget(at());
#endif

}

void ADNPart::RegisterBaseSegmentEnd(ADNPointer<ADNDoubleStrand> ds, ADNPointer<ADNBaseSegment> bs, bool addToDs) {

    if (addToDs) ds->AddBaseSegmentEnd(bs);

#if ADENITA_ADNPART_REGISTER_BASESEGMENTS
    baseSegmentsIndex_.addReferenceTarget(bs());
#endif
    //SetBoundingBox(bs);

}

unsigned int ADNPart::GetBaseSegmentIndex(ADNPointer<ADNBaseSegment> bs) {
    return baseSegmentsIndex_.getIndex(bs());
}

void ADNPart::RegisterDoubleStrand(ADNPointer<ADNDoubleStrand> ds) {

    if (ds->getName().empty()) {

        ds->setName("Double Strand " + std::to_string(doubleStrandId_));
        ++doubleStrandId_;

    }
    
    addChild(ds());

#if ADENITA_ADNPART_REGISTER_STRANDS
    doubleStrandsIndex_.addReferenceTarget(ds());
#endif

}

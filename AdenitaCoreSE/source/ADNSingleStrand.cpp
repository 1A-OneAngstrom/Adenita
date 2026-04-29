#include "ADNModel.hpp"
#include "ADNPart.hpp"


ADNSingleStrand::ADNSingleStrand(const ADNSingleStrand& other) {

    *this = other;

}

ADNSingleStrand& ADNSingleStrand::operator=(const ADNSingleStrand& other) {

    SBChain::operator =(other);

    this->scaffoldFlag = other.scaffoldFlag;
    this->fivePrimeNucleotide = other.fivePrimeNucleotide;
    this->threePrimeNucleotide = other.threePrimeNucleotide;

    return *this;

}

void ADNSingleStrand::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBChain::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeBoolElement("isScaffold", IsScaffold());
    serializer->writeBoolElement("isCircular", IsCircular());
    serializer->writeUnsignedIntElement("fivePrime", nodeIndexer.getIndex(fivePrimeNucleotide()));
    serializer->writeUnsignedIntElement("threePrime", nodeIndexer.getIndex(threePrimeNucleotide()));

}

void ADNSingleStrand::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBChain::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    bool isScaffold = serializer->readBoolElement();
    bool isCircular = serializer->readBoolElement();
    unsigned int fPrimeIdx = serializer->readUnsignedIntElement();
    unsigned int tPrimeIdx = serializer->readUnsignedIntElement();
    setScaffoldFlag(isScaffold);
    setCircularFlag(isCircular);
    SBNode* fPrime = nodeIndexer.getNode(fPrimeIdx);
    SBPointer<ADNNucleotide> fp = static_cast<ADNNucleotide*>(fPrime);
    SBNode* tPrime = nodeIndexer.getNode(tPrimeIdx);
    SBPointer<ADNNucleotide> tp = static_cast<ADNNucleotide*>(tPrime);
    fivePrimeNucleotide = fp;
    threePrimeNucleotide = tp;

}

SBPointer<ADNPart> ADNSingleStrand::GetPart() const {

    // the ADNPart is a structural model
    SBNode* model = getModel();
    if (!model) return nullptr;
    SBPointer<ADNPart> part = static_cast<ADNPart*>(model);
    return part;

}

SBPointer<ADNNucleotide> ADNSingleStrand::GetFivePrime() const {
    return fivePrimeNucleotide;
}

SBNode* ADNSingleStrand::getFivePrime() const {
    return fivePrimeNucleotide();
}

SBPointer<ADNNucleotide> ADNSingleStrand::GetThreePrime() const {
    return threePrimeNucleotide;
}

SBNode* ADNSingleStrand::getThreePrime() const {
    return threePrimeNucleotide();
}

void ADNSingleStrand::SetFivePrime(SBPointer<ADNNucleotide> nucleotide) {
    this->fivePrimeNucleotide = nucleotide;
}

void ADNSingleStrand::SetThreePrime(SBPointer<ADNNucleotide> nucleotide) {
    this->threePrimeNucleotide = nucleotide;
}

bool ADNSingleStrand::IsScaffold() const {
    return scaffoldFlag;
}

bool ADNSingleStrand::getScaffoldFlag() const {
    return scaffoldFlag;
}

void ADNSingleStrand::setScaffoldFlag(bool b) {
    this->scaffoldFlag = b;
}

bool ADNSingleStrand::IsCircular() const {
    return circularFlag;
}

bool ADNSingleStrand::getCircularFlag() const {
    return circularFlag;
}

void ADNSingleStrand::setCircularFlag(bool b) {
    this->circularFlag = b;
}

int ADNSingleStrand::getNumberOfNucleotides() const {
    return boost::numeric_cast<int>(GetNucleotides().size());
}

SBPointerIndexer<ADNNucleotide> ADNSingleStrand::GetNucleotides() const {

    SBPointerIndexer<ADNNucleotide> nucleotideList;

#if 0
    SBNodeIndexer nodeIndexer;
    getNodes(nodeIndexer, SBNode::IsType(SBNode::Residue) && (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    SB_FOR(SBNode * n, nodeIndexer)
        nucleotideList.addReferenceTarget(static_cast<ADNNucleotide*>(n));
#else
    const SBPointerList<SBStructuralNode>* children = getChildren();
    SB_FOR(SBStructuralNode * n, *children) {

        if (n->getProxy()->getName() == "ADNNucleotide" && n->getProxy()->getElementUUID() == SBUUID(SB_ELEMENT_UUID))
            nucleotideList.addReferenceTarget(static_cast<ADNNucleotide*>(n));

    }
#endif

    return nucleotideList;

}

void ADNSingleStrand::AddNucleotideThreePrime(SBPointer<ADNNucleotide> nucleotide) {

    if (!nucleotide.isValid()) return;

    addChild(nucleotide());

    if (threePrimeNucleotide != nullptr) {

        if (threePrimeNucleotide->getEndType() == ADNNucleotide::EndType::FiveAndThreePrime) threePrimeNucleotide->setEndType(ADNNucleotide::EndType::FivePrime);
        else threePrimeNucleotide->setEndType(ADNNucleotide::EndType::NotEnd);
        nucleotide->setEndType(ADNNucleotide::EndType::ThreePrime);

    }
    else {

        // nt is also fivePrimeNucleotide
        this->fivePrimeNucleotide = nucleotide;
        nucleotide->setEndType(ADNNucleotide::EndType::FiveAndThreePrime);

    }

    threePrimeNucleotide = nucleotide;

}

void ADNSingleStrand::AddNucleotideFivePrime(SBPointer<ADNNucleotide> nucleotide) {

    if (!nucleotide.isValid()) return;

    if (fivePrimeNucleotide != nullptr) {

        if (fivePrimeNucleotide->getEndType() == ADNNucleotide::EndType::FiveAndThreePrime) fivePrimeNucleotide->setEndType(ADNNucleotide::EndType::ThreePrime);
        else fivePrimeNucleotide->setEndType(ADNNucleotide::EndType::NotEnd);
        nucleotide->setEndType(ADNNucleotide::EndType::FivePrime);

    }
    else {

        // nt is also fivePrimeNucleotide
        this->threePrimeNucleotide = nucleotide;
        nucleotide->setEndType(ADNNucleotide::EndType::FiveAndThreePrime);

    }

    addChild(nucleotide(), fivePrimeNucleotide());
    fivePrimeNucleotide = nucleotide;

}

void ADNSingleStrand::AddNucleotide(SBPointer<ADNNucleotide> nucleotide, SBPointer<ADNNucleotide> nextNucleotide) {

    if (!nucleotide.isValid()) return;

    if (nextNucleotide == nullptr) return AddNucleotideThreePrime(nucleotide);
    if (nextNucleotide == GetFivePrime()) return AddNucleotideFivePrime(nucleotide);

    addChild(nucleotide(), nextNucleotide());

}

/*!
  \param a SBPointer to the ADNNucleotide which should be the new 5' of its strand
  \param whether to keep the sequence as it was (reset it from new 5' on)
*/
void ADNSingleStrand::ShiftStart(SBPointer<ADNNucleotide> nucleotide, bool shiftSeq) {

    if (nucleotide == nullptr) return;
    // skip if the nucleotide is already the 5'
    if (nucleotide == fivePrimeNucleotide) return;

    // perform some checks

    const int numberOfNucleotidesUsingSAMSONBefore = countNodes((SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    const int numberOfNucleotidesBefore = getNumberOfNucleotides();

    if (numberOfNucleotidesUsingSAMSONBefore != numberOfNucleotidesBefore)
        std::cerr << "ERROR: The number of nucleotides in nanorobot does not correspond to their number in the data graph (before shifting start 5'). " <<
        "The total number in nanorobot is " << numberOfNucleotidesBefore << " and using SAMSON it is " << numberOfNucleotidesUsingSAMSONBefore << std::endl;

    std::string seq = GetSequence();
    SBPointer<ADNNucleotide> origThreePrime = threePrimeNucleotide;
    SBPointer<ADNNucleotide> loopNt = origThreePrime;
    SBPointer<ADNNucleotide> stopNt = nucleotide->GetPrev();

    while (loopNt != stopNt) {

        SBPointer<ADNNucleotide> cpNt = loopNt;
        loopNt = loopNt->GetPrev();
        removeChild(cpNt());
        AddNucleotideFivePrime(cpNt);

    }

    stopNt->setEndType(ADNNucleotide::EndType::ThreePrime);
    threePrimeNucleotide = stopNt;

    if (shiftSeq) {
        SetSequence(seq);
    }

    // perform some checks

    const int numberOfNucleotidesUsingSAMSONAfter = countNodes((SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    const int numberOfNucleotidesAfter = getNumberOfNucleotides();

    if (numberOfNucleotidesUsingSAMSONAfter != numberOfNucleotidesAfter)
        std::cerr << "ERROR: The number of nucleotides in nanorobot does not correspond to their number in the data graph (after shifting start 5'). " <<
        "The total number in nanorobot is " << numberOfNucleotidesUsingSAMSONAfter << " and using SAMSON it is " << numberOfNucleotidesAfter << std::endl;

    if (numberOfNucleotidesUsingSAMSONBefore != numberOfNucleotidesUsingSAMSONAfter)
        std::cerr << "ERROR: The number of nucleotides in the data graph before and after shifting start 5' do not coincide. " <<
        "Before: " << numberOfNucleotidesUsingSAMSONBefore << " and after: " << numberOfNucleotidesUsingSAMSONAfter << std::endl;

    if (numberOfNucleotidesBefore != numberOfNucleotidesAfter)
        std::cerr << "ERROR: The number of nucleotides in the nanorobot before and after shifting start 5' do not coincide. " <<
        "Before: " << numberOfNucleotidesBefore << " and after: " << numberOfNucleotidesAfter << std::endl;

}

/**
* Returns the sequence of the strand
* \param sequence from 5' to 3'
*/
std::string ADNSingleStrand::GetSequence() const {

    std::string seq = "";
    SBPointer<ADNNucleotide> nt = fivePrimeNucleotide;
    while (nt != nullptr) {

        seq += nt->getOneLetterNucleotideTypeString();
        nt = nt->GetNext();

    }
    return seq;

}

std::string ADNSingleStrand::getSequence() const {
    return GetSequence();
}

std::string ADNSingleStrand::GetSequenceWithTags() const {

    std::string seq = "";
    SBPointer<ADNNucleotide> nt = fivePrimeNucleotide;
    while (nt != nullptr) {

        std::string totalBase = nt->getOneLetterNucleotideTypeString();
        if (nt->hasTag()) {

            totalBase = "[" + nt->getTag() + totalBase + "]";

        }
        seq += totalBase;
        nt = nt->GetNext();

    }

    return seq;

}

double ADNSingleStrand::GetGCContent() const {

    double gcCont = 0.0;
    auto nucleotides = GetNucleotides();

    SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) {

        if (nt->getNucleotideType() == DNABlocks::DC || nt->getNucleotideType() == DNABlocks::DG)
            gcCont += 1.0;

    }

    gcCont /= nucleotides.size();
    return gcCont * 100.0;

}

double ADNSingleStrand::getGCContent() const {

    return GetGCContent();

}

void ADNSingleStrand::SetSequence(std::string seq) {

    if (seq.empty()) return;

    if (scaffoldFlag != true) {

        std::string msg = "Forcing sequence on staple " + getName();
        ADNLogger::Log(msg);

    }
    SBPointer<ADNNucleotide> nt = fivePrimeNucleotide;
    int count = 0;
    while (nt != nullptr) {

        DNABlocks type = DNABlocks::DI;
        if (seq.size() > count) type = ADNModel::ResidueNameToType(seq[count]);
        nt->setNucleotideType(type);
        if (nt->GetPair() != nullptr) {

            DNABlocks compType = ADNModel::GetComplementaryBase(type);
            nt->GetPair()->setNucleotideType(compType);

        }
        nt = nt->GetNext();
        ++count;

    }

}

void ADNSingleStrand::setSequence(std::string seq) {

    SetSequence(seq);

}

void ADNSingleStrand::SetDefaultName() {

    std::string name = "Strand";
    if (scaffoldFlag) name = "Scaffold";
    setName(name);

}

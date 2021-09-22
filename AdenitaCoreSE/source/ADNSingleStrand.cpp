//#include "ADNSingleStrand.hpp"
//#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"


ADNSingleStrand::ADNSingleStrand(const ADNSingleStrand& other) {

    *this = other;

}

ADNSingleStrand& ADNSingleStrand::operator=(const ADNSingleStrand& other) {

    SBChain::operator =(other);

    isScaffold_ = other.isScaffold_;
    fivePrime_ = other.fivePrime_;
    threePrime_ = other.threePrime_;

    return *this;

}

void ADNSingleStrand::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBChain::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeBoolElement("isScaffold", IsScaffold());
    serializer->writeBoolElement("isCircular", IsCircular());
    serializer->writeUnsignedIntElement("fivePrime", nodeIndexer.getIndex(fivePrime_()));
    serializer->writeUnsignedIntElement("threePrime", nodeIndexer.getIndex(threePrime_()));

}

void ADNSingleStrand::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBChain::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    bool isScaffold = serializer->readBoolElement();
    bool isCircular = serializer->readBoolElement();
    unsigned int fPrimeIdx = serializer->readUnsignedIntElement();
    unsigned int tPrimeIdx = serializer->readUnsignedIntElement();
    IsScaffold(isScaffold);
    IsCircular(isCircular);
    SBNode* fPrime = nodeIndexer.getNode(fPrimeIdx);
    ADNPointer<ADNNucleotide> fp = static_cast<ADNNucleotide*>(fPrime);
    SBNode* tPrime = nodeIndexer.getNode(tPrimeIdx);
    ADNPointer<ADNNucleotide> tp = static_cast<ADNNucleotide*>(tPrime);
    fivePrime_ = fp;
    threePrime_ = tp;

}

ADNPointer<ADNNucleotide> ADNSingleStrand::GetFivePrime() {
    return fivePrime_;
}

SBNode* ADNSingleStrand::getFivePrime() const {
    return fivePrime_();
}

ADNPointer<ADNNucleotide> ADNSingleStrand::GetThreePrime() {
    return threePrime_;
}

SBNode* ADNSingleStrand::getThreePrime() const {
    return threePrime_();
}

ADNPointer<ADNNucleotide> ADNSingleStrand::GetNthNucleotide(int n) {

    ADNPointer<ADNNucleotide> nt = nullptr;
    if (n <= getNumberOfNucleotides()) {

        nt = fivePrime_;
        for (int i = 0; i < n; ++i) {
            nt = nt->GetNext();
        }

    }

    return nt;

}

void ADNSingleStrand::SetFivePrime(ADNPointer<ADNNucleotide> nt) {
    fivePrime_ = nt;
}

void ADNSingleStrand::SetThreePrime(ADNPointer<ADNNucleotide> nt) {
    threePrime_ = nt;
}

void ADNSingleStrand::IsScaffold(bool b) {
    isScaffold_ = b;
}

bool ADNSingleStrand::IsScaffold() const {
    return isScaffold_;
}

bool ADNSingleStrand::getIsScaffold() const {
    return IsScaffold();
}

void ADNSingleStrand::setIsScaffold(bool b) {
    IsScaffold(b);
}

void ADNSingleStrand::IsCircular(bool c) {
    isCircular_ = c;
}

bool ADNSingleStrand::IsCircular() const {
    return isCircular_;
}

bool ADNSingleStrand::getIsCircular() const {
    return IsCircular();
}

void ADNSingleStrand::setIsCircular(bool b) {
    IsCircular(b);
}

int ADNSingleStrand::getNumberOfNucleotides() const {
    return boost::numeric_cast<int>(GetNucleotides().size());
}

CollectionMap<ADNNucleotide> ADNSingleStrand::GetNucleotides() const {

    CollectionMap<ADNNucleotide> ntList;

    SBPointerList<SBStructuralNode> children = *getChildren();
    SB_FOR(SBStructuralNode * n, children) {

        ADNPointer<ADNNucleotide> a = static_cast<ADNNucleotide*>(n);
        ntList.addReferenceTarget(a());

    }

    return ntList;

}

//ADNPointer<ADNNucleotide> ADNSingleStrand::GetNucleotide(unsigned int id) const {
//  auto ntList = GetNucleotides();
//  return ntList[id];
//}

void ADNSingleStrand::AddNucleotideThreePrime(ADNPointer<ADNNucleotide> nt) {

    addChild(nt());

    if (threePrime_ != nullptr) {

        if (threePrime_->GetEnd() == End::FiveAndThreePrime) threePrime_->SetEnd(End::FivePrime);
        else threePrime_->SetEnd(End::NotEnd);
        nt->SetEnd(End::ThreePrime);

    }
    else {

        // nt is also fivePrime_
        fivePrime_ = nt;
        nt->SetEnd(End::FiveAndThreePrime);

    }

    threePrime_ = nt;

}

void ADNSingleStrand::AddNucleotideFivePrime(ADNPointer<ADNNucleotide> nt) {

    if (fivePrime_ != nullptr) {

        if (fivePrime_->GetEnd() == End::FiveAndThreePrime) fivePrime_->SetEnd(End::ThreePrime);
        else fivePrime_->SetEnd(End::NotEnd);
        nt->SetEnd(End::FivePrime);

    }
    else {

        // nt is also fivePrime_
        threePrime_ = nt;
        nt->SetEnd(End::FiveAndThreePrime);

    }

    addChild(nt(), fivePrime_());
    fivePrime_ = nt;

}

void ADNSingleStrand::AddNucleotide(ADNPointer<ADNNucleotide> nt, ADNPointer<ADNNucleotide> nextNt) {

    if (nextNt == nullptr) return AddNucleotideThreePrime(nt);
    if (nextNt == GetFivePrime()) return AddNucleotideFivePrime(nt);

    addChild(nt(), nextNt());

}

void ADNSingleStrand::ShiftStart(ADNPointer<ADNNucleotide> nucleotide, bool shiftSeq) {

    // skip if the nucleotide is already the 5'
    if (nucleotide == fivePrime_) return;

    // perform some checks

    const int numberOfNucleotidesUsingSAMSONBefore = countNodes((SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
    const int numberOfNucleotidesBefore = getNumberOfNucleotides();

    if (numberOfNucleotidesUsingSAMSONBefore != numberOfNucleotidesBefore)
        std::cerr << "ERROR: The number of nucleotides in nanorobot does not correspond to their number in the data graph (before shifting start 5'). " <<
        "The total number in nanorobot is " << numberOfNucleotidesBefore << " and using SAMSON it is " << numberOfNucleotidesUsingSAMSONBefore << std::endl;

    std::string seq = GetSequence();
    auto origThreePrime = threePrime_;
    auto loopNt = origThreePrime;
    auto stopNt = nucleotide->GetPrev();

    while (loopNt != stopNt) {

        auto cpNt = loopNt;
        loopNt = loopNt->GetPrev();
        removeChild(cpNt());
        AddNucleotideFivePrime(cpNt);

    }

    stopNt->SetEnd(End::ThreePrime);
    threePrime_ = stopNt;

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

std::string ADNSingleStrand::GetSequence() const {

    std::string seq = "";
    ADNPointer<ADNNucleotide> nt = fivePrime_;
    while (nt != nullptr) {

        seq += ADNModel::GetResidueName(nt->GetType());
        nt = nt->GetNext();

    }
    return seq;

}

std::string ADNSingleStrand::getSequence() const {
    return GetSequence();
}

std::string ADNSingleStrand::GetSequenceWithTags() const {

    std::string seq = "";
    ADNPointer<ADNNucleotide> nt = fivePrime_;
    while (nt != nullptr) {

        std::string totalBase(1, ADNModel::GetResidueName(nt->GetType()));
        if (nt->hasTag()) {

            std::string base(1, ADNModel::GetResidueName(nt->GetType()));
            totalBase = "[" + nt->getTag() + base + "]";

        }
        seq += totalBase;
        nt = nt->GetNext();

    }

    return seq;

}

double ADNSingleStrand::GetGCContent() const {

    double gcCont = 0.0;
    auto nucleotides = GetNucleotides();

    SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

        if (nt->GetType() == DNABlocks::DC || nt->GetType() == DNABlocks::DG) {
            gcCont += 1.0;
        }

    }

    gcCont /= nucleotides.size();
    return gcCont * 100.0;

}

double ADNSingleStrand::getGCContent() const {
    return GetGCContent();
}

void ADNSingleStrand::SetSequence(std::string seq) {

    if (seq.empty()) return;

    if (isScaffold_ != true) {

        std::string msg = "Forcing sequence on staple " + getName();
        ADNLogger::Log(msg);

    }
    ADNPointer<ADNNucleotide> nt = fivePrime_;
    int count = 0;
    while (nt != nullptr) {

        DNABlocks type = DNABlocks::DI;
        if (seq.size() > count) type = ADNModel::ResidueNameToType(seq[count]);
        nt->SetType(type);
        if (nt->GetPair() != nullptr) {

            DNABlocks compType = ADNModel::GetComplementaryBase(type);
            nt->GetPair()->SetType(compType);

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
    if (isScaffold_) name = "Scaffold";
    setName(name);

}

//#include "ADNNucleotide.hpp"
#include "ADNModel.hpp"

ADNNucleotide::ADNNucleotide(const ADNNucleotide& other) : PositionableSB(other), SBResidue(other), Orientable(other) {

    *this = other;

}

ADNNucleotide& ADNNucleotide::operator=(const ADNNucleotide& other) {

    PositionableSB::operator =(other);
    Orientable::operator =(other);
    SBResidue::operator =(other);

    return *this;

}

void ADNNucleotide::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBResidue::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    serializer->writeIntElement("end", static_cast<int>(endType));

    //ADNPointer<ADNAtom> at = GetCenterAtom();
    SBPosition3 pos = GetPosition();
    serializer->writeDoubleElement("x", pos[0].getValue());
    serializer->writeDoubleElement("y", pos[1].getValue());
    serializer->writeDoubleElement("z", pos[2].getValue());
    //serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(at()));

    serializer->writeStartElement("e3");
    auto e3 = GetE3();
    double e3x = e3[0];
    double e3y = e3[1];
    double e3z = e3[2];
    serializer->writeDoubleElement("x", e3x);
    serializer->writeDoubleElement("y", e3y);
    serializer->writeDoubleElement("z", e3z);
    serializer->writeEndElement();

    serializer->writeStartElement("e2");
    auto e2 = GetE2();
    double e2x = e2[0];
    double e2y = e2[1];
    double e2z = e2[2];
    serializer->writeDoubleElement("x", e2x);
    serializer->writeDoubleElement("y", e2y);
    serializer->writeDoubleElement("z", e2z);
    serializer->writeEndElement();

    serializer->writeStartElement("e1");
    auto e1 = GetE1();
    double e1x = e1[0];
    double e1y = e1[1];
    double e1z = e1[2];
    serializer->writeDoubleElement("x", e1x);
    serializer->writeDoubleElement("y", e1y);
    serializer->writeDoubleElement("z", e1z);
    serializer->writeEndElement();

    serializer->writeUnsignedIntElement("pair", nodeIndexer.getIndex(pair_()));
    serializer->writeUnsignedIntElement("base_segment", nodeIndexer.getIndex(bs_()));

    serializer->writeStringElement("tag", tag_);

}

void ADNNucleotide::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBResidue::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    setEndType(static_cast<ADNNucleotide::EndType>(serializer->readIntElement()));

    //unsigned int idx = serializer->readUnsignedIntElement();
    //ADNPointer<ADNAtom> at = (ADNAtom*)nodeIndexer.getNode(idx);
    //SetCenterAtom(at);
    double x = serializer->readDoubleElement();
    double y = serializer->readDoubleElement();
    double z = serializer->readDoubleElement();
    SBPosition3 pos = SBPosition3(SBQuantity::picometer(x), SBQuantity::picometer(y), SBQuantity::picometer(z));
    SetPosition(pos);

    serializer->readStartElement();
    double e3x = serializer->readDoubleElement();
    double e3y = serializer->readDoubleElement();
    double e3z = serializer->readDoubleElement();
    ublas::vector<double> e3(3, 0.0);
    e3[0] = e3x;
    e3[1] = e3y;
    e3[2] = e3z;
    SetE3(e3);
    serializer->readEndElement();

    serializer->readStartElement();
    double e2x = serializer->readDoubleElement();
    double e2y = serializer->readDoubleElement();
    double e2z = serializer->readDoubleElement();
    ublas::vector<double> e2(3, 0.0);
    e2[0] = e2x;
    e2[1] = e2y;
    e2[2] = e2z;
    SetE2(e2);
    serializer->readEndElement();

    serializer->readStartElement();
    double e1x = serializer->readDoubleElement();
    double e1y = serializer->readDoubleElement();
    double e1z = serializer->readDoubleElement();
    ublas::vector<double> e1(3, 0.0);
    e1[0] = e1x;
    e1[1] = e1y;
    e1[2] = e1z;
    SetE1(e1);
    serializer->readEndElement();

    unsigned int pIdx = serializer->readUnsignedIntElement();
    unsigned int bsIdx = serializer->readUnsignedIntElement();
    SBNode* pNode = nodeIndexer.getNode(pIdx);
    SBNode* bsNode = nodeIndexer.getNode(bsIdx);
    ADNPointer<ADNNucleotide> p = static_cast<ADNNucleotide*>(pNode);
    ADNPointer<ADNBaseSegment> bs = static_cast<ADNBaseSegment*>(bsNode);
    SetPair(p);
    SetBaseSegment(bs);

    std::string tag = serializer->readStringElement();
    setTag(tag);

}

void ADNNucleotide::SetType(DNABlocks t) {

    setResidueType(t);
    setName(ADNModel::GetResidueName(t) + std::to_string(getNodeIndex()));

}

DNABlocks ADNNucleotide::GetType() {
    return getResidueType();
}

DNABlocks ADNNucleotide::getNucleotideType() const {
    return getResidueType();
}

void ADNNucleotide::SetPair(ADNPointer<ADNNucleotide> nt) {
    pair_ = ADNWeakPointer<ADNNucleotide>(nt);
}

ADNPointer<ADNNucleotide> ADNNucleotide::GetPair() const {
    return pair_;
}

SBNode* ADNNucleotide::getPair() const {
    return GetPair()();
}

ADNPointer<ADNNucleotide> ADNNucleotide::GetPrev(bool checkCircular) const {

    ADNPointer<ADNNucleotide> p = static_cast<ADNNucleotide*>(getPreviousNucleicAcid());

    if (checkCircular) {

        auto strand = GetStrand();
        if (strand->IsCircular() && endType == EndType::FivePrime) {
            p = strand->GetThreePrime();
        }

    }

    return p;

}

SBNode* ADNNucleotide::getPrev() const {
    return GetPrev(true)();
}

ADNPointer<ADNNucleotide> ADNNucleotide::GetNext(bool checkCircular) const {

    ADNPointer<ADNNucleotide> p = static_cast<ADNNucleotide*>(getNextNucleicAcid());

    if (checkCircular) {

        auto strand = GetStrand();
        if (strand->IsCircular() && endType == EndType::ThreePrime) {
            p = strand->GetFivePrime();
        }

    }

    return p;

}

SBNode* ADNNucleotide::getNext() const {
    return GetNext(true)();
}

ADNPointer<ADNSingleStrand> ADNNucleotide::GetStrand() const {

    //check: if (parent->getProxy()->getUUID() == SBUUID("8EB118A4-A8BF-19F5-5171-C68582AC6262"))

    auto p = static_cast<ADNSingleStrand*>(getParent());
    return ADNPointer<ADNSingleStrand>(p);

}

SBNode* ADNNucleotide::getSingleStrand() const {
    return getParent();
}

void ADNNucleotide::SetBaseSegment(ADNPointer<ADNBaseSegment> bs) {
    bs_ = ADNWeakPointer<ADNBaseSegment>(bs);
}

void ADNNucleotide::AddAtom(NucleotideGroup g, ADNPointer<ADNAtom> a) {

    if (g == SBNode::Type::Backbone) {

        ADNPointer<ADNBackbone> bb = GetBackbone();
        bb->AddAtom(a);

    }
    else if (g == SBNode::Type::SideChain) {

        ADNPointer<ADNSidechain> sc = GetSidechain();
        sc->AddAtom(a);

    }

}

void ADNNucleotide::DeleteAtom(NucleotideGroup g, ADNPointer<ADNAtom> a) {

    if (g == SBNode::Type::Backbone) {

        ADNPointer<ADNBackbone> bb = GetBackbone();
        bb->DeleteAtom(a);

    }
    else if (g == SBNode::Type::SideChain) {

        ADNPointer<ADNSidechain> sc = GetSidechain();
        sc->DeleteAtom(a);

    }

}

CollectionMap<ADNAtom> ADNNucleotide::GetAtoms() {

    auto bb = GetBackbone();
    auto sc = GetSidechain();
    auto bbAtoms = bb->GetAtoms();
    auto scAtoms = sc->GetAtoms();

    auto atoms = bbAtoms;
    SB_FOR(ADNPointer<ADNAtom> a, scAtoms) atoms.addReferenceTarget(a());

    return atoms;

}

void ADNNucleotide::HideCenterAtoms() {

    auto bb = GetBackbone();
    auto sc = GetSidechain();

    HideCenterAtom();
    bb->HideCenterAtom();
    sc->HideCenterAtom();

}

ADNPointer<ADNAtom> ADNNucleotide::GetBackboneCenterAtom() {

    auto bb = GetBackbone();
    return bb->GetCenterAtom();

}

ADNPointer<ADNAtom> ADNNucleotide::GetSidechainCenterAtom() {

    auto sc = GetSidechain();
    return sc->GetCenterAtom();

}

CollectionMap<ADNAtom> ADNNucleotide::GetAtomsByName(std::string name) {

    CollectionMap<ADNAtom> res;
    /*SBNodeIndexer atoms;
    getNodes(atoms, SBNode::GetName() == name);

    SB_FOR(SBNode* a, atoms) {
      ADNPointer<ADNAtom> at = static_cast<ADNAtom*>(a);
      res.addReferenceTarget(at());
    }*/

    auto atoms = GetAtoms();
    SB_FOR(ADNPointer<ADNAtom> a, atoms) {

        if (a->getName() == name) {
            res.addReferenceTarget(a());
        }

    }

    return res;

}

ublas::matrix<double> ADNNucleotide::GetGlobalBasisTransformation() {

    ublas::matrix<double> transf(3, 3, 0.0);
    ublas::column(transf, 0) = GetE1();
    ublas::column(transf, 1) = GetE2();
    ublas::column(transf, 2) = GetE3();
    return transf;

}

ADNPointer<ADNBaseSegment> ADNNucleotide::GetBaseSegment() {
    return bs_;
}

SBNode* ADNNucleotide::getBaseSegment() const {
    return bs_();
}

std::string ADNNucleotide::getBaseSegmentTypeString() const {

    return bs_->getCellTypeString();

}

ADNPointer<ADNDoubleStrand> ADNNucleotide::GetDoubleStrand() {

    auto baseSegment = GetBaseSegment();
    if (baseSegment != nullptr)
        return baseSegment->GetDoubleStrand();
    else
        return nullptr;

}

std::string ADNNucleotide::getEndTypeString() const {

    std::string s = "Not end";

    if (endType == EndType::FivePrime) s = "5'";
    else if (endType == EndType::ThreePrime) s = "3'";
    else if (endType == EndType::FiveAndThreePrime) s = "5' and 3'";
    else if (endType == EndType::NotEnd) s = "Not end";

    return s;

}

ADNNucleotide::EndType ADNNucleotide::getEndType() {
    return endType;
}

void ADNNucleotide::setEndType(ADNNucleotide::EndType type) {
    this->endType = type;
}

bool ADNNucleotide::isEndTypeNucleotide() {
    return (endType != EndType::NotEnd);
}

void ADNNucleotide::Init() {

    SetType(DNABlocks::DI);
    ADNPointer<ADNBackbone> bb = new ADNBackbone();
    bb->setName(getName() + " Backbone");
    ADNPointer<ADNSidechain> sc = new ADNSidechain();
    sc->setName(getName() + " Side chain");
    addChild(bb());
    addChild(sc());

    if (isCreated()) {

        //if (SAMSON::isHolding()) SAMSON::hold(bb());
        //if (SAMSON::isHolding()) SAMSON::hold(sc());
        //bb->create();
        //sc->create();

    }

}

ADNPointer<ADNBackbone> ADNNucleotide::GetBackbone() const {

    auto bb = static_cast<ADNBackbone*>(getBackbone());
    if (bb)
        return ADNPointer<ADNBackbone>(bb);
    else
        return nullptr;

}

void ADNNucleotide::SetBackbone(ADNPointer<ADNBackbone> bb) {

    auto bbOld = GetBackbone();
    bbOld = bb;

}

ADNPointer<ADNSidechain> ADNNucleotide::GetSidechain() const {

    auto sc = static_cast<ADNSidechain*>(getSideChain());
    if (sc)
        return ADNPointer<ADNSidechain>(sc);
    else
        return nullptr;

}

void ADNNucleotide::SetSidechain(ADNPointer<ADNSidechain> sc) {

    auto scOld = GetSidechain();
    scOld = sc;

}

void ADNNucleotide::SetSidechainPosition(Position3D pos) {

    auto sc = GetSidechain();
    if (sc.isValid()) sc->SetPosition(pos);

}

Position3D ADNNucleotide::GetSidechainPosition() const {

    auto sc = GetSidechain();
    return sc->GetPosition();

}

void ADNNucleotide::SetBackbonePosition(Position3D pos) {

    auto bb = GetBackbone();
    if (bb.isValid()) bb->SetPosition(pos);

}

Position3D ADNNucleotide::GetBackbonePosition() const {

    auto bb = GetBackbone();
    return bb->GetPosition();

}

Position3D ADNNucleotide::GetPosition() const {
    return (GetBackbonePosition() + GetSidechainPosition()) * 0.5;
}

bool ADNNucleotide::GlobalBaseIsSet() {

    bool set = false;

    ublas::matrix<double> mat(3, 3);
    ublas::column(mat, 0) = GetE1();
    ublas::column(mat, 1) = GetE2();
    ublas::column(mat, 2) = GetE3();

    if (!ADNVectorMath::IsNearlyZero(ADNVectorMath::Determinant(mat))) set = true;

    return set;

}

bool ADNNucleotide::IsLeft() {
    return bs_->IsLeft(this);
}

bool ADNNucleotide::IsRight() {
    return bs_->IsRight(this);
}

std::string ADNNucleotide::getTag() const {
    return tag_;
}

void ADNNucleotide::setTag(std::string t) {
    this->tag_ = t;
}

bool ADNNucleotide::hasTag() {
    return !tag_.empty();
}

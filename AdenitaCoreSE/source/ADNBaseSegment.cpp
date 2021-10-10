//#include "ADNBaseSegment.hpp"
#include "ADNModel.hpp"


ADNBaseSegment::ADNBaseSegment(CellType cellType) : PositionableSB(), Orientable(), SBStructuralGroup() {

    if (cellType == CellType::BasePair)
        SetCell(new ADNBasePair());
    else if (cellType == CellType::LoopPair)
        SetCell(new ADNLoopPair());
    else if (cellType == CellType::SkipPair)
        SetCell(new ADNSkipPair());

}

ADNBaseSegment::ADNBaseSegment(const ADNBaseSegment& other) : PositionableSB(other), Orientable(other), SBStructuralGroup(other) {

    *this = other;

}

ADNBaseSegment& ADNBaseSegment::operator=(const ADNBaseSegment& other) {

    PositionableSB::operator =(other);
    Orientable::operator =(other);
    SBStructuralGroup::operator =(other);

    if (this != &other) {
        number_ = other.number_;
        //cell_ = other.GetCell();
    }
    return *this;

}

void ADNBaseSegment::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

    SBStructuralGroup::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    ADNPointer<ADNAtom> atom = GetCenterAtom();
    /*SBPosition3 pos = GetPosition();
    serializer->writeDoubleElement("x", pos[0].getValue());
    serializer->writeDoubleElement("y", pos[1].getValue());
    serializer->writeDoubleElement("z", pos[2].getValue());*/
    serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(atom()));

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

    serializer->writeIntElement("number", GetNumber());
    serializer->writeUnsignedIntElement("cell", nodeIndexer.getIndex(cell_()));

}

void ADNBaseSegment::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

    SBStructuralGroup::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

    unsigned int idx = serializer->readUnsignedIntElement();
    ADNPointer<ADNAtom> atom = (ADNAtom*)nodeIndexer.getNode(idx);
    SetCenterAtom(atom);
    //double x = serializer->readDoubleElement();
    //double y = serializer->readDoubleElement();
    //double z = serializer->readDoubleElement();
    //SBPosition3 pos = SBPosition3(SBQuantity::picometer(x), SBQuantity::picometer(y), SBQuantity::picometer(z));
    //SetPosition(pos);

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

    SetNumber(serializer->readIntElement());
    SBNode* cNode = nodeIndexer.getNode(serializer->readUnsignedIntElement());
    ADNPointer<ADNCell> cell = static_cast<ADNCell*>(cNode);
    SetCell(cell());

}

void ADNBaseSegment::SetNumber(int n) {
    number_ = n;
}

int ADNBaseSegment::GetNumber() const {
    return number_;
}

void ADNBaseSegment::setNumber(int n) {
    SetNumber(n);
}

int ADNBaseSegment::getNumber() const {
    return GetNumber();
}

ADNPointer<ADNBaseSegment> ADNBaseSegment::GetPrev(bool checkCircular) const {

    ADNPointer<ADNBaseSegment> p = static_cast<ADNBaseSegment*>(getPreviousStructuralNode());

    if (checkCircular) {
        auto ds = GetDoubleStrand();
        if (ds->IsCircular() && GetNumber() == 0) {
            // is the first bs
            p = ds->GetLastBaseSegment();
        }
    }

    return p;

}

ADNPointer<ADNBaseSegment> ADNBaseSegment::GetNext(bool checkCircular) const {

    ADNPointer<ADNBaseSegment> p = static_cast<ADNBaseSegment*>(getNextStructuralNode());

    if (checkCircular) {
        auto ds = GetDoubleStrand();
        if (ds->IsCircular() && this == ds->GetLastBaseSegment()()) {
            // is the last bs
            p = ds->GetFirstBaseSegment();
        }
    }

    return p;

}

bool ADNBaseSegment::IsEnd() const {

    return IsFirst() || IsLast();

}

bool ADNBaseSegment::IsFirst() const {

    bool e = false;
    auto ds = GetDoubleStrand();
    if (ds->GetFirstBaseSegment()() == this) e = true;

    return e;

}

bool ADNBaseSegment::IsLast() const {

    bool e = false;
    auto ds = GetDoubleStrand();
    if (ds->GetLastBaseSegment()() == this) e = true;

    return e;

}

ADNPointer<ADNDoubleStrand> ADNBaseSegment::GetDoubleStrand() const {

    auto p = static_cast<ADNDoubleStrand*>(getParent());
    return ADNPointer<ADNDoubleStrand>(p);

}

SBNode* ADNBaseSegment::getDoubleStrand() const {
    return GetDoubleStrand()();
}

unsigned int ADNBaseSegment::getNumberOfNucleotides() const {
    return GetNucleotides().size();
}

CollectionMap<ADNNucleotide> ADNBaseSegment::GetNucleotides() const {
    return cell_->GetNucleotides();
}

void ADNBaseSegment::SetCell(ADNCell* c) {

    this->cell_ = ADNPointer<ADNCell>(c);
    this->cell_->setName(cell_->getCellTypeString() + " " + std::to_string(this->cell_->getNodeIndex()));
    addChild(this->cell_());

}

ADNPointer<ADNCell> ADNBaseSegment::GetCell() const {
    return cell_;
}

CellType ADNBaseSegment::GetCellType() const {

    ADNPointer<ADNCell> cell = GetCell();
    if (cell.isValid()) return cell->GetCellType();
    return CellType::Undefined;

}

std::string ADNBaseSegment::getCellTypeString() const {

    return ADNCell::getCellTypeString(GetCellType());

}

void ADNBaseSegment::RemoveNucleotide(ADNPointer<ADNNucleotide> nt) {

    ADNPointer<ADNCell> cell = GetCell();
    if (cell.isValid()) cell->RemoveNucleotide(nt);

}

bool ADNBaseSegment::IsLeft(ADNPointer<ADNNucleotide> nt) {
    return cell_->IsLeft(nt);
}

bool ADNBaseSegment::IsRight(ADNPointer<ADNNucleotide> nt) {
    return cell_->IsRight(nt);
}

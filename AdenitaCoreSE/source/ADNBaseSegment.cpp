#include "ADNBaseSegment.hpp"
#include "ADNAtom.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNFrameAdapters.hpp"
#include "ADNNodeValidation.hpp"

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

    SBPointer<ADNAtom> atom = GetCenterAtom();
    /*SBPosition3 pos = GetPosition();
    serializer->writeDoubleElement("x", pos[0].getValue());
    serializer->writeDoubleElement("y", pos[1].getValue());
    serializer->writeDoubleElement("z", pos[2].getValue());*/
    serializer->writeUnsignedIntElement("centerAtom", nodeIndexer.getIndex(atom()));

    const ADNFrameUtils::Frame frame = ADNFrameAdapters::sanitizedFrame(*this);

    serializer->writeStartElement("e3");
    double e3x = frame.e3.x;
    double e3y = frame.e3.y;
    double e3z = frame.e3.z;
    serializer->writeDoubleElement("x", e3x);
    serializer->writeDoubleElement("y", e3y);
    serializer->writeDoubleElement("z", e3z);
    serializer->writeEndElement();

    serializer->writeStartElement("e2");
    double e2x = frame.e2.x;
    double e2y = frame.e2.y;
    double e2z = frame.e2.z;
    serializer->writeDoubleElement("x", e2x);
    serializer->writeDoubleElement("y", e2y);
    serializer->writeDoubleElement("z", e2z);
    serializer->writeEndElement();

    serializer->writeStartElement("e1");
    double e1x = frame.e1.x;
    double e1y = frame.e1.y;
    double e1z = frame.e1.z;
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
    SBPointer<ADNAtom> atom = ADNNodeValidation::GetSerializedAdenitaNode<ADNAtom>(nodeIndexer, idx, "ADNAtom");
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
    ADNFrameAdapters::sanitizeFrame(*this);

    SetNumber(serializer->readIntElement());
    SBPointer<ADNCell> cell = ADNNodeValidation::GetSerializedAdenitaCell(nodeIndexer, serializer->readUnsignedIntElement());
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

SBPointer<ADNBaseSegment> ADNBaseSegment::GetPrev(bool checkCircular) const {

    SBPointer<ADNBaseSegment> p;
    if (SBNode* previous = getPreviousStructuralNode())
        if (ADNNodeValidation::IsAdenitaNode(previous, "ADNBaseSegment"))
            p = dynamic_cast<ADNBaseSegment*>(previous);

    if (checkCircular) {

        auto ds = GetDoubleStrand();
        if (ds != nullptr) if (ds->IsCircular() && GetNumber() == 0) {

            // is the first bs
            p = ds->GetLastBaseSegment();

        }

    }

    return p;

}

SBPointer<ADNBaseSegment> ADNBaseSegment::GetNext(bool checkCircular) const {

    SBPointer<ADNBaseSegment> p;
    if (SBNode* next = getNextStructuralNode())
        if (ADNNodeValidation::IsAdenitaNode(next, "ADNBaseSegment"))
            p = dynamic_cast<ADNBaseSegment*>(next);

    if (checkCircular) {

        auto ds = GetDoubleStrand();
        if (ds != nullptr) if (ds->IsCircular() && this == ds->GetLastBaseSegment()()) {

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
    if (ds != nullptr) if (ds->GetFirstBaseSegment()() == this) e = true;

    return e;

}

bool ADNBaseSegment::IsLast() const {

    bool e = false;
    auto ds = GetDoubleStrand();
    if (ds != nullptr) if (ds->GetLastBaseSegment()() == this) e = true;

    return e;

}

SBPointer<ADNDoubleStrand> ADNBaseSegment::GetDoubleStrand() const {

    SBNode* parent = getParent();
    if (!ADNNodeValidation::IsAdenitaNode(parent, "ADNDoubleStrand")) return nullptr;

    return SBPointer<ADNDoubleStrand>(dynamic_cast<ADNDoubleStrand*>(parent));

}

SBNode* ADNBaseSegment::getDoubleStrand() const {

    return GetDoubleStrand()();

}

unsigned int ADNBaseSegment::getNumberOfNucleotides() const {

    return GetNucleotides().size();

}

SBPointerIndexer<ADNNucleotide> ADNBaseSegment::GetNucleotides() const {
    
    if (this->cell_ != nullptr) return cell_->GetNucleotides();
    else return SBPointerIndexer<ADNNucleotide>();

}

void ADNBaseSegment::SetCell(ADNCell* c) {

    SBPointer<ADNCell> previousCell = this->cell_;
    const bool replacingCell = previousCell() != c;
    if (previousCell != nullptr && replacingCell) {
        removeChild(previousCell());
    }

    this->cell_ = SBPointer<ADNCell>(c);
    if (this->cell_ != nullptr) {

        this->cell_->setName(cell_->getCellTypeString() + " " + std::to_string(this->cell_->getNodeIndex()));
        this->cell_->setStructuralID(this->cell_->getNodeIndex());
        if (replacingCell) addChild(this->cell_());

    }

}

SBPointer<ADNCell> ADNBaseSegment::GetCell() const {

    return cell_;

}

CellType ADNBaseSegment::GetCellType() const {

    SBPointer<ADNCell> cell = GetCell();
    if (cell.isValid()) return cell->GetCellType();
    return CellType::Undefined;

}

std::string ADNBaseSegment::getCellTypeString() const {

    return ADNCell::getCellTypeString(GetCellType());

}

void ADNBaseSegment::RemoveNucleotide(SBPointer<ADNNucleotide> nt) {

    SBPointer<ADNCell> cell = GetCell();
    if (cell.isValid()) cell->RemoveNucleotide(nt);

}

bool ADNBaseSegment::IsLeft(SBPointer<ADNNucleotide> nt) const {

    if (cell_ != nullptr) return cell_->IsLeft(nt);
    else return false;

}

bool ADNBaseSegment::IsRight(SBPointer<ADNNucleotide> nt) const {

    if (cell_ != nullptr) return cell_->IsRight(nt);
    else return false;

}

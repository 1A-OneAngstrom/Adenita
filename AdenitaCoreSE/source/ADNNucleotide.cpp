#include "ADNNucleotide.hpp"
#include "ADNAtom.hpp"
#include "ADNBackbone.hpp"
#include "ADNSidechain.hpp"
#include "ADNBaseSegment.hpp"
#include "ADNSingleStrand.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNModel.hpp"

ADNNucleotide::ADNNucleotide(const ADNNucleotide& other) : PositionableSB(other), SBResidue(other), Orientable(other) {

	*this = other;
	pairNucleotide = nullptr;

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
	const auto& e3 = GetE3();
	double e3x = e3[0];
	double e3y = e3[1];
	double e3z = e3[2];
	serializer->writeDoubleElement("x", e3x);
	serializer->writeDoubleElement("y", e3y);
	serializer->writeDoubleElement("z", e3z);
	serializer->writeEndElement();

	serializer->writeStartElement("e2");
	const auto& e2 = GetE2();
	double e2x = e2[0];
	double e2y = e2[1];
	double e2z = e2[2];
	serializer->writeDoubleElement("x", e2x);
	serializer->writeDoubleElement("y", e2y);
	serializer->writeDoubleElement("z", e2z);
	serializer->writeEndElement();

	serializer->writeStartElement("e1");
	const auto& e1 = GetE1();
	double e1x = e1[0];
	double e1y = e1[1];
	double e1z = e1[2];
	serializer->writeDoubleElement("x", e1x);
	serializer->writeDoubleElement("y", e1y);
	serializer->writeDoubleElement("z", e1z);
	serializer->writeEndElement();

	serializer->writeUnsignedIntElement("pair", nodeIndexer.getIndex(pairNucleotide()));
	serializer->writeUnsignedIntElement("base_segment", nodeIndexer.getIndex(baseSegment()));

	serializer->writeStringElement("tag", tag);

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

void ADNNucleotide::setNucleotideType(DNABlocks t) {

	setResidueType(t);
	setName(std::string(1, ADNModel::GetResidueName(t)) + " " + std::to_string(getNodeIndex()));
	setStructuralID(getNodeIndex());

}

DNABlocks ADNNucleotide::getNucleotideType() const {

	return getResidueType();

}

std::string ADNNucleotide::getNucleotideTypeString() const {

	return std::string(1, ADNModel::GetResidueName(getNucleotideType()));

}

void ADNNucleotide::SetPair(ADNPointer<ADNNucleotide> nucleotide) {

	ADNNucleotide* oldNucleotide = this->pairNucleotide();

	this->pairNucleotide = ADNWeakPointer<ADNNucleotide>(nucleotide);

	if (oldNucleotide) oldNucleotide->disconnectPair(this);

}

void ADNNucleotide::disconnectPair(ADNPointer<ADNNucleotide> nucleotide) {

	if (nucleotide != nullptr && this->pairNucleotide == nucleotide) {

		this->pairNucleotide->pairNucleotide = nullptr;
		this->pairNucleotide = nullptr;

	}

}

void ADNNucleotide::disconnectPair() {

	if (this->pairNucleotide != nullptr) {

		this->pairNucleotide->pairNucleotide = nullptr;
		this->pairNucleotide = nullptr;

	}

}

ADNPointer<ADNNucleotide> ADNNucleotide::GetPair() const {
	return pairNucleotide;
}

SBNode* ADNNucleotide::getPair() const {
	return GetPair()();
}

ADNPointer<ADNNucleotide> ADNNucleotide::GetPrev(bool checkCircular) const {

	ADNPointer<ADNNucleotide> p;

	if (SBResidue* prevResidue = getPreviousNucleicAcid())
		p = static_cast<ADNNucleotide*>(prevResidue);

	if (checkCircular) {

		auto strand = GetStrand();
		if (strand != nullptr)
			if (strand->IsCircular() && endType == EndType::FivePrime)
				p = strand->GetThreePrime();

	}

	return p;

}

SBNode* ADNNucleotide::getPrev() const {
	return GetPrev(true)();
}

ADNPointer<ADNNucleotide> ADNNucleotide::GetNext(bool checkCircular) const {

	ADNPointer<ADNNucleotide> p;

	if (SBResidue* nextResidue = getNextNucleicAcid())
		p = static_cast<ADNNucleotide*>(nextResidue);

	if (checkCircular) {

		auto strand = GetStrand();
		if (strand != nullptr)
			if (strand->IsCircular() && endType == EndType::ThreePrime)
				p = strand->GetFivePrime();

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

	if (this->baseSegment.isValid()) this->baseSegment->RemoveNucleotide(this);

	this->baseSegment = ADNWeakPointer<ADNBaseSegment>(bs);

}

void ADNNucleotide::addAtom(NucleotideGroup g, ADNPointer<ADNAtom> a) {

	if (g == SBNode::Type::Backbone) {

		ADNPointer<ADNBackbone> bb = GetBackbone();
		if (bb != nullptr) bb->addAtom(a);

	}
	else if (g == SBNode::Type::SideChain) {

		ADNPointer<ADNSidechain> sc = GetSidechain();
		if (sc != nullptr) sc->addAtom(a);

	}

}

void ADNNucleotide::deleteAtom(NucleotideGroup g, ADNPointer<ADNAtom> a) {

	if (g == SBNode::Type::Backbone) {

		ADNPointer<ADNBackbone> bb = GetBackbone();
		if (bb != nullptr) bb->deleteAtom(a);

	}
	else if (g == SBNode::Type::SideChain) {

		ADNPointer<ADNSidechain> sc = GetSidechain();
		if (sc != nullptr) sc->deleteAtom(a);

	}

}

CollectionMap<ADNAtom> ADNNucleotide::GetAtoms() const {

	CollectionMap<ADNAtom> atoms;

	auto bb = GetBackbone();
	if (bb != nullptr) {

		CollectionMap<ADNAtom> bbAtoms = bb->GetAtoms();
		SB_FOR(ADNPointer<ADNAtom> a, bbAtoms) atoms.addReferenceTarget(a());

	}

	auto sc = GetSidechain();
	if (sc != nullptr) {

		CollectionMap<ADNAtom> scAtoms = sc->GetAtoms();
		SB_FOR(ADNPointer<ADNAtom> a, scAtoms) atoms.addReferenceTarget(a());

	}

	return atoms;

}

int ADNNucleotide::getNumberOfAtoms() const {

#if 1
	return countNodes(SBNode::IsType(SBNode::Atom) && (SBNode::GetClass() == std::string("ADNAtom")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
#else
	return static_cast<int>(GetAtoms().size());
#endif

}

void ADNNucleotide::HideCenterAtoms() {

	auto bb = GetBackbone();
	auto sc = GetSidechain();

	HideCenterAtom();
	if (bb != nullptr) bb->HideCenterAtom();
	if (sc != nullptr) sc->HideCenterAtom();

}

ADNPointer<ADNAtom> ADNNucleotide::GetBackboneCenterAtom() const {

	auto bb = GetBackbone();
	if (bb != nullptr) return bb->GetCenterAtom();
	else return nullptr;

}

ADNPointer<ADNAtom> ADNNucleotide::GetSidechainCenterAtom() const {

	auto sc = GetSidechain();
	if (sc != nullptr) return sc->GetCenterAtom();
	else return nullptr;

}

CollectionMap<ADNAtom> ADNNucleotide::GetAtomsByName(const std::string& name) const {

	CollectionMap<ADNAtom> res;
	/*SBNodeIndexer atoms;
	getNodes(atoms, SBNode::GetName() == name);

	SB_FOR(SBNode* a, atoms) {
	  ADNPointer<ADNAtom> at = static_cast<ADNAtom*>(a);
	  res.addReferenceTarget(at());
	}*/

	auto atoms = GetAtoms();
	SB_FOR(ADNPointer<ADNAtom> a, atoms) {

		if (a->getName() == name)
			res.addReferenceTarget(a());

	}

	return res;

}

ublas::matrix<double> ADNNucleotide::GetGlobalBasisTransformation() const {

	ublas::matrix<double> transf(3, 3, 0.0);
	ublas::column(transf, 0) = GetE1();
	ublas::column(transf, 1) = GetE2();
	ublas::column(transf, 2) = GetE3();
	return transf;

}

ADNPointer<ADNBaseSegment> ADNNucleotide::GetBaseSegment() const {
	return baseSegment;
}

SBNode* ADNNucleotide::getBaseSegment() const {
	return baseSegment();
}

std::string ADNNucleotide::getBaseSegmentTypeString() const {

	if (baseSegment != nullptr)
		return baseSegment->getCellTypeString();
	else
		return std::string();

}

ADNPointer<ADNDoubleStrand> ADNNucleotide::GetDoubleStrand() const {

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

class setNucleotideEndTypeCommand : public SBCUndoCommand {

public:

	setNucleotideEndTypeCommand(ADNNucleotide::EndType type, ADNNucleotide* propertyOwner) { this->owner = propertyOwner; this->oldValue = propertyOwner->getEndType(); this->newValue = type; }
	virtual ~setNucleotideEndTypeCommand() {}

	virtual std::string getName() const { return "Set nucleotide end type"; }

	void redo() { if (owner.isValid()) owner->setEndType(newValue); }
	void undo() { if (owner.isValid()) owner->setEndType(oldValue); }

private:

	ADNNucleotide::EndType                                      oldValue;
	ADNNucleotide::EndType                                      newValue;
	SBPointer<ADNNucleotide>								    owner;

};

ADNNucleotide::EndType ADNNucleotide::getEndType() const {
	return endType;
}

void ADNNucleotide::setEndType(ADNNucleotide::EndType type) {

	//if (SAMSON::isHolding()) SAMSON::hold(new setNucleotideEndTypeCommand(type, this));

	this->endType = type;

}

bool ADNNucleotide::isEndTypeNucleotide() const {
	return (endType != EndType::NotEnd);
}

void ADNNucleotide::Init() {

	setNucleotideType(DNABlocks::DI);
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

	SBBackbone* backbone = getBackbone();
	if (!backbone) return nullptr;

	auto bb = static_cast<ADNBackbone*>(backbone);
	if (bb)
		return ADNPointer<ADNBackbone>(bb);
	else
		return nullptr;

}

ADNPointer<ADNSidechain> ADNNucleotide::GetSidechain() const {

	SBSideChain* sideChain = getSideChain();
	if (!sideChain) return nullptr;

	auto sc = static_cast<ADNSidechain*>(sideChain);
	if (sc)
		return ADNPointer<ADNSidechain>(sc);
	else
		return nullptr;

}

void ADNNucleotide::SetSidechainPosition(const Position3D& pos) {

	auto sc = GetSidechain();
	if (sc.isValid()) sc->SetPosition(pos);

}

const Position3D& ADNNucleotide::GetSidechainPosition() const {

	auto sc = GetSidechain();
    if (sc.isValid()) return sc->GetPosition();
    else return Position3D::zero;

}

void ADNNucleotide::SetBackbonePosition(const Position3D& pos) {

	auto bb = GetBackbone();
	if (bb.isValid()) bb->SetPosition(pos);

}

const Position3D& ADNNucleotide::GetBackbonePosition() const {

	auto bb = GetBackbone();
    if (bb.isValid()) return bb->GetPosition();
    else return Position3D::zero;

}

Position3D ADNNucleotide::GetPosition() const {

	return (GetBackbonePosition() + GetSidechainPosition()) * 0.5;

}

bool ADNNucleotide::GlobalBaseIsSet() const {

	bool set = false;

	ublas::matrix<double> mat(3, 3);
	ublas::column(mat, 0) = GetE1();
	ublas::column(mat, 1) = GetE2();
	ublas::column(mat, 2) = GetE3();

	if (!ADNVectorMath::IsNearlyZero(ADNVectorMath::Determinant(mat))) set = true;

	return set;

}

bool ADNNucleotide::IsLeft() {

	if (baseSegment != nullptr) return baseSegment->IsLeft(this);
	else return false;

}

bool ADNNucleotide::IsRight() {

	if (baseSegment != nullptr) return baseSegment->IsRight(this);
	else return false;

}

class setNucleotideTagCommand : public SBCUndoCommand {

public:

	setNucleotideTagCommand(const std::string& tag, ADNNucleotide* propertyOwner) { this->owner = propertyOwner; this->oldValue = propertyOwner->getTag(); this->newValue = tag; }
	virtual ~setNucleotideTagCommand() {}

	virtual std::string getName() const { return "Set nucleotide tag"; }

	void redo() { if (owner.isValid()) owner->setTag(newValue); }
	void undo() { if (owner.isValid()) owner->setTag(oldValue); }

private:

	std::string                                                 oldValue;
	std::string                                                 newValue;
	SBPointer<ADNNucleotide>									owner;

};

std::string ADNNucleotide::getTag() const {

	return tag;

}

void ADNNucleotide::setTag(std::string t) {

	if (isLocked()) return;

	if (SAMSON::isHolding()) SAMSON::hold(new setNucleotideTagCommand(t, this));

	this->tag = t;

}

bool ADNNucleotide::hasTag() const {

	return !tag.empty();

}

void ADNNucleotide::print(unsigned int offset) const {

	std::cout << "Nucleotide " << getName() << ": position = " << GetPosition() << "; backbone position = " << GetBackbonePosition() << "; side-chain position = " << GetSidechainPosition() << std::endl;

}

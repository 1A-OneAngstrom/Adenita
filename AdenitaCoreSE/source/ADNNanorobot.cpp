#include "ADNNanorobot.hpp"


ADNNanorobot::ADNNanorobot(const ADNNanorobot & n) : Nameable(n), Positionable(n), Orientable(n) {

    *this = n;

}

ADNNanorobot & ADNNanorobot::operator=(const ADNNanorobot& other) {

    Nameable::operator =(other);
    Positionable::operator =(other);
    Orientable::operator =(other);

    return *this;

}

CollectionMap<ADNSingleStrand> ADNNanorobot::GetSingleStrands() const {

    CollectionMap<ADNSingleStrand> singleStrands;

#if ADENITA_NANOROBOT_REGISTER_PARTS
    SB_FOR(ADNPointer<ADNPart> p, partsIndex_) {

        auto pSingleStrands = p->GetSingleStrands();
        SB_FOR(ADNPointer<ADNSingleStrand> ss, pSingleStrands)
            singleStrands.addReferenceTarget(ss());

    }
#else
    // single strands are chains, so for performance reasons we first get all the chains and then check their class name and the extensions' UUID

    SBNodeIndexer chainIndexer;
    SAMSON::getActiveDocument()->getNodes(chainIndexer, SBNode::Chain);

    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode* node, chainIndexer)
        node->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer)
        singleStrands.addReferenceTarget(static_cast<ADNSingleStrand*>(node));
#endif

    return singleStrands;

}

void ADNNanorobot::RegisterPart(ADNPointer<ADNPart> part) {

    if (part->getName().empty()) {

        part->setName("Adenita component " + std::to_string(partId_));
        ++partId_;

    }

#if ADENITA_NANOROBOT_REGISTER_PARTS
    partsIndex_.addReferenceTarget(part());
#endif
}

void ADNNanorobot::DeregisterPart(ADNPointer<ADNPart> part) {
#if ADENITA_NANOROBOT_REGISTER_PARTS
    partsIndex_.removeReferenceTarget(part());
#endif
}

int ADNNanorobot::GetNumberOfDoubleStrands() {

    auto parts = GetParts();
    int count = 0;

    SB_FOR (ADNPointer<ADNPart> part, parts)
        count += part->GetNumberOfDoubleStrands();

    return count;

}

int ADNNanorobot::GetNumberOfBaseSegments() {

    auto parts = GetParts();
    int count = 0;

    SB_FOR(ADNPointer<ADNPart> part, parts)
        count += part->GetNumberOfBaseSegments();

    return count;

}

int ADNNanorobot::GetNumberOfSingleStrands() {

    auto parts = GetParts();
    int count = 0;

    SB_FOR(ADNPointer<ADNPart> part, parts)
        count += part->GetNumberOfSingleStrands();

    return count;

}

int ADNNanorobot::GetNumberOfNucleotides() {

    auto parts = GetParts();
    int count = 0;

    SB_FOR(ADNPointer<ADNPart> part, parts)
        count += part->GetNumberOfNucleotides();

    return count;

}

CollectionMap<ADNPart> ADNNanorobot::GetParts() const {

#if ADENITA_NANOROBOT_REGISTER_PARTS
    return partsIndex_;
#else
    CollectionMap<ADNPart> parts;

    SBNodeIndexer structuralModelIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralModelIndexer, SBNode::StructuralModel);

    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralModelIndexer)
        node->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNPart")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode * node, nodeIndexer)
        if (!node->isErased()) parts.addReferenceTarget(static_cast<ADNPart*>(node));

    return parts;
#endif

}

/*!
\param a pointer to a ADNSingleStrand
\return the ADNPart to which the ADNSingleStrand belongs to
*/
ADNPointer<ADNPart> ADNNanorobot::GetPart(ADNPointer<ADNSingleStrand> singleStrand) {

    if (singleStrand == nullptr) return nullptr;
    if (!singleStrand->getParent()) return nullptr;
    if (!singleStrand->getParent()->getParent()) return nullptr;

    SBNode* parent = singleStrand->getParent()->getParent();  // first parent is the structural model root
    ADNPointer<ADNPart> part = static_cast<ADNPart*>(parent);
    return part;

}

/*!
\param a pointer to a ADNDoubleStrand
\return the ADNPart to which the ADNDoubleStrand belongs to
*/
ADNPointer<ADNPart> ADNNanorobot::GetPart(ADNPointer<ADNDoubleStrand> doubleStrand) {

    if (doubleStrand == nullptr) return nullptr;
    if (!doubleStrand->getParent()) return nullptr;
    if (!doubleStrand->getParent()->getParent()) return nullptr;

    SBNode* parent = doubleStrand->getParent()->getParent();  // first parent is the structural model root
    ADNPointer<ADNPart> part = static_cast<ADNPart*>(parent);
    return part;

}

CollectionMap<ADNNucleotide> ADNNanorobot::GetSelectedNucleotides() {

    CollectionMap<ADNNucleotide> nucleotideIndexer;

    SBNodeIndexer residueIndexer;
    SAMSON::getActiveDocument()->getNodes(residueIndexer, SBNode::Residue);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, residueIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNNucleotide> nucleotide = static_cast<ADNNucleotide*>(node);
        nucleotideIndexer.addReferenceTarget(nucleotide());

    }

    return nucleotideIndexer;

}

CollectionMap<ADNPart> ADNNanorobot::GetSelectedParts() {

    CollectionMap<ADNPart> partIndexer;

    SBNodeIndexer structuralModelIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralModelIndexer, SBNode::StructuralModel);

    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralModelIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNPart")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNPart> part = static_cast<ADNPart*>(node);
        partIndexer.addReferenceTarget(part());

    }

    return partIndexer;

}

CollectionMap<SBAtom> ADNNanorobot::GetHighlightedAtoms() {

    CollectionMap<SBAtom> atoms;

    SBNodeIndexer atomIndexer;
    SAMSON::getActiveDocument()->getNodes(atomIndexer, SBNode::Atom);

    SB_FOR(SBNode* node, atomIndexer) {
        if (node->isHighlighted()) {

            ADNPointer<SBAtom> a = static_cast<SBAtom*>(node);
            atoms.addReferenceTarget(a());

        }
    }

    return atoms;

}

CollectionMap<ADNNucleotide> ADNNanorobot::GetHighlightedNucleotides() {

    CollectionMap<ADNNucleotide> nucleotideIndexer;

    SBNodeIndexer residueIndexer;
    SAMSON::getActiveDocument()->getNodes(residueIndexer, SBNode::Residue);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, residueIndexer)
        node->getNodes(nodeIndexer, SBNode::IsHighlighted() && (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNNucleotide> nucleotide = static_cast<ADNNucleotide*>(node);
        nucleotideIndexer.addReferenceTarget(nucleotide());

    }

    return nucleotideIndexer;

}

CollectionMap<ADNBaseSegment> ADNNanorobot::GetSelectedBaseSegmentsFromNucleotides() {

    CollectionMap<ADNBaseSegment> baseSegmentIndexer;

#if 1
    auto nucleotideIndexer = GetSelectedNucleotides();

    std::vector<ADNPointer<ADNBaseSegment>> added;

    SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotideIndexer) {

        ADNPointer<ADNBaseSegment> bs = nt->GetBaseSegment();
        if (std::find(added.begin(), added.end(), bs) == added.end()) {

            baseSegmentIndexer.addReferenceTarget(bs());
            added.push_back(bs);

        }

    }
#else
    SBNodeIndexer structuralGroupIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralGroupIndexer, SBNode::StructuralGroup);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode* node, structuralGroupIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNBaseSegment> bs = static_cast<ADNBaseSegment*>(node);
        baseSegmentIndexer.addReferenceTarget(bs());

    }
#endif

    return baseSegmentIndexer;

}

CollectionMap<ADNSingleStrand> ADNNanorobot::GetSelectedSingleStrands() {

    CollectionMap<ADNSingleStrand> singleStrandIndexer;

    SBNodeIndexer chainIndexer;
    SAMSON::getActiveDocument()->getNodes(chainIndexer, SBNode::Chain);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, chainIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNSingleStrand> ss = static_cast<ADNSingleStrand*>(node);
        singleStrandIndexer.addReferenceTarget(ss());

    }

    return singleStrandIndexer;

}

CollectionMap<ADNDoubleStrand> ADNNanorobot::GetSelectedDoubleStrands() {

    CollectionMap<ADNDoubleStrand> doubleStrandIndexer;

    SBNodeIndexer structuralGroupIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralGroupIndexer, SBNode::StructuralGroup);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralGroupIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(node);
        doubleStrandIndexer.addReferenceTarget(ds());

    }

    return doubleStrandIndexer;

}

CollectionMap<ADNDoubleStrand> ADNNanorobot::GetHighlightedDoubleStrands() {

    CollectionMap<ADNDoubleStrand> doubleStrands;

    SBNodeIndexer nodeIndexer;
    SAMSON::getActiveDocument()->getNodes(nodeIndexer, SBNode::IsHighlighted() && (SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(node);
        doubleStrands.addReferenceTarget(ds());

    }

    return doubleStrands;

}

/*!
\param a ADNPointer to a ADNPart
\return a CollectionMap of ADNSingleStrand
*/
CollectionMap<ADNSingleStrand> ADNNanorobot::GetSingleStrands(ADNPointer<ADNPart> part) {
    return part->GetSingleStrands();
}

/*!
\param a ADNPointer to a ADNSingleStrand. ADNSingleStrand is deleted
*/
void ADNNanorobot::RemoveSingleStrand(ADNPointer<ADNSingleStrand> singleStrand) {

    auto part = GetPart(singleStrand);
    part->DeregisterSingleStrand(singleStrand);

}

/*!
\param a ADNPointer to a ADNDoubleStrand. ADNDoubleStrand is deleted
*/
void ADNNanorobot::RemoveDoubleStrand(ADNPointer<ADNDoubleStrand> doubleStrand) {

    auto part = GetPart(doubleStrand);
    part->DeregisterDoubleStrand(doubleStrand);

}

/*!
\param a ADNPointer to a ADNSingleStrand
\param a ADNPointer to a ADNPart
*/
void ADNNanorobot::AddSingleStrand(ADNPointer<ADNSingleStrand> singleStrand, ADNPointer<ADNPart> part) {
    part->RegisterSingleStrand(singleStrand);
}

/*!
\param a ADNPointer to a ADNPart
\return a CollectionMap of ADNSingleStrand
*/
CollectionMap<ADNSingleStrand> ADNNanorobot::GetScaffolds(ADNPointer<ADNPart> part) {
    return part->GetScaffolds();
}

/*!
\param a ADNPointer to a ADNPart
\return a CollectionMap of ADNSingleStrand
*/
CollectionMap<ADNDoubleStrand> ADNNanorobot::GetDoubleStrands(ADNPointer<ADNPart> part) {
    return part->GetDoubleStrands();
}

/*!
\param a ADNPointer to a ADNNucleotide
\return the ADNDoubleStrand to which it belongs
*/
ADNPointer<ADNDoubleStrand> ADNNanorobot::GetDoubleStrand(ADNPointer<ADNNucleotide> nucleotide) {

    auto baseSegment = nucleotide->GetBaseSegment();
    if (baseSegment != nullptr) 
        return baseSegment->GetDoubleStrand();
    else
        return nullptr;

}

/*!
\param a ADNPointer to a ADNSingleStrand
\return a CollectionMap of ADNNucleotides
*/
CollectionMap<ADNNucleotide> ADNNanorobot::GetSingleStrandNucleotides(ADNPointer<ADNSingleStrand> singleStrand) {
    return singleStrand->GetNucleotides();
}

/*!
\param a ADNPointer to a ADNSingleStrand
\return a ADNPointer to the five prime ADNNucleotide
*/
ADNPointer<ADNNucleotide> ADNNanorobot::GetSingleStrandFivePrime(ADNPointer<ADNSingleStrand> singleStrand) {
    return singleStrand->GetFivePrime();
}

/*!
\param a ADNPointer to a ADNSingleStrand
\return a bool value true if it is a scaffold
*/
bool ADNNanorobot::IsScaffold(ADNPointer<ADNSingleStrand> singleStrand) {
    return singleStrand->IsScaffold();
}

/*!
\param a ADNPointer to a ADNNucleotide
\return the End position on the strand
*/
End ADNNanorobot::GetNucleotideEnd(ADNPointer<ADNNucleotide> nucleotide) {
    return nucleotide->GetEnd();
}

/*!
\param a ADNPointer to a ADNNucleotide
\return a ADNPointer to the ADNNucleotide next on the single strand
*/
ADNPointer<ADNNucleotide> ADNNanorobot::GetNucleotideNext(ADNPointer<ADNNucleotide> nucleotide, bool circular) {
    return nucleotide->GetNext(circular);
}

/*!
\param a ADNPointer to a ADNNucleotide
\return a ADNPointer to the ADNNucleotide pair
*/
ADNPointer<ADNNucleotide> ADNNanorobot::GetNucleotidePair(ADNPointer<ADNNucleotide> nucleotide) {
    return nucleotide->GetPair();
}

/*!
\param a ADNPointer to a ADNNucleotide
\return the positon of the nucleotide as SBPosition3
*/
SBPosition3 ADNNanorobot::GetNucleotidePosition(ADNPointer<ADNNucleotide> nucleotide) {
    return nucleotide->GetPosition();
}

/*!
\param a ADNPointer to a ADNNucleotide
\return the positon of the backbone of the nucleotide as SBPosition3
*/
SBPosition3 ADNNanorobot::GetNucleotideBackbonePosition(ADNPointer<ADNNucleotide> nucleotide) {
    return nucleotide->GetBackbonePosition();
}

/*!
\param a ADNPointer to a ADNNucleotide
\return the positon of the sidechain of the nucleotide as SBPosition3
*/
SBPosition3 ADNNanorobot::GetNucleotideSidechainPosition(ADNPointer<ADNNucleotide> nucleotide) {
    return nucleotide->GetSidechainPosition();
}

/*!
\param a ADNPointer to a ADNNucleotide
*/
void ADNNanorobot::HideCenterAtoms(ADNPointer<ADNNucleotide> nucleotide) {
    nucleotide->HideCenterAtoms();
}

/*!
\return A CollectionMap with all conformations
*/
CollectionMap<ADNConformation> ADNNanorobot::GetConformations() {

#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    return conformationsIndex_;
#else
    CollectionMap<ADNConformation> conformationIndexer;

    SBNodeIndexer auxIndexer;
    SAMSON::getActiveDocument()->getNodes(auxIndexer, SBNode::Conformation);

    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, auxIndexer)
        node->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNConformation")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode * node, nodeIndexer)
        conformationIndexer.addReferenceTarget(static_cast<ADNConformation*>(node));

    return conformationIndexer;
#endif

}

#if 0
/*!
\param A ADNPointer to a ADNPart
\return A CollectionMap with all conformations belonging to a given ADNPart
*/
CollectionMap<ADNConformation> ADNNanorobot::GetConformations(ADNPointer<ADNPart> part) {

    CollectionMap<ADNConformation> confs;
    SB_FOR(ADNPointer<ADNConformation> conf, conformationsIndex_) {

        // TODO: fix this implementation
        // conformations are in the document or in the folder, but not in the ADNPart (SM)
        auto parent = conf->getParent();
        ADNPointer<ADNPart> p = static_cast<ADNPart*>(parent);
        if (p == part) confs.addReferenceTarget(conf());

    }

    return confs;

}
#endif

/*!
\param A ADNPart to whcih the conformation belongs to
\param A ADNPointer to a ADNConformation
*/
void ADNNanorobot::RegisterConformation(ADNPointer<ADNConformation> conformation) {
#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    conformationsIndex_.addReferenceTarget(conformation());
#endif
}

/*!
\param The conformation
\param a ADNPointer to the nucleotide
\return The position of the backbone of the nucleotide in that conformation
*/
SBPosition3 ADNNanorobot::GetNucleotideBackbonePosition(ADNConformation conformation, ADNPointer<ADNNucleotide> nucleotide) {

    SBPosition3 pos;
    auto at = nucleotide->GetBackboneCenterAtom();
    conformation.getPosition(at(), pos);
    return pos;

}

/*!
\param The conformation
\param a ADNPointer to the nucleotide
\return The position of the side chain of the nucleotide in that conformation
*/
SBPosition3 ADNNanorobot::GetNucleotideSidechainPosition(ADNConformation conformation, ADNPointer<ADNNucleotide> nucleotide) {

    SBPosition3 pos;
    auto at = nucleotide->GetSidechainCenterAtom();
    conformation.getPosition(at(), pos);
    return pos;

}

CollectionMap<ADNBaseSegment> ADNNanorobot::GetHighlightedBaseSegmentsFromNucleotides() {

    CollectionMap<ADNBaseSegment> baseSegmentIndexer;

    auto nucleotideIndexer = GetHighlightedNucleotides();

    std::vector<ADNPointer<ADNBaseSegment>> added;

    SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotideIndexer) {

        ADNPointer<ADNBaseSegment> bs = nt->GetBaseSegment();
        if (std::find(added.begin(), added.end(), bs) == added.end()) {

            baseSegmentIndexer.addReferenceTarget(bs());
            added.push_back(bs);

        }

    }

    return baseSegmentIndexer;

}

CollectionMap<ADNBaseSegment> ADNNanorobot::GetHighlightedBaseSegments() {

    CollectionMap<ADNBaseSegment> baseSegmentIndexer;

    SBNodeIndexer structuralGroupIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralGroupIndexer, SBNode::StructuralGroup);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralGroupIndexer)
        node->getNodes(nodeIndexer, SBNode::IsHighlighted() && (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        ADNPointer<ADNBaseSegment> bs = static_cast<ADNBaseSegment*>(node);
        baseSegmentIndexer.addReferenceTarget(bs());

    }

    return baseSegmentIndexer;

}

std::pair<SBPosition3, SBPosition3> ADNNanorobot::GetBoundingBox(CollectionMap<ADNPart> parts) {

    auto maxVal = SBQuantity::picometer(std::numeric_limits<double>::max());
    auto minBox = SBPosition3(maxVal, maxVal, maxVal);
    auto maxBox = SBPosition3(-maxVal, -maxVal, -maxVal);

    SB_FOR(ADNPointer<ADNPart> part, parts) {

        auto bbPart = part->GetBoundingBox();
        if (bbPart.first[0] < minBox[0]) minBox[0] = bbPart.first[0];
        if (bbPart.first[1] < minBox[1]) minBox[1] = bbPart.first[1];
        if (bbPart.first[2] < minBox[2]) minBox[2] = bbPart.first[2];
        if (bbPart.second[0] > maxBox[0]) maxBox[0] = bbPart.second[0];
        if (bbPart.second[1] > maxBox[1]) maxBox[1] = bbPart.second[1];
        if (bbPart.second[2] > maxBox[2]) maxBox[2] = bbPart.second[2];

    }

    std::pair<SBPosition3, SBPosition3> bb(minBox, maxBox);
    return bb;

}

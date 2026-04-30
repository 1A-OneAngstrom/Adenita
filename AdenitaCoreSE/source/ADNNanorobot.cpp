#include "ADNNanorobot.hpp"
#include "ADNModel.hpp"

ADNNanorobot::ADNNanorobot() {// : Nameable(), Positionable(), Orientable() {}
}

ADNNanorobot::ADNNanorobot(const ADNNanorobot & n) {//: Nameable(n), Positionable(n), Orientable(n) {

    *this = n;

}

ADNNanorobot & ADNNanorobot::operator=(const ADNNanorobot& other) {

    //Nameable::operator =(other);
    //Positionable::operator =(other);
    //Orientable::operator =(other);

    return *this;

}

SBPointerIndexer<ADNSingleStrand> ADNNanorobot::GetSingleStrands() const {

    SBPointerIndexer<ADNSingleStrand> singleStrands;

#if ADENITA_NANOROBOT_REGISTER_PARTS
    SB_FOR(SBPointer<ADNPart> p, partsIndex_) {

        auto pSingleStrands = p->GetSingleStrands();
        SB_FOR(SBPointer<ADNSingleStrand> ss, pSingleStrands)
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

void ADNNanorobot::RegisterPart(SBPointer<ADNPart> part) {

    if (part->getName().empty()) {

        part->setName("Adenita component " + std::to_string(partId_));
        ++partId_;

    }

#if ADENITA_NANOROBOT_REGISTER_PARTS
    partsIndex_.addReferenceTarget(part());
#endif
}

void ADNNanorobot::DeregisterPart(SBPointer<ADNPart> part) {
#if ADENITA_NANOROBOT_REGISTER_PARTS
    partsIndex_.removeReferenceTarget(part());
#endif
}

unsigned int ADNNanorobot::GetNumberOfDoubleStrands() const {

    auto parts = GetParts();
    unsigned int count = 0;

    SB_FOR (SBPointer<ADNPart> part, parts)
        count += part->GetNumberOfDoubleStrands();

    return count;

}

unsigned int ADNNanorobot::GetNumberOfBaseSegments() const {

    auto parts = GetParts();
    unsigned int count = 0;

    SB_FOR(SBPointer<ADNPart> part, parts)
        count += part->GetNumberOfBaseSegments();

    return count;

}

unsigned int ADNNanorobot::GetNumberOfSingleStrands() const {

    auto parts = GetParts();
    unsigned int count = 0;

    SB_FOR(SBPointer<ADNPart> part, parts)
        count += part->GetNumberOfSingleStrands();

    return count;

}

unsigned int ADNNanorobot::GetNumberOfNucleotides() const {

    auto parts = GetParts();
    unsigned int count = 0;

    SB_FOR(SBPointer<ADNPart> part, parts)
        count += part->GetNumberOfNucleotides();

    return count;

}

SBPointerIndexer<ADNPart> ADNNanorobot::GetParts() const {

#if ADENITA_NANOROBOT_REGISTER_PARTS
    return partsIndex_;
#else
    SBPointerIndexer<ADNPart> parts;

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

unsigned int ADNNanorobot::GetNumberOfParts() const {

#if ADENITA_NANOROBOT_REGISTER_PARTS
    return partsIndex_.size();
#else
    SBNodeIndexer structuralModelIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralModelIndexer, SBNode::StructuralModel);

    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralModelIndexer)
        node->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNPart")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    return nodeIndexer.size();
#endif

}

SBPointerIndexer<ADNNucleotide> ADNNanorobot::GetSelectedNucleotides() const {

    SBPointerIndexer<ADNNucleotide> nucleotideIndexer;

    SBNodeIndexer residueIndexer;
    SAMSON::getActiveDocument()->getNodes(residueIndexer, SBNode::Residue);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, residueIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNNucleotide> nucleotide = static_cast<ADNNucleotide*>(node);
        nucleotideIndexer.addReferenceTarget(nucleotide());

    }

    return nucleotideIndexer;

}

SBPointerIndexer<ADNPart> ADNNanorobot::GetSelectedParts() const {

    SBPointerIndexer<ADNPart> partIndexer;

    SBNodeIndexer structuralModelIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralModelIndexer, SBNode::StructuralModel);

    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralModelIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNPart")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNPart> part = static_cast<ADNPart*>(node);
        partIndexer.addReferenceTarget(part());

    }

    return partIndexer;

}

SBPointerIndexer<SBAtom> ADNNanorobot::GetHighlightedAtoms() const {

    SBPointerIndexer<SBAtom> atoms;

    SBNodeIndexer atomIndexer;
    SAMSON::getActiveDocument()->getNodes(atomIndexer, SBNode::Atom);

    SB_FOR(SBNode* node, atomIndexer) {
        if (node->isHighlighted()) {

            SBPointer<SBAtom> a = static_cast<SBAtom*>(node);
            atoms.addReferenceTarget(a());

        }
    }

    return atoms;

}

SBPointerIndexer<ADNNucleotide> ADNNanorobot::GetHighlightedNucleotides() const {

    SBPointerIndexer<ADNNucleotide> nucleotideIndexer;

    SBNodeIndexer residueIndexer;
    SAMSON::getActiveDocument()->getNodes(residueIndexer, SBNode::Residue);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, residueIndexer)
        node->getNodes(nodeIndexer, SBNode::IsHighlighted() && (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNNucleotide> nucleotide = static_cast<ADNNucleotide*>(node);
        nucleotideIndexer.addReferenceTarget(nucleotide());

    }

    return nucleotideIndexer;

}

SBPointerIndexer<ADNBaseSegment> ADNNanorobot::GetSelectedBaseSegmentsFromNucleotides() const {

    SBPointerIndexer<ADNBaseSegment> baseSegmentIndexer;

#if 1
    auto nucleotideIndexer = GetSelectedNucleotides();

    std::vector<SBPointer<ADNBaseSegment>> added;

    SB_FOR(SBPointer<ADNNucleotide> nt, nucleotideIndexer) {

        SBPointer<ADNBaseSegment> bs = nt->GetBaseSegment();
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

        SBPointer<ADNBaseSegment> bs = static_cast<ADNBaseSegment*>(node);
        baseSegmentIndexer.addReferenceTarget(bs());

    }
#endif

    return baseSegmentIndexer;

}

SBPointerIndexer<ADNSingleStrand> ADNNanorobot::GetSelectedSingleStrands() const {

    SBPointerIndexer<ADNSingleStrand> singleStrandIndexer;

    SBNodeIndexer chainIndexer;
    SAMSON::getActiveDocument()->getNodes(chainIndexer, SBNode::Chain);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, chainIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNSingleStrand> ss = static_cast<ADNSingleStrand*>(node);
        singleStrandIndexer.addReferenceTarget(ss());

    }

    return singleStrandIndexer;

}

SBPointerIndexer<ADNDoubleStrand> ADNNanorobot::GetSelectedDoubleStrands() const {

    SBPointerIndexer<ADNDoubleStrand> doubleStrandIndexer;

    SBNodeIndexer structuralGroupIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralGroupIndexer, SBNode::StructuralGroup);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralGroupIndexer)
        node->getNodes(nodeIndexer, SBNode::IsSelected() && (SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(node);
        doubleStrandIndexer.addReferenceTarget(ds());

    }

    return doubleStrandIndexer;

}

SBPointerIndexer<ADNDoubleStrand> ADNNanorobot::GetHighlightedDoubleStrands() const {

    SBPointerIndexer<ADNDoubleStrand> doubleStrands;

    SBNodeIndexer nodeIndexer;
    SAMSON::getActiveDocument()->getNodes(nodeIndexer, SBNode::IsHighlighted() && (SBNode::GetClass() == std::string("ADNDoubleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(node);
        doubleStrands.addReferenceTarget(ds());

    }

    return doubleStrands;

}

/*!
\return A SBPointerIndexer with all conformations
*/
SBPointerIndexer<ADNConformation> ADNNanorobot::GetConformations() const {

#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    return conformationsIndex_;
#else
    SBPointerIndexer<ADNConformation> conformationIndexer;

    SBNodeIndexer auxIndexer;
    SAMSON::getActiveDocument()->getNodes(auxIndexer, SBNode::Conformation);

    //SBNodeIndexer nodeIndexer;
    //SB_FOR(SBNode * node, auxIndexer)
    //    node->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNConformation")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode * node, auxIndexer) {

        const QString name = QString::fromStdString(node->getName());

        if (name.endsWith(" 1D") || name.endsWith(" 2D") || name.endsWith(" 3D"))
            conformationIndexer.addReferenceTarget(static_cast<ADNConformation*>(node));

    }

    return conformationIndexer;
#endif

}

#if 0
/*!
\param A SBPointer to a ADNPart
\return A SBPointerIndexer with all conformations belonging to a given ADNPart
*/
SBPointerIndexer<ADNConformation> ADNNanorobot::GetConformations(SBPointer<ADNPart> part) {

    SBPointerIndexer<ADNConformation> confs;
    SB_FOR(SBPointer<ADNConformation> conf, conformationsIndex_) {

        // TODO: fix this implementation
        // conformations are in the document or in the folder, but not in the ADNPart (SM)
        auto parent = conf->getParent();
        SBPointer<ADNPart> p = static_cast<ADNPart*>(parent);
        if (p == part) confs.addReferenceTarget(conf());

    }

    return confs;

}
#endif

/*!
\param A ADNPart to which the conformation belongs to
\param A SBPointer to a ADNConformation
*/
void ADNNanorobot::RegisterConformation(SBPointer<ADNConformation> conformation) {
#if ADENITA_NANOROBOT_REGISTER_CONFORMATIONS
    conformationsIndex_.addReferenceTarget(conformation());
#endif
}

/*!
\param The conformation
\param a SBPointer to the nucleotide
\return The position of the backbone of the nucleotide in that conformation
*/
SBPosition3 ADNNanorobot::GetNucleotideBackbonePosition(SBPointer<ADNConformation> conformation, SBPointer<ADNNucleotide> nucleotide) const {

    SBPosition3 pos;
    auto at = nucleotide->GetBackboneCenterAtom();
    conformation->getPosition(at(), pos);
    return pos;

}

/*!
\param The conformation
\param a SBPointer to the nucleotide
\return The position of the side chain of the nucleotide in that conformation
*/
SBPosition3 ADNNanorobot::GetNucleotideSideChainPosition(SBPointer<ADNConformation> conformation, SBPointer<ADNNucleotide> nucleotide) const {

    SBPosition3 pos;
    auto at = nucleotide->GetSidechainCenterAtom();
    conformation->getPosition(at(), pos);
    return pos;

}

SBPointerIndexer<ADNBaseSegment> ADNNanorobot::GetHighlightedBaseSegmentsFromNucleotides() const {

    SBPointerIndexer<ADNBaseSegment> baseSegmentIndexer;

    auto nucleotideIndexer = GetHighlightedNucleotides();

    std::vector<SBPointer<ADNBaseSegment>> added;

    SB_FOR(SBPointer<ADNNucleotide> nt, nucleotideIndexer) {

        SBPointer<ADNBaseSegment> bs = nt->GetBaseSegment();
        if (std::find(added.begin(), added.end(), bs) == added.end()) {

            baseSegmentIndexer.addReferenceTarget(bs());
            added.push_back(bs);

        }

    }

    return baseSegmentIndexer;

}

SBPointerIndexer<ADNBaseSegment> ADNNanorobot::GetHighlightedBaseSegments() const {

    SBPointerIndexer<ADNBaseSegment> baseSegmentIndexer;

    SBNodeIndexer structuralGroupIndexer;
    SAMSON::getActiveDocument()->getNodes(structuralGroupIndexer, SBNode::StructuralGroup);
    SBNodeIndexer nodeIndexer;
    SB_FOR(SBNode * node, structuralGroupIndexer)
        node->getNodes(nodeIndexer, SBNode::IsHighlighted() && (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

    SB_FOR(SBNode* node, nodeIndexer) {

        SBPointer<ADNBaseSegment> bs = static_cast<ADNBaseSegment*>(node);
        baseSegmentIndexer.addReferenceTarget(bs());

    }

    return baseSegmentIndexer;

}

SBIAPosition3 ADNNanorobot::GetBoundingBox(SBPointerIndexer<ADNPart> parts) const {

    auto maxVal = SBQuantity::picometer(std::numeric_limits<double>::max()); 
    auto bb = SBIAPosition3(maxVal, -maxVal, maxVal, -maxVal, maxVal, -maxVal);

    SB_FOR(ADNPart* part, parts) if (part) {

        const auto& bbPart = part->GetBoundingBox();
        bb.bound(bbPart);

    }

    return bb;

}

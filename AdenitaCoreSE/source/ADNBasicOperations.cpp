#include "ADNBasicOperations.hpp"


SBPointer<ADNSingleStrand> ADNBasicOperations::MergeSingleStrands(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2, SBPointer<ADNSingleStrand> first_strand, SBPointer<ADNSingleStrand> second_strand) {

    if (first_strand == nullptr && second_strand == nullptr) return nullptr;
    else if (first_strand == nullptr) return second_strand;
    else if (second_strand == nullptr) return first_strand;

    SBPointer<ADNSingleStrand> ss = SBPointer<ADNSingleStrand>(new ADNSingleStrand());
    ss->setName("Merged Strand");
    if (SAMSON::isHolding()) SAMSON::hold(ss());
    ss->create();
    part1->RegisterSingleStrand(ss);

    auto fivePrimeF = first_strand->GetFivePrime();
    SBPointer<ADNNucleotide> nt = fivePrimeF;
    while (nt != nullptr) {

        SBPointer<ADNNucleotide> ntNext = nt->GetNext();
        // keep record of nt position in base segment
        auto info = GetBaseSegmentInfo(nt);

        part1->DeregisterNucleotide(nt, true, false);
        part1->RegisterNucleotideThreePrime(ss, nt);

        // nt was removed from base segment add it again
        SetBackNucleotideIntoBaseSegment(nt, info);
        nt = ntNext;

    }

    auto fivePrimeS = second_strand->GetFivePrime();
    nt = fivePrimeS;
    while (nt != nullptr) {

        SBPointer<ADNNucleotide> ntNext = nt->GetNext();
        // keep record of nt position in base segment
        auto info = GetBaseSegmentInfo(nt);

        part2->DeregisterNucleotide(nt, true, false);
        part1->RegisterNucleotideThreePrime(ss, nt);

        // nt was removed from base segment add it again
        SetBackNucleotideIntoBaseSegment(nt, info);

        nt = ntNext;

    }

    if (first_strand->IsScaffold() || second_strand->IsScaffold())
        ss->setScaffoldFlag(true);

    const auto firstSize = first_strand->getNumberOfNucleotides();
    const auto secondSize = second_strand->getNumberOfNucleotides();
    if (firstSize > 0 || secondSize > 0) {

        std::string msg = "Possible error when merging strands inside part";
        ADNLogger::LogDebug(msg);

    }
    else {

        // deregister single strands
        part1->DeregisterSingleStrand(first_strand);
        part2->DeregisterSingleStrand(second_strand);

    }

    return ss;

}

SBPointer<ADNDoubleStrand> ADNBasicOperations::MergeDoubleStrand(SBPointer<ADNPart> part, SBPointer<ADNDoubleStrand> first_strand, SBPointer<ADNDoubleStrand> second_strand) {

    if (first_strand == nullptr && second_strand == nullptr) return nullptr;
    else if (first_strand == nullptr) return second_strand;
    else if (second_strand == nullptr) return first_strand;

    SBPointer<ADNDoubleStrand> ds = SBPointer<ADNDoubleStrand>(new ADNDoubleStrand());
    ds->setName("Merged Double Strand");
    if (SAMSON::isHolding()) SAMSON::hold(ds());
    ds->create();
    part->RegisterDoubleStrand(ds);

    auto firstF = first_strand->GetFirstBaseSegment();
    SBPointer<ADNBaseSegment> bs = firstF;
    while (bs != nullptr) {

        SBPointer<ADNBaseSegment> nextBs = bs->GetNext();
        part->DeregisterBaseSegment(bs, true, true);
        part->RegisterBaseSegmentEnd(ds, bs);
        bs = nextBs;

    }

    auto firstS = second_strand->GetFirstBaseSegment();
    bs = firstS;
    while (bs != nullptr) {

        SBPointer<ADNBaseSegment> nextBs = bs->GetNext();
        part->DeregisterBaseSegment(bs, true, true);
        part->RegisterBaseSegmentEnd(ds, bs);
        bs = nextBs;

    }

    const auto firstSize = first_strand->GetLength();
    const auto secondSize = second_strand->GetLength();
    if (firstSize > 0 || secondSize > 0) {

        std::string msg = "Possible error when merging strands inside part";
        ADNLogger::LogDebug(msg);

    }

    return ds;

}

SBPointerIndexer<ADNNucleotide> ADNBasicOperations::AddNucleotidesThreePrime(SBPointer<ADNPart> part, SBPointer<ADNSingleStrand> ss, int number, SBVector3 dir) {

    SBPointerIndexer<ADNNucleotide> nts;

    if (part == nullptr || ss == nullptr) return nts;

    for (int i = 0; i < number; ++i) {

        SBPointer<ADNNucleotide> nt = ss->GetThreePrime();
        auto bs = nt->GetBaseSegment();
        auto ds = bs->GetDoubleStrand();
        bool isLeft = bs->IsLeft(nt);
        auto n = GetNextBaseSegment(nt);
        ADNNucleotide::EndType e = n.first;
        SBPointer<ADNBaseSegment> nextBs = n.second;
        SBPosition3 pos = nt->GetPosition() + SBQuantity::nanometer(ADNConstants::BP_RISE) * dir;

        if (nextBs == nullptr) {

            nextBs = new ADNBaseSegment(CellType::BasePair);
            if (SAMSON::isHolding()) SAMSON::hold(nextBs());
            nextBs->create();
            if (e == ADNNucleotide::EndType::ThreePrime) {
                ds->AddBaseSegmentEnd(nextBs);
            }
            else {
                ds->AddBaseSegmentBeginning(nextBs);
            }
            nextBs->SetPosition(pos);
            nextBs->SetE3(ADNAuxiliary::SBVectorToUblasVector(dir));

        }
        SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(nextBs->GetCell()());

        SBPointer<ADNNucleotide> newNt = new ADNNucleotide();
        newNt->Init();

        newNt->SetPosition(pos);
        newNt->SetBaseSegment(nextBs);
        if (isLeft) {
            bp->SetLeftNucleotide(newNt);
        }
        else {
            bp->SetRightNucleotide(newNt);
        }
        if (SAMSON::isHolding()) SAMSON::hold(newNt());
        newNt->create();
        part->RegisterNucleotideThreePrime(ss, newNt);

        nts.addReferenceTarget(newNt());

    }

    return nts;

}

SBPointer<ADNPart> ADNBasicOperations::MergeParts(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2) {

    if (part1 == nullptr && part2 == nullptr) return nullptr;
    else if (part1 == nullptr) return part2;
    else if (part2 == nullptr) return part1;

    SBPointer<ADNPart> part = part1;

    auto doubleStrands = part2->GetDoubleStrands();
    SB_FOR(SBPointer<ADNDoubleStrand> ds, doubleStrands) {

        part2->DeregisterDoubleStrand(ds);
        part->RegisterDoubleStrand(ds);

        // the code below does something only if ADNPart registers nodes
        auto bs = ds->GetFirstBaseSegment();
        while (bs != nullptr) {

            auto bsNext = bs->GetNext();
            part2->DeregisterBaseSegment(bs, false);
            part->RegisterBaseSegmentEnd(ds, bs, false);
            bs = bsNext;

        }
        //auto numBs = ds->GetBaseSegments().size();
        //int test = 1;

    }

    auto singleStrands = part2->GetSingleStrands();
    SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

        part2->DeregisterSingleStrand(ss);
        part->RegisterSingleStrand(ss);

        auto nt = ss->GetFivePrime();
        while (nt != nullptr) {

            auto ntNext = nt->GetNext();
            auto info = GetBaseSegmentInfo(nt);

            part2->DeregisterNucleotide(nt, false);
            // nt has de-registered from bp or loop pair
            part->RegisterNucleotideThreePrime(ss, nt, false);

            SetBackNucleotideIntoBaseSegment(nt, info);

            auto atoms = nt->GetAtoms();
            SB_FOR(SBPointer<ADNAtom> atom, atoms) {

                part2->DeregisterAtom(atom, false);
                NucleotideGroup g = NucleotideGroup::SideChain;
                if (atom->IsInADNBackbone()) g = NucleotideGroup::Backbone;
                part->RegisterAtom(nt, g, atom);

            }

            nt = ntNext;

        }

    }

    return part;

}

std::pair<SBPointer<ADNSingleStrand>, SBPointer<ADNSingleStrand>> ADNBasicOperations::BreakSingleStrand(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt) {

    if (!part.isValid() || !nt.isValid()) return { nullptr, nullptr };

    // TODO: take care of the following cases
    // 1. there is a single nucleotide in the single strand and this nucleotide is to be broken
    // 2. the nucleotide nt has not next or previous nucleotide (it's and end nucleotide)

    SBPointer<ADNSingleStrand> ssFP = new ADNSingleStrand();
    ssFP->setName("Broken Strand 1");
    if (SAMSON::isHolding()) SAMSON::hold(ssFP());
    ssFP->create();
    part->RegisterSingleStrand(ssFP);

    SBPointer<ADNSingleStrand> ssTP = new ADNSingleStrand();
    ssTP->setName("Broken Strand 2");
    if (SAMSON::isHolding()) SAMSON::hold(ssTP());
    ssTP->create();
    part->RegisterSingleStrand(ssTP);

    SBPointer<ADNSingleStrand> ss = nt->GetStrand();

    auto fivePrime = ss->GetFivePrime();
    SBPointer<ADNNucleotide> nucleotide = fivePrime;

    while (nucleotide != nullptr && nucleotide != nt) {

        SBPointer<ADNNucleotide> ntNext = nucleotide->GetNext();
        auto info = GetBaseSegmentInfo(nucleotide);
        part->DeregisterNucleotide(nucleotide, true, false);
        part->RegisterNucleotideThreePrime(ssFP, nucleotide);
        SetBackNucleotideIntoBaseSegment(nucleotide, info);
        nucleotide = ntNext;

    }

    while (nucleotide != nullptr) {

        SBPointer<ADNNucleotide> ntNext = nucleotide->GetNext();
        auto info = GetBaseSegmentInfo(nucleotide);
        part->DeregisterNucleotide(nucleotide, true, false);
        part->RegisterNucleotideThreePrime(ssTP, nucleotide);
        SetBackNucleotideIntoBaseSegment(nucleotide, info);
        nucleotide = ntNext;

    }

    if (ss->IsScaffold()) {

        // largest strand stays as scaffold
        const auto sizeF = ssFP->getNumberOfNucleotides();
        const auto sizeT = ssTP->getNumberOfNucleotides();
        if (sizeF > sizeT)
            ssFP->setScaffoldFlag(true);
        else
            ssTP->setScaffoldFlag(true);

    }

    const auto sz = ss->getNumberOfNucleotides();
    if (sz > 0) {

        std::string msg = "Possible error when breaking strands inside part";
        ADNLogger::LogDebug(msg);

    }
    else {

        // Deregister old strand
        part->DeregisterSingleStrand(ss);
        ss->erase();
        ss.deleteReferenceTarget();

    }

    std::pair<SBPointer<ADNSingleStrand>, SBPointer<ADNSingleStrand>> ssPair = std::make_pair(ssFP, ssTP);
    return ssPair;

}

std::pair<SBPointer<ADNDoubleStrand>, SBPointer<ADNDoubleStrand>> ADNBasicOperations::BreakDoubleStrand(SBPointer<ADNPart> part, SBPointer<ADNBaseSegment> bs) {

    if (!part.isValid() || !bs.isValid()) return { nullptr, nullptr };

    SBPointer<ADNDoubleStrand> dsFP = new ADNDoubleStrand();
    dsFP->setName("Broken Double Strand 1");
    if (SAMSON::isHolding()) SAMSON::hold(dsFP());
    dsFP->create();
    part->RegisterDoubleStrand(dsFP);
    SBPointer<ADNDoubleStrand> dsTP = new ADNDoubleStrand();
    dsTP->setName("Broken Double Strand 2");
    if (SAMSON::isHolding()) SAMSON::hold(dsTP());
    dsTP->create();
    part->RegisterDoubleStrand(dsTP);

    SBPointer<ADNDoubleStrand> ds = bs->GetDoubleStrand();

    auto firstBs = ds->GetFirstBaseSegment();
    SBPointer<ADNBaseSegment> baseS = firstBs;
    const int num = bs->GetNumber();  // store number for calculating angle twist later

    while (baseS != nullptr && baseS != bs) {

        SBPointer<ADNBaseSegment> bsNext = baseS->GetNext();
        part->DeregisterBaseSegment(baseS, true, false);
        part->RegisterBaseSegmentEnd(dsFP, baseS);
        baseS = bsNext;

    }

    while (baseS != nullptr) {

        SBPointer<ADNBaseSegment> bsNext = baseS->GetNext();
        part->DeregisterBaseSegment(baseS, true, false);
        part->RegisterBaseSegmentEnd(dsTP, baseS);
        baseS = bsNext;

    }

    auto initAngle = ds->GetInitialTwistAngle();
    dsFP->SetInitialTwistAngle(initAngle);
    dsTP->SetInitialTwistAngle(initAngle + ADNConstants::BP_ROT * num);

    auto sz = ds->GetBaseSegments().size();
    if (sz > 0) {

        std::string msg = "Possible error when breaking strands inside part";
        ADNLogger::LogDebug(msg);

    }
    else {

        // Deregister old strand
        part->DeregisterDoubleStrand(ds);
        ds->erase();
        ds.deleteReferenceTarget();

    }

    std::pair<SBPointer<ADNDoubleStrand>, SBPointer<ADNDoubleStrand>> dsPair = std::make_pair(dsFP, dsTP);
    return dsPair;

}

std::pair<SBPointer<ADNSingleStrand>, SBPointer<ADNSingleStrand>> ADNBasicOperations::DeleteNucleotide(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt) {

    if (!part.isValid() || !nt.isValid()) return { nullptr, nullptr };

    SEConfig& config = SEConfig::GetInstance();

    const auto numNts = part->GetNumberOfNucleotides();
    const auto numSS = part->GetNumberOfSingleStrands();

    auto ss = nt->GetStrand();
    ADNNucleotide::EndType e = nt->getEndType();

    std::pair<SBPointer<ADNSingleStrand>, SBPointer<ADNSingleStrand>> res = std::make_pair(nullptr, nullptr);

    if (e == ADNNucleotide::EndType::FiveAndThreePrime || e == ADNNucleotide::EndType::FivePrime) {

        // we don't need to break, just delete
        SBPointer<ADNNucleotide> ntNext = nt->GetNext();
        if (ntNext != nullptr) {

            if (ntNext->getEndType() == ADNNucleotide::EndType::ThreePrime)
                ntNext->setEndType(ADNNucleotide::EndType::FiveAndThreePrime);
            else
                ntNext->setEndType(e);

            ss->SetFivePrime(ntNext);
            res.first = ss;

        }
        else {

            // e should be FiveAndThreePrime
            part->DeregisterSingleStrand(ss);
            ss->erase();
            ss.deleteReferenceTarget();

        }

        part->DeregisterNucleotide(nt);
        nt->disconnectPair();
        nt->erase();
        nt.deleteReferenceTarget();

    }
    else {

        // first break
        auto ssPair = BreakSingleStrand(part, nt);
        res.first = ssPair.first;
        part->RegisterSingleStrand(res.first);  // register new strand

        if (e == ADNNucleotide::EndType::ThreePrime) {

            part->DeregisterSingleStrand(ssPair.second);

        }
        else {

            // second break
            auto ssPair2 = BreakSingleStrand(part, nt->GetNext());
            res.second = ssPair2.second;
            part->RegisterSingleStrand(res.second);
            part->DeregisterSingleStrand(ssPair2.first);  // deregister strand containing only the nt we want to delete

        }

        auto bs = nt->GetBaseSegment();
        if (bs != nullptr && bs->GetCellType() == CellType::LoopPair) {

            SBPointer<ADNLoopPair> loopPair = static_cast<ADNLoopPair*>(bs->GetCell()());
            loopPair->RemoveNucleotide(nt);

        }

        part->DeregisterNucleotide(nt);
        nt->disconnectPair();
        nt->erase();
        nt.deleteReferenceTarget();

    }

    const auto numNtsNew = part->GetNumberOfNucleotides();
    const auto numSSNew = part->GetNumberOfSingleStrands();

    if (config.mode == SEConfigMode::DEBUG_NO_LOG || config.mode == SEConfigMode::DEBUG_LOG) {
        std::string msg = "  --> DELETING NUCLEOTIDE";
        ADNLogger::LogDebug(msg);
        msg = "         Nucleotides before deletion: " + std::to_string(numNts) + "\n";
        msg += "         Nucleotides after deletion: " + std::to_string(numNtsNew);
        ADNLogger::LogDebug(msg);
        msg = "         Single Strands before deletion: " + std::to_string(numSS) + "\n";
        msg += "         Single Strands after deletion: " + std::to_string(numSSNew);
        ADNLogger::LogDebug(msg);
    }

    return res;

}

void ADNBasicOperations::DeleteNucleotideWithoutBreak(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt) {

    if (!part.isValid() || !nt.isValid()) return;

    if (nt->getEndType() != ADNNucleotide::EndType::FiveAndThreePrime) {

        SBPointer<ADNNucleotide> next = nt->GetNext();
        SBPointer<ADNNucleotide> prev = nt->GetPrev();

        if (nt->getEndType() == ADNNucleotide::EndType::FivePrime)
            next->setEndType(ADNNucleotide::EndType::FivePrime);

        if (nt->getEndType() == ADNNucleotide::EndType::ThreePrime)
            prev->setEndType(ADNNucleotide::EndType::ThreePrime);

    }

    SBPointer<ADNSingleStrand> ss = nt->GetStrand();
    ss->removeChild(nt());

}

std::pair<SBPointer<ADNDoubleStrand>, SBPointer<ADNDoubleStrand>> ADNBasicOperations::DeleteBaseSegment(SBPointer<ADNPart> part, SBPointer<ADNBaseSegment> bs) {

    if (!part.isValid() || !bs.isValid()) return { nullptr, nullptr };

    SEConfig& config = SEConfig::GetInstance();

    const auto numBss = part->GetNumberOfBaseSegments();
    const auto numDS = part->GetNumberOfDoubleStrands();

    std::pair<SBPointer<ADNDoubleStrand>, SBPointer<ADNDoubleStrand>> res = std::make_pair(nullptr, nullptr);

    SBPointer<ADNBaseSegment> oldNext = bs->GetNext();
    const bool fst = bs->GetPrev() == nullptr;
    const bool fstAndLst = bs->GetNext() == nullptr && fst;

    if (fstAndLst || fst) {

        SBPointer<ADNDoubleStrand> ds = bs->GetDoubleStrand();

        // we don't need to break, just delete
        if (!fstAndLst) {

            if (oldNext->GetNext() == nullptr) {
                ds->SetEnd(oldNext);
            }
            ds->SetStart(oldNext);
            res.first = ds;

        }
        else {

            // last base segment of the double strand
            part->DeregisterDoubleStrand(ds);
            ds->erase();
            ds.deleteReferenceTarget();

        }

    }
    else {

        // first break
        auto dsPair = BreakDoubleStrand(part, bs);
        res.first = dsPair.first;
        part->RegisterDoubleStrand(res.first);  // register new strand

        if (oldNext == nullptr) {

            part->DeregisterDoubleStrand(dsPair.second);

        }
        else {

            // second break
            auto dsPair2 = BreakDoubleStrand(part, oldNext);
            res.second = dsPair2.second;
            part->RegisterDoubleStrand(res.second);
            part->DeregisterDoubleStrand(dsPair2.first);  // deregister strand containing only the nt we want to delete

        }

    }

    part->DeregisterBaseSegment(bs);
    bs->erase();
    bs.deleteReferenceTarget();

    const auto numBssNew = part->GetNumberOfBaseSegments();
    const auto numDSNew = part->GetNumberOfDoubleStrands();

    if (config.mode == SEConfigMode::DEBUG_NO_LOG || config.mode == SEConfigMode::DEBUG_LOG) {

        std::string msg = "  --> DELETING NUCLEOTIDE";
        ADNLogger::LogDebug(msg);
        msg = "         Nucleotides before deletion: " + std::to_string(numBss) + "\n";
        msg += "         Nucleotides after deletion: " + std::to_string(numBssNew);
        ADNLogger::LogDebug(msg);
        msg = "         Single Strands before deletion: " + std::to_string(numDS) + "\n";
        msg += "         Single Strands after deletion: " + std::to_string(numDSNew);
        ADNLogger::LogDebug(msg);

    }

    return res;

}

void ADNBasicOperations::DeleteSingleStrand(SBPointer<ADNSingleStrand> ss) {

    if (ss != nullptr)
        ss.deleteReferenceTarget();

}

void ADNBasicOperations::DeleteDoubleStrand(SBPointer<ADNDoubleStrand> ds) {

    if (ds != nullptr)
        ds.deleteReferenceTarget();

}

void ADNBasicOperations::MutateNucleotide(SBPointer<ADNNucleotide> nt, DNABlocks newType, bool changePair) {

    if (!nt.isValid()) return;

    nt->setNucleotideType(newType);
    if (changePair) {

        auto ntPair = nt->GetPair();
        if (ntPair != nullptr)
            ntPair->setNucleotideType(ADNModel::GetComplementaryBase(newType));

    }

}

void ADNBasicOperations::SetSingleStrandSequence(SBPointer<ADNSingleStrand> ss, const std::string& seq, bool changePair, bool overwrite) {

    if (!ss.isValid()) return;

    SBPointer<ADNNucleotide> fivePrime = ss->GetFivePrime();
    SBPointer<ADNNucleotide> nt = fivePrime;
    int count = 0;

    while (count < seq.size() && nt != nullptr) {

        DNABlocks t = ADNModel::ResidueNameToType(seq[count]);
        if (overwrite || (!overwrite && nt->getNucleotideType() == DNABlocks::DI)) MutateNucleotide(nt, t, changePair);
        ++count;
        nt = nt->GetNext();

    }

}

void ADNBasicOperations::MutateBasePairIntoLoopPair(SBPointer<ADNBaseSegment> bs) {

    if (!bs.isValid()) return;

    auto cell = bs->GetCell();
    if (cell == nullptr) return;
    if (cell->GetCellType() != CellType::BasePair) return;

    SBPointer<ADNBasePair> bp(static_cast<ADNBasePair*>(cell()));
    SBPointer<ADNNucleotide> leftNt = bp->GetLeftNucleotide();
    SBPointer<ADNNucleotide> rightNt = bp->GetRightNucleotide();
    // unpair
    leftNt->SetPair(nullptr);
    rightNt->SetPair(nullptr);

    SBPointer<ADNLoopPair> lp = SBPointer<ADNLoopPair>(new ADNLoopPair());
    SBPointer<ADNLoop> leftLoop = SBPointer<ADNLoop>(new ADNLoop());
    SBPointer<ADNLoop> rightLoop = SBPointer<ADNLoop>(new ADNLoop());
    lp->SetLeftLoop(leftLoop);
    lp->SetRightLoop(rightLoop);
    leftLoop->AddNucleotide(leftNt);
    leftLoop->SetStart(leftNt);
    leftLoop->SetEnd(leftNt);
    rightLoop->AddNucleotide(rightNt);
    rightLoop->SetStart(rightNt);
    rightLoop->SetEnd(rightNt);

    bs->SetCell(lp());

}

void ADNBasicOperations::SetStart(SBPointer<ADNNucleotide> nucleotide, bool resetSequence) {

    if (!nucleotide.isValid()) return;

    auto singleStrand = nucleotide->GetStrand();
    if (singleStrand.isValid())
        singleStrand->ShiftStart(nucleotide, resetSequence);

}

void ADNBasicOperations::MoveStrand(SBPointer<ADNPart> oldPart, SBPointer<ADNPart> part, SBPointer<ADNDoubleStrand> ds) {

    if (!oldPart.isValid()) return;
    if (!part.isValid()) return;
    if (!ds.isValid()) return;

    oldPart->DeregisterDoubleStrand(ds);
    part->RegisterDoubleStrand(ds);

    auto bs = ds->GetFirstBaseSegment();
    std::vector<SBPointer<ADNSingleStrand>> strands;
    while (bs != nullptr) {

        oldPart->DeregisterBaseSegment(bs);
        part->RegisterBaseSegmentEnd(ds, bs);

        auto nts = bs->GetNucleotides();
        SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

            if (!nt.isValid()) continue;
            SBPointer<ADNSingleStrand> ss = nt->GetStrand();
            if (std::find(strands.begin(), strands.end(), ss) == strands.end()) {

                MoveStrand(oldPart, part, ss);
                strands.push_back(ss);

            }

        }

        bs = bs->GetNext();

    }

}

void ADNBasicOperations::MoveStrand(SBPointer<ADNPart> oldPart, SBPointer<ADNPart> part, SBPointer<ADNSingleStrand> ss) {

    if (!oldPart.isValid()) return;
    if (!part.isValid()) return;
    if (!ss.isValid()) return;

    oldPart->DeregisterSingleStrand(ss);
    part->RegisterSingleStrand(ss);

    auto nt = ss->GetFivePrime();
    while (nt != nullptr) {

        oldPart->DeregisterNucleotide(nt);
        part->RegisterNucleotideThreePrime(ss, nt);

        auto atoms = nt->GetAtoms();
        SB_FOR(SBPointer<ADNAtom> at, atoms) {

            if (!at.isValid()) continue;

            NucleotideGroup g = NucleotideGroup::Backbone;
            if (!at->IsInADNBackbone()) g = NucleotideGroup::SideChain;
            oldPart->DeregisterAtom(at);
            part->RegisterAtom(nt, g, at);

        }

        nt = nt->GetNext();

    }

}

void ADNBasicOperations::TwistDoubleHelix(SBPointer<ADNDoubleStrand> ds, double deg) {

    if (ds.isValid())
        ds->SetInitialTwistAngle(deg);

}

void ADNBasicOperations::CenterPart(SBPointer<ADNPart> part) {

    if (!part.isValid()) return;

    SEConfig& config = SEConfig::GetInstance();
    SBPosition3 trans = -CalculateCenterOfMass(part);
    auto baseSegments = part->GetBaseSegments();
    SB_FOR(SBPointer<ADNBaseSegment> bs, baseSegments) {

        if (!bs.isValid()) continue;

        bs->SetPosition(bs->GetPosition() + trans);
        auto nucleotides = bs->GetNucleotides();
        SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) {

            if (!nt.isValid()) continue;

            nt->SetPosition(nt->GetPosition() + trans);
            nt->SetBackbonePosition(nt->GetBackbonePosition() + trans);
            nt->SetSidechainPosition(nt->GetSidechainPosition() + trans);
            if (config.use_atomic_details) {

                auto atoms = nt->GetAtoms();
                SB_FOR(SBPointer<ADNAtom> a, atoms)
                    if (a.isValid()) a->setPosition(a->getPosition() + trans);

            }

        }

    }

}

SBPosition3 ADNBasicOperations::CalculateCenterOfMass(SBPointer<ADNPart> part) {

    SBPosition3 cm(SBQuantity::picometer(0.0));

    if (!part.isValid()) return cm;

    auto atoms = part->GetAtoms();

    SB_FOR(SBPointer<ADNAtom> a, atoms) if (a.isValid()) cm += a->getPosition();

    auto sz = atoms.size();
    cm *= (1.0 / sz);
    return cm;

}

std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>> ADNBasicOperations::OrderNucleotides(SBPointer<ADNNucleotide> nt1, SBPointer<ADNNucleotide> nt2) {

    std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>> res = std::make_pair(nullptr, nullptr);

    if (nt1 == nullptr || nt2 == nullptr) return res;
    if (nt1->GetStrand() != nt2->GetStrand()) return res;

    SBPointer<ADNSingleStrand> ss = nt1->GetStrand();
    SBPointer<ADNNucleotide> nt = ss->GetFivePrime();

    std::vector<SBPointer<ADNNucleotide>> list;
    while (nt != nullptr) {

        if (nt == nt1 || nt == nt2)
            list.push_back(nt);

        nt = nt->GetNext();

    }

    if (list.size() == 1 && nt1 == nt2) {

        // loop has only one nucleotide
        res.first = list[0];
        res.second = list[0];

    }
    else if (list.size() == 2) {

        res.first = list[0];
        res.second = list[1];

    }

    return res;

}

std::pair<ADNNucleotide::EndType, SBPointer<ADNBaseSegment>> ADNBasicOperations::GetNextBaseSegment(SBPointer<ADNNucleotide> nt) {

    SBPointer<ADNBaseSegment> nextBs = nullptr;
    ADNNucleotide::EndType end = ADNNucleotide::EndType::NotEnd;

    if (nt == nullptr) return { end, nextBs };

    auto bs = nt->GetBaseSegment();
    if (bs == nullptr) return { end, nextBs };

    auto ds = bs->GetDoubleStrand();
    const auto& e3 = nt->GetE3();
    const auto& bsE3 = bs->GetE3();

    if (ublas::inner_prod(e3, bsE3) > 0) {

        nextBs = bs->GetNext(true);
        end = ADNNucleotide::EndType::ThreePrime;

    }
    else {

        nextBs = bs->GetPrev(true);
        end = ADNNucleotide::EndType::FivePrime;

    }

    return std::make_pair(end, nextBs);

}

std::tuple<SBPointer<ADNBaseSegment>, bool, bool, bool> ADNBasicOperations::GetBaseSegmentInfo(SBPointer<ADNNucleotide> nt) {

    if (nt == nullptr) return { nullptr, false, false, false };
    auto bs = nt->GetBaseSegment();
    if (bs == nullptr) return { nullptr, false, false, false };

    auto cell = bs->GetCell();
    bool left = bs->IsLeft(nt);
    bool start = false;
    bool end = false;

    if (bs->GetCellType() == CellType::LoopPair) {

        SBPointer<ADNLoopPair> lp = static_cast<ADNLoopPair*>(cell());
        if (left) {

            auto leftLoop = lp->GetLeftLoop();
            start = leftLoop->GetStart() == nt;
            end = leftLoop->GetEnd() == nt;

        }
        else {

            auto rightLoop = lp->GetRightLoop();
            start = rightLoop->GetStart() == nt;
            end = rightLoop->GetEnd() == nt;

        }

    }

    return std::make_tuple(bs, left, start, end);

}

void ADNBasicOperations::SetBackNucleotideIntoBaseSegment(SBPointer<ADNNucleotide> nt, std::tuple<SBPointer<ADNBaseSegment>, bool, bool, bool> info) {

    SBPointer<ADNBaseSegment> bs = std::get<0>(info);
    const bool left = std::get<1>(info);
    const bool start = std::get<2>(info);
    const bool end = std::get<3>(info);

    if (bs == nullptr) return;

    auto cell = bs->GetCell();
    if (cell == nullptr) return;

    if (bs->GetCellType() == CellType::BasePair) {

        SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(cell());
        if (left) bp->SetLeftNucleotide(nt);
        else bp->SetRightNucleotide(nt);

    }
    else if (bs->GetCellType() == CellType::LoopPair) {

        SBPointer<ADNLoopPair> lp = static_cast<ADNLoopPair*>(cell());
        if (left) {

            auto leftLoop = lp->GetLeftLoop();
            leftLoop->AddNucleotide(nt);
            if (start) leftLoop->SetStart(nt);
            if (end) leftLoop->SetEnd(nt);

        }
        else {

            auto rightLoop = lp->GetRightLoop();
            rightLoop->AddNucleotide(nt);
            if (start) rightLoop->SetStart(nt);
            if (end) rightLoop->SetEnd(nt);

        }

    }

}

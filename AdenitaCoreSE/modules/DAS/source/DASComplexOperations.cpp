#include "DASComplexOperations.hpp"
#include "DASBackToTheAtom.hpp"
#include "ADNLogger.hpp"

namespace {

#ifndef NDEBUG
void ValidateCircularStrandTopology(SBPointer<ADNSingleStrand> strand) {

	if (strand == nullptr) {
		ADNLogger::LogDebug(std::string("Circular strand validation skipped for a null strand."));
		return;
	}

	SBPointer<ADNNucleotide> fivePrime = strand->GetFivePrime();
	SBPointer<ADNNucleotide> threePrime = strand->GetThreePrime();
	if (fivePrime == nullptr || threePrime == nullptr) {
		ADNLogger::LogDebug("Circular strand '" + strand->getName() + "' has missing 5'/3' endpoints.");
		return;
	}

	if (threePrime->GetNext(true) != fivePrime)
		ADNLogger::LogDebug("Circular strand '" + strand->getName() + "' does not wrap 3' -> 5'.");

	if (fivePrime->GetPrev(true) != threePrime)
		ADNLogger::LogDebug("Circular strand '" + strand->getName() + "' does not wrap 5' -> 3'.");

}
#else
void ValidateCircularStrandTopology(SBPointer<ADNSingleStrand>) {}
#endif

void MarkCircularAndValidate(SBPointer<ADNSingleStrand> strand) {

	if (strand == nullptr) return;

	strand->setCircularFlag(true);
	ValidateCircularStrandTopology(strand);

}

}

DASOperations::Connections DASOperations::PrepareStrandsForConnection(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2,
	SBPointer<ADNNucleotide> nt1, SBPointer<ADNNucleotide> nt2)
{
	Connections conn;

	if (nt1->getEndType() == ADNNucleotide::EndType::ThreePrime && nt2->getEndType() == ADNNucleotide::EndType::ThreePrime) return conn;

	// nt1 will be left as 3' while nt2 will be left as 5'
	if (nt1->getEndType() == ADNNucleotide::EndType::FivePrime) {
		if (nt2->getEndType() == ADNNucleotide::EndType::FivePrime) return conn;
		SBPointer<ADNNucleotide> tmp = nt1;
		nt1 = nt2;
		nt2 = tmp;
		SBPointer<ADNPart> tmpPart = part1;
		part1 = part2;
		part2 = tmpPart;
	}

	if (nt1->isEndTypeNucleotide() && nt2->isEndTypeNucleotide()) {
		// we merge in the order they are now
		MergePair pair;
		pair.first = nt1->GetStrand();
		pair.second = nt2->GetStrand();
		pair.firstPart = part1;
		pair.secondPart = part2;
		conn.stringPair = pair;
	}
	else {
		MergePair pair;
		MergePair compPair;

		SBPointer<ADNNucleotide> firstNext = nullptr;
		SBPointer<ADNNucleotide> secondPrev = nullptr;

		// break first nucleotide in 3'
		if (nt1->getEndType() != ADNNucleotide::EndType::ThreePrime) {
			firstNext = nt1->GetNext(true);
			auto p = ADNBasicOperations::BreakSingleStrand(part1, firstNext);
		}

		// break second nucleotide in 5'
		if (nt2->getEndType() != ADNNucleotide::EndType::FivePrime) {
			secondPrev = nt2->GetPrev();
			auto p = ADNBasicOperations::BreakSingleStrand(part2, nt2);
		}

		pair.first = nt1->GetStrand();
		if (firstNext != nullptr) compPair.second = firstNext->GetStrand();
		pair.second = nt2->GetStrand();
		if (secondPrev != nullptr) compPair.first = secondPrev->GetStrand();

		pair.firstPart = part1;
		pair.secondPart = part2;
		compPair.firstPart = part2;
		compPair.secondPart = part1;
		conn.stringPair = pair;
		conn.compStringPair = compPair;
	}

	return conn;
}

void DASOperations::CreateCrossover(SBPointer<ADNPart> part1, SBPointer<ADNPart> part2,
	SBPointer<ADNNucleotide> nt1, SBPointer<ADNNucleotide> nt2, bool two, std::string seq)
{
	const auto conn = PrepareStrandsForConnection(part1, part2, nt1, nt2);
	const auto& pair = conn.stringPair;
	const auto& compPair = conn.compStringPair;

	// create joint strands if necessary
	SBPointer<ADNSingleStrand> joinStrand1 = nullptr;
	SBPointer<ADNSingleStrand> joinStrand2 = nullptr;

	if (pair.first != nullptr && pair.second != nullptr) {
		if (!seq.empty()) {
			const size_t seqLength = seq.size();

			SBPointer<ADNBaseSegment> bs1 = pair.first->GetThreePrime()->GetBaseSegment();
			SBPointer<ADNBaseSegment> bs2 = pair.second->GetFivePrime()->GetBaseSegment();
			SBVector3 direction = (bs2->GetPosition() - bs1->GetPosition()).normalizedVersion();
			SBQuantity::length availLength = (bs2->GetPosition() - bs1->GetPosition()).norm();
			SBQuantity::length expectedLength = SBQuantity::nanometer(ADNConstants::BP_RISE) * (seqLength + 1);  // we need to accommodate space for distance between the ends
			SBQuantity::length offset = (availLength - expectedLength) * 0.5;
			SBPosition3 startPos = bs1->GetPosition() + (SBQuantity::nanometer(ADNConstants::BP_RISE) + offset) * direction;
			if (two) {
				auto res = DASCreator::AddDoubleStrandToADNPart(pair.firstPart, seqLength, startPos, direction);
				joinStrand1 = res.ss1;
				joinStrand2 = res.ss2;

				joinStrand1->SetSequence(seq);
				DASBackToTheAtom* btta = new DASBackToTheAtom();
				btta->SetPositionsForNewNucleotides(pair.firstPart, joinStrand1->GetNucleotides());

				// since we are modifying created parts, we need to call samson creator
				// after generating atoms
				if (SAMSON::isHolding()) SAMSON::hold(res.ds());
				res.ds->create();
				pair.firstPart->DeregisterDoubleStrand(res.ds);
				pair.firstPart->RegisterDoubleStrand(res.ds);

				if (SAMSON::isHolding()) SAMSON::hold(joinStrand1());
				joinStrand1->create();
				pair.firstPart->DeregisterSingleStrand(joinStrand1);
				pair.firstPart->RegisterSingleStrand(joinStrand1);
				if (SAMSON::isHolding()) SAMSON::hold(joinStrand2());
				joinStrand2->create();
				pair.firstPart->DeregisterSingleStrand(joinStrand2);
				pair.firstPart->RegisterSingleStrand(joinStrand2);
			}
			else {
				auto res = DASCreator::AddSingleStrandToADNPart(pair.firstPart, seqLength, startPos, direction);
				joinStrand1 = res.ss1;
				joinStrand1->SetSequence(seq);

				DASBackToTheAtom* btta = new DASBackToTheAtom();
				btta->SetPositionsForNewNucleotides(pair.firstPart, joinStrand1->GetNucleotides());

				if (SAMSON::isHolding()) SAMSON::hold(res.ds());
				res.ds->create();
				pair.firstPart->DeregisterDoubleStrand(res.ds);  // we need to register after creation
				pair.firstPart->RegisterDoubleStrand(res.ds);
				if (SAMSON::isHolding()) SAMSON::hold(joinStrand1());
				joinStrand1->create();
				pair.firstPart->DeregisterSingleStrand(joinStrand1);
				pair.firstPart->RegisterSingleStrand(joinStrand1);
			}
		}
		// connect
		if (pair.first != pair.second) {
			if (joinStrand1 == nullptr) {
				ADNBasicOperations::MergeSingleStrands(pair.firstPart, pair.secondPart, pair.first, pair.second);
			}
			else {
				auto ss = ADNBasicOperations::MergeSingleStrands(pair.firstPart, pair.firstPart, pair.first, joinStrand1);
				ADNBasicOperations::MergeSingleStrands(pair.firstPart, pair.secondPart, ss, pair.second);
			}
		}
		else {
			if (joinStrand1 != nullptr) {
				auto ss = ADNBasicOperations::MergeSingleStrands(pair.firstPart, pair.firstPart, pair.first, joinStrand1);
				MarkCircularAndValidate(ss);
			}
			else {
				MarkCircularAndValidate(pair.first);
			}
		}
	}

	if (two) {
		if (compPair.first != nullptr && compPair.second != nullptr) {
			// connect
			if (compPair.first != compPair.second) {
				if (joinStrand2 == nullptr) {
					ADNBasicOperations::MergeSingleStrands(compPair.firstPart, compPair.secondPart, compPair.first, compPair.second);
				}
				else {
					auto ss = ADNBasicOperations::MergeSingleStrands(compPair.firstPart, pair.firstPart, compPair.first, joinStrand2);
					ADNBasicOperations::MergeSingleStrands(compPair.firstPart, compPair.secondPart, ss, compPair.second);
				}
			}
			else {
				if (joinStrand2 != nullptr) {
					auto ss = ADNBasicOperations::MergeSingleStrands(compPair.firstPart, pair.firstPart, compPair.first, joinStrand2);
					MarkCircularAndValidate(ss);
				}
				else {
					MarkCircularAndValidate(compPair.first);
				}
			}
		}
	}
}

void DASOperations::AddComplementaryStrands(ADNNanorobot* nanorobot, SBPointerIndexer<ADNNucleotide> selectedNucleotides)
{
	DASBackToTheAtom btta = DASBackToTheAtom();

	SBPointer<ADNPart> prevPart = nullptr;
	SBPointer<ADNSingleStrand> ss = nullptr;
	int i = 1;
	SBPointerIndexer<ADNNucleotide> nucleotides;
	bool createSs = true;

	SB_FOR(SBPointer<ADNNucleotide> nt, selectedNucleotides) {
		auto bs = nt->GetBaseSegment();
		SBPointer<ADNPart> part = bs->GetDoubleStrand()->GetPart();
		auto next = nt->GetNext(true);

		if (nt->GetPair() == nullptr) {
			if (prevPart != part) {
				if (ss != nullptr) {
					btta.SetPositionsForNewNucleotides(prevPart, nucleotides);
					if (SAMSON::isHolding()) SAMSON::hold(ss());
					ss->create();
					prevPart->DeregisterSingleStrand(ss);
					prevPart->RegisterSingleStrand(ss);
					nucleotides.clear();
				}
				createSs = true;
			}

			if (createSs) {
				// create new strand if we change part
				ss = new ADNSingleStrand();
				ss->setName("Paired Strand " + std::to_string(i));
				ss->setStructuralID(i);
				++i;
				part->RegisterSingleStrand(ss);
				createSs = false;
			}

			SBPointer<ADNNucleotide> pair = new ADNNucleotide();
			pair->Init();
			pair->setNucleotideType(ADNModel::GetComplementaryBase(nt->getNucleotideType()));
			nucleotides.addReferenceTarget(pair());

			auto cellType = bs->GetCellType();
			if (cellType == CellType::BasePair) {
				SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(bs->GetCell()());
				if (bp->IsLeft(nt)) {
					bp->SetRightNucleotide(pair);
				}
				else {
					bp->SetLeftNucleotide(pair);
				}
				bp->PairNucleotides();
			}
			pair->SetBaseSegment(bs);

			part->RegisterNucleotideFivePrime(ss, pair);

			if (!selectedNucleotides.hasIndex(next())) {
				if (ss != nullptr) {
					btta.SetPositionsForNewNucleotides(part, nucleotides);
					if (SAMSON::isHolding()) SAMSON::hold(ss());
					ss->create();
					part->DeregisterSingleStrand(ss);
					part->RegisterSingleStrand(ss);
					nucleotides.clear();
				}
				createSs = true;
			}
		}
		else {
			// if ss has nucleotides create and start a new one
			if (ss != nullptr) {
				btta.SetPositionsForNewNucleotides(part, nucleotides);
				if (SAMSON::isHolding()) SAMSON::hold(ss());
				ss->create();
				part->DeregisterSingleStrand(ss);
				part->RegisterSingleStrand(ss);
				nucleotides.clear();
			}
			createSs = true;
		}

		prevPart = part;
	}
}

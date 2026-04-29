#include "PICrossovers.hpp"

#include "ADNNeighbors.hpp"
#include "ADNPart.hpp"

std::vector<XOPair> PICrossovers::GetCrossovers(SBPointer<ADNPart> part) {

    auto nucleotides = part->GetNucleotides();

    std::vector<XOPair> xos;

    SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) {

        SBPointer<ADNBaseSegment> bs = nt->GetBaseSegment();
        if (!bs.isValid()) continue;
        SBPointer<ADNDoubleStrand> ds1 = bs->GetDoubleStrand();
        if (!ds1.isValid()) continue;

        SBPointer<ADNNucleotide> next = nt->GetNext();
        if (next != nullptr) {

            SBPointer<ADNDoubleStrand> ds2 = next->GetBaseSegment()->GetDoubleStrand();
            if (ds1 != ds2) {

                XOPair xo = std::make_pair(nt, next);
                xos.push_back(xo);

                /*nt->setHighlightingFlag(true);
                next->setHighlightingFlag(true);*/

            }

        }

    }

    return xos;

}

std::vector<XOPair> PICrossovers::GetPossibleCrossovers(SBPointer<ADNPart> part, SBPointer<ADNNucleotide> nt, ADNNeighbors* neigh) {

    // highlight neighbors of selected nucleotide
    auto ntNeighbors = neigh->GetNeighbors(nt);
    SB_FOR(SBPointer<ADNNucleotide> ntN, ntNeighbors) {
        ntN->setSelectionFlag(true);
    }

    return std::vector<XOPair>();

}

std::vector<XOPair> PICrossovers::GetPossibleCrossovers(SBPointer<ADNPart> part, ADNNeighbors* neigh) {

    if (neigh == nullptr) {

        // create neighbors
        neigh = new ADNNeighbors();
        SEConfig& c = SEConfig::GetInstance();
        neigh->SetMaxCutOff(SBQuantity::nanometer(c.debugOptions.maxCutOff));
        neigh->SetMinCutOff(SBQuantity::nanometer(c.debugOptions.minCutOff));
        neigh->SetIncludePairs(false);
        neigh->SetFromOwnSingleStrand(true);
        neigh->InitializeNeighbors(part);

    }

    std::vector<XOPair> xos;

    // highlight neighbors of selected nucleotide
    auto nts = part->GetNucleotides();
    SB_FOR(SBPointer<ADNNucleotide> nt, nts) {

        auto ntXO = GetPossibleCrossovers(part, nt, neigh);
        xos.insert(xos.end(), ntXO.begin(), ntXO.end());

    }

    return std::vector<XOPair>();

}

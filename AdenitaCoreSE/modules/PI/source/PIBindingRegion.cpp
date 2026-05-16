#include "PIBindingRegion.hpp"

#include "ADNPart.hpp"
#include "ADNSamsonContext.hpp"

#include <unordered_set>

PIBindingRegion& PIBindingRegion::operator=(const PIBindingRegion& other) {

    SBNodeGroup::operator =(other);
    return *this;

}

double PIBindingRegion::getEntropy() const {
    return thermodynamicParameters_.dS_;
}

double PIBindingRegion::getEnthalpy() const {
    return thermodynamicParameters_.dH_;
}

double PIBindingRegion::getGibbs() const {
    return thermodynamicParameters_.dG_;
}

double PIBindingRegion::getTemp() const {
    return thermodynamicParameters_.T_;
}

std::string PIBindingRegion::getStatusString() const {

    if (thermodynamicParameters_.isValid) return "OK";
    else return "Could not compute for this binding site.";

}

void PIBindingRegion::SetLastNt(SBPointer<ADNNucleotide> nt) {
    lastNt_ = nt;
}

void PIBindingRegion::SetFirstNt(SBPointer<ADNNucleotide> nt) {
    firstNt_ = nt;
}

SBPointer<ADNPart> PIBindingRegion::GetPart() const {
    return part_;
}

void PIBindingRegion::SetPart(SBPointer<ADNPart> part) {
    part_ = part;
}

void PIBindingRegion::SetThermodynamicParameters(ThermodynamicParameters res) {
    thermodynamicParameters_ = res;
}

void PIBindingRegion::RegisterBindingRegion(SBFolder* folder) {

    SBDocument* document = nullptr;
    if (folder == nullptr) {
        document = ADNSamsonContext::GetActiveDocument(__func__);
        if (document == nullptr) return;
    }

    if (SAMSON::isHolding()) SAMSON::hold(this);
    create();
    if (folder) folder->addChild(this);
    else document->addChild(this);

}

void PIBindingRegion::UnregisterBindingRegion() {

    auto parent = this->getParent();
    if (parent != nullptr)
        parent->removeChild(this);

}

std::pair<std::string, std::string> PIBindingRegion::GetSequences() const {

    std::string leftSeq;
    std::string rightSeq;

    if (firstNt_ == nullptr || lastNt_ == nullptr)
        return std::make_pair(leftSeq, rightSeq);

    std::unordered_set<const ADNNucleotide*> visitedNucleotides;

    auto ntLeft = lastNt_;
    while (ntLeft != nullptr) {

        if (!visitedNucleotides.insert(ntLeft()).second)
            return std::make_pair(std::string(), std::string());

        std::string left = ntLeft->getOneLetterNucleotideTypeString();
        leftSeq.insert(0, left);
        auto pair = ntLeft->GetPair();

        if (pair != nullptr) {

            std::string right = pair->getOneLetterNucleotideTypeString();
            rightSeq.append(right);

        }

        if (ntLeft == firstNt_)
            return std::make_pair(leftSeq, rightSeq);

        ntLeft = ntLeft->GetPrev();

    }

    return std::make_pair(std::string(), std::string());

}

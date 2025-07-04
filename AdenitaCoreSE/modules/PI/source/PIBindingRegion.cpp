#include "PIBindingRegion.hpp"

#include "ADNPart.hpp"


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

void PIBindingRegion::SetLastNt(ADNPointer<ADNNucleotide> nt) {
    lastNt_ = nt;
}

void PIBindingRegion::SetFirstNt(ADNPointer<ADNNucleotide> nt) {
    firstNt_ = nt;
}

ADNPointer<ADNPart> PIBindingRegion::GetPart() const {
    return part_;
}

void PIBindingRegion::SetPart(ADNPointer<ADNPart> part) {
    part_ = part;
}

void PIBindingRegion::SetThermodynamicParameters(ThermodynamicParameters res) {
    thermodynamicParameters_ = res;
}

void PIBindingRegion::RegisterBindingRegion(SBFolder* folder) {

    if (SAMSON::isHolding()) SAMSON::hold(this);
    create();
    if (folder) folder->addChild(this);
    else SAMSON::getActiveDocument()->addChild(this);

}

void PIBindingRegion::UnregisterBindingRegion() {

    auto parent = this->getParent();
    parent->removeChild(this);

}

std::pair<std::string, std::string> PIBindingRegion::GetSequences() const {

    std::string leftSeq;
    std::string rightSeq;

    auto ntLeft = lastNt_;
    while (ntLeft != firstNt_) {

        std::string left = ntLeft->getOneLetterNucleotideTypeString();
        leftSeq.insert(0, left);
        auto pair = ntLeft->GetPair();

        if (pair != nullptr) {

            std::string right = pair->getOneLetterNucleotideTypeString();
            rightSeq.append(right);

        }

        ntLeft = ntLeft->GetPrev();

    }

    return std::make_pair(leftSeq, rightSeq);

}

#include "ADNModel.hpp"


DNABlocks ADNModel::GetComplementaryBase(DNABlocks base) {

    DNABlocks comp_base = SBResidue::ResidueType::DI;
    if (base == SBResidue::ResidueType::DA) {
        comp_base = SBResidue::ResidueType::DT;
    }
    else if (base == SBResidue::ResidueType::DT) {
        comp_base = SBResidue::ResidueType::DA;
    }
    else if (base == SBResidue::ResidueType::DG) {
        comp_base = SBResidue::ResidueType::DC;
    }
    else if (base == SBResidue::ResidueType::DC) {
        comp_base = SBResidue::ResidueType::DG;
    }

    return comp_base;

}

char ADNModel::GetResidueName(DNABlocks t) {

    std::string name = SBResidue::getResidueTypeString(t);
    char n = name[0];
    if (name.size() > 0) n = name[1];

    if (n == 'I') n = 'N';

    return n;

}

DNABlocks ADNModel::ResidueNameToType(char n) {

    DNABlocks t = SBResidue::ResidueType::DI;
    if (n == 'A') {
        t = SBResidue::ResidueType::DA;
    }
    else if (n == 'T') {
        t = SBResidue::ResidueType::DT;
    }
    else if (n == 'C') {
        t = SBResidue::ResidueType::DC;
    }
    else if (n == 'G') {
        t = SBResidue::ResidueType::DG;
    }

    return t;

}

bool ADNModel::IsAtomInBackboneByName(std::string name) {

    bool res = false;
    if (std::find(backbone_names_.begin(), backbone_names_.end(), name) != backbone_names_.end()) {
        res = true;
    }
    return res;

}

SBElement::Type ADNModel::GetElementType(std::string atomName) {

    std::map<std::string, SBElement::Type> atomType = {
      { "P", SBElement::Phosphorus },
      { "OP1", SBElement::Oxygen },
      { "O1P", SBElement::Oxygen },
      { "OP2", SBElement::Oxygen },
      { "O2P", SBElement::Oxygen },
      { "O5'", SBElement::Oxygen },
      { "O4'", SBElement::Oxygen },
      { "O3'", SBElement::Oxygen },
      { "O6", SBElement::Oxygen },
      { "O4", SBElement::Oxygen },
      { "O2", SBElement::Oxygen },
      { "C5'", SBElement::Carbon },
      { "C4'", SBElement::Carbon },
      { "C3'", SBElement::Carbon },
      { "C2'", SBElement::Carbon },
      { "C1'", SBElement::Carbon },
      { "C8", SBElement::Carbon },
      { "C7", SBElement::Carbon },
      { "C6", SBElement::Carbon },
      { "C5", SBElement::Carbon },
      { "C4", SBElement::Carbon },
      { "C2", SBElement::Carbon },
      { "N9", SBElement::Nitrogen },
      { "N7", SBElement::Nitrogen },
      { "N6", SBElement::Nitrogen },
      { "N4", SBElement::Nitrogen },
      { "N3", SBElement::Nitrogen },
      { "N2", SBElement::Nitrogen },
      { "N1", SBElement::Nitrogen }
    };

    SBElement::Type t = SBElement::Type::Unknown;
    if (atomType.find(atomName) != atomType.end()) {
        t = atomType.at(atomName);
    }

    return t;

}

std::map<std::string, std::vector<std::string>> ADNModel::GetNucleotideBonds(DNABlocks t) {

    std::map<std::string, std::vector<std::string>> res;
    // P
    std::vector<std::string> pConn = { "OP2", "OP1", "O5'" };
    res.insert(std::make_pair("P", pConn));
    // OP2
    std::vector<std::string> op2Conn = { "P" };
    res.insert(std::make_pair("OP2", op2Conn));
    // OP1
    std::vector<std::string> op1Conn = { "P" };
    res.insert(std::make_pair("OP1", op1Conn));
    // O5'
    std::vector<std::string> o5pConn = { "P",  "C5'" };
    res.insert(std::make_pair("O5'", o5pConn));
    // C5'
    std::vector<std::string> c5pConn = { "O5'",  "C4'" };
    res.insert(std::make_pair("C5'", c5pConn));
    // C4'
    std::vector<std::string> c4pConn = { "C5'",  "O4'", "C3'" };
    res.insert(std::make_pair("C4'", c4pConn));
    // C3'
    std::vector<std::string> c3pConn = { "C4'",  "O3'", "C2'" };
    res.insert(std::make_pair("C3'", c3pConn));
    // O3'
    std::vector<std::string> o3pConn = { "C3'" };
    res.insert(std::make_pair("O3'", o3pConn));
    // C2'
    std::vector<std::string> c2pConn = { "C3'",  "C1'" };
    res.insert(std::make_pair("C2'", c2pConn));
    // C1'
    std::vector<std::string> c1pConn = { "C2'",  "O4'" };
    res.insert(std::make_pair("C1'", c1pConn));
    // O4'
    std::vector<std::string> o4pConn = { "C4'",  "C1'" };
    res.insert(std::make_pair("O4'", o4pConn));

    if (t == DNABlocks::DA || t == DNABlocks::DI) {

        std::vector<std::string> c1pConn = { "C2'",  "O4'", "N9" };
        res["C1'"] = c1pConn;
        std::vector<std::string> c8Conn = { "N9",  "N7" };
        res.insert(std::make_pair("C8", c8Conn));
        std::vector<std::string> n7Conn = { "C8",  "C5" };
        res.insert(std::make_pair("N7", n7Conn));
        std::vector<std::string> c5Conn = { "C4",  "N7", "C6" };
        res.insert(std::make_pair("C5", c5Conn));
        std::vector<std::string> c4Conn = { "C5",  "N9", "N3" };
        res.insert(std::make_pair("C4", c4Conn));
        std::vector<std::string> n3Conn = { "C4",  "C2" };
        res.insert(std::make_pair("N3", n3Conn));
        std::vector<std::string> c2Conn = { "N3",  "N1" };
        res.insert(std::make_pair("C2", c2Conn));
        std::vector<std::string> n1Conn = { "C2",  "C6" };
        res.insert(std::make_pair("N1", n1Conn));
        std::vector<std::string> c6Conn = { "N1",  "C5", "N6" };
        res.insert(std::make_pair("C6", c6Conn));

    }
    else if (t == DNABlocks::DG) {

        std::vector<std::string> c1pConn = { "C2'",  "O4'", "N9" };
        res["C1'"] = c1pConn;
        std::vector<std::string> c8Conn = { "N9",  "N7" };
        res.insert(std::make_pair("C8", c8Conn));
        std::vector<std::string> n7Conn = { "C8",  "C5" };
        res.insert(std::make_pair("N7", n7Conn));
        std::vector<std::string> c5Conn = { "C4",  "N7", "C6" };
        res.insert(std::make_pair("C5", c5Conn));
        std::vector<std::string> c4Conn = { "C5",  "N9", "N3" };
        res.insert(std::make_pair("C4", c4Conn));
        std::vector<std::string> n3Conn = { "C4",  "C2" };
        res.insert(std::make_pair("N3", n3Conn));
        std::vector<std::string> c2Conn = { "N3",  "N2", "N1" };
        res.insert(std::make_pair("C2", c2Conn));
        std::vector<std::string> n1Conn = { "C2",  "C6" };
        res.insert(std::make_pair("N1", n1Conn));
        std::vector<std::string> c6Conn = { "N1",  "C5", "O6" };
        res.insert(std::make_pair("C6", c6Conn));

    }
    else if (t == DNABlocks::DT) {

        std::vector<std::string> c1pConn = { "C2'",  "O4'", "N1" };
        res["C1'"] = c1pConn;
        std::vector<std::string> c5Conn = { "C4",  "C6", "C7" };
        res.insert(std::make_pair("C5", c5Conn));
        std::vector<std::string> c4Conn = { "O4", "N3" };
        res.insert(std::make_pair("C4", c4Conn));
        std::vector<std::string> n3Conn = { "C2" };
        res.insert(std::make_pair("N3", n3Conn));
        std::vector<std::string> c2Conn = { "N1", "O2" };
        res.insert(std::make_pair("C2", c2Conn));
        std::vector<std::string> n1Conn = { "C6" };
        res.insert(std::make_pair("N1", n1Conn));

    }
    else if (t == DNABlocks::DC) {

        std::vector<std::string> c1pConn = { "C2'",  "O4'", "N1" };
        res["C1'"] = c1pConn;
        std::vector<std::string> c5Conn = { "C4",  "C6" };
        res.insert(std::make_pair("C5", c5Conn));
        std::vector<std::string> c4Conn = { "C5",  "N4", "N3" };
        res.insert(std::make_pair("C4", c4Conn));
        std::vector<std::string> n3Conn = { "C4",  "C2" };
        res.insert(std::make_pair("N3", n3Conn));
        std::vector<std::string> c2Conn = { "N3",  "N1", "O2" };
        res.insert(std::make_pair("C2", c2Conn));
        std::vector<std::string> n1Conn = { "C2",  "C6" };
        res.insert(std::make_pair("N1", n1Conn));
        std::vector<std::string> c6Conn = { "N1",  "C5" };
        res.insert(std::make_pair("C6", c6Conn));

    }

    return res;

}

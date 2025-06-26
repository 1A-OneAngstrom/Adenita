#include "PIPrimer3.hpp"
#include "ADNPart.hpp"

#include "SAMSON.hpp"

#include <cfloat>

#include <QProcess>

CollectionMap<PIBindingRegion> PIPrimer3::GetBindingRegions() const {

    CollectionMap<PIBindingRegion> regions;

    for (auto it = regionsMap_.begin(); it != regionsMap_.end(); ++it) {

        CollectionMap<PIBindingRegion> regs = it->second;
        SB_FOR(ADNPointer<PIBindingRegion> r, regs) {

            if (r != nullptr)
                regions.addReferenceTarget(r());

        }

    }

    return regions;

}

PIPrimer3& PIPrimer3::GetInstance() {

    static PIPrimer3 instance;
    return instance;

}

CollectionMap<PIBindingRegion> PIPrimer3::GetBindingRegions(ADNPointer<ADNPart> p) const {

    CollectionMap<PIBindingRegion> regions;
    if (regionsMap_.find(p()) != regionsMap_.end()) {

        regions = regionsMap_.at(p());

    }

    return regions;

}

void PIPrimer3::DeleteBindingRegions(ADNPointer<ADNPart> p) {

    auto regions = GetBindingRegions(p);
    SB_FOR(ADNPointer<PIBindingRegion> r, regions) {
        
        if (r != nullptr)
            r->UnregisterBindingRegion();

    }

}

ThermodynamicParameters PIPrimer3::ExecuteNtthal(std::string leftSequence, std::string rightSequence, int oligo_conc, int mv, int dv) {

    ThermodynamicParameters res;
    res.isValid = false;
    res.dS_ = FLT_MAX;
    res.dH_ = FLT_MAX;
    res.dG_ = FLT_MAX;
    res.T_ = FLT_MAX;

    const SEConfig& c = SEConfig::GetInstance();

    QFileInfo ntthalFileInfo(QString::fromStdString(c.ntthal));
    if (!ntthalFileInfo.exists()) return res;

    const QString workingDirectory = ntthalFileInfo.absolutePath();
    const QString program = ntthalFileInfo.absoluteFilePath();

    QStringList arguments;
    arguments << "-s1" << leftSequence.c_str();
    arguments << "-s2" << rightSequence.c_str();
    arguments << "-mv" << QString::number(mv);
    arguments << "-dv" << QString::number(dv);
    arguments << "-dna_conc" << QString::number(oligo_conc);

    QProcess* myProcess = new QProcess();
    myProcess->setWorkingDirectory(workingDirectory);
    myProcess->start(program, arguments);
    myProcess->waitForFinished();

    QByteArray standardOutput = myProcess->readAllStandardOutput();

    //qDebug() << "workingDirectory: " << workingDirectory;
    //qDebug() << "program:          " << program;
    //qDebug() << "arguments:        " << arguments;
    //qDebug() << "standardOutput:   " << standardOutput;
    //qDebug() << "standardError:    " << myProcess->readAllStandardError();

    // output example:
    // Calculated thermodynamical parameters for dimer:\tdS = -68.9269\tdH = -24400\tdG = -3022.33\tt = -37.8822\r\nSEQ\t    \r\nSEQ\tTCGG\r\nSTR\tAGCC\r\nSTR\t    \r\n
    const QString stdOutputStr = QString(standardOutput);
    if (stdOutputStr.size() == 0) return res;

    QStringList strLines = stdOutputStr.split("\n");

    // if the region is unbound
    if (strLines.size() < 5) return res;

    const QString firstLine = strLines[0];

    const QString dS = "dS =";
    const QString dH = "dH =";
    const QString dG = "dG =";
    const QString t = "t =";
    if (!firstLine.contains(dS) || !firstLine.contains(dH) || !firstLine.contains(dG) || !firstLine.contains(t)) return res;

    const int idS = firstLine.indexOf(dS);
    const int idSEnd = firstLine.indexOf(dH, idS);
    const int idH = firstLine.indexOf(dH);
    const int idHEnd = firstLine.indexOf(dG, idH);
    const int idG = firstLine.indexOf(dG);
    const int idGEnd = firstLine.indexOf(t, idG);
    const int it = firstLine.indexOf(t);

    const QString dSVal = firstLine.mid(idS + 5, idSEnd - idS - 6);
    const QString dHVal = firstLine.mid(idH + 5, idHEnd - idH - 5);
    const QString dGVal = firstLine.mid(idG + 5, idGEnd - idG - 6);
    const QString tVal = firstLine.mid(it + 4, firstLine.size() - it);

    res.dS_ = dSVal.toDouble();
    res.dH_ = dHVal.toDouble();
    res.dG_ = dGVal.toDouble();
    res.T_ = tVal.toDouble();
    res.isValid = true;

    return res;

}

void PIPrimer3::Calculate(ADNPointer<ADNPart> p, int oligo_conc, int mv, int dv) const {

    auto regions = GetBindingRegions(p);

    SB_FOR(ADNPointer<PIBindingRegion> r, regions) if (r != nullptr) {

        auto seqs = r->GetSequences();
        ThermodynamicParameters res = ExecuteNtthal(seqs.first, seqs.second, oligo_conc, mv, dv);
        r->SetThermodynamicParameters(res);

    }

}

void PIPrimer3::UpdateBindingRegions(ADNPointer<ADNPart> p) {

    if (regionsMap_.find(p()) != regionsMap_.end()) {

        regionsMap_[p()].clear();

    }
    else {

        regionsMap_.insert(std::make_pair(p(), CollectionMap<PIBindingRegion>()));

    }

    auto singleStrands = p->GetSingleStrands();

    std::vector<ADNPointer<ADNNucleotide>> added_nt;
    ADNPointer<ADNNucleotide> firstNt;
    unsigned int numRegions = 0;

    SAMSON::beginHolding("Update binding regions");

    SBFolder* bindingRegionsFolder = new SBFolder("Binding regions");
    SAMSON::hold(bindingRegionsFolder);
    bindingRegionsFolder->create();
    SAMSON::getActiveDocument()->addChild(bindingRegionsFolder);

    SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) if (ss != nullptr) {

        ADNPointer<ADNNucleotide> nt = ss->GetFivePrime();

        int regionSize = 0;

        SBNodeIndexer nodeIndexer;
        while (nt != nullptr) {

            if (std::find(added_nt.begin(), added_nt.end(), nt) == added_nt.end()) {

                bool endOfRegion = true;

                auto st_cur = nt->GetPair();
                auto sc_next = nt->GetNext();

                if (sc_next != nullptr && st_cur != nullptr && st_cur->GetPrev() != nullptr) {

                    if (sc_next->GetPair() == st_cur->GetPrev())
                        endOfRegion = false;

                }
                else if (sc_next != nullptr && st_cur == nullptr) {

                    // group up in one binding region the contiguous unpaired nts
                    if (sc_next->GetPair() == nullptr)
                        endOfRegion = false;

                }

                nodeIndexer.addNode(nt());
                added_nt.push_back(nt);
                if (regionSize == 0) firstNt = nt;

                auto pair = nt->GetPair();
                if (pair != nullptr) {

                    nodeIndexer.addNode(pair());
                    added_nt.push_back(pair);

                }

                ++regionSize;

                if (endOfRegion) {

                    regionSize = 0;
                    const std::string name = "Binding region " + std::to_string(numRegions);
                    ADNPointer<PIBindingRegion> region = new PIBindingRegion(name, nodeIndexer);
                    region->SetPart(p);
                    region->RegisterBindingRegion(bindingRegionsFolder);
                    regionsMap_[p()].addReferenceTarget(region());
                    region->SetLastNt(nt);
                    region->SetFirstNt(firstNt);
                    ++numRegions;
                    nodeIndexer.clear();

                }

            }

            nt = nt->GetNext();

        }

    }

    SAMSON::endHolding();

}

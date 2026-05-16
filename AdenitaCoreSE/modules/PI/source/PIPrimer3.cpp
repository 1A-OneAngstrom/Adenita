#include "PIPrimer3.hpp"
#include "ADNPart.hpp"
#include "ADNSamsonContext.hpp"

#include "SAMSON.hpp"

#include <cerrno>
#include <cctype>
#include <cfloat>
#include <cstdlib>
#include <sstream>

#include <QProcess>

namespace {

ThermodynamicParameters InvalidThermodynamicParameters() {

    ThermodynamicParameters res;
    res.isValid = false;
    res.dS_ = FLT_MAX;
    res.dH_ = FLT_MAX;
    res.dG_ = FLT_MAX;
    res.T_ = FLT_MAX;
    return res;

}

bool ParseLabeledDouble(const std::string& line, const std::string& label, double& value) {

    const size_t labelPosition = line.find(label);
    if (labelPosition == std::string::npos) return false;

    const char* start = line.c_str() + labelPosition + label.size();
    while (*start == ' ' || *start == '\t') ++start;

    errno = 0;
    char* end = nullptr;
    value = std::strtod(start, &end);
    if (start == end || errno == ERANGE) return false;
    if (*end != '\0' && !std::isspace(static_cast<unsigned char>(*end))) return false;

    return true;

}

}

SBPointerIndexer<PIBindingRegion> PIPrimer3::GetBindingRegions() const {

    SBPointerIndexer<PIBindingRegion> regions;

    for (auto it = regionsMap_.begin(); it != regionsMap_.end(); ++it) {

        SBPointerIndexer<PIBindingRegion> regs = it->second;
        SB_FOR(SBPointer<PIBindingRegion> r, regs) {

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

SBPointerIndexer<PIBindingRegion> PIPrimer3::GetBindingRegions(SBPointer<ADNPart> p) const {

    SBPointerIndexer<PIBindingRegion> regions;
    if (regionsMap_.find(p()) != regionsMap_.end()) {

        regions = regionsMap_.at(p());

    }

    return regions;

}

void PIPrimer3::DeleteBindingRegions(SBPointer<ADNPart> p) {

    auto regions = GetBindingRegions(p);
    SB_FOR(SBPointer<PIBindingRegion> r, regions) {
        
        if (r != nullptr)
            r->UnregisterBindingRegion();

    }

}

ThermodynamicParameters PIPrimer3::ExecuteNtthal(std::string leftSequence, std::string rightSequence, int oligo_conc, int mv, int dv) {

    ThermodynamicParameters res = InvalidThermodynamicParameters();

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

    constexpr int startTimeoutMs = 5000;
    constexpr int finishTimeoutMs = 30000;
    constexpr int killTimeoutMs = 3000;

    QProcess process;
    process.setWorkingDirectory(workingDirectory);
    process.start(program, arguments);

    if (!process.waitForStarted(startTimeoutMs)) return res;

    if (!process.waitForFinished(finishTimeoutMs)) {

        process.kill();
        process.waitForFinished(killTimeoutMs);
        return res;

    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) return res;

    QByteArray standardOutput = process.readAllStandardOutput();

    //qDebug() << "workingDirectory: " << workingDirectory;
    //qDebug() << "program:          " << program;
    //qDebug() << "arguments:        " << arguments;
    //qDebug() << "standardOutput:   " << standardOutput;
    //qDebug() << "standardError:    " << process.readAllStandardError();

    return ParseNtthalOutput(standardOutput.toStdString());

}

ThermodynamicParameters PIPrimer3::ParseNtthalOutput(const std::string& output) {

    ThermodynamicParameters res = InvalidThermodynamicParameters();
    if (output.empty()) return res;

    std::istringstream stream(output);
    std::string firstLine;
    if (!std::getline(stream, firstLine)) return res;

    unsigned int lineCount = 1;
    std::string line;
    while (std::getline(stream, line))
        ++lineCount;

    // ntthal reports fewer lines when the region is unbound.
    if (lineCount < 5) return res;

    if (!ParseLabeledDouble(firstLine, "dS =", res.dS_)) return res;
    if (!ParseLabeledDouble(firstLine, "dH =", res.dH_)) return res;
    if (!ParseLabeledDouble(firstLine, "dG =", res.dG_)) return res;
    if (!ParseLabeledDouble(firstLine, "t =", res.T_)) return res;

    res.isValid = true;
    return res;

}

void PIPrimer3::Calculate(SBPointer<ADNPart> p, int oligo_conc, int mv, int dv) const {

    auto regions = GetBindingRegions(p);

    SB_FOR(SBPointer<PIBindingRegion> r, regions) if (r != nullptr) {

        auto seqs = r->GetSequences();
        ThermodynamicParameters res = ExecuteNtthal(seqs.first, seqs.second, oligo_conc, mv, dv);
        r->SetThermodynamicParameters(res);

    }

}

void PIPrimer3::UpdateBindingRegions(SBPointer<ADNPart> p) {

    if (regionsMap_.find(p()) != regionsMap_.end()) {

        regionsMap_[p()].clear();

    }
    else {

        regionsMap_.insert(std::make_pair(p(), SBPointerIndexer<PIBindingRegion>()));

    }

    auto singleStrands = p->GetSingleStrands();

    std::vector<SBPointer<ADNNucleotide>> added_nt;
    SBPointer<ADNNucleotide> firstNt;
    unsigned int numRegions = 0;

    SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
    if (document == nullptr) return;

    SAMSON::beginHolding("Update binding regions");

    SBFolder* bindingRegionsFolder = new SBFolder("Binding regions");
    SAMSON::hold(bindingRegionsFolder);
    bindingRegionsFolder->create();
    document->addChild(bindingRegionsFolder);

    SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) if (ss != nullptr) {

        SBPointer<ADNNucleotide> nt = ss->GetFivePrime();

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
                    SBPointer<PIBindingRegion> region = new PIBindingRegion(name, nodeIndexer);
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

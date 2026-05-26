#include "SEAdenitaCoreSEApp.hpp"
#include "SEAdenitaCoreSEAppGUI.hpp"
#include "SEAdenitaVisualModel.hpp"
#include "SEAdenitaVisualModelProperties.hpp"

#include "ADNScaffoldReader.hpp"
#include "PICrossovers.hpp"
#include "DASAlgorithms.hpp"
#include "ADNSamsonContext.hpp"
#include "ADNAtom.hpp"
#include "ADNGeometrySynchronization.hpp"

#include <exception>

#include <QCoreApplication>
#include <QTimer>

SEAdenitaCoreSEApp* SEAdenitaCoreSEApp::adenitaApp = nullptr;

namespace {

bool g_pendingSAMLoadVisualModelReset = false;

[[nodiscard]] double degreesToRadians(double degrees) {

	constexpr double pi = 3.141592653589793238462643383279502884;
	return degrees * pi / 180.0;

}

class ScopedGeometrySyncGuard {

public:

	explicit ScopedGeometrySyncGuard(bool& flag) : flag_(flag), oldValue_(flag) {

		flag_ = true;

	}

	~ScopedGeometrySyncGuard() {

		flag_ = oldValue_;

	}

private:

	bool& flag_;
	bool oldValue_;

};

void syncPartsBeforeGeometryEdit(const SBPointerIndexer<ADNPart>& parts) {

	SB_FOR(SBPointer<ADNPart> part, parts) {

		if (part != nullptr)
			ADNGeometrySynchronization::syncPartFramesBeforeGeometryEdit(*part);

	}

}

void syncPartsAfterGeometryEdit(const SBPointerIndexer<ADNPart>& parts) {

	SB_FOR(SBPointer<ADNPart> part, parts) {

		if (part != nullptr)
			ADNGeometrySynchronization::syncPartFramesAfterGeometryEdit(*part);

	}

}

} // namespace

SEAdenitaCoreSEApp::SEAdenitaCoreSEApp() {

	setGUI(new SEAdenitaCoreSEAppGUI(this));
	getGUI()->loadDefaultSettings();

	SEConfig& config = SEConfig::GetInstance();

	SB_INFORMATION("Adenita started");

	adenitaApp = this;

}

SEAdenitaCoreSEApp::~SEAdenitaCoreSEApp() {

	getGUI()->saveDefaultSettings();
	delete getGUI();
	ClearNanorobots();

	if (adenitaApp == this) adenitaApp = nullptr;

}

SEAdenitaCoreSEAppGUI* SEAdenitaCoreSEApp::getGUI() const { return static_cast<SEAdenitaCoreSEAppGUI*>(SBDApp::getGUI()); }

SEAdenitaCoreSEApp* SEAdenitaCoreSEApp::getAdenitaApp() {

	//if (!adenitaApp) {
	//
	//	SBDApp* app = SAMSON::getApp(SBUUID("85DB7CE6-AE36-0CF1-7195-4A5DF69B1528"), SBUUID(SB_ELEMENT_UUID));
	//	SEAdenitaCoreSEApp::adenitaApp = static_cast<SEAdenitaCoreSEApp*>(app);
	//
	//}

	if (!SEAdenitaCoreSEApp::adenitaApp) {

		// Adenita app is not initialized. Initializing...
		SBAction* appAction = SAMSON::getAction(SBUUID("386506A7-DD8B-69DD-4599-F136C1B91610"));
		if (appAction) {

			appAction->trigger();

			if (!SEAdenitaCoreSEApp::adenitaApp) {

				SBDApp* app = SAMSON::getApp(SBUUID("85DB7CE6-AE36-0CF1-7195-4A5DF69B1528"), SBUUID(SB_ELEMENT_UUID));
				if (app) SEAdenitaCoreSEApp::adenitaApp = static_cast<SEAdenitaCoreSEApp*>(app);

			}

			if (SEAdenitaCoreSEApp::adenitaApp) if (SEAdenitaCoreSEApp::adenitaApp->getGUI()) SEAdenitaCoreSEApp::adenitaApp->getGUI()->hide();

		}

	}

	if (!SEAdenitaCoreSEApp::adenitaApp) {

		SAMSON::informUser("Adenita Error", "Adenita is not initialized. Please start Adenita first by opening the Adenita app from the App menu.\n"
			"Please report this issue to the developers.");
		return nullptr;

	}

	return SEAdenitaCoreSEApp::adenitaApp;

}

bool SEAdenitaCoreSEApp::loadPart(const QString& filename, SBDDocumentFolder* preferredFolder) {

	SAMSON::setStatusMessage(QString("Loading component from ") + filename);

	SBPointer<ADNPart> part = ADNLoader::LoadPartFromJson(filename.toStdString());
	if (part == nullptr) return false;
	addPartToDocument(part, false, preferredFolder);

	return true;

}

void SEAdenitaCoreSEApp::loadParts(const QString& filename, SBDDocumentFolder* preferredFolder) {

	SAMSON::setStatusMessage(QString("Loading components from ") + filename);

	std::vector<SBPointer<ADNPart>> parts = ADNLoader::LoadPartsFromJson(filename.toStdString());
	for (SBPointer<ADNPart> part : parts) if (part != nullptr) addPartToDocument(part, false, preferredFolder);

}

void SEAdenitaCoreSEApp::SaveFile(QString filename, SBPointer<ADNPart> part) {

	if (part == nullptr) {

		SAMSON::setStatusMessage(QString("Saving all designs to ") + filename);
		ADNNanorobot* nanorobot = GetNanorobot();
		if (nanorobot == nullptr) return;
		ADNLoader::SaveNanorobotToJson(nanorobot, filename.toStdString());

	}
	else {

		SAMSON::setStatusMessage(QString("Saving ") + QString::fromStdString(part->getName()) + QString(" to ") + filename);
		ADNLoader::SavePartToJson(part, filename.toStdString());

	}

}

void SEAdenitaCoreSEApp::LoadPartWithDaedalus(QString filename, int minEdgeSize) {

	SAMSON::setStatusMessage(QString("Loading ") + filename);

	// Apply algorithm
	DASDaedalus alg;
	alg.SetMinEdgeLength(minEdgeSize);
	std::string seq = "";
	auto part = alg.ApplyAlgorithm(seq, filename.toStdString());

	if (part == nullptr) return;

	QFileInfo fi(filename);
	QString s = fi.baseName();
	part->setName(s.toStdString());

	addPartToDocument(part);

}

bool SEAdenitaCoreSEApp::importFromCadnano(const QString& filename, SBDDocumentFolder* preferredFolder) {

	SAMSON::setStatusMessage(QString("Loading ") + filename);
	SAMSON::showProgressBar("Loading cadnano file...", 0, 100);
	SAMSON::setProgressBarValue(0);

	const QFileInfo fileInfo(filename);
	const auto failImport = [&](const QString& details = QString()) {

		SAMSON::hideProgressBar();
		SAMSON::setStatusMessage(QString("Could not load ") + filename);
		QString message = "Sorry, could not load the cadnano file:\n" + fileInfo.fileName();
		if (!details.isEmpty()) message += "\n\nDetails:\n" + details;
		SAMSON::informUser("Adenita", message);
		return false;

	};

	DASCadnano cad = DASCadnano();
	SBPointer<ADNPart> part = nullptr;

	try {
		part = cad.CreateCadnanoPart(filename.toStdString());
	}
	catch (const std::exception& exception) {
		return failImport(QString("Unexpected parser error: %1").arg(exception.what()));
	}
	catch (...) {
		return failImport("Unexpected parser error.");
	}
	SAMSON::setProgressBarValue(50);

	if (part == nullptr) {

		return failImport(QString::fromStdString(cad.GetLastError()));

	}

	QString s = fileInfo.baseName();
	part->setName(s.toStdString());

	try {
		if (!cad.CreateConformations(part))
			return failImport(QString::fromStdString(cad.GetLastError()));
	}
	catch (const std::exception& exception) {
		return failImport(QString("Unexpected conformation error: %1").arg(exception.what()));
	}
	catch (...) {
		return failImport("Unexpected conformation error.");
	}

	SAMSON::setProgressBarValue(70);

	SBDocument* document = nullptr;
	if (preferredFolder == nullptr) {
		document = ADNSamsonContext::GetActiveDocument(__func__);
		if (document == nullptr) {
			return failImport("No active SAMSON document is available.");
		}
	}

	SBFolder* folderWithModel = new SBFolder(s.toStdString());

	//bool isAlreadyHolding = SAMSON::isHolding();
	//if (!isAlreadyHolding) SAMSON::beginHolding("Load cadnano model");

	if (SAMSON::isHolding()) SAMSON::hold(folderWithModel);
	folderWithModel->create();
	if (preferredFolder) preferredFolder->addChild(folderWithModel);
	else document->addChild(folderWithModel);

	SAMSON::setProgressBarValue(80);

	addPartToDocument(part, false, folderWithModel);

	SAMSON::setProgressBarValue(90);

	addConformationToDocument(cad.Get3DConformation(), folderWithModel);
	addConformationToDocument(cad.Get2DConformation(), folderWithModel);
	addConformationToDocument(cad.Get1DConformation(), folderWithModel);

	//if (!isAlreadyHolding) SAMSON::endHolding();

	SAMSON::setProgressBarValue(100);
	SAMSON::hideProgressBar();

	return true;

}

void SEAdenitaCoreSEApp::ExportToSequenceList(QString filename, SBPointerIndexer<ADNPart> parts) {

	QFileInfo file = QFileInfo(filename);
	ADNLoader::OutputToCSV(parts, file.fileName().toStdString(), file.path().toStdString());

}

void SEAdenitaCoreSEApp::SetScaffoldSequence(std::string filename) {

	if (filename.empty()) {
		filename = SB_ELEMENT_PATH + "/Data/m13mp18.fasta";
	}

	std::string s = SEAdenitaCoreSEApp::readScaffoldFilename(filename);

	// get selected part
	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto parts = nanorobot->GetSelectedParts();

	if (parts.size() == 0) {
		SAMSON::informUser(QString("Adenita: Set Scaffold"), QString("Please select the component whose scaffold sequence you want to set."));
	}

	SB_FOR(SBPointer<ADNPart> part, parts) {
		auto scafs = part->GetScaffolds();
		SB_FOR(SBPointer<ADNSingleStrand> ss, scafs) {
			ADNBasicOperations::SetSingleStrandSequence(ss, s);
		}
	}

}

void SEAdenitaCoreSEApp::ExportToOxDNA(QString folder, ADNAuxiliary::OxDNAOptions options, SBPointerIndexer<ADNPart> parts) {

	SAMSON::setStatusMessage(QString("Exporting to oxDNA..."));
	if (parts.size() > 0) {
		ADNLoader::OutputToOxDNA(parts, folder.toStdString(), options);
	}
	SAMSON::setStatusMessage(QString("Done export to oxDNA."));

}

void SEAdenitaCoreSEApp::AddNtThreeP(int numNt) {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto nts = nanorobot->GetSelectedNucleotides();
	if (nts.size() == 1) {

		SBPointer<ADNNucleotide> nt = nts[0];
		auto ss = nt->GetStrand();
		auto part = ss->GetPart();
		SBVector3 dir = ADNAuxiliary::UblasVectorToSBVector(nt->GetBaseSegment()->GetE3());

		auto nts = ADNBasicOperations::AddNucleotidesThreePrime(part, ss, numNt, dir);
		DASBackToTheAtom* btta = new DASBackToTheAtom();
		btta->SetPositionsForNewNucleotides(part, nts);

		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

/// \brief Explicitly centers on all Adenita structures in the active document
void SEAdenitaCoreSEApp::centerCameraOnAllADNParts() {

	SBNodeIndexer nodeIndexer;
	SEAdenitaCoreSEApp::getAdenitaParts(nodeIndexer);

	if (nodeIndexer.size())
		SAMSON::getActiveCamera()->center(nodeIndexer, SBNode::All());	// take into account the hidden dummy atoms

}

void SEAdenitaCoreSEApp::centerCameraOnLoadedSystem() {

#if 1
	// explicitly center on the just loaded system, i.e. on the last structural model in the active document
	SBNodeIndexer nodeIndexer;
	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return;
	document->getNodes(nodeIndexer, SBNode::StructuralModel);
	if (nodeIndexer.size()) {

		SBStructuralModel* structuralModel = static_cast<SBStructuralModel*>(nodeIndexer[nodeIndexer.size() - 1]);
		SBNodeIndexer tmpIndexer;
		tmpIndexer.push_back(structuralModel);
		SAMSON::getActiveCamera()->center(tmpIndexer, SBNode::All());	// take into account the hidden dummy atoms

	}
#else
	// warning: does not center on the hidden atoms and since Adenita hides the dummy atoms then it won't center on the just loaded system
	SAMSON::getActiveCamera()->center();
#endif

}

void SEAdenitaCoreSEApp::centerCameraOnLoadedSystemWithTimer() const {

	// call to center the camera in 500ms
	QTimer::singleShot(500, getGUI(), []() { SEAdenitaCoreSEApp::centerCameraOnLoadedSystem(); });

}

void SEAdenitaCoreSEApp::CenterPart() {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto parts = nanorobot->GetSelectedParts();
	SB_FOR(SBPointer<ADNPart> part, parts) ADNBasicOperations::CenterPart(part);

}

void SEAdenitaCoreSEApp::GenerateSequence(double gcCont, int maxContGs, bool overwrite) {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto strands = nanorobot->GetSelectedSingleStrands();

	if (strands.size() == 0) {
		SAMSON::informUser(QString("Adenita: Set random sequence"), QString("Please select the single strands whose sequence you want to set."));
	}

	SB_FOR(SBPointer<ADNSingleStrand> ss, strands) {
		std::string seq = DASAlgorithms::GenerateSequence(gcCont, maxContGs, ss->getNumberOfNucleotides());
		ADNBasicOperations::SetSingleStrandSequence(ss, seq, true, overwrite);
	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

bool SEAdenitaCoreSEApp::isAdenitaPart(SBNode* node) {

	if (!node) return false;

	if (SBProxy* proxy = node->getProxy())
		return (proxy->getName() == "ADNPart" && proxy->getElementUUID() == SBUUID(SB_ELEMENT_UUID));

	return false;

}

void SEAdenitaCoreSEApp::getAdenitaParts(SBNodeIndexer& nodeIndexer, SBNode* parent) {

	if (!parent)
		parent = ADNSamsonContext::GetActiveDocument(__func__);

	if (!parent) return;

	SBNodeIndexer structuralModels;
	parent->getNodes(structuralModels, SBNode::StructuralModel);

	SB_FOR(SBNode * node, structuralModels) {

		if (SEAdenitaCoreSEApp::isAdenitaPart(node))
			nodeIndexer.push_back(node);

	}

}

/// \brief Returns whether `node` has an Adenita's structural model (`ADNPart`)
/// \param node A folder or document node.
bool SEAdenitaCoreSEApp::hasAdenitaPart(SBNode* parent) {

	if (!parent) return false;

	SBNodeIndexer structuralModels;
	parent->getNodes(structuralModels, SBNode::StructuralModel);

	SB_FOR(SBNode * node, structuralModels) {

		if (SEAdenitaCoreSEApp::isAdenitaPart(node))
			return true;

	}

	return false;

}

void SEAdenitaCoreSEApp::requestVisualModelUpdate() {

	SEAdenitaVisualModel* adenitaVisualModel = SEAdenitaCoreSEApp::getVisualModel();

	if (adenitaVisualModel) {

		adenitaVisualModel->requestUpdate();

	}
	else {

		SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
		if (document != nullptr) SEAdenitaCoreSEApp::addVisualModel(document);

		ADNLogger::LogDebug(std::string("Adding visual model"));

	}

}

void SEAdenitaCoreSEApp::requestDeferredVisualModelResetAfterSAMLoad() {

	if (g_pendingSAMLoadVisualModelReset) return;

	g_pendingSAMLoadVisualModelReset = true;

	// Note: QTimer::singleShot(0, ...) should be enough in synchronous treatment within one event-loop turn.

	QTimer::singleShot(0, QCoreApplication::instance(), []() {

		g_pendingSAMLoadVisualModelReset = false;

		SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);

		if (!document) return;
		if (!SEAdenitaCoreSEApp::hasAdenitaPart(document)) return;

		// ensure that Adenita app is initialized
		SEAdenitaCoreSEApp* adenitaApp = SEAdenitaCoreSEApp::getAdenitaApp();
		if (!adenitaApp) return;

		SEAdenitaCoreSEApp::resetVisualModel(document);
		SAMSON::requestViewportUpdate();

		// note: calling this will change the camera position saved in the document which is not a desired behaviour, so use it only for debug/tests
		//SEAdenitaCoreSEApp::centerCameraOnAllADNParts();

		});

}

void SEAdenitaCoreSEApp::resetVisualModel() {

	SEAdenitaCoreSEApp::resetVisualModel(ADNSamsonContext::GetActiveDocument(__func__));

}

void SEAdenitaCoreSEApp::resetVisualModel(SBNode* parent) {

	if (!parent) return;

	// create visual model per nanorobot

	if (SEAdenitaVisualModel* adenitaVisualModel = SEAdenitaCoreSEApp::getVisualModel(parent)) {

		adenitaVisualModel->update();
		SAMSON::requestViewportUpdate();

	}
	else {

		bool ret = SEAdenitaCoreSEApp::addVisualModel(parent);

	}

	ADNLogger::LogDebug(std::string("Restarting visual model"));

}

bool SEAdenitaCoreSEApp::addVisualModel(SBNode* parent) {

	if (parent) {

		SEAdenitaVisualModel* newVisualModel = new SEAdenitaVisualModel();

		if (SAMSON::isHolding())
			SAMSON::hold(newVisualModel);

		if (parent->isCreated())
			newVisualModel->create();

		bool ret = parent->addChild(newVisualModel);
		return ret;

	}

	return false;

}

SEAdenitaVisualModel* SEAdenitaCoreSEApp::getVisualModel() {

	return SEAdenitaCoreSEApp::getVisualModel(ADNSamsonContext::GetActiveDocument(__func__));

}

SEAdenitaVisualModel* SEAdenitaCoreSEApp::getVisualModel(SBNode* parent) {

	if (!parent) return nullptr;

	SBNodeIndexer nodeIndexer;
	parent->getNodes(nodeIndexer, SBNode::VisualModel);

	SEAdenitaVisualModel* adenitaVisualModel = nullptr;

	SB_FOR(SBNode * node, nodeIndexer) {

		if (node->getType() == SBNode::VisualModel) {

			SBVisualModel* visualModel = static_cast<SBVisualModel*>(node);

			if (visualModel->getProxy()->getName() == "SEAdenitaVisualModel" && visualModel->getProxy()->getElementUUID() == SBUUID(SB_ELEMENT_UUID)) {

				adenitaVisualModel = static_cast<SEAdenitaVisualModel*>(visualModel);
				break;

			}

		}

	}

	return adenitaVisualModel;

}

void SEAdenitaCoreSEApp::BreakSingleStrand(bool fivePrimeMode) {

	mod_ = true;

	SBPointer<ADNNucleotide> breakNucleotide = nullptr;
	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) {
		mod_ = false;
		return;
	}
	auto nucleotides = nanorobot->GetHighlightedNucleotides();
	if (nucleotides.size() == 1) {

		SBPointer<ADNNucleotide> nucleotide = nucleotides[0];
		if (nucleotide->getEndType() != ADNNucleotide::EndType::ThreePrime) {

			SBPointer<ADNSingleStrand> singleStrand = nucleotide->GetStrand();
			const bool circular = singleStrand->IsCircular();

			SBPointer<ADNPart> part = singleStrand->GetPart();

			// to break in the 5' or 3' direction
			if (fivePrimeMode) breakNucleotide = nucleotide;
			else breakNucleotide = nucleotide->GetNext(true);

			if (breakNucleotide != nullptr) {

				auto newStrands = ADNBasicOperations::BreakSingleStrand(part, breakNucleotide);

				if (circular) {
					ADNBasicOperations::MergeSingleStrands(part, part, newStrands.second, newStrands.first);
				}

				SEAdenitaCoreSEApp::resetVisualModel();

			}

		}

	}

	mod_ = false;

}

void SEAdenitaCoreSEApp::TwistDoubleHelix(SBPointerIndexer<ADNDoubleStrand> dss, double angle) {

	const SBPointerIndexer<ADNPart> affectedParts =
		ADNGeometrySynchronization::collectPartsFromDoubleStrands(dss);

	syncPartsBeforeGeometryEdit(affectedParts);

	SB_FOR(SBPointer<ADNDoubleStrand> ds, dss) {

		if (ds == nullptr) continue;

		double newDeg = ds->GetInitialTwistAngle() + angle;
		ADNBasicOperations::TwistDoubleHelix(ds, newDeg);
		ADNGeometrySynchronization::rotateDoubleStrandGeometry(*ds, -degreesToRadians(angle));

	}

	syncPartsAfterGeometryEdit(affectedParts);

	if (dss.size() > 0) SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::LinearCatenanes(SBQuantity::length radius, SBPosition3 center, SBVector3 normal, int num) {

	auto part = DASCreator::CreateLinearCatenanes(radius, center, normal, num);
	addPartToDocument(part);
	SEAdenitaCoreSEApp::resetVisualModel();
	SEAdenitaCoreSEApp::centerCameraOnLoadedSystem();

}

void SEAdenitaCoreSEApp::Kinetoplast(SBQuantity::length radius, SBPosition3 center, SBVector3 normal, int rows, int cols) {

	auto part = DASCreator::CreateHexagonalCatenanes(radius, center, normal, rows, cols);
	addPartToDocument(part);
	SEAdenitaCoreSEApp::resetVisualModel();
	SEAdenitaCoreSEApp::centerCameraOnLoadedSystem();

}

void SEAdenitaCoreSEApp::setStartNucleotide() {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto nucleotides = nanorobot->GetSelectedNucleotides();
	if (nucleotides.size() > 1) {

		// order the nts w.r.t. the single strand they belong
		// and perform the operation only once per ss

		SAMSON::informUser(QString("Adenita: Set 5'"), QString("Please select a single nucleotide you want to set as new 5'."));
		return;

	}
	else if (nucleotides.size() == 1) {

		ADNNucleotide* nucleotide = nucleotides[0];
		if (nucleotide->getEndType() != ADNNucleotide::EndType::FivePrime) {

			ADNBasicOperations::SetStart(nucleotide, true);

		}
		else {

			SAMSON::informUser(QString("Adenita: Set 5'"), QString("This nucleotide is already 5'."));
			return;

		}

	}
	else if (nucleotides.size() == 0) {

		SAMSON::informUser(QString("Adenita: Set 5'"), QString("Please select a nucleotide you want to set as new 5'."));
		return;

	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::MergeComponents(SBPointer<ADNPart> p1, SBPointer<ADNPart> p2) {

	if (p1 == nullptr || p2 == nullptr) return;
	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;

	SBPointer<ADNPart> newPart = ADNBasicOperations::MergeParts(p1, p2);

	nanorobot->DeregisterPart(p2);
	if (p2->getParent())
		p2->getParent()->removeChild(p2());

	p1 = newPart;

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::MoveDoubleStrand(SBPointer<ADNDoubleStrand> ds, SBPointer<ADNPart> p) {

	SBPointer<ADNPart> oldPart = ds->GetPart();
	if (oldPart != p) ADNBasicOperations::MoveStrand(oldPart, p, ds);

}

void SEAdenitaCoreSEApp::MoveSingleStrand(SBPointer<ADNSingleStrand> ss, SBPointer<ADNPart> p) {

	SBPointer<ADNPart> oldPart = ss->GetPart();
	if (oldPart != p) ADNBasicOperations::MoveStrand(oldPart, p, ss);

}

bool SEAdenitaCoreSEApp::CalculateBindingRegions(int oligoConc, int monovalentConc, int divalentConc) {

	bool res = false;
	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return false;
	auto parts = nanorobot->GetSelectedParts();

	if (parts.size() == 0) {

		SAMSON::informUser(QString("Adenita: Calculate Thermodynamic Properties"), QString("The selection is empty. Please select one or more components."));
		return false;

	}

	SB_FOR(SBPointer<ADNPart> part, parts) if (part != nullptr) {

		PIPrimer3& p = PIPrimer3::GetInstance();
		p.UpdateBindingRegions(part);
		p.Calculate(part, oligoConc, monovalentConc, divalentConc);
		res = true;

	}

	return res;

}

void SEAdenitaCoreSEApp::TwistDoubleHelix() {

	double deg = ADNConstants::BP_ROT;
	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto dss = nanorobot->GetSelectedDoubleStrands();

	const SBPointerIndexer<ADNPart> affectedParts =
		ADNGeometrySynchronization::collectPartsFromDoubleStrands(dss);

	syncPartsBeforeGeometryEdit(affectedParts);

	SB_FOR(SBPointer<ADNDoubleStrand> ds, dss) {

		if (ds == nullptr) continue;

		double newDeg = ds->GetInitialTwistAngle() + deg;
		ADNBasicOperations::TwistDoubleHelix(ds, newDeg);
		ADNGeometrySynchronization::rotateDoubleStrandGeometry(*ds, -degreesToRadians(deg));

	}

	syncPartsAfterGeometryEdit(affectedParts);

	if (dss.size() > 0) SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::TestNeighbors() {

	// get selected nucleotide and part
	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto nts = nanorobot->GetSelectedNucleotides();
	if (nts.size() == 0) return;

	SBPointer<ADNNucleotide> nt = nts[0];
	SBPointer<ADNPart> part = nt->GetStrand()->GetPart();
	// create neighbor list
	SEConfig& config = SEConfig::GetInstance();
	auto neighbors = ADNNeighbors();
	neighbors.SetMaxCutOff(SBQuantity::nanometer(config.debugOptions.maxCutOff));
	neighbors.SetMinCutOff(SBQuantity::nanometer(config.debugOptions.minCutOff));
	neighbors.SetIncludePairs(true);
	neighbors.InitializeNeighbors(part);

	// highlight neighbors of selected nucleotide
	auto ntNeighbors = neighbors.GetNeighbors(nt);
	SB_FOR(SBPointer<ADNNucleotide> ntN, ntNeighbors) ntN->setSelectionFlag(true);

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::ImportFromOxDNA(const std::string& topoFile, const std::string& configFile) {

	auto res = ADNLoader::InputFromOxDNA(topoFile, configFile);
	if (res.succeeded()) {

		SBPointer<ADNPart> p = res.part;
		addPartToDocument(p, true);
		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SEAdenitaCoreSEApp::FromDataGraph(bool resetVisualModel) {

	SBNodeIndexer nodeIndexer;
	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return;
	document->getNodes(nodeIndexer, SBNode::StructuralModel);

	SB_FOR(auto node, nodeIndexer) {

		if (node->isSelected()) {

			SBPointer<ADNPart> part = ADNLoader::GenerateModelFromDataGraph(node);
			addPartToDocument(part, true);

		}

	}

	if (resetVisualModel)
		SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::HighlightXOs() {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto parts = nanorobot->GetParts();
	if (parts.size() == 0) return;

	SAMSON::beginHolding("Select crossovers");

	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) {
		SAMSON::endHolding();
		return;
	}
	document->clearSelection();

	SB_FOR(SBPointer<ADNPart> p, parts) {
		
		auto xos = PICrossovers::GetCrossovers(p);

		for (const auto& p : xos) {

			if (p.first.isValid()) p.first->setSelectionFlag(true);
			if (p.second.isValid()) p.second->setSelectionFlag(true);

		}

	}

	SAMSON::endHolding();

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::HighlightPosXOs() {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto parts = nanorobot->GetParts();
	if (parts.size() == 0) return;

	SAMSON::beginHolding("Select possible crossovers");

	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) {
		SAMSON::endHolding();
		return;
	}
	document->clearSelection();

	SB_FOR(SBPointer<ADNPart> p, parts) PICrossovers::GetPossibleCrossovers(p);

	SAMSON::endHolding();

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::ExportToCanDo(const QString& filename) {

	SBNodeIndexer nodeIndexer;
	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return;
	document->getNodes(nodeIndexer, SBNode::StructuralModel);

	SBPointerIndexer<ADNPart> parts;

	SB_FOR(auto node, nodeIndexer) {

		if (node->isSelected() && SEAdenitaCoreSEApp::isAdenitaPart(node)) {

			SBPointer<ADNPart> part = static_cast<ADNPart*>(node);
			parts.addReferenceTarget(part());

		}

	}

	if (parts.size() == 1) {

		SBPointer<ADNPart> part = parts[0];
		ADNLoader::OutputToCanDo(part, filename.toStdString());

	}
	else {

		auto nanorobot = GetNanorobot();
		if (nanorobot == nullptr) return;
		ADNLoader::OutputToCanDo(nanorobot, filename.toStdString());

	}

}

void SEAdenitaCoreSEApp::FixDesigns() {

	SBNodeIndexer nodeIndexer;
	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return;
	document->getNodes(nodeIndexer, SBNode::StructuralModel);

	SBPointerIndexer<ADNPart> parts;

	SB_FOR(auto node, nodeIndexer) {

		if (!node->isSelected()) continue;

		SBPointer<ADNPart> part = static_cast<ADNPart*>(node);

		// .sam format fix
		//auto nucleotides = part->GetNucleotides();
		//SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) {
		//  auto bbPos = nt->GetBackbonePosition();
		//  auto scPos = nt->GetSidechainPosition();
		//  auto pos = (bbPos + scPos)*0.5;

		//  auto at = nt->GetCenterAtom();
		//  if (at == nullptr) at = new ADNAtom();
		//  at->setElementType(SBElement::Unknown);//Meitnerium);
		//  nt->SetCenterAtom(at);

		//  nt->SetPosition(pos);
		//  ublas::vector<double> e(3, 0.0);
		//  nt->SetE3(e);
		//  nt->SetE2(e);
		//  nt->SetE1(e);
		//}

		//auto baseSegments = part->GetBaseSegments();
		//SB_FOR(SBPointer<ADNBaseSegment> bs, baseSegments) {
		//  auto nucleotides = bs->GetNucleotides();
		//  SBPosition3 pos;
		//  SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) {
		//    pos += nt->GetBackbonePosition();
		//    pos += nt->GetSidechainPosition();
		//  }
		//  pos /= nucleotides.size()*2;

		//  auto at = bs->GetCenterAtom();
		//  if (at == nullptr) at = new ADNAtom();
		//  at->setElementType(SBElement::Unknown);//Meitnerium);
		//  bs->SetCenterAtom(at);
		//  part->RegisterAtom(bs, at, true);
		//  // hiding atoms here cause when they are created is too slow
		//  bs->HideCenterAtom();

		//  bs->SetPosition(pos);
		//  ublas::vector<double> e(3, 0.0);
		//  bs->SetE3(e);
		//  bs->SetE2(e);
		//  bs->SetE1(e);
		//}
		//AddLoadedPartToNanorobot(part);

		// fix for cadnano designs
		auto strands = part->GetSingleStrands();
		SB_FOR(SBPointer<ADNSingleStrand> ss, strands) {

			SBPointer<ADNSingleStrand> newSs = new ADNSingleStrand();
			newSs->setName(ss->getName());
			newSs->setScaffoldFlag(ss->IsScaffold());
			if (SAMSON::isHolding()) SAMSON::hold(newSs());
			newSs->create();
			part->RegisterSingleStrand(newSs);

			// reverse 5'->3' direction
			SBPointer<ADNNucleotide> nt = ss->GetFivePrime();
			while (nt != nullptr) {

				auto next = nt->GetNext();
				part->DeregisterNucleotide(nt, true, false);
				part->RegisterNucleotideFivePrime(newSs, nt);
				nt = next;

			}

			part->DeregisterSingleStrand(ss);
			ss->erase();
			ss.deleteReferenceTarget();

		}

	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::CreateBasePair() {

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return;
	auto selectedNucleotides = nanorobot->GetSelectedNucleotides();
	if (selectedNucleotides.size() > 0) {

		const SBPointerIndexer<ADNPart> affectedParts =
			ADNGeometrySynchronization::collectPartsFromNucleotides(selectedNucleotides);

		syncPartsBeforeGeometryEdit(affectedParts);
		DASOperations::AddComplementaryStrands(nanorobot, selectedNucleotides);
		syncPartsAfterGeometryEdit(affectedParts);
		SEAdenitaCoreSEApp::resetVisualModel();

	}
	else {

		SAMSON::informUser(QString("Adenita: Create base pair"), QString("Please select the nucleotides or single strands whose pairs you want to create."));

	}

}

void SEAdenitaCoreSEApp::MarkGeometryDirty(SBNode* node) {

	if (node == nullptr) return;

	SBPointer<ADNPart> part = ADNGeometrySynchronization::findOwningPart(node);
	if (part == nullptr) return;

	if (!dirtyGeometryParts_.hasIndex(part()))
		dirtyGeometryParts_.addReferenceTarget(part());

	if (geometrySyncPending_) return;

	geometrySyncPending_ = true;

	QObject* receiver = QCoreApplication::instance();
	if (receiver == nullptr) {

		FlushDeferredGeometrySynchronization();
		return;

	}

	QTimer::singleShot(0, receiver, [this]() {

		FlushDeferredGeometrySynchronization();

		});

}

void SEAdenitaCoreSEApp::FlushDeferredGeometrySynchronization() {

	geometrySyncPending_ = false;
	if (dirtyGeometryParts_.size() == 0) return;
	if (geometrySyncInProgress_) return;

	SBPointerIndexer<ADNPart> parts = dirtyGeometryParts_;
	dirtyGeometryParts_.clear();

	ScopedGeometrySyncGuard guard(geometrySyncInProgress_);

	SB_FOR(SBPointer<ADNPart> part, parts) {

		if (part != nullptr)
			ADNGeometrySynchronization::syncPartFramesFromGeometry(
				*part,
				ADNGeometrySynchronization::SyncReason::AfterStructuralPositionChange);

	}

	SEAdenitaCoreSEApp::requestVisualModelUpdate();
	SAMSON::requestViewportUpdate();

}

void SEAdenitaCoreSEApp::onDocumentEvent(SBDocumentEvent* documentEvent) {

	if (documentEvent == nullptr) return;

	const SBDocumentEvent::Type eventType = documentEvent->getType();
	SBNode* node = documentEvent->getAuxiliaryNode();

	if (eventType == SBDocumentEvent::DocumentCloseBegin ||
		eventType == SBDocumentEvent::DocumentCloseEnd ||
		eventType == SBDocumentEvent::DocumentRemoved) {

		if (documentEvent->getSender() && documentEvent->getSender()->getType() == SBNode::Document)
			RemoveNanorobot(dynamic_cast<SBDocument*>(documentEvent->getSender()));
		if (node && node->getType() == SBNode::Document)
			RemoveNanorobot(dynamic_cast<SBDocument*>(node));

	}

	if (!node) return;
	if (node->getProxy()->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return;

#if 0
	// is handled in the Adenita Visual Model
	// handle addition and deletion of ADN nodes for updating the Adenita Visual Model

	if (eventType == SBDocumentEvent::StructuralModelAdded || eventType == SBDocumentEvent::StructuralModelRemoved) {

		if (SEAdenitaCoreSEApp::isAdenitaPart(node))
			requestVisualModelUpdate();

	}
#endif

	if (mod_) return; // modifications handle themselves deletions

	// handle deletion of ADN nodes for bookkeeping used in ADN nodes

	if (eventType == SBDocumentEvent::StructuralModelRemoved) {

		// on delete a registered ADNPart

		if (SEAdenitaCoreSEApp::isAdenitaPart(node)) {

			SBPointer<ADNPart> part = dynamic_cast<ADNPart*>(node);
			ADNNanorobot* nanorobot = GetNanorobot();
			if (part != nullptr && nanorobot != nullptr)
				nanorobot->DeregisterPart(part);

		}

	}

}

void SEAdenitaCoreSEApp::onStructuralEvent(SBStructuralEvent* structuralEvent) {

	const SBStructuralEvent::Type eventType = structuralEvent->getType();
	SBNode* node = structuralEvent->getAuxiliaryNode();
	if (!node) node = dynamic_cast<SBNode*>(structuralEvent->getSender());
	if (!node) return;
	if (node->getProxy()->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return;

	const std::string nodeClassName = node->getProxy()->getName();

	if (eventType == SBStructuralEvent::AtomPositionChanged ||
		eventType == SBStructuralEvent::TransformChanged) {

		if (!geometrySyncInProgress_)
			MarkGeometryDirty(node);
		return;

	}

#if 0
	// is handled in the Adenita Visual Model
	// handle addition and deletion of ADN nodes for updating the Adenita Visual Model

	if (eventType == SBStructuralEvent::ChainAdded || eventType == SBStructuralEvent::ChainRemoved) {

		if (nodeClassName == "ADNSingleStrand")
			requestVisualModelUpdate();

	}
	else if (eventType == SBStructuralEvent::ResidueAdded || eventType == SBStructuralEvent::ResidueRemoved) {

		if (nodeClassName == "ADNNucleotide")
			requestVisualModelUpdate();

	}
	else if (eventType == SBStructuralEvent::BackboneAdded || eventType == SBStructuralEvent::BackboneRemoved) {

		if (nodeClassName == "ADNBackbone")
			requestVisualModelUpdate();

	}
	else if (eventType == SBStructuralEvent::SideChainAdded || eventType == SBStructuralEvent::SideChainRemoved) {

		if (nodeClassName == "ADNSidechain")
			requestVisualModelUpdate();

	}
	else if (eventType == SBStructuralEvent::StructuralGroupAdded || eventType == SBStructuralEvent::StructuralGroupRemoved) {

		if (nodeClassName == "ADNBaseSegment" || nodeClassName == "ADNDoubleStrand" || nodeClassName == "ADNLoop" ||
			nodeClassName == "ADNCell" || nodeClassName == "ADNBasePair" || nodeClassName == "ADNSkipPair" || nodeClassName == "ADNLoopPair")
			requestVisualModelUpdate();

	}
	else if (eventType == SBStructuralEvent::ParticlePositionChanged) {

		if (nodeClassName == "ADNAtom")
			requestVisualModelUpdate();

	}
#endif

	if (mod_) return; // modifications handle themselves deletions

	// handle deletion of ADN nodes for bookkeeping used in ADN nodes

	if (eventType == SBStructuralEvent::ChainRemoved) {

		SBPointer<ADNSingleStrand> ss = dynamic_cast<ADNSingleStrand*>(node);
		if (ss != nullptr) {

			auto part = static_cast<ADNPart*>(structuralEvent->getSender()->getParent());
			part->DeregisterSingleStrand(ss, false);

		}

	}
	else if (eventType == SBStructuralEvent::ResidueRemoved) {

		auto node = structuralEvent->getAuxiliaryNode();
		SBPointer<ADNNucleotide> nt = dynamic_cast<ADNNucleotide*>(node);
		if (nt != nullptr) {

			SBPointer<ADNSingleStrand> ss = static_cast<ADNSingleStrand*>(structuralEvent->getSender());
			if (ss != nullptr) {

				auto part = ss->GetPart();
				if (part != nullptr) {

					part->DeregisterNucleotide(nt, false, true, true);
					nt->disconnectPair();

				}
				if (ss->getNumberOfNucleotides() == 0) {

					if (part != nullptr) part->DeregisterSingleStrand(ss);
					ss->erase();
					//ss.deleteReferenceTarget();	// causes a crash

				}

			}

		}

	}
	else if (eventType == SBStructuralEvent::StructuralGroupRemoved) {

		auto node = structuralEvent->getAuxiliaryNode();
		SBPointer<ADNBaseSegment> bs = dynamic_cast<ADNBaseSegment*>(node);
		if (bs != nullptr) {

			auto nucleotides = bs->GetNucleotides();
			SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) if (nt != nullptr) nt->erase();

			SBPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(structuralEvent->getSender());
			if (ds != nullptr) {

				auto part = ds->GetPart();
				if (part != nullptr) part->DeregisterBaseSegment(bs, false, true);

			}

		}
		else {

			SBPointer<ADNDoubleStrand> ds = dynamic_cast<ADNDoubleStrand*>(node);
			if (ds != nullptr) {

				SBPointer<ADNPart> part = static_cast<ADNPart*>(structuralEvent->getSender()->getParent());
				if (part != nullptr) part->DeregisterDoubleStrand(ds, false, true);

			}

		}

	}

}

void SEAdenitaCoreSEApp::ConnectToDocument(SBDocument* doc) {

	if (doc == nullptr) return;

	if (doc->documentSignalIsConnectedToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onDocumentEvent)) == false)
		doc->connectDocumentSignalToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onDocumentEvent));

}

void SEAdenitaCoreSEApp::ConnectToDocument() {

	ConnectToDocument(ADNSamsonContext::GetActiveDocument(__func__));

}

ADNNanorobot* SEAdenitaCoreSEApp::GetNanorobot() {

	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return nullptr;

	return getNanorobot(document);

}

ADNNanorobot* SEAdenitaCoreSEApp::getNanorobot(SBDocument* document) {

	if (document == nullptr) return nullptr;

	if (nanorobotMap.find(document) == nanorobotMap.end()) {

		// create new nanorobot for this document
		nanorobotMap.insert(std::make_pair(document, new ADNNanorobot()));
		ConnectToDocument(document);

	}

	return nanorobotMap.at(document);

}

void SEAdenitaCoreSEApp::RemoveNanorobot(SBDocument* document) {

	if (document == nullptr) return;

	auto it = nanorobotMap.find(document);
	if (it == nanorobotMap.end()) return;

	if (document->documentSignalIsConnectedToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onDocumentEvent)))
		document->disconnectDocumentSignalFromSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onDocumentEvent));

	delete it->second;
	nanorobotMap.erase(it);

}

void SEAdenitaCoreSEApp::ClearNanorobots() {

	for (auto& entry : nanorobotMap)
		delete entry.second;

	nanorobotMap.clear();

}

std::string SEAdenitaCoreSEApp::readScaffoldFilename(const std::string& filename) {

	std::string seq = "";
	if (filename.size() > 0) {

		std::vector<std::string> lines;
		SBIFileReader::getFileLines(filename, lines);

		seq = ADNScaffoldReader::readScaffoldLines(lines);

	}

	return seq;

}

QStringList SEAdenitaCoreSEApp::getListOfPartNames() {

	QStringList names;

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return names;

	auto parts = nanorobot->GetParts();
	SB_FOR(SBPointer<ADNPart> p, parts)
		names << QString::fromStdString(p->getName());

	return names;

}

std::string SEAdenitaCoreSEApp::getUniquePartName(const std::string& partName) {

	QString uniquePartName = QString::fromStdString(partName);
	unsigned int idx = 1;
	QStringList partNames = getListOfPartNames();

	while (partNames.contains(uniquePartName)) {

		++idx;
		uniquePartName = QString::fromStdString(partName) + " " + QString::number(idx);

	}

	return uniquePartName.toStdString();

}

SBPosition3 SEAdenitaCoreSEApp::getSnappedPosition(const SBPosition3& currentPosition) {

	SBPosition3 snappedPosition = currentPosition;

	ADNNanorobot* nanorobot = GetNanorobot();
	if (nanorobot == nullptr) return snappedPosition;

	const auto highlightedBaseSegments = nanorobot->GetHighlightedBaseSegments();
	const auto highlightedBaseSegmentsFromNucleotides = nanorobot->GetHighlightedBaseSegmentsFromNucleotides();
	const auto highlightedAtoms = nanorobot->GetHighlightedAtoms();

	if (highlightedAtoms.size() == 1)
		snappedPosition = highlightedAtoms[0]->getPosition();
	else if (highlightedBaseSegments.size() == 1)
		snappedPosition = highlightedBaseSegments[0]->GetPosition();
	else if (highlightedBaseSegmentsFromNucleotides.size() == 1)
		snappedPosition = highlightedBaseSegmentsFromNucleotides[0]->GetPosition();

	return snappedPosition;

}

void SEAdenitaCoreSEApp::addPartToDocument(SBPointer<ADNPart> part, bool positionsData, SBFolder* preferredFolder) {

	if (part == nullptr) return;
	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return;
	ADNNanorobot* nanorobot = getNanorobot(document);
	if (nanorobot == nullptr) return;

	SEConfig& config = SEConfig::GetInstance();
	if (config.auto_set_scaffold_sequence) {

		const std::string fname = SEAdenitaCoreSEAppGUI::getScaffoldFilename();
		const std::string seq = SEAdenitaCoreSEApp::readScaffoldFilename(fname);
		if (seq.size()) {

			auto scaffolds = part->GetScaffolds();
			SB_FOR(SBPointer<ADNSingleStrand> ss, scaffolds) {

				ADNBasicOperations::SetSingleStrandSequence(ss, seq);

			}

		}

	}

	DASBackToTheAtom btta = DASBackToTheAtom();
	btta.PopulateWithMockAtoms(part, positionsData);
	if (!positionsData) {

		btta.SetNucleotidesPositions(part);
		if (config.use_atomic_details) {

			btta.GenerateAllAtomModel(part);

		}
		//btta.CheckDistances(part);

	}

	part->ResetBoundingBox();
	nanorobot->RegisterPart(part);

	//events
	ConnectStructuralSignalSlots(part);

	//bool isAlreadyHolding = SAMSON::isHolding();
	//if (!isAlreadyHolding) SAMSON::beginHolding("Add model");

	if (SAMSON::isHolding()) SAMSON::hold(part());
	part->create();

	if (preferredFolder) preferredFolder->addChild(part());
	else document->addChild(part());

	//if (!isAlreadyHolding) SAMSON::endHolding();

}

void SEAdenitaCoreSEApp::addConformationToDocument(SBPointer<ADNConformation> conf, SBFolder* preferredFolder) {

	if (conf == nullptr) return;
	SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
	if (document == nullptr) return;
	ADNNanorobot* nanorobot = getNanorobot(document);
	if (nanorobot == nullptr) return;

	nanorobot->RegisterConformation(conf);

	//bool isAlreadyHolding = SAMSON::isHolding();
	//if (!isAlreadyHolding) SAMSON::beginHolding("Add conformation");

	if (SAMSON::isHolding()) SAMSON::hold(conf());
	conf->create();

	if (preferredFolder) preferredFolder->addChild(conf());
	else document->addChild(conf());

	//if (!isAlreadyHolding) SAMSON::endHolding();

}

void SEAdenitaCoreSEApp::AddLoadedPartToNanorobot(SBPointer<ADNPart> part) {

	if (part->isLoadedViaSAMSON()) {

		ADNNanorobot* nanorobot = GetNanorobot();
		if (nanorobot == nullptr) return;
		nanorobot->RegisterPart(part);

		//events
		ConnectStructuralSignalSlots(part);

		part->setLoadedViaSAMSON(false);

		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SEAdenitaCoreSEApp::ConnectStructuralSignalSlots(SBPointer<ADNPart> part) {

	part->connectStructuralSignalToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onStructuralEvent));

}

void SEAdenitaCoreSEApp::keyPressEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_0) {
		SAMSON::requestViewportUpdate();
	}

}

void SEAdenitaCoreSEApp::SetMod(bool m) { mod_ = m; }

#include "SEAdenitaCoreSEApp.hpp"
#include "SEAdenitaCoreSEAppGUI.hpp"
#include "SEAdenitaVisualModel.hpp"
#include "SEAdenitaVisualModelProperties.hpp"

#include "PICrossovers.hpp"
#include "DASAlgorithms.hpp"


SEAdenitaCoreSEApp::SEAdenitaCoreSEApp() {

	setGUI(new SEAdenitaCoreSEAppGUI(this));
	getGUI()->loadDefaultSettings();

	SEConfig& config = SEConfig::GetInstance();

	SB_INFORMATION("Adenita started");

}

SEAdenitaCoreSEApp::~SEAdenitaCoreSEApp() {

	getGUI()->saveDefaultSettings();
	delete getGUI();

}

SEAdenitaCoreSEAppGUI* SEAdenitaCoreSEApp::getGUI() const { return static_cast<SEAdenitaCoreSEAppGUI*>(SBDApp::getGUI()); }

SEAdenitaCoreSEApp* SEAdenitaCoreSEApp::getAdenitaApp() {

	return static_cast<SEAdenitaCoreSEApp*>(SAMSON::getApp(SBCContainerUUID("85DB7CE6-AE36-0CF1-7195-4A5DF69B1528"), SBUUID(SB_ELEMENT_UUID)));

}

bool SEAdenitaCoreSEApp::loadPart(const QString& filename, SBDDocumentFolder* preferredFolder) {

	SAMSON::setStatusMessage(QString("Loading component from ") + filename);

	ADNPointer<ADNPart> part = ADNLoader::LoadPartFromJson(filename.toStdString());
	if (part == nullptr) return false;
	addPartToDocument(part, false, preferredFolder);

	return true;

}

void SEAdenitaCoreSEApp::loadParts(const QString& filename, SBDDocumentFolder* preferredFolder) {

	SAMSON::setStatusMessage(QString("Loading components from ") + filename);

	std::vector<ADNPointer<ADNPart>> parts = ADNLoader::LoadPartsFromJson(filename.toStdString());
	for (ADNPointer<ADNPart> part : parts) if (part != nullptr) addPartToDocument(part, false, preferredFolder);

}

void SEAdenitaCoreSEApp::SaveFile(QString filename, ADNPointer<ADNPart> part) {

	if (part == nullptr) {

		SAMSON::setStatusMessage(QString("Saving all designs to ") + filename);
		ADNLoader::SaveNanorobotToJson(GetNanorobot(), filename.toStdString());

	}
	else {

		SAMSON::setStatusMessage(QString("Saving ") + QString::fromStdString(part->getName()) + QString(" to ") + filename);
		ADNLoader::SavePartToJson(part, filename.toStdString());

	}

}

void SEAdenitaCoreSEApp::LoadPartWithDaedalus(QString filename, int minEdgeSize) {

	SAMSON::setStatusMessage(QString("Loading ") + filename);

	// Apply algorithm
	DASDaedalus *alg = new DASDaedalus();
	alg->SetMinEdgeLength(minEdgeSize);
	std::string seq = "";
	auto part = alg->ApplyAlgorithm(seq, filename.toStdString());

	QFileInfo fi(filename);
	QString s = fi.baseName();
	part->setName(s.toStdString());

	addPartToDocument(part);

}

bool SEAdenitaCoreSEApp::importFromCadnano(const QString& filename, SBDDocumentFolder* preferredFolder) {

	SAMSON::setStatusMessage(QString("Loading ") + filename);
	DASCadnano cad = DASCadnano();

	ADNPointer<ADNPart> part = cad.CreateCadnanoPart(filename.toStdString());

	if (part == nullptr) return false;
  
	QFileInfo fi(filename);
	QString s = fi.baseName();
	part->setName(s.toStdString());

	SBFolder* folderWithModel = new SBFolder(s.toStdString());

	//SAMSON::beginHolding("Add cadnano model");
	if (SAMSON::isHolding()) SAMSON::hold(folderWithModel);
	folderWithModel->create();
	if (preferredFolder) preferredFolder->addChild(folderWithModel);
	else SAMSON::getActiveDocument()->addChild(folderWithModel);

	addPartToDocument(part, false, folderWithModel);

	cad.CreateConformations(part);
	addConformationToDocument(cad.Get3DConformation(), folderWithModel);
	addConformationToDocument(cad.Get2DConformation(), folderWithModel);
	addConformationToDocument(cad.Get1DConformation(), folderWithModel);

	//SAMSON::endHolding();

	return true;

}

void SEAdenitaCoreSEApp::ExportToSequenceList(QString filename, CollectionMap<ADNPart> parts) {

	QFileInfo file = QFileInfo(filename);
	ADNLoader::OutputToCSV(parts, file.fileName().toStdString(), file.path().toStdString());

}

void SEAdenitaCoreSEApp::SetScaffoldSequence(std::string filename) {

	if (filename.empty()) {
		filename = SB_ELEMENT_PATH + "/Data/m13mp18.fasta";
	}

	std::string s = SEAdenitaCoreSEApp::readScaffoldFilename(filename);

	// get selected part
	auto parts = GetNanorobot()->GetSelectedParts();

	if (parts.size() == 0) {
		SAMSON::informUser(QString("Adenita: Set Scaffold"), QString("Please select the component whose scaffold sequence you want to set."));
	}

	SB_FOR(ADNPointer<ADNPart> part, parts) {
		auto scafs = part->GetScaffolds();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, scafs) {
			ADNBasicOperations::SetSingleStrandSequence(ss, s);
		}
	}

}

void SEAdenitaCoreSEApp::ExportToOxDNA(QString folder, ADNAuxiliary::OxDNAOptions options, CollectionMap<ADNPart> parts) {

	SAMSON::setStatusMessage(QString("Exporting to oxDNA..."));
	if (parts.size() > 0) {
		ADNLoader::OutputToOxDNA(parts, folder.toStdString(), options);
	}
	SAMSON::setStatusMessage(QString("Done export to oxDNA."));

}

void SEAdenitaCoreSEApp::AddNtThreeP(int numNt) {

	auto nts = GetNanorobot()->GetSelectedNucleotides();
	if (nts.size() == 1) {

		ADNPointer<ADNNucleotide> nt = nts[0];
		auto ss = nt->GetStrand();
		auto part = ss->GetPart();
		SBVector3 dir = ADNAuxiliary::UblasVectorToSBVector(nt->GetBaseSegment()->GetE3());

		auto nts = ADNBasicOperations::AddNucleotidesThreePrime(part, ss, numNt, dir);
		DASBackToTheAtom* btta = new DASBackToTheAtom();
		btta->SetPositionsForNewNucleotides(part, nts);
		
		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SEAdenitaCoreSEApp::CenterPart() {

	auto parts = GetNanorobot()->GetSelectedParts();
	SB_FOR(ADNPointer<ADNPart> part, parts) ADNBasicOperations::CenterPart(part);

}

void SEAdenitaCoreSEApp::GenerateSequence(double gcCont, int maxContGs, bool overwrite) {

	auto strands = GetNanorobot()->GetSelectedSingleStrands();

	if (strands.size() == 0) {
		SAMSON::informUser(QString("Adenita: Set random sequence"), QString("Please select the single strands whose sequence you want to set."));
	}

	SB_FOR(ADNPointer<ADNSingleStrand> ss, strands) {
		std::string seq = DASAlgorithms::GenerateSequence(gcCont, maxContGs, ss->getNumberOfNucleotides());
		ADNBasicOperations::SetSingleStrandSequence(ss, seq, true, overwrite);
	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::requestVisualModelUpdate() {

	SEAdenitaVisualModel* adenitaVisualModel = SEAdenitaCoreSEApp::getVisualModel();

	if (adenitaVisualModel) {

		adenitaVisualModel->requestUpdate();

	}
	else {

		//SBProxy* visualModelProxy = SAMSON::getProxy("SEAdenitaVisualModel", SBUUID(SB_ELEMENT_UUID));
		SEAdenitaVisualModel* newVisualModel = new SEAdenitaVisualModel();// visualModelProxy->createInstance();
		if (SAMSON::isHolding()) SAMSON::hold(newVisualModel);
		newVisualModel->create();
		SAMSON::getActiveDocument()->addChild(newVisualModel);

		ADNLogger::LogDebug(std::string("Adding visual model"));

	}

}

void SEAdenitaCoreSEApp::resetVisualModel() {

	//create visual model per nanorobot

	SEAdenitaVisualModel* adenitaVisualModel = SEAdenitaCoreSEApp::getVisualModel();

	if (adenitaVisualModel) {

		adenitaVisualModel->update();
		SAMSON::requestViewportUpdate();

	}
	else {

		//SBProxy* visualModelProxy = SAMSON::getProxy("SEAdenitaVisualModel", SBUUID(SB_ELEMENT_UUID));
		SEAdenitaVisualModel* newVisualModel = new SEAdenitaVisualModel();// visualModelProxy->createInstance();
		if (SAMSON::isHolding()) SAMSON::hold(newVisualModel);
		newVisualModel->create();
		SAMSON::getActiveDocument()->addChild(newVisualModel);

	}

	ADNLogger::LogDebug(std::string("Restarting visual model"));

}

SEAdenitaVisualModel* SEAdenitaCoreSEApp::getVisualModel() {

	SBNodeIndexer nodeIndexer;
	SAMSON::getActiveDocument()->getNodes(nodeIndexer, SBNode::VisualModel);
  
	SBVisualModel* visualModel = nullptr;
	SEAdenitaVisualModel* adenitaVisualModel = nullptr;

	SB_FOR(SBNode* node, nodeIndexer) {

		if (node->getType() == SBNode::VisualModel) {

			visualModel = static_cast<SBVisualModel*>(node);
			
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

	ADNPointer<ADNNucleotide> breakNucleotide = nullptr;
	auto nucleotides = GetNanorobot()->GetHighlightedNucleotides();
	if (nucleotides.size() == 1) {

		ADNPointer<ADNNucleotide> nucleotide = nucleotides[0];
		if (nucleotide->getEndType() != ADNNucleotide::EndType::ThreePrime) {

			ADNPointer<ADNSingleStrand> singleStrand = nucleotide->GetStrand();
			const bool circular = singleStrand->IsCircular();

			ADNPointer<ADNPart> part = singleStrand->GetPart();

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

void SEAdenitaCoreSEApp::TwistDoubleHelix(CollectionMap<ADNDoubleStrand> dss, double angle) {

	DASBackToTheAtom btta = DASBackToTheAtom();
	SEConfig& config = SEConfig::GetInstance();

	SB_FOR(ADNPointer<ADNDoubleStrand> ds, dss) {

		double newDeg = ds->GetInitialTwistAngle() + angle;
		ADNBasicOperations::TwistDoubleHelix(ds, newDeg);
		// recalculate positions
		btta.SetDoubleStrandPositions(ds);
		if (config.use_atomic_details) {
			// todo: calculate all atoms just for a double strand
			//btta.GenerateAllAtomModel(ds);
		}

	}

	if (dss.size() > 0) SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::LinearCatenanes(SBQuantity::length radius, SBPosition3 center, SBVector3 normal, int num) {

	auto part = DASCreator::CreateLinearCatenanes(radius, center, normal, num);
	addPartToDocument(part);
	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::Kinetoplast(SBQuantity::length radius, SBPosition3 center, SBVector3 normal, int rows, int cols) {

	auto part = DASCreator::CreateHexagonalCatenanes(radius, center, normal, rows, cols);
	addPartToDocument(part);
	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::setStartNucleotide() {

	auto nucleotides = GetNanorobot()->GetSelectedNucleotides();
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

void SEAdenitaCoreSEApp::MergeComponents(ADNPointer<ADNPart> p1, ADNPointer<ADNPart> p2) {

	ADNPointer<ADNPart> newPart = ADNBasicOperations::MergeParts(p1, p2);
	
	GetNanorobot()->DeregisterPart(p2);
	if (p2->getParent()) p2->getParent()->removeChild(p2());
	p1 = newPart;

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::MoveDoubleStrand(ADNPointer<ADNDoubleStrand> ds, ADNPointer<ADNPart> p) {

	ADNPointer<ADNPart> oldPart = ds->GetPart();
	if (oldPart != p) ADNBasicOperations::MoveStrand(oldPart, p, ds);

}

void SEAdenitaCoreSEApp::MoveSingleStrand(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNPart> p) {

	ADNPointer<ADNPart> oldPart = ss->GetPart();
	if (oldPart != p) ADNBasicOperations::MoveStrand(oldPart, p, ss);

}

bool SEAdenitaCoreSEApp::CalculateBindingRegions(int oligoConc, int monovalentConc, int divalentConc) {

	bool res = false;
	auto parts = GetNanorobot()->GetSelectedParts();

	if (parts.size() == 0) {

		SAMSON::informUser(QString("Adenita: Calculate Thermodynamic Properties"), QString("The selection is empty. Please select one or more components."));
		return false;

	}

	SB_FOR(ADNPointer<ADNPart> part, parts) {

		PIPrimer3& p = PIPrimer3::GetInstance();
		p.UpdateBindingRegions(part);
		p.Calculate(part, oligoConc, monovalentConc, divalentConc);
		res = true;

	}

	return res;

}

void SEAdenitaCoreSEApp::TwistDoubleHelix() {

	double deg = ADNConstants::BP_ROT;
	auto dss = GetNanorobot()->GetSelectedDoubleStrands();

	DASBackToTheAtom btta = DASBackToTheAtom();
	SEConfig& config = SEConfig::GetInstance();

	SB_FOR(ADNPointer<ADNDoubleStrand> ds, dss) {

		double newDeg = ds->GetInitialTwistAngle() + deg;
		ADNBasicOperations::TwistDoubleHelix(ds, newDeg);
		// recalculate positions
		btta.SetDoubleStrandPositions(ds);
		if (config.use_atomic_details) {
			// todo: calculate all atoms just for a double strand
			//btta.GenerateAllAtomModel(ds);
		}

	}

}

void SEAdenitaCoreSEApp::TestNeighbors() {

	// get selected nucleotide and part
	auto nts = GetNanorobot()->GetSelectedNucleotides();
	if (nts.size() == 0) return;

	ADNPointer<ADNNucleotide> nt = nts[0];
	ADNPointer<ADNPart> part = nt->GetStrand()->GetPart();
	// create neighbor list
	SEConfig& config = SEConfig::GetInstance();
	auto neighbors = ADNNeighbors();
	neighbors.SetMaxCutOff(SBQuantity::nanometer(config.debugOptions.maxCutOff));
	neighbors.SetMinCutOff(SBQuantity::nanometer(config.debugOptions.minCutOff));
	neighbors.SetIncludePairs(true);
	neighbors.InitializeNeighbors(part);

	// highlight neighbors of selected nucleotide
	auto ntNeighbors = neighbors.GetNeighbors(nt);
	SB_FOR(ADNPointer<ADNNucleotide> ntN, ntNeighbors) ntN->setSelectionFlag(true);

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::ImportFromOxDNA(std::string topoFile, std::string configFile) {

	auto res = ADNLoader::InputFromOxDNA(topoFile, configFile);
	if (!res.first) {

		ADNPointer<ADNPart> p = res.second;
		addPartToDocument(p, true);
		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SEAdenitaCoreSEApp::FromDatagraph() {

	SBNodeIndexer nodeIndexer;
	SAMSON::getActiveDocument()->getNodes(nodeIndexer, SBNode::StructuralModel);

	SB_FOR(auto node, nodeIndexer) {

		if (node->isSelected()) {

			ADNPointer<ADNPart> part = ADNLoader::GenerateModelFromDatagraph(node);
			addPartToDocument(part, true);

		}

	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::HighlightXOs() {

	auto parts = GetNanorobot()->GetParts();

	SB_FOR(ADNPointer<ADNPart> p, parts) PICrossovers::GetCrossovers(p);

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::HighlightPosXOs() {

	auto parts = GetNanorobot()->GetParts();

	SB_FOR(ADNPointer<ADNPart> p, parts) PICrossovers::GetPossibleCrossovers(p);

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::ExportToCanDo(QString filename) {

	SBNodeIndexer nodeIndexer;
	SAMSON::getActiveDocument()->getNodes(nodeIndexer, SBNode::StructuralModel);

	CollectionMap<ADNPart> parts;

	SB_FOR(auto node, nodeIndexer) {

		if (node->isSelected()) {

			ADNPointer<ADNPart> part = static_cast<ADNPart*>(node);
			parts.addReferenceTarget(part());

		}

	}

	if (nodeIndexer.size() == 1) {

		ADNPointer<ADNPart> part = parts[0];
		ADNLoader::OutputToCanDo(part, filename.toStdString());

	}
	else {

		auto nanorobot = GetNanorobot();
		ADNLoader::OutputToCanDo(nanorobot, filename.toStdString());

	}
  
}

void SEAdenitaCoreSEApp::FixDesigns() {

	SBNodeIndexer nodeIndexer;
	SAMSON::getActiveDocument()->getNodes(nodeIndexer, SBNode::StructuralModel);

	CollectionMap<ADNPart> parts;

	SB_FOR(auto node, nodeIndexer) {

		if (!node->isSelected()) continue;

		ADNPointer<ADNPart> part = static_cast<ADNPart*>(node);

		// .sam format fix
		//auto nucleotides = part->GetNucleotides();
		//SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {
		//  auto bbPos = nt->GetBackbonePosition();
		//  auto scPos = nt->GetSidechainPosition();
		//  auto pos = (bbPos + scPos)*0.5;

		//  auto at = nt->GetCenterAtom();
		//  if (at == nullptr) at = new ADNAtom();
		//  at->setElementType(SBElement::Meitnerium);
		//  nt->SetCenterAtom(at);

		//  nt->SetPosition(pos);
		//  ublas::vector<double> e(3, 0.0);
		//  nt->SetE3(e);
		//  nt->SetE2(e);
		//  nt->SetE1(e);
		//}

		//auto baseSegments = part->GetBaseSegments();
		//SB_FOR(ADNPointer<ADNBaseSegment> bs, baseSegments) {
		//  auto nucleotides = bs->GetNucleotides();
		//  SBPosition3 pos;
		//  SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {
		//    pos += nt->GetBackbonePosition();
		//    pos += nt->GetSidechainPosition();
		//  }
		//  pos /= nucleotides.size()*2;

		//  auto at = bs->GetCenterAtom();
		//  if (at == nullptr) at = new ADNAtom();
		//  at->setElementType(SBElement::Meitnerium);
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
		SB_FOR(ADNPointer<ADNSingleStrand> ss, strands) {

			ADNPointer<ADNSingleStrand> newSs = new ADNSingleStrand();
			newSs->setName(ss->getName());
			newSs->IsScaffold(ss->IsScaffold());
			if (SAMSON::isHolding()) SAMSON::hold(newSs());
			newSs->create();
			part->RegisterSingleStrand(newSs);
			// reverse 5'->3' direction
			ADNPointer<ADNNucleotide> nt = ss->GetFivePrime();
			while (nt != nullptr) {

				auto next = nt->GetNext();
				part->DeregisterNucleotide(nt, true, false);
				part->RegisterNucleotideFivePrime(newSs, nt);
				nt = next;

			}

			part->DeregisterSingleStrand(ss);

		}
    
	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEAdenitaCoreSEApp::CreateBasePair() {

	auto selectedNucleotides = GetNanorobot()->GetSelectedNucleotides();
	if (selectedNucleotides.size() > 0) {

		DASOperations::AddComplementaryStrands(GetNanorobot(), selectedNucleotides);
		SEAdenitaCoreSEApp::resetVisualModel();

	}
	else {

		SAMSON::informUser(QString("Adenita: Create base pair"), QString("Please select the nucleotides or single strands whose pairs you want to create."));

	}

}

void SEAdenitaCoreSEApp::onDocumentEvent(SBDocumentEvent* documentEvent) {

	const SBDocumentEvent::Type eventType = documentEvent->getType();
	SBNode* node = documentEvent->getAuxiliaryNode();
	if (!node) return;
	if (node->getProxy()->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return;

#if 0
	// is handled in the Adenita Visual Model
	// handle addition and deletion of ADN nodes for updating the Adenita Visual Model

	if (eventType == SBDocumentEvent::StructuralModelAdded || eventType == SBDocumentEvent::StructuralModelRemoved) {

		if (node->getProxy()->getName() == "ADNPart")
			requestVisualModelUpdate();

	}
#endif

	if (mod_) return; // modifications handle themselves deletions

	// handle deletion of ADN nodes for bookkeeping used in ADN nodes

	if (eventType == SBDocumentEvent::StructuralModelRemoved) {

		// on delete a registered ADNPart

		if (node->getProxy()->getName() == "ADNPart") {

			ADNPointer<ADNPart> part = dynamic_cast<ADNPart*>(node);
			if (part != nullptr)
				GetNanorobot()->DeregisterPart(part);

		}

	}

}

void SEAdenitaCoreSEApp::onStructuralEvent(SBStructuralEvent* structuralEvent) {

	const SBStructuralEvent::Type eventType = structuralEvent->getType();
	SBNode* node = structuralEvent->getAuxiliaryNode();
	if (!node) return;
	if (node->getProxy()->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return;

	const std::string nodeClassName = node->getProxy()->getName();

#if 0
	// is handled in the Adenita Visual Model
	// handle additiona and deletion of ADN nodes for updating the Adenita Visual Model

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

		ADNPointer<ADNSingleStrand> ss = dynamic_cast<ADNSingleStrand*>(node);
		if (ss != nullptr) {

			auto part = static_cast<ADNPart*>(structuralEvent->getSender()->getParent());
			part->DeregisterSingleStrand(ss, false);

		}

	}
	else if (eventType == SBStructuralEvent::ResidueRemoved) {

		auto node = structuralEvent->getAuxiliaryNode();
		ADNPointer<ADNNucleotide> nt = dynamic_cast<ADNNucleotide*>(node);
		if (nt != nullptr) {

			ADNPointer<ADNSingleStrand> ss = static_cast<ADNSingleStrand*>(structuralEvent->getSender());
			if (ss != nullptr) {

				auto part = ss->GetPart();
				if (part != nullptr) part->DeregisterNucleotide(nt, false, true, true);
				if (ss->getNumberOfNucleotides() == 0) ss->erase();

			}

		}

	}
	else if (eventType == SBStructuralEvent::StructuralGroupRemoved) {

		auto node = structuralEvent->getAuxiliaryNode();
		ADNPointer<ADNBaseSegment> bs = dynamic_cast<ADNBaseSegment*>(node);
		if (bs != nullptr) {

			auto nucleotides = bs->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) if (nt != nullptr) nt->erase();
			
			ADNPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(structuralEvent->getSender());
			if (ds != nullptr) {

				auto part = ds->GetPart();
				if (part != nullptr) part->DeregisterBaseSegment(bs, false, true);

			}

		}
		else {

			ADNPointer<ADNDoubleStrand> ds = dynamic_cast<ADNDoubleStrand*>(node);
			if (ds != nullptr) {

				ADNPointer<ADNPart> part = static_cast<ADNPart*>(structuralEvent->getSender()->getParent());
				if (part != nullptr) part->DeregisterDoubleStrand(ds, false, true);

			}

		}   

	}

}

void SEAdenitaCoreSEApp::ConnectToDocument(SBDocument* doc) {

	if (doc->documentSignalIsConnectedToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onDocumentEvent)) == false)
		doc->connectDocumentSignalToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onDocumentEvent));

}

void SEAdenitaCoreSEApp::ConnectToDocument() {

	ConnectToDocument(SAMSON::getActiveDocument());
	
}

ADNNanorobot* SEAdenitaCoreSEApp::GetNanorobot() {

	return getNanorobot(SAMSON::getActiveDocument());

}

ADNNanorobot* SEAdenitaCoreSEApp::getNanorobot(SBDocument * document) {

	ADNNanorobot* nanorobot = nullptr;

	if (nanorobotMap.find(document) == nanorobotMap.end()) {

		// create new nanorobot for this document
		nanorobot = new ADNNanorobot();
		nanorobotMap.insert(std::make_pair(document, nanorobot));

	}
	else {

		nanorobot = nanorobotMap.at(document);

	}

	return nanorobot;

}

std::string SEAdenitaCoreSEApp::readScaffoldFilename(std::string filename) {

	std::string seq = "";
	if (filename.size() > 0) {

		std::vector<std::string> lines;
		SBIFileReader::getFileLines(filename, lines);

		for (unsigned int i = 1; i < lines.size(); i++) {

			std::string line = lines[i];
			if (line[0] != '>')
				seq.append(line);

		}

	}

	return seq;

}

QStringList SEAdenitaCoreSEApp::getListOfPartNames() {

	QStringList names;

	auto parts = GetNanorobot()->GetParts();
	SB_FOR(ADNPointer<ADNPart> p, parts)
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

	auto highlightedBaseSegments = nanorobot->GetHighlightedBaseSegments();
	auto highlightedBaseSegmentsFromNucleotides = nanorobot->GetHighlightedBaseSegmentsFromNucleotides();
	auto highlightedAtoms = nanorobot->GetHighlightedAtoms();

	if (highlightedAtoms.size() == 1)
		snappedPosition = highlightedAtoms[0]->getPosition();
	else if (highlightedBaseSegments.size() == 1)
		snappedPosition = highlightedBaseSegments[0]->GetPosition();
	else if (highlightedBaseSegmentsFromNucleotides.size() == 1)
		snappedPosition = highlightedBaseSegmentsFromNucleotides[0]->GetPosition();

	return snappedPosition;

}

void SEAdenitaCoreSEApp::addPartToDocument(ADNPointer<ADNPart> part, bool positionsData, SBFolder* preferredFolder) {

	if (part == nullptr) return;

	SEConfig& config = SEConfig::GetInstance();
	if (config.auto_set_scaffold_sequence) {

		std::string fname = SEAdenitaCoreSEAppGUI::getScaffoldFilename();
		std::string seq = SEAdenitaCoreSEApp::readScaffoldFilename(fname);
		auto scafs = part->GetScaffolds();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, scafs) {

			ADNBasicOperations::SetSingleStrandSequence(ss, seq);

		}

	}

	DASBackToTheAtom btta = DASBackToTheAtom();
	btta.PopulateWithMockAtoms(part, positionsData);
	if (!positionsData) {

		btta.SetNucleotidesPostions(part);
		if (config.use_atomic_details) {

			btta.GenerateAllAtomModel(part);

		}
		//btta.CheckDistances(part);

	}

	part->ResetBoundingBox();
	GetNanorobot()->RegisterPart(part);

	//events
	ConnectStructuralSignalSlots(part);

	//bool holding = SAMSON::isHolding();
	//if (!holding) SAMSON::beginHolding("Add model");
	if (SAMSON::isHolding()) SAMSON::hold(part());
	part->create();
	
	if (preferredFolder) preferredFolder->addChild(part());
	else SAMSON::getActiveDocument()->addChild(part());

	//if (!holding) SAMSON::endHolding();

}

void SEAdenitaCoreSEApp::addConformationToDocument(ADNPointer<ADNConformation> conf, SBFolder* preferredFolder) {

	if (conf == nullptr) return;

	GetNanorobot()->RegisterConformation(conf);

	//bool holding = SAMSON::isHolding();
	//if (!holding) SAMSON::beginHolding("Add conformation");
	if (SAMSON::isHolding()) SAMSON::hold(conf());
	conf->create();

	if (preferredFolder) preferredFolder->addChild(conf());
	else SAMSON::getActiveDocument()->addChild(conf());

	//if (!holding) SAMSON::endHolding();

}

void SEAdenitaCoreSEApp::AddLoadedPartToNanorobot(ADNPointer<ADNPart> part) {

	if (part->isLoadedViaSAMSON()) {

		GetNanorobot()->RegisterPart(part);

		//events
		ConnectStructuralSignalSlots(part);

		part->setLoadedViaSAMSON(false);

		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SEAdenitaCoreSEApp::ConnectStructuralSignalSlots(ADNPointer<ADNPart> part) {

	part->connectStructuralSignalToSlot(this, SB_SLOT(&SEAdenitaCoreSEApp::onStructuralEvent));

}

void SEAdenitaCoreSEApp::keyPressEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_0) {
		SAMSON::requestViewportUpdate();
	}

}

void SEAdenitaCoreSEApp::SetMod(bool m) { mod_ = m; }

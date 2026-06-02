#include "SEBreakEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "ADNSamsonContext.hpp"
#include "ADNConfig.hpp"

#include "SAMSON.hpp"

#include <array>


SEBreakEditor::SEBreakEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SEBreakEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SEBreakEditor::~SEBreakEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SEBreakEditorGUI* SEBreakEditor::getPropertyWidget() const { return static_cast<SEBreakEditorGUI*>(propertyWidget); }

void SEBreakEditor::setFivePrimeModeFlag(bool fivePrimeModeFlag) {

	this->fivePrimeModeFlag = fivePrimeModeFlag;
	SAMSON::requestViewportUpdate();

}

void SEBreakEditor::updateCursor() {

	const double currentViewportPixelRatio = SAMSON::getViewportPixelRatio();
	if (viewportPixelRatio_ != currentViewportPixelRatio) {

		viewportPixelRatio_ = currentViewportPixelRatio;
		breakCursor_ = SAMSON::makeViewportCursor(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/break.png"), 32, 0, 0);

	}

}

void SEBreakEditor::restoreCursor() {

	updateCursor();
	SAMSON::setViewportCursor(breakCursor_);

}

void SEBreakEditor::setCursor(QMouseEvent* event) {

	updateCursor();

	const bool hasCameraButton =
		(event->buttons() & Qt::MiddleButton) ||
		(event->buttons() & Qt::RightButton);

	if (!hasCameraButton) SAMSON::setViewportCursor(breakCursor_);

}

void SEBreakEditor::setCursor(QKeyEvent*) {

	restoreCursor();

}

SEBreakEditor::BreakTarget SEBreakEditor::getBreakTarget(SBPointer<ADNNucleotide> nucleotide) const {

	BreakTarget target;
	target.clickedNucleotide = nucleotide;

	if (nucleotide == nullptr) return target;

	target.singleStrand = nucleotide->GetStrand();
	if (target.singleStrand == nullptr) return target;

	const SBPointer<ADNNucleotide> previousNucleotide = nucleotide->GetPrev();
	const SBPointer<ADNNucleotide> nextNucleotide = nucleotide->GetNext();

	if (target.singleStrand->getNumberOfNucleotides() == 1) return target;
	if (nucleotide->isEndTypeNucleotide()) return target;
	if (previousNucleotide == nullptr || nextNucleotide == nullptr) return target;

	if (fivePrimeModeFlag) {

		target.firstNucleotide = previousNucleotide;
		target.secondNucleotide = nucleotide;

	}
	else {

		target.firstNucleotide = nucleotide;
		target.secondNucleotide = nextNucleotide;

	}

	target.valid = target.firstNucleotide != nullptr && target.secondNucleotide != nullptr;
	return target;

}

SBCContainerUUID SEBreakEditor::getUUID() const { return SBCContainerUUID("2FACBF90-F7E2-AFCB-5E37-AA86763DDBC2"); }

QString SEBreakEditor::getName() const { 

	// SAMSON Element generator pro tip: this name should not be changed

	return "SEBreakEditor"; 

}

QPixmap SEBreakEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/break.png"));

}

int SEBreakEditor::getFormat() const {
  
	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SEBreakEditor::getShortcut() const { 
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SEBreakEditor::getToolTip() const { 
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Break a single DNA strand at the highlighted bond. Choose whether the cut is before or after the clicked nucleotide.");

}

void SEBreakEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEBreakEditor::saveSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEBreakEditor::getDescription() const { return QObject::tr("Adenita | Break Single Strand DNA"); }

void SEBreakEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	previousSelectionFilter = SAMSON::getActiveSelectionFilterName();
	SAMSON::setActiveSelectionFilterByName("Any node");
	restoreCursor();

}

void SEBreakEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	if (SAMSON::getActiveSelectionFilterName() == "Any node")
		SAMSON::setActiveSelectionFilterByName(previousSelectionFilter);

	SAMSON::unsetViewportCursor();

}

void SEBreakEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEBreakEditor::display(SBNode::RenderingPass renderingPass) {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	if (renderingPass != SBNode::RenderingPass::OpaqueGeometry && renderingPass != SBNode::RenderingPass::TransparentGeometry) return;

	auto app = SEAdenitaCoreSEApp::getAdenitaApp();
	if (app == nullptr) return;
	ADNNanorobot* nanorobot = app->GetNanorobot();
	if (nanorobot == nullptr) return;

	auto highlightedNucleotides = nanorobot->GetHighlightedNucleotides();
	if (highlightedNucleotides.size() != 1) return;

	const BreakTarget target = getBreakTarget(highlightedNucleotides[0]);
	if (!target.valid) return;

	const SEConfig& config = SEConfig::GetInstance();

	const SBPosition3 firstPosition = target.firstNucleotide->GetBackbonePosition();
	const SBPosition3 secondPosition = target.secondNucleotide->GetBackbonePosition();

	std::array<float, 6> positionData = {
		static_cast<float>(firstPosition[0].getValue()),
		static_cast<float>(firstPosition[1].getValue()),
		static_cast<float>(firstPosition[2].getValue()),
		static_cast<float>(secondPosition[0].getValue()),
		static_cast<float>(secondPosition[1].getValue()),
		static_cast<float>(secondPosition[2].getValue())
	};

	const float previewSphereRadius = 1.5f * config.nucleotide_V_radius;
	const float previewCylinderRadius = std::max(1.35f * config.nucleotide_V_radius, 80.0f);

	const unsigned int nCylinders = 1;
	const unsigned int nPositions = 2;
	std::array<unsigned int, 2> cylinderIndexData = { 0, 1 };
	std::array<float, 2> cylinderRadiusData = { previewCylinderRadius, previewCylinderRadius };
	std::array<float, 2> sphereRadiusData = { previewSphereRadius, previewSphereRadius };
	std::array<unsigned int, 2> capData = { 1, 1 };
	std::array<unsigned int, 2> flagData = { 0, 0 };
	std::array<float, 8> colorData = {
		1.0f, 0.95f, 0.15f, 1.0f,
		1.0f, 0.55f, 0.05f, 1.0f
	};

	SAMSON::displayCylinders(
		nCylinders,
		nPositions,
		cylinderIndexData.data(),
		positionData.data(),
		cylinderRadiusData.data(),
		capData.data(),
		colorData.data(),
		flagData.data());

	SAMSON::displaySpheres(
		nPositions,
		positionData.data(),
		sphereRadiusData.data(),
		colorData.data(),
		flagData.data());

}

void SEBreakEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool isLeftButton = event->button() & Qt::LeftButton;
	if (!isLeftButton) {

		setCursor(event);
		return;

	}

	setCursor(event);
	event->accept();

	auto app = SEAdenitaCoreSEApp::getAdenitaApp();
	if (app == nullptr) return;
	ADNNanorobot* nanorobot = app->GetNanorobot();
	if (nanorobot == nullptr) return;

	auto highlightedNucleotides = nanorobot->GetHighlightedNucleotides();
	auto numberOfHighlightedNucleotides = highlightedNucleotides.size();

	if (numberOfHighlightedNucleotides == 1) {

		// Skip the following cases:
		// 1. the nucleotide is not in a single strand
		// 2. the nucleotide is the only nucleotide in the single strand
		// 3. the nucleotide is the end nucleotide, or there is no next or previous nucleotide 

		auto highlightedNucleotide = highlightedNucleotides[0];
		const BreakTarget target = getBreakTarget(highlightedNucleotide);
		auto singleStrand = target.singleStrand;

		// clear the current selection

		SBDocument* document = ADNSamsonContext::GetActiveDocument(__func__);
		if (document == nullptr) return;
		document->clearSelection();

		// select the nucleotide
		//highlightedNucleotide->setSelectionFlag(true);

		if (singleStrand == nullptr) {
			SAMSON::informUser("Adenita - Break editor", "The nucleotide is not in any single strand - cannot break it.");
		}
		else if (singleStrand->getNumberOfNucleotides() == 1) {
			SAMSON::informUser("Adenita - Break editor", "Cannot break a single strand that contains only one nucleotide. If you want to delete it use the Delete editor.");
		}
		else if (!target.valid) {
			SAMSON::informUser("Adenita - Break editor", "The nucleotide is the end nucleotide - cannot break here. If you want to delete this nucleotide use the Delete editor.");
		}
		else {

			app->BreakSingleStrand(fivePrimeModeFlag);

		}

	}

	setCursor(event);

}

void SEBreakEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->button() & Qt::LeftButton) {

		setCursor(event);
		event->accept();
		return;

	}

	setCursor(event);

}

void SEBreakEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	setCursor(event);
	SAMSON::requestViewportUpdate();

	const bool hasCameraButton =
		(event->buttons() & Qt::MiddleButton) ||
		(event->buttons() & Qt::RightButton);

	if (!hasCameraButton) event->accept();

}

void SEBreakEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	setCursor(event);
	if (event->button() & Qt::LeftButton) event->accept();

}

void SEBreakEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEBreakEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	setCursor(event);

}

void SEBreakEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	setCursor(event);

}

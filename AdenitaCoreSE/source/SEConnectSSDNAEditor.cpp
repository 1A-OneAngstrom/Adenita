#include "SEConnectSSDNAEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "SAMSON.hpp"


SEConnectSSDNAEditor::SEConnectSSDNAEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SEConnectSSDNAEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SEConnectSSDNAEditor::~SEConnectSSDNAEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.
  
	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SEConnectSSDNAEditorGUI* SEConnectSSDNAEditor::getPropertyWidget() const { return static_cast<SEConnectSSDNAEditorGUI*>(propertyWidget); }

void SEConnectSSDNAEditor::setConnectionMode(bool xo) {
	
	if (xo) connectionMode = ConnectionMode::Single;
	else connectionMode = ConnectionMode::Double;

}

void SEConnectSSDNAEditor::setSequence(std::string seq) {

	sequence = seq;

}

void SEConnectSSDNAEditor::setAutoSequenceFlag(bool s) {

	autoSequenceFlag = s;

}

void SEConnectSSDNAEditor::setConcatFlag(bool c) {

	concatFlag = c;

}

SBCContainerUUID SEConnectSSDNAEditor::getUUID() const { return SBCContainerUUID("0854A585-E146-954F-616C-B4532A1B2555"); }

QString SEConnectSSDNAEditor::getName() const { 

	// SAMSON Element generator pro tip: this name should not be changed

	return "SEConnectSSDNAEditor"; 

}

QPixmap SEConnectSSDNAEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/connectSS.png"));

}

int SEConnectSSDNAEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SEConnectSSDNAEditor::getShortcut() const { 
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SEConnectSSDNAEditor::getToolTip() const { 
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Connect and create crossovers"); 

}

void SEConnectSSDNAEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEConnectSSDNAEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEConnectSSDNAEditor::getDescription() const {

	return QObject::tr("Adenita | Connect DNA strands");

}

void SEConnectSSDNAEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/cursor_connectSS.png");
	SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

	displayFlag = false;
	start_ = nullptr;

	previousSelectionFilter = SAMSON::getCurrentSelectionFilter();
	SAMSON::setCurrentSelectionFilter("Any node");
  
}

void SEConnectSSDNAEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	displayFlag = false;
	start_ = nullptr;

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	if (SAMSON::getCurrentSelectionFilter() == "Any node")
		SAMSON::setCurrentSelectionFilter(previousSelectionFilter);

	SAMSON::unsetViewportCursor();

}

void SEConnectSSDNAEditor::getActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEConnectSSDNAEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	if (displayFlag && start_.isValid()) {

		SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());
    
		//check if a nucleotide got selected
		auto app = SEAdenitaCoreSEApp::getAdenitaApp();
		auto nanorobot = app->GetNanorobot();

		auto highlightedNucleotides = nanorobot->GetHighlightedNucleotides();
    
		if (highlightedNucleotides.size() == 1)
			currentPosition = highlightedNucleotides[0]->GetBackbone()->GetPosition();
    
		ADNDisplayHelper::displayCylinder(start_->GetBackbone()->GetPosition(), currentPosition);

	}

}

void SEConnectSSDNAEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEConnectSSDNAEditor::displayInterface() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to display the editor 2D interface in viewports. 
	// Implement this function if your editor displays a 2D user interface. For example, a rectangle selection editor would display a 2D rectangle in the active viewport. 
	// You may use utility functions provided by SAMSON (e.g. displayLinesOrtho and displayTrianglesOrtho).

}

void SEConnectSSDNAEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->buttons() == Qt::LeftButton && !displayFlag) {

		//check if a nucleotide got selected

		auto nanorobot = SEAdenitaCoreSEApp::getAdenitaApp()->GetNanorobot();

		// deselect nodes

		//auto selectedNucleotides = nanorobot->GetSelectedNucleotides();
		//SB_FOR(auto node, selectedNucleotides) node->setSelectionFlag(false);

		SAMSON::getActiveDocument()->clearSelection();

		// consider the single highlighted nucleotide as the selected one and remember it

		auto highlightedNucleotides = nanorobot->GetHighlightedNucleotides();

		if (highlightedNucleotides.size() == 1) {

			auto nucleotide = highlightedNucleotides[0];
			nucleotide->setHighlightingFlag(false);
			nucleotide->setSelectionFlag(true);
			start_ = nucleotide;
			displayFlag = true;

			// TODO: connect start_ to base event

			event->accept();

		}

	}

}

void SEConnectSSDNAEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (!displayFlag) return;
  
	if (start_.isValid() && event->button() == Qt::LeftButton) {

		displayFlag = false;

		auto app = SEAdenitaCoreSEApp::getAdenitaApp();
		auto nanorobot = app->GetNanorobot();

		auto highlightedNucleotides = nanorobot->GetHighlightedNucleotides();

		// consider the highlighted nucleotide as the end one
    
		if (highlightedNucleotides.size() == 1 && start_.isValid()) {

			auto startNulecotide = start_;
			ADNPointer<ADNNucleotide> endNulecotide = highlightedNucleotides[0];
			ADNPointer<ADNPart> part1 = nanorobot->GetPart(startNulecotide->GetStrand());
			ADNPointer<ADNPart> part2 = nanorobot->GetPart(endNulecotide->GetStrand());

			if (startNulecotide->GetStrand() == endNulecotide->GetStrand() && !startNulecotide->IsEnd() && !endNulecotide->IsEnd()) {

				SAMSON::informUser("Adenita - Connect editor", "Cannot connect these two nucleotides.\nOne of the nucleotides should be the end nucleotide or both nucleotides should be from different strands.");

			}
			else {

				bool two = (connectionMode == ConnectionMode::Double);

				std::string seq = "";
				if (concatFlag) {

					if (!autoSequenceFlag) {

						seq = sequence;

					}
					else {

						auto dist = (endNulecotide->GetBaseSegment()->GetPosition() - startNulecotide->GetBaseSegment()->GetPosition()).norm();
						const int length = round((dist / SBQuantity::nanometer(ADNConstants::BP_RISE)).getValue()) - 6;

						for (int i = 0; i < length; ++i)
							seq += "N";

					}

				}

				app->SetMod(true);
				DASOperations::CreateCrossover(part1, part2, startNulecotide, endNulecotide, two, seq);
				SEAdenitaCoreSEApp::resetVisualModel();
				app->SetMod(false);

			}

		}

		start_ = nullptr;

		event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SEConnectSSDNAEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (displayFlag) {

		//if (event->button() == Qt::LeftButton)
			event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SEConnectSSDNAEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEConnectSSDNAEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEConnectSSDNAEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEConnectSSDNAEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEConnectSSDNAEditor::onBaseEvent(SBBaseEvent* baseEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle base events

}

void SEConnectSSDNAEditor::onDocumentEvent(SBDocumentEvent* documentEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle document events 

}

void SEConnectSSDNAEditor::onDynamicalEvent(SBDynamicalEvent* dynamicalEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle dynamical events 

}

void SEConnectSSDNAEditor::onStructuralEvent(SBStructuralEvent* documentEvent) {
	
	// SAMSON Element generator pro tip: implement this function if you need to handle structural events

}

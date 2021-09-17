#include "SEBreakEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "SAMSON.hpp"


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

	return QObject::tr("Break ssDNA"); 

}

void SEBreakEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEBreakEditor::saveSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEBreakEditor::getDescription() const { return QObject::tr("Adenita | Break ssDNA"); }

void SEBreakEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/break.png");
	SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

	previousSelectionFilter = SAMSON::getCurrentSelectionFilter();
	SAMSON::setCurrentSelectionFilter("Any node");

}

void SEBreakEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	if (SAMSON::getCurrentSelectionFilter() == "Any node")
		SAMSON::setCurrentSelectionFilter(previousSelectionFilter);

	SAMSON::unsetViewportCursor();

}

void SEBreakEditor::getActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEBreakEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)
  
}

void SEBreakEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEBreakEditor::displayInterface() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to display the editor 2D interface in viewports. 
	// Implement this function if your editor displays a 2D user interface. For example, a rectangle selection editor would display a 2D rectangle in the active viewport. 
	// You may use utility functions provided by SAMSON (e.g. displayLinesOrtho and displayTrianglesOrtho).
  

}

void SEBreakEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	auto app = SEAdenitaCoreSEApp::getAdenitaApp();
	auto highlightedNucleotides = app->GetNanorobot()->GetHighlightedNucleotides();
	auto numberOfHighlightedNucleotides = highlightedNucleotides.size();

	if (numberOfHighlightedNucleotides == 1) {

		// Skip the following cases:
		// 1. the nucleotide is not in a single strand
		// 2. the nucleotide is the only nucleotide in the single strand
		// 3. the cnuleotide is the end nucleotide, or there is no next or previous nucleotide 

		auto highlightedNucleotide = highlightedNucleotides[0];
		auto singleStrand = highlightedNucleotide->GetStrand();
		auto nextNucleotide = highlightedNucleotide->GetNext();
		auto prevNucleotide = highlightedNucleotide->GetPrev();

		// clear the current selection

		SAMSON::getActiveDocument()->clearSelection();

		// select the nucleotide
		//highlightedNucleotide->setSelectionFlag(true);

		if (singleStrand == nullptr) {
			SAMSON::informUser("Adenita - Break editor", "The nucleotide is not in any single strand - cannot break it.");
		}
		else if (singleStrand->getNumberOfNucleotides() == 1) {
			SAMSON::informUser("Adenita - Break editor", "Cannot break a single strand that contains only one nucleotide. If you want to delete it use the Delete editor.");
		}
		else if (highlightedNucleotide->IsEnd() || nextNucleotide == nullptr || prevNucleotide == nullptr) {
			SAMSON::informUser("Adenita - Break editor", "The nucleotide is the end nucleotide - cannot break here. If you want to delete this nucleotide use the Delete editor.");
		}
		else {

			app->BreakSingleStrand(fivePrimeModeFlag);

		}

		event->accept();

	}

}

void SEBreakEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEBreakEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEBreakEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEBreakEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEBreakEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEBreakEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

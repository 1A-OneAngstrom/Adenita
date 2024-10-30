#include "SEMergePartsEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "SAMSON.hpp"


SEMergePartsEditor::SEMergePartsEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SEMergePartsEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SEMergePartsEditor::~SEMergePartsEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SEMergePartsEditorGUI* SEMergePartsEditor::getPropertyWidget() const { return static_cast<SEMergePartsEditorGUI*>(propertyWidget); }

std::map<int, ADNPointer<ADNPart>> SEMergePartsEditor::getPartsList() {

	mapOfParts.clear();
	int lastId = 0;

	auto nr = SEAdenitaCoreSEApp::getAdenitaApp()->GetNanorobot();
	auto parts = nr->GetParts();

	SB_FOR(ADNPointer<ADNPart> p, parts) {

		++lastId; // the 0-th element in the comboBox is None
		mapOfParts.insert(std::make_pair(lastId, p));

	}

	return mapOfParts;

}

std::map<int, SEMergePartsEditor::Element> SEMergePartsEditor::getElementsList() {

	mapOfElements.clear();
	int lastId = 0;

	auto nr = SEAdenitaCoreSEApp::getAdenitaApp()->GetNanorobot();
	auto parts = nr->GetParts();
	SB_FOR(ADNPointer<ADNPart> part, parts) {

		auto dss = part->GetDoubleStrands();
		SB_FOR(ADNPointer<ADNDoubleStrand> ds, dss) {

			++lastId; // the 0-th element in the comboBox is None
			SEMergePartsEditor::Element el;
			el.type = 0;
			el.ds = ds;
			mapOfElements.insert(std::make_pair(lastId, el));

		}
		auto sss = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, sss) {

			++lastId; // the 0-th element in the comboBox is None
			SEMergePartsEditor::Element el;
			el.type = 1;
			el.ss = ss;
			mapOfElements.insert(std::make_pair(lastId, el));

		}

	}

	return mapOfElements;

}

void SEMergePartsEditor::selectComponent(int idx) {

	if (mapOfParts.find(idx) != mapOfParts.end())
		mapOfParts[idx]->setSelectionFlag(true);

}

void SEMergePartsEditor::selectElement(int idx) {

	if (mapOfElements.find(idx) != mapOfElements.end()) {

		if (mapOfElements[idx].ss != nullptr) mapOfElements[idx].ss->setSelectionFlag(true);
		if (mapOfElements[idx].ds != nullptr) mapOfElements[idx].ds->setSelectionFlag(true);

	}

}

bool SEMergePartsEditor::mergeParts(int idx, int jdx) {

	if (idx == jdx) {

		SAMSON::informUser("Adenita", "Cannot merge a component with itself.\nPlease select two different components.");
		return false;

	}

	auto app = SEAdenitaCoreSEApp::getAdenitaApp();
	app->SetMod(true);

	ADNPointer<ADNPart> p1 = nullptr;
	ADNPointer<ADNPart> p2 = nullptr;
	if (mapOfParts.find(idx) != mapOfParts.end()) p1 = mapOfParts.at(idx);
	if (mapOfParts.find(jdx) != mapOfParts.end()) p2 = mapOfParts.at(jdx);

	if (p1 != nullptr && p2 != nullptr) {
		
		app->MergeComponents(p1, p2);
		if (p2 != nullptr) {

			p2->erase();
			p2.deleteReferenceTarget();

		}

	}

	app->SetMod(false);

	return true;

}

bool SEMergePartsEditor::moveElement(int edx, int pdx) {

	auto app = SEAdenitaCoreSEApp::getAdenitaApp();
	app->SetMod(true);

	ADNPointer<ADNPart> p = nullptr;
	if (mapOfParts.find(pdx) != mapOfParts.end()) p = mapOfParts.at(pdx);
	SEMergePartsEditor::Element el;
	if (mapOfElements.find(edx) != mapOfElements.end()) el = mapOfElements.at(edx);

	if (p != nullptr && el.type != -1) {

		if (el.type == 0)
			app->MoveDoubleStrand(el.ds, p);
		else if (el.type == 1)
			app->MoveSingleStrand(el.ss, p);

	}

	app->SetMod(false);

	return true;

}

SBCContainerUUID SEMergePartsEditor::getUUID() const { return SBCContainerUUID("EB812444-8EA8-BD83-988D-AFF5987461D8"); }

QString SEMergePartsEditor::getName() const {

	// SAMSON Element generator pro tip: this name should not be changed

	return "SEMergePartsEditor"; 

}

QPixmap SEMergePartsEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/merge.png"));

}

int SEMergePartsEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SEMergePartsEditor::getShortcut() const {
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SEMergePartsEditor::getToolTip() const {
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Merge components"); 

}

void SEMergePartsEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEMergePartsEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEMergePartsEditor::getDescription() const {

	return QObject::tr("Adenita | Merge Components");

}

void SEMergePartsEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	getPropertyWidget()->updatePartsList();

}

void SEMergePartsEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	SAMSON::unsetViewportCursor();

}

void SEMergePartsEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEMergePartsEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEMergePartsEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEMergePartsEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.
  
}

void SEMergePartsEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.
  
}

void SEMergePartsEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEMergePartsEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEMergePartsEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEMergePartsEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEMergePartsEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

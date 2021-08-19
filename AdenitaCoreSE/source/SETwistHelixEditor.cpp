#include "SETwistHelixEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SAMSON.hpp"


SETwistHelixEditor::SETwistHelixEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SETwistHelixEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SETwistHelixEditor::~SETwistHelixEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SETwistHelixEditorGUI* SETwistHelixEditor::getPropertyWidget() const { return static_cast<SETwistHelixEditorGUI*>(propertyWidget); }

void SETwistHelixEditor::SetTwistAngle(double angle)
{
  twistAngle_ = angle;

	if (twistAngle_ < 0) {
		std::string iconPath = SB_ELEMENT_PATH + "/Resource/icons/cursor_twistMinus1BP.png";
		SAMSON::setViewportCursor(QCursor(QPixmap(iconPath.c_str())));
	}
	else {
		std::string iconPath = SB_ELEMENT_PATH + "/Resource/icons/cursor_twistPlus1BP.png";
		SAMSON::setViewportCursor(QCursor(QPixmap(iconPath.c_str())));
	}
}

void SETwistHelixEditor::SetMode(bool t)
{
	SETwistHelixEditorGUI* gui = getPropertyWidget();
	gui->CheckPlusOrMinus(t);
}

SBCContainerUUID SETwistHelixEditor::getUUID() const { return SBCContainerUUID("4B60FECA-2A79-680F-F289-B4908A924409"); }

QString SETwistHelixEditor::getName() const {

	// SAMSON Element generator pro tip: this name should not be changed

	return "SETwistHelixEditor"; 

}

QPixmap SETwistHelixEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/twistDS.png"));

}

int SETwistHelixEditor::getFormat() const
{

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SETwistHelixEditor::getShortcut() const {
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SETwistHelixEditor::getToolTip() const {
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Twist dsDNA along helical axis"); 

}

void SETwistHelixEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SETwistHelixEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SETwistHelixEditor::getDescription() const
{
	return QObject::tr("Adenita | dsDNA Helical Twist");
}

void SETwistHelixEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.
	if (twistAngle_ < 0) {
		std::string iconPath = SB_ELEMENT_PATH + "/Resource/icons/cursor_twistMinus1BP.png";
		SAMSON::setViewportCursor(QCursor(QPixmap(iconPath.c_str())));
	}
	else {
		std::string iconPath = SB_ELEMENT_PATH + "/Resource/icons/cursor_twistPlus1BP.png";
		SAMSON::setViewportCursor(QCursor(QPixmap(iconPath.c_str())));
	}

}

void SETwistHelixEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.
	SAMSON::unsetViewportCursor();
}

void SETwistHelixEditor::getActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SETwistHelixEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)


}

void SETwistHelixEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SETwistHelixEditor::displayInterface() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to display the editor 2D interface in viewports. 
	// Implement this function if your editor displays a 2D user interface. For example, a rectangle selection editor would display a 2D rectangle in the active viewport. 
	// You may use utility functions provided by SAMSON (e.g. displayLinesOrtho and displayTrianglesOrtho).

}

void SETwistHelixEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.
  if (event->buttons() == Qt::LeftButton) {
    auto app = SEAdenitaCoreSEApp::getAdenitaApp();
    auto nanorobot = app->GetNanorobot();

    auto highlightedNucleotides = nanorobot->GetHighlightedNucleotides();

    CollectionMap<ADNDoubleStrand> highlightedDoubleStrands;
    SB_FOR(ADNPointer<ADNNucleotide> nt, highlightedNucleotides) {
      ADNPointer<ADNDoubleStrand> ds = nanorobot->GetDoubleStrand(nt);
      highlightedDoubleStrands.addReferenceTarget(ds());
    }

    app->TwistDoubleHelix(highlightedDoubleStrands, twistAngle_);
  }
}

void SETwistHelixEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.
  
}

void SETwistHelixEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETwistHelixEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETwistHelixEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETwistHelixEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETwistHelixEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETwistHelixEditor::onBaseEvent(SBBaseEvent* baseEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle base events

}

void SETwistHelixEditor::onDocumentEvent(SBDocumentEvent* documentEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle document events 

}

void SETwistHelixEditor::onDynamicalEvent(SBDynamicalEvent* dynamicalEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle dynamical events 

}

void SETwistHelixEditor::onStructuralEvent(SBStructuralEvent* documentEvent) {
	
	// SAMSON Element generator pro tip: implement this function if you need to handle structural events

}

#include "SEDNATwisterEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "MSVDisplayHelper.hpp"

#include "SAMSON.hpp"

#include "SBGRenderOpenGLFunctions.hpp"


SEDNATwisterEditor::SEDNATwisterEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SEDNATwisterEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SEDNATwisterEditor::~SEDNATwisterEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SEDNATwisterEditorGUI* SEDNATwisterEditor::getPropertyWidget() const { return static_cast<SEDNATwisterEditorGUI*>(propertyWidget); }

void SEDNATwisterEditor::setBendingType(BendingType type) {

	this->bendingType = type;
	updateEditorText();

}

void SEDNATwisterEditor::untwisting() {

	SBNodeIndexer baseSegmentIndexer;
	SAMSON::getActiveDocument()->getNodes(baseSegmentIndexer, (SBNode::GetClass() == std::string("ADNBaseSegment")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

	DASBackToTheAtom btta;

	SB_FOR(SBNode* node, baseSegmentIndexer) {

		ADNPointer<ADNBaseSegment> baseSegment = static_cast<ADNBaseSegment*>(node);
		const SBPosition3 baseSegmentPosition = baseSegment->GetPosition();
		const SBQuantity::length distanceFromSphereCenter = (baseSegmentPosition - spherePosition).norm();

		if (distanceFromSphereCenter < sphereRadius) {

			if (forwardActionSphereActive)
				btta.UntwistNucleotidesPosition(baseSegment);
			else if (reverseActionSphereActive)
				btta.SetNucleotidePosition(baseSegment, true);

		}

	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

void SEDNATwisterEditor::makeInvisible() {

	SBNodeIndexer nucleotideIndexer;
	SAMSON::getActiveDocument()->getNodes(nucleotideIndexer, (SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

	SB_FOR(SBNode* node, nucleotideIndexer) {

		ADNPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(node);

		SBPosition3 pos = nt->GetPosition();
		SBPosition3 vectorFromSphereCenter = pos - spherePosition;

		if (vectorFromSphereCenter.norm() < sphereRadius) {

			if (forwardActionSphereActive)
				nt->setVisibilityFlag(false);
			else if (reverseActionSphereActive)
				nt->setVisibilityFlag(true);

		}

	}

	SEAdenitaCoreSEApp::resetVisualModel();

}

SBPosition3 SEDNATwisterEditor::getSnappedPosition(const SBPosition3& currentPosition) {

	if (snappingIsActive)
		return SEAdenitaCoreSEApp::getAdenitaApp()->getSnappedPosition(currentPosition);

	return currentPosition;

}

SBCContainerUUID SEDNATwisterEditor::getUUID() const { return SBCContainerUUID("BF86253A-9F66-9F3C-4039-A711891C8670"); }

QString SEDNATwisterEditor::getName() const { 

	// SAMSON Element generator pro tip: this name should not be changed

	return "SEDNATwisterEditor"; 

}

QPixmap SEDNATwisterEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/SEDNATwisterEditorIcon.png"));

}

int SEDNATwisterEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SEDNATwisterEditor::getShortcut() const { 
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SEDNATwisterEditor::getToolTip() const { 
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Twist/Untwist Double Strand DNA"); 

}

void SEDNATwisterEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEDNATwisterEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEDNATwisterEditor::getDescription() const {

	return QObject::tr("Adenita | Twist/Untwist Double Strand DNA");

}

void SEDNATwisterEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	//const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/twister.png");
	//SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

	altPressed = false;
	forwardActionSphereActive = false;
	reverseActionSphereActive = false;

}

void SEDNATwisterEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	altPressed = false;
	forwardActionSphereActive = false;
	reverseActionSphereActive = false;

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	//SAMSON::unsetViewportCursor();

}

void SEDNATwisterEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEDNATwisterEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	updateEditorText();

	const SBPosition3 textOffset = SBPosition3(SBQuantity::angstrom(0.0), SBQuantity::angstrom(sphereRadius), SBQuantity::angstrom(0.0));
	const SBPosition3 textPosition = spherePosition - textOffset;

	positionData[0] = spherePosition[0].getValue();
	positionData[1] = spherePosition[1].getValue();
	positionData[2] = spherePosition[2].getValue();

	radiusData[0] = sphereRadius.getValue();

	if (forwardActionSphereActive) {
		colorData[0] = 0.f;
		colorData[1] = 1.f;
		colorData[2] = 0.f;
		colorData[3] = 0.3f;
	}
	else if (reverseActionSphereActive) {
		colorData[0] = 0.f;
		colorData[1] = 0.f;
		colorData[2] = 1.f;
		colorData[3] = 0.3f;
	}
	else {
		colorData[0] = 0.f;
		colorData[1] = 1.f;
		colorData[2] = 1.f;
		colorData[3] = 0.3f;
	}

	flagData[0] = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	SAMSON::displaySpheres(1, positionData, radiusData, colorData, flagData);

	ADNDisplayHelper::displayText(textPosition, editorText);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

}

void SEDNATwisterEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEDNATwisterEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool isLeftButton = event->button() & Qt::LeftButton;

	if (isLeftButton) {

		updateForwardReverseState();

		const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
		spherePosition = getSnappedPosition(currentPosition);

		if (bendingType == BendingType::UNTWIST)
			untwisting();
		else if (bendingType == BendingType::SPHEREVISIBILITY)
			makeInvisible();

		event->accept();
		SAMSON::requestViewportUpdate();

	}

}

void SEDNATwisterEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool isLeftButton = event->button() & Qt::LeftButton;

	if (isLeftButton) {

		event->accept();
		SAMSON::requestViewportUpdate();

	}

}

void SEDNATwisterEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
	spherePosition = getSnappedPosition(currentPosition);

	const bool hasLeftButton = event->buttons() & Qt::LeftButton;

	if (hasLeftButton) {

		updateForwardReverseState();

		if (forwardActionSphereActive || reverseActionSphereActive) {

			if (bendingType == BendingType::UNTWIST)
				untwisting();
			else if (bendingType == BendingType::SPHEREVISIBILITY)
				makeInvisible();

		}

		event->accept();

	}

	SAMSON::requestViewportUpdate();

}

void SEDNATwisterEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEDNATwisterEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->modifiers() & Qt::ControlModifier) {

		updateForwardReverseState();

		int angle = event->angleDelta().y();
		sphereRadius = sphereRadius * pow(1.002, angle);

		event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SEDNATwisterEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->key() == Qt::Key::Key_Alt) {

		altPressed = true;
		updateForwardReverseState();
		updateEditorText();

		event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SEDNATwisterEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->key() == Qt::Key::Key_Alt) {

		altPressed = false;
		updateForwardReverseState();
		updateEditorText();

		event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SEDNATwisterEditor::updateEditorText() {

	if (bendingType == BendingType::UNTWIST)
		editorText = (altPressed ? "Twisting" : "Untwisting");
	else if (bendingType == BendingType::SPHEREVISIBILITY)
		editorText = (altPressed ? "Visible" : "Invisible");

}

void SEDNATwisterEditor::updateForwardReverseState() {

	if (!altPressed) {

		forwardActionSphereActive = true;
		reverseActionSphereActive = false;

	}
	else {

		forwardActionSphereActive = false;
		reverseActionSphereActive = true;

	}

}

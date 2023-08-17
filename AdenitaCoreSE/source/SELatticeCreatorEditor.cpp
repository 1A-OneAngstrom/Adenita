#include "SELatticeCreatorEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "MSVDisplayHelper.hpp"

#include "SAMSON.hpp"

#include <QOpenGLFunctions_4_3_Core>


SELatticeCreatorEditor::SELatticeCreatorEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SELatticeCreatorEditorGUI(this);
	propertyWidget->loadDefaultSettings();

	setLatticeType(LatticeType::Honeycomb);

}

SELatticeCreatorEditor::~SELatticeCreatorEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SELatticeCreatorEditorGUI* SELatticeCreatorEditor::getPropertyWidget() const { return static_cast<SELatticeCreatorEditorGUI*>(propertyWidget); }

void SELatticeCreatorEditor::setMaxXDoubleStrands(int val) {
	maxXDoubleStrands = val;
}

void SELatticeCreatorEditor::setMaxYDoubleStrands(int val) {
	maxYDoubleStrands = val;
}

void SELatticeCreatorEditor::setMaxZBasePairs(int val) {
	maxZBasePairs = val;
}

ADNPointer<ADNPart> SELatticeCreatorEditor::generateLattice(bool mock /*= false*/) {

	//ADNLogger& logger = ADNLogger::GetLogger();

	ADNPointer<ADNPart> part = nullptr;

	SBPosition3 currentPos = secondPosition - firstPosition;

	SBPosition3 xPos = currentPos;
	xPos[1].setValue(0);
	SBPosition3 yPos = currentPos;
	yPos[2].setValue(0);
  
	const SBQuantity::nanometer x = xPos.norm();
	const SBQuantity::nanometer y = yPos.norm();
	SBQuantity::nanometer z = SBQuantity::nanometer(3.4);

	if (!heightSelected) {

		SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());
		z = (currentPosition - firstPosition).norm();

	}
	else if (heightSelected) {

		z = (thirdPosition - firstPosition).norm();

	}
  
	auto xNumStrands = round((x / SBQuantity::nanometer(ADNConstants::DH_DIAMETER)).getValue());
	auto yNumStrands = round((y / SBQuantity::nanometer(ADNConstants::DH_DIAMETER)).getValue());
	auto numBps = round((z / SBQuantity::nanometer(ADNConstants::BP_RISE)).getValue());

	if (xNumStrands < 1) xNumStrands = 1;
	if (yNumStrands < 1) yNumStrands = 1;
	if (numBps < 1) numBps = 1;

	if (xNumStrands > maxXDoubleStrands)  xNumStrands = maxXDoubleStrands;
	if (yNumStrands > maxYDoubleStrands)  yNumStrands = maxYDoubleStrands;
	if (numBps > maxZBasePairs)  numBps = maxZBasePairs;

	xyText = "x: ";
	xyText += std::to_string(int(xNumStrands));
	xyText += " ds / ";
	auto xLen = SBQuantity::nanometer(x).getValue();
	if (latticeType == LatticeType::Honeycomb)
		xLen *= 1.5;
	xyText += std::to_string(int(xLen));
	xyText += " nm; ";
	xyText += "y: ";
	xyText += std::to_string(int(yNumStrands));
	xyText += " ds / ";
	xyText += std::to_string(int(SBQuantity::nanometer(y).getValue()));
	xyText += " nm; ";
	zText = "z: ";
	zText += std::to_string(int(numBps));
	zText += " bps / ";
	zText += std::to_string(int(SBQuantity::nanometer(z).getValue()));
	zText += " nm; ";

	part = new ADNPart();

	SBVector3 dir = SBVector3(1, 0, 0);
	for (int xt = 0; xt < xNumStrands; xt++) {

		for (int yt = 0; yt < yNumStrands; yt++) {

			auto pos = vGrid.GetGridCellPos3D(0, xt, yt);
			pos += firstPosition;

			int zLength = numBps;
			if (zPattern == ZLatticePattern::TRIANGLE) {
				zLength = (xt / xNumStrands) * numBps;
			}

			if (zLength > 0) auto ds = DASCreator::CreateDoubleStrand(part, zLength, pos, dir, mock);

		}

	}

	if (!mock && part != nullptr) {

		std::string partName = "Square lattice";
		if (latticeType == LatticeType::Honeycomb) partName = "Honeycomb lattice";
		part->setName(SEAdenitaCoreSEApp::getAdenitaApp()->getUniquePartName(partName));

	}

	return part;

}

void SELatticeCreatorEditor::displayLattice() {

	ADNDisplayHelper::displayPart(tempPart);

}

void SELatticeCreatorEditor::sendPartToAdenita(ADNPointer<ADNPart> lattice) {

	if (lattice != nullptr) {

		SEAdenitaCoreSEApp* adenita = SEAdenitaCoreSEApp::getAdenitaApp();
		adenita->addPartToDocument(lattice);

		//DASCadnano cad = DASCadnano();
		//cad.CreateConformations(lattice);
		//adenita->addConformationToDocument(cad.Get3DConformation());
		//adenita->addConformationToDocument(cad.Get2DConformation());
		//adenita->addConformationToDocument(cad.Get1DConformation());

		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

SBCContainerUUID SELatticeCreatorEditor::getUUID() const { return SBCContainerUUID("7297F9AE-9237-0720-03B1-B4BDF45D33F9"); }

QString SELatticeCreatorEditor::getName() const { 

	// SAMSON Element generator pro tip: this name should not be changed

	return "SELatticeCreatorEditor"; 

}

QPixmap SELatticeCreatorEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/SELatticeCreatorEditorIcon.png"));

}

int SELatticeCreatorEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SELatticeCreatorEditor::getShortcut() const { 
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SELatticeCreatorEditor::getToolTip() const { 
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Create double strand DNA on a square or honeycomb lattice"); 

}

void SELatticeCreatorEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SELatticeCreatorEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SELatticeCreatorEditor::getDescription() const {

	return QObject::tr("Adenita | Lattice Creator");

}

void SELatticeCreatorEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	SAMSON::getActiveCamera()->rightView();

	resetData();

}

void SELatticeCreatorEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	resetData();

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	SAMSON::unsetViewportCursor();

}

void SELatticeCreatorEditor::resetData() {

	isPressing = false;
	lengthSelected = false;
	heightSelected = false;

	firstPosition = SBPosition3();
	secondPosition = SBPosition3();
	thirdPosition = SBPosition3();
	displayFlag = false;
	tempPart == nullptr;

}

void SELatticeCreatorEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SELatticeCreatorEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	if (!displayFlag) return;

	SEConfig& config = SEConfig::GetInstance();

	const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());

	tempPart = generateLattice(true);

	if (tempPart != nullptr) {

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		displayLattice();

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

	}

	const SBPosition3 offset = SBPosition3(SBQuantity::angstrom(5), SBQuantity::angstrom(5), SBQuantity::angstrom(5));

	if (!lengthSelected) {

		ADNDisplayHelper::displayLine(firstPosition, currentPosition);
			
		const SBPosition3 xyPos = currentPosition + offset;
		ADNDisplayHelper::displayText(xyPos, xyText);

	}
	else {

		ADNDisplayHelper::displayLine(firstPosition, secondPosition);
		ADNDisplayHelper::displayLine(firstPosition, currentPosition);

		const SBPosition3 zPos = currentPosition + offset;
		ADNDisplayHelper::displayText(zPos, zText);

	}

}

void SELatticeCreatorEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SELatticeCreatorEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	/*
	* The Flow should be the same as the flow for the DNA Nanotube creator:
	* 1st mousePressEvent - set the first position and temporary second and third positions, set isPressing = true
	* mouseMoveEvent - update the temporary second and third positions to display the mock up, set displayFlag = true
	* 1st mouseReleaseEvent - set the second position - the first dimension is set
	* mouseMoveEvent - update the temporary third position to display the mock up
	* 2nd mouseReleaseEvent - set the third position and generate the nanotube, then clean/reset the data
	*/

	if (isPressing) {

		event->accept();
		return;

	}

	if (event->button() & Qt::LeftButton) {

		if (!lengthSelected) {

			resetData();

			SAMSON::getActiveCamera()->rightView();
			firstPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			secondPosition = firstPosition;
			thirdPosition = firstPosition;

		}
		else {

			thirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

		}

		isPressing = true;

		event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SELatticeCreatorEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool isLeftButton = event->button() & Qt::LeftButton;

	// takes care of the issue: press right mouse button then left mouse button, then release RMB, then LMB (so no holding) leads to camera editor, then move mouse and click LMB -> adds strands
	if (isPressing) event->accept();

	if (!displayFlag) {

		//if (isLeftButton)
		resetData();

		return;

	}

	if (lengthSelected) event->accept();

	if (isPressing && event->button() & Qt::LeftButton) {

		event->accept();

		isPressing = false;

		// set temporary or final third position
		thirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

		if (!lengthSelected) {

			// set final second position, the length is defined
			lengthSelected = true;
			secondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

			if ((firstPosition - secondPosition).norm() < SBQuantity::angstrom(1.0)) {

				resetData();

			}

			//SAMSON::getActiveCamera()->topView();
			SAMSON::requestViewportUpdate();
			return;

		}

		//SAMSON::beginHolding("Add lattice");

		heightSelected = true;
		
		ADNPointer<ADNPart> part = generateLattice();
		sendPartToAdenita(part);

		//SAMSON::getActiveCamera()->rightView();
		
		//SAMSON::endHolding();

		resetData();

		SAMSON::requestViewportUpdate();

	}

}

void SELatticeCreatorEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool hasMiddleButton = event->buttons() & Qt::MiddleButton;
	const bool hasLeftButton = event->buttons() & Qt::LeftButton;
	const bool hasRightButton = event->buttons() & Qt::RightButton;

	if (!hasLeftButton) {

		isPressing = false;
		if (isPressing)
			resetData();

	}

	if (isPressing && hasLeftButton) displayFlag = true;

	if (!hasMiddleButton && !hasLeftButton && !hasRightButton) {

		event->accept();

	}

	if (!lengthSelected) {

		if (hasLeftButton) {

			secondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			event->accept();
			//SAMSON::requestViewportUpdate();
			//return;

		}

	}
	else {

		thirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

		if (!hasMiddleButton && !hasRightButton)
			event->accept();

	}

	SAMSON::requestViewportUpdate();

}

void SELatticeCreatorEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (isPressing) event->accept();

}

void SELatticeCreatorEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SELatticeCreatorEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (displayFlag) {

		if (event->key() == Qt::Key::Key_Escape) {

			resetData();

			event->accept();

			SAMSON::requestViewportUpdate();

		}
		else if (event->key() == Qt::Key::Key_1) {
			event->accept();
			SAMSON::getActiveCamera()->leftView();
		}
		else if (event->key() == Qt::Key::Key_2) {
			event->accept();
			SAMSON::getActiveCamera()->rightView();
		}
		else if (event->key() == Qt::Key::Key_3) {
			event->accept();
			SAMSON::getActiveCamera()->frontView();
		}
		else if (event->key() == Qt::Key::Key_4) {
			event->accept();
			SAMSON::getActiveCamera()->backView();
		}
		else if (event->key() == Qt::Key::Key_5) {
			event->accept();
			SAMSON::getActiveCamera()->topView();
		}
		else if (event->key() == Qt::Key::Key_6) {
			event->accept();
			SAMSON::getActiveCamera()->bottomView();
		}

	}

}

void SELatticeCreatorEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SELatticeCreatorEditor::setLatticeType(LatticeType type) {

	vGrid.CreateLattice(type);
	latticeType = type;
	if (this == SAMSON::getActiveEditor())
		SAMSON::getActiveCamera()->rightView();

}

void SELatticeCreatorEditor::setZPattern(ZLatticePattern pattern) {

	this->zPattern = pattern;

}

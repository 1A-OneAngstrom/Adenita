#include "SEDSDNACreatorEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "MSVDisplayHelper.hpp"

#include "SAMSON.hpp"

#include "SBGRenderOpenGLFunctions.hpp"


SEDSDNACreatorEditor::SEDSDNACreatorEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	SEConfig& config = SEConfig::GetInstance();

	propertyWidget = new SEDSDNACreatorEditorGUI(this);
	propertyWidget->loadDefaultSettings();

	basePairRadius = config.base_pair_radius;

}

SEDSDNACreatorEditor::~SEDSDNACreatorEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SEDSDNACreatorEditorGUI* SEDSDNACreatorEditor::getPropertyWidget() const { return static_cast<SEDSDNACreatorEditorGUI*>(propertyWidget); }

void SEDSDNACreatorEditor::setDoubleStrandMode(bool m) {

	this->doubleStrandMode = m;

}

void SEDSDNACreatorEditor::setShowBoxFlag(bool s) {

	this->showBoxFlag = s;

}

void SEDSDNACreatorEditor::setBoxSize(SBQuantity::nanometer height, SBQuantity::nanometer width, SBQuantity::nanometer depth) {

	this->boxHeight = height;
	this->boxWidth = width;
	this->boxDepth = depth;

}

void SEDSDNACreatorEditor::setCircularStrandsMode(bool c) {
	this->circularStrandsMode = c;
}

void SEDSDNACreatorEditor::setManualFlag(bool m) {
	this->manualFlag = m;
}

void SEDSDNACreatorEditor::setNumberOfNucleotides(int n) {
	this->numberOfNucleotides = n;
}

void SEDSDNACreatorEditor::setSequenceFlag(bool s) {
	this->sequenceFlag = s;
}

SBPosition3 SEDSDNACreatorEditor::getSnappedPosition(const SBPosition3& currentPosition) {

	if (snappingIsActive)
		return SEAdenitaCoreSEApp::getAdenitaApp()->getSnappedPosition(currentPosition);

	return currentPosition;

}

ADNPointer<ADNPart> SEDSDNACreatorEditor::generateStrand(bool mock) {

	auto length = (positionData.SecondPosition - positionData.FirstPosition).norm();
	auto numNucleotides = numberOfNucleotides;
	if (!manualFlag || numNucleotides == 0) {

		numNucleotides = round((length / SBQuantity::nanometer(ADNConstants::BP_RISE)).getValue());

	}
	// if (manualFlag) we already have the number of nucleotides, we just need a direction

	ADNPointer<ADNPart> part = nullptr;

	if (numNucleotides > 0) {

		part = new ADNPart();

		const SBVector3 dir = (positionData.SecondPosition - positionData.FirstPosition).normalizedVersion();

		if (doubleStrandMode)
			auto doubleStrand = DASCreator::CreateDoubleStrand(part, numNucleotides, positionData.FirstPosition, dir, mock);
		else
			auto singleStrand = DASCreator::CreateSingleStrand(part, numNucleotides, positionData.FirstPosition, dir, mock);

		if (!mock && part != nullptr) {

			std::string partName = (doubleStrandMode ? "Double" : "Single");
			partName += " strand structure";
			part->setName(SEAdenitaCoreSEApp::getAdenitaApp()->getUniquePartName(partName));

		}

	}

	return part;

}

ADNPointer<ADNPart> SEDSDNACreatorEditor::generateCircularStrand(bool mock) {

	ADNPointer<ADNPart> part = nullptr;

	//auto radius = (positionData.SecondPosition - positionData.FirstPosition).norm();
	const double pi = atan(1) * 4;
	auto numNucleotides = numberOfNucleotides;
	SBQuantity::length radius = numNucleotides * SBQuantity::nanometer(ADNConstants::BP_RISE) * 0.5 / pi;
	if (!manualFlag || numNucleotides == 0) {

		radius = (positionData.SecondPosition - positionData.FirstPosition).norm();
		numNucleotides = round((2 * pi * radius / SBQuantity::nanometer(ADNConstants::BP_RISE)).getValue());

	}

	if (numNucleotides > 6) {   // the smallest circle consists of 10 base pairs

		if (doubleStrandMode)
			part = DASCreator::CreateDSRing(radius, positionData.FirstPosition, positionData.FirstVector, mock);
		else
			part = DASCreator::CreateSSRing(radius, positionData.FirstPosition, positionData.FirstVector, mock);

		if (!mock && part != nullptr) {

			std::string partName = (doubleStrandMode ? "Double" : "Single");
			partName += " strand circular structure";
			part->setName(SEAdenitaCoreSEApp::getAdenitaApp()->getUniquePartName(partName));

		}

	}

	return part;

}

void SEDSDNACreatorEditor::displayBox() {

	if (showBoxFlag) {

		// draw a box centered at origin
		const SBVector3 x = SBVector3(1.0, 0.0, 0.0);
		const SBVector3 y = SBVector3(0.0, 1.0, 0.0);
		const SBVector3 z = SBVector3(0.0, 0.0, 1.0);

		const SBQuantity::length xMax = boxWidth * 0.5;
		const SBQuantity::length xMin = -xMax;
		const SBQuantity::length yMax = boxHeight * 0.5;
		const SBQuantity::length yMin = -yMax;
		const SBQuantity::length zMax = boxDepth * 0.5;
		const SBQuantity::length zMin = -zMax;

		const SBPosition3 v1 = xMin * x + yMax * y + zMax * z;
		const SBPosition3 v2 = xMax * x + yMax * y + zMax * z;
		const SBPosition3 v3 = xMax * x + yMin * y + zMax * z;
		const SBPosition3 v4 = xMin * x + yMin * y + zMax * z;
		const SBPosition3 v5 = xMin * x + yMax * y + zMin * z;
		const SBPosition3 v6 = xMax * x + yMax * y + zMin * z;
		const SBPosition3 v7 = xMax * x + yMin * y + zMin * z;
		const SBPosition3 v8 = xMin * x + yMin * y + zMin * z;

		ADNDisplayHelper::displayLine(v1, v2);
		ADNDisplayHelper::displayLine(v2, v3);
		ADNDisplayHelper::displayLine(v3, v4);
		ADNDisplayHelper::displayLine(v4, v1);

		ADNDisplayHelper::displayLine(v5, v6);
		ADNDisplayHelper::displayLine(v6, v7);
		ADNDisplayHelper::displayLine(v7, v8);
		ADNDisplayHelper::displayLine(v8, v5);

		ADNDisplayHelper::displayLine(v1, v5);
		ADNDisplayHelper::displayLine(v2, v6);
		ADNDisplayHelper::displayLine(v3, v7);
		ADNDisplayHelper::displayLine(v4, v8);

	}

}

void SEDSDNACreatorEditor::sendPartToAdenita(ADNPointer<ADNPart> nanotube) {

	if (nanotube != nullptr) {

		setSequence(nanotube);

		SEAdenitaCoreSEApp::getAdenitaApp()->addPartToDocument(nanotube);
		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SEDSDNACreatorEditor::setSequence(ADNPointer<ADNPart> nanotube) {

	if (sequenceFlag) {

		auto singleStrands = nanotube->GetSingleStrands();
		ADNPointer<ADNSingleStrand> singleStrand = nullptr;
		SB_FOR(ADNPointer<ADNSingleStrand> currentSingleStrand, singleStrands) {

			ADNPointer<ADNNucleotide> fPrime = currentSingleStrand->GetFivePrime();
			ADNPointer<ADNBaseSegment> bs = fPrime->GetBaseSegment();
			const SBPosition3& pos = bs->GetPosition();
			if (pos == positionData.FirstPosition) {

				singleStrand = currentSingleStrand;
				break;

			}

		}

		if (singleStrand != nullptr) {

			int length = singleStrand->getNumberOfNucleotides();
			SEDSDNACreatorEditorGUI* editor = getPropertyWidget();
			std::string seq = editor->AskUserForSequence(length);
			singleStrand->SetSequence(seq);

		}

	}

}

void SEDSDNACreatorEditor::displayStrand() {

	ADNDisplayHelper::displayPart(tempPart, basePairRadius, opaqueness);

	//string text = "Opaqueness: ";
	//stringstream s1;
	//s1 << fixed << setprecision(2) << opaqueness;
	//text += s1.str();

	//text += ", Radius: ";
	//stringstream s2;
	//s2 << fixed << setprecision(2) << basePairRadius;
	//text += s2.str();

	const SBPosition3 offset = SBPosition3(SBQuantity::nanometer(0.0), SBQuantity::nanometer(-1.0), SBQuantity::nanometer(0.0));
	const std::string text = std::to_string(tempPart->GetNumberOfBaseSegments()) + "bp";
	ADNDisplayHelper::displayText(SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport()) + offset, text);

}

SBCContainerUUID SEDSDNACreatorEditor::getUUID() const { return SBCContainerUUID("2F353D32-A630-8800-5FCA-14EBA6AC36F9"); }

QString SEDSDNACreatorEditor::getName() const {

	// SAMSON Element generator pro tip: this name should not be changed

	return "SEDSDNACreatorEditor";

}

QPixmap SEDSDNACreatorEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/SEDSDNACreatorEditorIcon.png"));

}

QKeySequence SEDSDNACreatorEditor::getShortcut() const {

	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence("");

}

QString SEDSDNACreatorEditor::getToolTip() const {

	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Add single and double strand DNA to your model");

}

void SEDSDNACreatorEditor::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEDSDNACreatorEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEDSDNACreatorEditor::getDescription() const {

	return QObject::tr("Adenita | DNA Strands Creator");

}

int SEDSDNACreatorEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

void SEDSDNACreatorEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	//const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/dsCreator.png");
	//SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

	resetData();

}

void SEDSDNACreatorEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	resetData();

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	SAMSON::unsetViewportCursor();

}

void SEDSDNACreatorEditor::resetData() {

	isPressing = false;

	DASCreatorEditors::resetPositions(positionData);
	displayFlag = false;
	tempPart = nullptr;

}

void SEDSDNACreatorEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEDSDNACreatorEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	SEConfig& config = SEConfig::GetInstance();
	displayBox();

	if (displayFlag) {

		if (positionData.positionsCounter == 1) {

			const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());
			positionData.SecondPosition = getSnappedPosition(currentPosition);

			ADNDisplayHelper::displayLine(positionData.FirstPosition, positionData.SecondPosition);

		}

		if (config.preview_editor) {

			if (!circularStrandsMode) tempPart = generateStrand(true);
			else tempPart = generateCircularStrand(true);

		}

		if (tempPart != nullptr) {

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			displayStrand();

			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

		}

	}

}

void SEDSDNACreatorEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEDSDNACreatorEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (isPressing) {

		event->accept();
		return;

	}

	if (event->button() & Qt::LeftButton) {

		resetData();

		isPressing = true;
		event->accept();

		if (positionData.positionsCounter == 0) {

			const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			positionData.FirstPosition = getSnappedPosition(currentPosition);
			positionData.SecondPosition = positionData.FirstPosition;
			positionData.positionsCounter = 1;

			positionData.FirstVector = SAMSON::getActiveCamera()->getBasisZ().normalizedVersion();
			positionData.vectorsCounter = 1;

		}

		SAMSON::requestViewportUpdate();

	}

}

void SEDSDNACreatorEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool isLeftButton = event->button() & Qt::LeftButton;

	// takes care of the issue: press right mouse button then left mouse button, then release RMB, then LMB (so no holding) leads to camera editor, then move mouse and click LMB -> adds strands
	if (isPressing) event->accept();

	if (!displayFlag) {

		// takes care of the case when mouse has been pressed while editing, then the right click has been pressed leading to the camera editor,
		// the the left mouse button has been released leaving the right mouse button pressed and having the camera editor active 
		// meaning that the mouse move events were redirected to the camera editor

		//if (isLeftButton)
		resetData();

		return;

	}

	if (isPressing && isLeftButton) {

		isPressing = false;

		if (positionData.positionsCounter == 1) {

			const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			positionData.SecondPosition = getSnappedPosition(currentPosition);
			positionData.positionsCounter = 2;

			//SAMSON::beginHolding("Add DNA strands");

			ADNPointer<ADNPart> part = nullptr;
			if (!circularStrandsMode) part = generateStrand();
			else part = generateCircularStrand();

			if (part != nullptr) {

				if (part->getNumberOfNucleotides() > 0) {

					//int test3 = part->GetNumberOfSingleStrands();
					sendPartToAdenita(part);

				}
				else {

					part = nullptr;

				}

			}

			//SAMSON::endHolding();

			resetData();

			event->accept();

			SAMSON::requestViewportUpdate();

		}

	}

}

void SEDSDNACreatorEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool hasMiddleButton = event->buttons() & Qt::MiddleButton;
	const bool hasLeftButton = event->buttons() & Qt::LeftButton;
	const bool hasRightButton = event->buttons() & Qt::RightButton;

	if (!hasLeftButton) {

		if (isPressing)
			resetData();

	}

	if (isPressing && hasLeftButton) displayFlag = true;

	if (!hasMiddleButton && !hasLeftButton && !hasRightButton) {

		event->accept();

	}

	if (positionData.positionsCounter == 1) {

		if (hasLeftButton) {

			const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			positionData.SecondPosition = getSnappedPosition(currentPosition);

			event->accept();

		}

	}

	SAMSON::requestViewportUpdate();

}

void SEDSDNACreatorEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (isPressing) event->accept();

}

void SEDSDNACreatorEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEDSDNACreatorEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (displayFlag) {

		SEConfig& config = SEConfig::GetInstance();

		if (event->key() == Qt::Key_Escape) {

			resetData();
			event->accept();
			SAMSON::requestViewportUpdate();

		}
		else if (event->key() == Qt::Key_Up) {

			opaqueness += 0.1f;
			if (opaqueness > 1.0f) opaqueness = 1.0f;
			event->accept();
			SAMSON::requestViewportUpdate();

		}
		else if (event->key() == Qt::Key_Down) {

			opaqueness -= 0.1f;
			if (opaqueness < 0.0f) opaqueness = 0.0f;
			event->accept();
			SAMSON::requestViewportUpdate();

		}
		else if (event->key() == Qt::Key_Left) {

			basePairRadius -= 100.0f;
			if (basePairRadius < 200.0f) basePairRadius = 200.0f;
			event->accept();
			SAMSON::requestViewportUpdate();

		}
		else if (event->key() == Qt::Key_Right) {

			basePairRadius += 100.0f;
			if (basePairRadius > config.base_pair_radius) basePairRadius = config.base_pair_radius;
			event->accept();
			SAMSON::requestViewportUpdate();

		}

	}

	if (event->key() == Qt::Key_Control) {

		snappingIsActive = false;

		event->accept();
		SAMSON::requestViewportUpdate();

	}

}

void SEDSDNACreatorEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->key() == Qt::Key_Control) {

		snappingIsActive = true;

		event->accept();

	}

}

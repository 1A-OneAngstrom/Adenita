#include "SENanotubeCreatorEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "SAMSON.hpp"


SENanotubeCreatorEditor::SENanotubeCreatorEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SENanotubeCreatorEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SENanotubeCreatorEditor::~SENanotubeCreatorEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SENanotubeCreatorEditorGUI* SENanotubeCreatorEditor::getPropertyWidget() const { return static_cast<SENanotubeCreatorEditorGUI*>(propertyWidget); }

void SENanotubeCreatorEditor::SetRouting(RoutingType t) {

	routing_ = t;

}

void SENanotubeCreatorEditor::SetPredefined(bool predefined, double radius, int numBp) {

	predefined_ = false;
	if (radius > 0.0 && numBp > 0) {

		predefined_ = predefined;
		numBp_ = numBp;
		radius_ = radius;

	}

}

void SENanotubeCreatorEditor::SetRadius(double radius) {
	radius_ = radius;
}

void SENanotubeCreatorEditor::SetBp(int bp) {
	numBp_ = bp;
}

ADNPointer<ADNPart> SENanotubeCreatorEditor::generateNanotube(bool mock) {

	ADNPointer<ADNPart> part = nullptr;

	auto radius = (thirdPosition - secondPosition).norm();
	auto roundHeight = (secondPosition - firstPosition).norm();
	auto numNucleotides = round((roundHeight / SBQuantity::nanometer(ADNConstants::BP_RISE)).getValue());
	SBVector3 dir = (secondPosition - firstPosition).normalizedVersion();

	if (predefined_) {

		numNucleotides = numBp_;
		radius = SBQuantity::nanometer(radius_);

	}

	if (numNucleotides > 0) {

		if (mock) {

			if (!lengthSelected && !predefined_) {

				radius = SBQuantity::picometer(1.0);
				part = DASCreator::CreateMockNanotube(radius, firstPosition, dir, numNucleotides);

			}
			else if (lengthSelected || predefined_) {

				part = DASCreator::CreateMockNanotube(radius, firstPosition, dir, numNucleotides);

			}

		}
		else {

			part = DASCreator::CreateNanotube(radius, firstPosition, dir, numNucleotides);

		}

		updateGUI(radius, numNucleotides);

	}

	return part;

}

void SENanotubeCreatorEditor::displayNanotube()
{
	ADNDisplayHelper::displayPart(tempPart_);
}

void SENanotubeCreatorEditor::sendPartToAdenita(ADNPointer<ADNPart> nanotube) {

	if (nanotube != nullptr) {

		SEAdenitaCoreSEApp* adenita = static_cast<SEAdenitaCoreSEApp*>(SAMSON::getApp(SBCContainerUUID("85DB7CE6-AE36-0CF1-7195-4A5DF69B1528"), SBUUID(SB_ELEMENT_UUID)));
		adenita->AddPartToActiveLayer(nanotube);
		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SENanotubeCreatorEditor::updateGUI(SBQuantity::length radius, int numBp, bool clear) {

	if (numBp < 1) numBp = 0;

	int minNumDs = 3;
	SBQuantity::length minRadius = ADNVectorMath::CalculateNanotubeRadius(minNumDs);

	int numDs = ADNVectorMath::CalculateNanotubeDoubleStrands(radius);
	if (numDs < minNumDs || radius < minRadius) {

		numDs = minNumDs;
		radius = minRadius;

	}

	SENanotubeCreatorEditorGUI* gui = static_cast<SENanotubeCreatorEditorGUI*>(propertyWidget);
	gui->updateInfo(radius, numDs, numBp, clear);

}

SBCContainerUUID SENanotubeCreatorEditor::getUUID() const { return SBCContainerUUID("F9068FA3-69DE-B6FA-2B42-C80DA5302A0D"); }

QString SENanotubeCreatorEditor::getName() const { 

	// SAMSON Element generator pro tip: this name should not be changed

	return "SENanotubeCreatorEditor"; 

}

QPixmap SENanotubeCreatorEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/SENanotubeCreatorEditorIcon.png"));

}

int SENanotubeCreatorEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SENanotubeCreatorEditor::getShortcut() const { 
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SENanotubeCreatorEditor::getToolTip() const { 
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("DNA Nanotube Creator"); 

}

void SENanotubeCreatorEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SENanotubeCreatorEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SENanotubeCreatorEditor::getDescription() const {

	return QObject::tr("Adenita | DNA Nanotube Editor");

}

void SENanotubeCreatorEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	//const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/nanotubeCreator.png");
	//SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

	resetData();

}

void SENanotubeCreatorEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	resetData();

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	SAMSON::unsetViewportCursor();

}

void SENanotubeCreatorEditor::resetData() {
	
	isPressing = false;
	lengthSelected = false;

	firstPosition = SBPosition3();
	secondPosition = SBPosition3();
	thirdPosition = SBPosition3();
	display_ = false;
	tempPart_ == nullptr;

}

void SENanotubeCreatorEditor::getActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SENanotubeCreatorEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	SEConfig& config = SEConfig::GetInstance();

	if (display_) {

		const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());

		if (!lengthSelected) {

			ADNDisplayHelper::displayLine(firstPosition, currentPosition);
			secondPosition = currentPosition;

		}
		else {

			ADNDisplayHelper::displayLine(firstPosition, secondPosition);
			ADNDisplayHelper::displayLine(secondPosition, currentPosition);

			thirdPosition = currentPosition;

		}

		if (config.preview_editor) tempPart_ = generateNanotube(true);

		if (tempPart_ != nullptr) {

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			displayNanotube();

			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

		}

	}

}

void SENanotubeCreatorEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SENanotubeCreatorEditor::displayInterface() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to display the editor 2D interface in viewports. 
	// Implement this function if your editor displays a 2D user interface. For example, a rectangle selection editor would display a 2D rectangle in the active viewport. 
	// You may use utility functions provided by SAMSON (e.g. displayLinesOrtho and displayTrianglesOrtho).

}

void SENanotubeCreatorEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	/*
	* The Flow should be the same as for the Carbon Nanotube creator:
	* 1st mousePressEvent - set the first position and temporary second position, set isPressing = true
	* mouseMoveEvent - update the temporary second position to display the mock up, set display_ = true
	* 1st mouseReleaseEvent - set the second position - the length is set (lengthSelected = true)
	* mouseMoveEvent - update the temporary third position to display the mock up 
	* 2nd mouseReleaseEvent - set the third position and generate the nanotube, then clean/reset the data
	*/

	if (isPressing) {

		event->accept();
		return;

	}

	if (event->button() & Qt::LeftButton) {

		isPressing = true;
		event->accept();

		if (!lengthSelected) {

			updateGUI(SBQuantity::nanometer(0.0), 0, true);

			firstPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			secondPosition = firstPosition;

		}
		else {

			thirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

		}

		SAMSON::requestViewportUpdate();

	}

}

void SENanotubeCreatorEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (!display_) return;

	if (lengthSelected) event->accept();
	if (!isPressing) return;

	if (event->button() & Qt::LeftButton) {

		event->accept();

		isPressing = false;

		// set temporary or final third position
		thirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

		if (!lengthSelected) {

			// set final second position, the length is defined
			lengthSelected = true;
			secondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
			SAMSON::requestViewportUpdate();
			return;

		}

		//SAMSON::beginHolding("Add DNA nanotube");

		ADNPointer<ADNPart> part = generateNanotube();
		//DASRouter* router = DASRouter::GetRouter(routing_);
		//router->Route(part);
		sendPartToAdenita(part);

		//SAMSON::endHolding();

		resetData();

		SAMSON::requestViewportUpdate();

	}

}

void SENanotubeCreatorEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool hasMidButton = event->buttons() & Qt::MidButton;
	const bool hasLeftButton = event->buttons() & Qt::LeftButton;
	const bool hasRightButton = event->buttons() & Qt::RightButton;

	if (!hasLeftButton) {

		isPressing = false;

	}

	if (isPressing && hasLeftButton) display_ = true;

	if (!hasMidButton && !hasLeftButton && !hasRightButton) {

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

		if (!hasMidButton && !hasRightButton)
			event->accept();

	}

	SAMSON::requestViewportUpdate();

}

void SENanotubeCreatorEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (isPressing) event->accept();

}

void SENanotubeCreatorEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SENanotubeCreatorEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (display_) {

		if (event->key() == Qt::Key_Escape) {

			resetData();

			event->accept();

			SAMSON::requestViewportUpdate();

		}

	}

}

void SENanotubeCreatorEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SENanotubeCreatorEditor::onBaseEvent(SBBaseEvent* baseEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle base events

}

void SENanotubeCreatorEditor::onDocumentEvent(SBDocumentEvent* documentEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle document events 

}

void SENanotubeCreatorEditor::onDynamicalEvent(SBDynamicalEvent* dynamicalEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle dynamical events 

}

void SENanotubeCreatorEditor::onStructuralEvent(SBStructuralEvent* documentEvent) {
	
	// SAMSON Element generator pro tip: implement this function if you need to handle structural events

}

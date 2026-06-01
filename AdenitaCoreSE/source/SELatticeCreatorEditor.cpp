#include "SELatticeCreatorEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "MSVDisplayHelper.hpp"
#include "SELatticeCreatorEditorMath.hpp"

#include "SAMSON.hpp"

#include "SBGRenderOpenGLFunctions.hpp"

#include <algorithm>
#include <cmath>

namespace {

struct LatticeMeasurements {

	SBQuantity::nanometer x = SBQuantity::nanometer(0.0);
	SBQuantity::nanometer y = SBQuantity::nanometer(0.0);
	SBQuantity::nanometer z = SBQuantity::nanometer(0.0);
	int xNumStrands{ 1 };
	int yNumStrands{ 1 };
	int numBps{ 1 };

};

struct LatticeBuildData {

	LatticeMeasurements measurements;
	SBVector3 columnAxis = SBVector3(0.0, 1.0, 0.0);
	SBVector3 rowAxis = SBVector3(0.0, 0.0, 1.0);
	SBVector3 strandAxis = SBVector3(1.0, 0.0, 0.0);

};

SBVector3 normalizedCrossProduct(const SBVector3& v, const SBVector3& w, const SBVector3& fallback) {

	const double x = v[1].getValue() * w[2].getValue() - v[2].getValue() * w[1].getValue();
	const double y = v[2].getValue() * w[0].getValue() - v[0].getValue() * w[2].getValue();
	const double z = v[0].getValue() * w[1].getValue() - v[1].getValue() * w[0].getValue();
	const double norm = std::sqrt(x * x + y * y + z * z);

	if (norm < 1.0e-9) return fallback;

	return SBVector3(x / norm, y / norm, z / norm);

}

SBVector3 signedAxis(const SBVector3& axis, double signedLength) {

	if (signedLength < 0.0) return -axis;
	return axis;

}

double signedProjectionInNanometers(const SBPosition3& vector, const SBVector3& axis) {

	const auto projection = vector[0] * axis[0] + vector[1] * axis[1] + vector[2] * axis[2];
	return SBQuantity::nanometer(projection).getValue();

}

SBVector3 signedNormalAxisFromHeightDrag(const SBVector3& normalAxis, const SBPosition3& heightVector) {

	const double signedHeight = signedProjectionInNanometers(heightVector, normalAxis);
	if (std::abs(signedHeight) < 0.1) return normalAxis;
	return signedAxis(normalAxis, signedHeight);

}

LatticeMeasurements calculateLatticeMeasurements(
	const SBPosition3& firstPosition,
	const SBPosition3& secondPosition,
	const SBPosition3& thirdPosition,
	const SBPosition3& currentPosition,
	bool heightSelected,
	const SBVector3& columnAxis,
	const SBVector3& rowAxis,
	int maxXDoubleStrands,
	int maxYDoubleStrands,
	int maxZBasePairs) {

	const SBPosition3 currentPos = secondPosition - firstPosition;
	const double signedRowLength = signedProjectionInNanometers(currentPos, rowAxis);
	const double signedColumnLength = signedProjectionInNanometers(currentPos, columnAxis);

	LatticeMeasurements measurements;
	measurements.x = SBQuantity::nanometer(std::abs(signedRowLength));
	measurements.y = SBQuantity::nanometer(std::abs(signedColumnLength));
	measurements.z = heightSelected ?
		(thirdPosition - firstPosition).norm() :
		(currentPosition - firstPosition).norm();

	measurements.xNumStrands = static_cast<int>(std::round((measurements.x / SBQuantity::nanometer(ADNConstants::DH_DIAMETER)).getValue()));
	measurements.yNumStrands = static_cast<int>(std::round((measurements.y / SBQuantity::nanometer(ADNConstants::DH_DIAMETER)).getValue()));
	measurements.numBps = static_cast<int>(std::round((measurements.z / SBQuantity::nanometer(ADNConstants::BP_RISE)).getValue()));

	measurements.xNumStrands = std::clamp(measurements.xNumStrands, 1, maxXDoubleStrands);
	measurements.yNumStrands = std::clamp(measurements.yNumStrands, 1, maxYDoubleStrands);
	measurements.numBps = std::clamp(measurements.numBps, 1, maxZBasePairs);

	return measurements;

}

LatticeBuildData calculateLatticeBuildData(
	const SBPosition3& firstPosition,
	const SBPosition3& secondPosition,
	const SBPosition3& thirdPosition,
	const SBPosition3& currentPosition,
	bool lengthSelected,
	bool heightSelected,
	const SBVector3& baseColumnAxis,
	const SBVector3& baseRowAxis,
	const SBVector3& baseNormalAxis,
	int maxXDoubleStrands,
	int maxYDoubleStrands,
	int maxZBasePairs) {

	const SBPosition3 crossSectionVector = secondPosition - firstPosition;
	const double signedRowLength = signedProjectionInNanometers(crossSectionVector, baseRowAxis);
	const double signedColumnLength = signedProjectionInNanometers(crossSectionVector, baseColumnAxis);

	LatticeBuildData buildData;
	buildData.measurements = calculateLatticeMeasurements(
		firstPosition,
		secondPosition,
		thirdPosition,
		currentPosition,
		heightSelected,
		baseColumnAxis,
		baseRowAxis,
		maxXDoubleStrands,
		maxYDoubleStrands,
		maxZBasePairs);

	// VGrid returns positive local distances as (strand axis, lattice column, lattice row).
	// The user drag supplies the signs, so the same local grid can extend into negative
	// world directions without changing Cadnano lattice spacing.
	buildData.columnAxis = signedAxis(baseColumnAxis, signedColumnLength);
	buildData.rowAxis = signedAxis(baseRowAxis, signedRowLength);

	const SBPosition3 heightVector = heightSelected ?
		(thirdPosition - firstPosition) :
		(currentPosition - firstPosition);
	// Keep local z perpendicular to the cross-section plane locked by the first drag.
	// The second drag keeps its old role of setting height, with only its sign projected
	// onto the plane normal used to choose the growth direction when available.
	buildData.strandAxis = lengthSelected ?
		signedNormalAxisFromHeightDrag(baseNormalAxis, heightVector) :
		baseNormalAxis;

	return buildData;

}

SBPosition3 transformLocalGridPosition(
	const SBPosition3& origin,
	const SBPosition3& localPosition,
	const LatticeBuildData& buildData) {

	return origin
		+ localPosition[0] * buildData.strandAxis
		+ localPosition[1] * buildData.columnAxis
		+ localPosition[2] * buildData.rowAxis;

}

void updateLatticeText(const LatticeMeasurements& measurements, LatticeType latticeType, std::string& xyText, std::string& zText) {

	xyText = "x: ";
	xyText += std::to_string(measurements.xNumStrands);
	xyText += " ds / ";
	auto xLen = SBQuantity::nanometer(measurements.x).getValue();
	if (latticeType == LatticeType::Honeycomb)
		xLen *= 1.5;
	xyText += std::to_string(static_cast<int>(xLen));
	xyText += " nm; ";
	xyText += "y: ";
	xyText += std::to_string(measurements.yNumStrands);
	xyText += " ds / ";
	xyText += std::to_string(static_cast<int>(SBQuantity::nanometer(measurements.y).getValue()));
	xyText += " nm; ";

	zText = "z: ";
	zText += std::to_string(measurements.numBps);
	zText += " bps / ";
	zText += std::to_string(static_cast<int>(SBQuantity::nanometer(measurements.z).getValue()));
	zText += " nm; ";

}

} // namespace


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

SBPointer<ADNPart> SELatticeCreatorEditor::generateLattice(bool mock /*= false*/) {

	//ADNLogger& logger = ADNLogger::GetLogger();

	SBPointer<ADNPart> part = nullptr;

	if (!crossSectionFrameLocked) updateCrossSectionFrameFromCamera();

	const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());
	const LatticeBuildData buildData = calculateLatticeBuildData(
		firstPosition,
		secondPosition,
		thirdPosition,
		currentPosition,
		lengthSelected,
		heightSelected,
		latticeColumnAxis,
		latticeRowAxis,
		latticeNormalAxis,
		maxXDoubleStrands,
		maxYDoubleStrands,
		maxZBasePairs);
	updateLatticeText(buildData.measurements, latticeType, xyText, zText);

	part = new ADNPart();

	for (int xt = 0; xt < buildData.measurements.xNumStrands; xt++) {

		for (int yt = 0; yt < buildData.measurements.yNumStrands; yt++) {

			const SBPosition3 localPos = vGrid.GetGridCellPos3D(0, xt, yt);
			const SBPosition3 pos = transformLocalGridPosition(firstPosition, localPos, buildData);

			int zLength = buildData.measurements.numBps;
			if (zPattern == ZLatticePattern::TRIANGLE) {
				zLength = SELatticeCreatorEditorMath::calculateTriangleLatticeLength(
					xt,
					buildData.measurements.xNumStrands,
					buildData.measurements.numBps);
			}

			if (zLength > 0) auto ds = DASCreator::CreateDoubleStrand(part, zLength, pos, buildData.strandAxis, mock);

		}

	}

	if (!mock && part != nullptr) {

		std::string partName = "Square lattice";
		if (latticeType == LatticeType::Honeycomb) partName = "Honeycomb lattice";
		part->setName(SEAdenitaCoreSEApp::getAdenitaApp()->getUniquePartName(partName));

	}

	return part;

}

void SELatticeCreatorEditor::sendPartToAdenita(SBPointer<ADNPart> lattice) {

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
	latticeColumnAxis = SBVector3(0.0, 1.0, 0.0);
	latticeRowAxis = SBVector3(0.0, 0.0, 1.0);
	latticeNormalAxis = SBVector3(1.0, 0.0, 0.0);
	crossSectionFrameLocked = false;
	displayFlag = false;
	tempPart = nullptr;

}

void SELatticeCreatorEditor::updateCrossSectionFrameFromCamera() {

	auto camera = SAMSON::getActiveCamera();
	if (camera == nullptr) {

		latticeColumnAxis = SBVector3(0.0, 1.0, 0.0);
		latticeRowAxis = SBVector3(0.0, 0.0, 1.0);
		latticeNormalAxis = SBVector3(1.0, 0.0, 0.0);
		return;

	}

	latticeColumnAxis = camera->getBasisX().normalizedVersion();
	latticeRowAxis = camera->getBasisY().normalizedVersion();
	latticeNormalAxis = normalizedCrossProduct(latticeColumnAxis, latticeRowAxis, SBVector3(1.0, 0.0, 0.0));

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

	if (!crossSectionFrameLocked) updateCrossSectionFrameFromCamera();

	const SBPosition3 offset = SBPosition3(SBQuantity::angstrom(5), SBQuantity::angstrom(5), SBQuantity::angstrom(5));
	const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());
	const LatticeBuildData buildData = calculateLatticeBuildData(
		firstPosition,
		secondPosition,
		thirdPosition,
		currentPosition,
		lengthSelected,
		heightSelected,
		latticeColumnAxis,
		latticeRowAxis,
		latticeNormalAxis,
		maxXDoubleStrands,
		maxYDoubleStrands,
		maxZBasePairs);
	updateLatticeText(buildData.measurements, latticeType, xyText, zText);

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

	tempPart = nullptr;
	if (config.preview_editor) tempPart = generateLattice(true);

	if (tempPart != nullptr) {

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ADNDisplayHelper::displayPart(tempPart);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

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
			updateCrossSectionFrameFromCamera();
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
			else {

				updateCrossSectionFrameFromCamera();
				crossSectionFrameLocked = true;

			}

			//SAMSON::getActiveCamera()->topView();
			SAMSON::requestViewportUpdate();
			return;

		}

		//SAMSON::beginHolding("Add lattice");

		heightSelected = true;
		
		SBPointer<ADNPart> part = generateLattice();
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

			if (!crossSectionFrameLocked) updateCrossSectionFrameFromCamera();
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

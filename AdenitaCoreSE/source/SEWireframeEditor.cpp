#include "SEWireframeEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "MSVDisplayHelper.hpp"

#include "SAMSON.hpp"

#include "SBGRenderOpenGLFunctions.hpp"


SEWireframeEditor::SEWireframeEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SEWireframeEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SEWireframeEditor::~SEWireframeEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SEWireframeEditorGUI* SEWireframeEditor::getPropertyWidget() const { return static_cast<SEWireframeEditorGUI*>(propertyWidget); }

void SEWireframeEditor::setWireframeType(DASCreator::EditorType type) {

	this->wireframeType = type;

}

ADNPointer<ADNPart> SEWireframeEditor::generateCuboid(const SBPosition3& currentPosition, bool mock /*= false*/) {

	SEConfig& config = SEConfig::GetInstance();

	ADNPointer<ADNPart> part = nullptr;

	int zSize = 31;

	SBQuantity::length faceRadius;

	if (positionData.positionsCounter < 0) return nullptr;
	else if (positionData.positionsCounter == 1) {

		faceRadius = (currentPosition - positionData.FirstPosition).norm();

	}
	else if (positionData.positionsCounter > 1) {

		faceRadius = (positionData.SecondPosition - positionData.FirstPosition).norm();
		SBQuantity::length zNM = (currentPosition - positionData.SecondPosition).norm();
		zSize = DASDaedalus::CalculateEdgeSize(zNM);

	}

	const int bpSize = DASDaedalus::CalculateEdgeSize(faceRadius * 2);

	const int xSize = bpSize;
	const int ySize = bpSize;

	part = new ADNPart();

	const SBVector3 xDir(1.0, 0.0, 0.0);
	const SBVector3 yDir(0.0, 1.0, 0.0);
	const SBVector3 zDir(0.0, 0.0, 1.0);

	const SBQuantity::nanometer xLength = SBQuantity::nanometer(ADNConstants::BP_RISE * xSize);
	const SBQuantity::nanometer yLength = SBQuantity::nanometer(ADNConstants::BP_RISE * ySize);
	const SBQuantity::nanometer zLength = SBQuantity::nanometer(ADNConstants::BP_RISE * zSize);

	if (mock) {

		const SBPosition3 topLeftFront = positionData.FirstPosition;

		// create 12 double helices
		DASCreator::AddDoubleStrandToADNPart(part, xSize, topLeftFront,  xDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, ySize, topLeftFront, -yDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, zSize, topLeftFront, -zDir, true);

		const SBPosition3 bottomRightFront = topLeftFront + (xLength * xDir - yLength * yDir);

		DASCreator::AddDoubleStrandToADNPart(part, xSize, bottomRightFront, -xDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, ySize, bottomRightFront,  yDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, zSize, bottomRightFront, -zDir, true);

		const SBPosition3 bottomLeftBack = topLeftFront - (yLength * yDir + zLength * zDir);

		DASCreator::AddDoubleStrandToADNPart(part, xSize, bottomLeftBack, xDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, ySize, bottomLeftBack, yDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, zSize, bottomLeftBack, zDir, true);

		const SBPosition3 topRightBack = topLeftFront + (xLength * xDir - zLength * zDir);

		DASCreator::AddDoubleStrandToADNPart(part, xSize, topRightBack, -xDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, ySize, topRightBack, -yDir, true);
		DASCreator::AddDoubleStrandToADNPart(part, zSize, topRightBack,  zDir, true);

		return part;

	}
	else {

		DASPolyhedron p = DASPolyhedron();
		std::map<int, SBPosition3> vertices;
		std::map<int, std::vector<int>> faces;

		// current position is top-left
		const SBPosition3 vertex1 = positionData.FirstPosition;
		const SBPosition3 vertex2 = vertex1 + xLength * xDir;
		const SBPosition3 vertex3 = vertex2 - yLength * yDir;
		const SBPosition3 vertex4 = vertex3 - xLength * xDir;
		const SBPosition3 vertex5 = vertex1 - zLength * zDir;
		const SBPosition3 vertex6 = vertex5 + xLength * xDir;
		const SBPosition3 vertex7 = vertex6 - yLength * yDir;
		const SBPosition3 vertex8 = vertex7 - xLength * xDir;

		vertices.insert(std::make_pair(0, vertex2));
		vertices.insert(std::make_pair(1, vertex1));
		vertices.insert(std::make_pair(2, vertex4));
		vertices.insert(std::make_pair(3, vertex3));
		vertices.insert(std::make_pair(4, vertex7));
		vertices.insert(std::make_pair(5, vertex6));
		vertices.insert(std::make_pair(6, vertex5));
		vertices.insert(std::make_pair(7, vertex8));

		// faces
		std::vector<int> face1 = { 3, 0, 1, 2 };
		std::vector<int> face2 = { 3, 4, 5, 0 };
		std::vector<int> face3 = { 0, 5, 6, 1 };
		std::vector<int> face4 = { 1, 6, 7, 2 };
		std::vector<int> face5 = { 2, 7, 4, 3 };
		std::vector<int> face6 = { 5, 4, 7, 6 };

		faces.insert(std::make_pair(0, face1));
		faces.insert(std::make_pair(1, face2));
		faces.insert(std::make_pair(2, face3));
		faces.insert(std::make_pair(3, face4));
		faces.insert(std::make_pair(4, face5));
		faces.insert(std::make_pair(5, face6));

		p.BuildPolyhedron(vertices, faces);

		DASDaedalus* alg = new DASDaedalus();
		int minSize = std::min(bpSize, zSize);
		std::string seq = "";
		alg->SetMinEdgeLength(minSize);
		part = alg->ApplyAlgorithm(seq, p, false);

		if (part != nullptr) {

			part->setName(SEAdenitaCoreSEApp::getAdenitaApp()->getUniquePartName("Cuboid"));

		}

	}

	return part;

}

ADNPointer<ADNPart> SEWireframeEditor::generateWireframe(bool mock) {

	auto radius = (positionData.SecondPosition - positionData.FirstPosition).norm();
	unsigned int numNucleotides = 0;

	ADNPointer<ADNPart> part = nullptr;
	std::string filename = std::string();

	std::string partName = "";

	double a = sqrt(pow(radius.getValue(), 2) * 2);

	if (wireframeType == DASCreator::EditorType::Tetrahedron) {
		part = new ADNPart();
		partName = "Tetrahedron";
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) * 1.3;
		filename = SB_ELEMENT_PATH + "/Data/01_tetrahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Cube) {
		part = new ADNPart();
		partName = "Cube";
		numNucleotides = a / (ADNConstants::BP_RISE * 1000);
		filename = SB_ELEMENT_PATH + "/Data/02_cube.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Octahedron) {
		part = new ADNPart();
		partName = "Octahedron";
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/03_octahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Dodecahedron) {
		part = new ADNPart();
		partName = "Dodecahedron";
		a /= 2;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/04_dodecahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Icosahedron) {
		part = new ADNPart();
		partName = "Icosahedron";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/05_icosahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Cuboctahedron) {
		part = new ADNPart();
		partName = "Cuboctahedron";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/06_cuboctahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Icosidodecahedron) {
		part = new ADNPart();
		partName = "Icosidodecahedron";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/07_icosidodecahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Rhombicuboctahedron) {
		part = new ADNPart();
		partName = "Rhombicuboctahedron";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/08_rhombicuboctahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Snub_cube) {
		part = new ADNPart();
		partName = "Snub cube";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/09_snub_cube.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Truncated_cube) {
		part = new ADNPart();
		partName = "Truncated cube";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/10_truncated_cube.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Truncated_cuboctahedron) {
		part = new ADNPart();
		partName = "Truncated cuboctahedron";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/11_truncated_cuboctahedron.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Helix) {
		part = new ADNPart();
		partName = "Helix";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/49_helix.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Stickman) {
		part = new ADNPart();
		partName = "Stickman";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/51_stickman.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Bottle) {
		part = new ADNPart();
		partName = "Bottle";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/52_bottle.ply";
	}
	else if (wireframeType == DASCreator::EditorType::Bunny) {
		part = new ADNPart();
		partName = "Bunny";
		a /= 4;
		numNucleotides = a / (ADNConstants::BP_RISE * 1000) / 1.5;
		filename = SB_ELEMENT_PATH + "/Data/53_bunny.ply";
	}

	int min_edge_size = 31;
	if (numNucleotides > 31) {

		int quot;
		remquo(numNucleotides, 10.5, &quot);
		min_edge_size = int(std::floor(float(quot * 10.5)));

	}

	if (filename.size()) if (QFileInfo::exists(QString::fromStdString(filename))) {

		DASPolyhedron polyhedron = DASPolyhedron(filename);
		polyhedron.Center(positionData.FirstPosition);

		if (mock) {

			part = createMockDaedalusWireframe(polyhedron, min_edge_size);

		}
		else {

			DASDaedalus* alg = new DASDaedalus();
			alg->SetMinEdgeLength(min_edge_size);
			std::string seq = "";
			part = alg->ApplyAlgorithm(seq, polyhedron, false, true);

			if (part != nullptr && !partName.empty()) {

				part->setName(SEAdenitaCoreSEApp::getAdenitaApp()->getUniquePartName(partName));

			}

		}

	}

	return part;

}

void SEWireframeEditor::sendPartToAdenita(ADNPointer<ADNPart> part) {

	if (part != nullptr) {

		SEAdenitaCoreSEApp::getAdenitaApp()->addPartToDocument(part);
		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

ADNPointer<ADNPart> SEWireframeEditor::createMockDaedalusWireframe(DASPolyhedron& polyhedron, int min_edge_length) {

	ADNPointer<ADNPart> mock = new ADNPart();

	DASDaedalus* alg = new DASDaedalus();
	alg->SetMinEdgeLength(min_edge_length);
	alg->SetEdgeBps(min_edge_length, mock, polyhedron);
	alg->SetVerticesPositions(mock, polyhedron, false);
	alg->InitEdgeMap(mock, polyhedron);

	return mock;

}

SBCContainerUUID SEWireframeEditor::getUUID() const { return SBCContainerUUID("ED358EAC-14D1-A0EA-9A3A-F8035E019249"); }

QString SEWireframeEditor::getName() const {

	// SAMSON Element generator pro tip: this name should not be changed

	return "SEWireframeEditor";

}

QPixmap SEWireframeEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/SEWireframeEditorIcon.png"));

}

int SEWireframeEditor::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SEWireframeEditor::getShortcut() const {

	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence("");

}

QString SEWireframeEditor::getToolTip() const {

	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Create wireframe DNA nanostructures using Daedalus");

}

void SEWireframeEditor::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEWireframeEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SEWireframeEditor::getDescription() const {

	return QObject::tr("Adenita | Daedalus DNA Nanostructures");

}

void SEWireframeEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	//const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/wireframeCreator.png");
	//SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

}

void SEWireframeEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	SAMSON::unsetViewportCursor();

}

void SEWireframeEditor::resetData() {

	isPressing = false;

	DASCreatorEditors::resetPositions(positionData);
	displayFlag = false;
	tempPart = nullptr;

}

void SEWireframeEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SEWireframeEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	SEConfig& config = SEConfig::GetInstance();

	if (!displayFlag) return;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const SBPosition3 currentPosition = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());

	if (wireframeType == DASCreator::EditorType::Cuboid) {

		if (positionData.positionsCounter < 3) {

			SBVector3 xDir(1.0, 0.0, 0.0);
			SBVector3 yDir(0.0, 1.0, 0.0);
			SBVector3 zDir(0.0, 0.0, 1.0);

			if (positionData.positionsCounter == 1) {

				const SBQuantity::length radius = (currentPosition - positionData.FirstPosition).norm();
				const SBQuantity::length side = 2 * radius;
				auto xySide = DASDaedalus::CalculateEdgeSize(side);
				const SBPosition3 xPos = positionData.FirstPosition + SBQuantity::nanometer(xySide * ADNConstants::BP_RISE) * xDir;
				const SBPosition3 yPos = positionData.FirstPosition - SBQuantity::nanometer(xySide * ADNConstants::BP_RISE) * yDir;

				const std::string xyText = std::to_string(xySide) + " bp";
				ADNDisplayHelper::displayLine(positionData.FirstPosition, xPos, xyText);
				ADNDisplayHelper::displayLine(positionData.FirstPosition, yPos, xyText);
				if (config.preview_editor) tempPart = generateCuboid(currentPosition, true);
				ADNDisplayHelper::displayPart(tempPart);

			}
			else if (positionData.positionsCounter == 2) {

				const SBQuantity::length radius = (positionData.SecondPosition - positionData.FirstPosition).norm();
				const SBQuantity::length side = 2 * radius;
				auto xySide = DASDaedalus::CalculateEdgeSize(side);
				auto zLength = DASDaedalus::CalculateEdgeSize((currentPosition - positionData.SecondPosition).norm());
				const SBPosition3 xPos = positionData.FirstPosition + SBQuantity::nanometer(xySide * ADNConstants::BP_RISE) * xDir;
				const SBPosition3 yPos = positionData.FirstPosition - SBQuantity::nanometer(xySide * ADNConstants::BP_RISE) * yDir;
				const SBPosition3 zPos = positionData.FirstPosition - SBQuantity::nanometer(zLength * ADNConstants::BP_RISE) * zDir;

				const std::string xyText = std::to_string(xySide) + " bp";
				const std::string zText = std::to_string(zLength) + " bp";
				ADNDisplayHelper::displayLine(positionData.FirstPosition, xPos, xyText);
				ADNDisplayHelper::displayLine(positionData.FirstPosition, yPos, xyText);
				ADNDisplayHelper::displayLine(positionData.FirstPosition, zPos, zText);

				//positionData.ThirdPosition = currentPosition;

				if (config.preview_editor) tempPart = generateCuboid(currentPosition, true);
				ADNDisplayHelper::displayPart(tempPart);

			}

		}

	}
	else {

		if (positionData.positionsCounter == 1) {

			ADNDisplayHelper::displayLine(positionData.FirstPosition, currentPosition);
			positionData.SecondPosition = currentPosition;

		}

		if (config.preview_editor) {

			tempPart = generateWireframe(true);
			ADNDisplayHelper::displayPart(tempPart, 1000.0f, 1.0f);

		}

	}

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

}

void SEWireframeEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SEWireframeEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (isPressing) {

		event->accept();
		return;

	}

	if (event->button() & Qt::LeftButton) {

		if (positionData.positionsCounter == 0)
			resetData();

		if (wireframeType == DASCreator::EditorType::Cuboid) {

			if (positionData.positionsCounter == 0) {

				positionData.FirstPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
				positionData.SecondPosition = positionData.FirstPosition;
				positionData.ThirdPosition = positionData.FirstPosition;
				positionData.positionsCounter = 1;

			}
			else if (positionData.positionsCounter == 2) {

				positionData.ThirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

			}

		}
		else {

			if (positionData.positionsCounter == 0) {

				positionData.FirstPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
				positionData.SecondPosition = positionData.FirstPosition;
				positionData.positionsCounter = 1;

				positionData.FirstVector = SAMSON::getActiveCamera()->getBasisZ().normalizedVersion();
				positionData.vectorsCounter = 1;

			}

		}

		isPressing = true;

		event->accept();

		SAMSON::requestViewportUpdate();

	}

}

void SEWireframeEditor::mouseReleaseEvent(QMouseEvent* event) {

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

	if (isPressing && event->button() & Qt::LeftButton) {

		event->accept();

		isPressing = false;

		if (wireframeType == DASCreator::EditorType::Cuboid) {

			if (positionData.positionsCounter == 1) {

				positionData.SecondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
				positionData.positionsCounter = 2;

				if (wireframeType == DASCreator::EditorType::Cuboid)
					positionData.ThirdPosition = positionData.SecondPosition;

			}
			else if (positionData.positionsCounter == 2) {

				positionData.ThirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
				positionData.positionsCounter = 3;

				//SAMSON::beginHolding("Add cuboid");

				ADNPointer<ADNPart> part = generateCuboid(positionData.ThirdPosition);
				sendPartToAdenita(part);

				//SAMSON::endHolding();

				resetData();

			}

		}
		else {

			if (positionData.positionsCounter == 1) {

				positionData.SecondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
				positionData.positionsCounter = 2;

				//SAMSON::beginHolding("Add wireframe");

				ADNPointer<ADNPart> part = generateWireframe();
				sendPartToAdenita(part);

				//SAMSON::endHolding();

				resetData();

			}

		}

		SAMSON::requestViewportUpdate();

	}

}

void SEWireframeEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	const bool hasMiddleButton = event->buttons() & Qt::MiddleButton;
	const bool hasLeftButton = event->buttons() & Qt::LeftButton;
	const bool hasRightButton = event->buttons() & Qt::RightButton;

	if (!hasLeftButton) {

		isPressing = false;

	}

	if (isPressing && hasLeftButton) displayFlag = true;

	if (!hasMiddleButton && !hasLeftButton && !hasRightButton) {

		event->accept();

	}

	if (positionData.positionsCounter == 1) {

		positionData.SecondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());
		if (wireframeType == DASCreator::EditorType::Cuboid)
			positionData.ThirdPosition = positionData.SecondPosition;
		event->accept();

		//if (wireframeType == DASCreator::EditorType::Cuboid)
		//    positionData.SecondPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

	}
	else if (positionData.positionsCounter == 2) {

		if (wireframeType == DASCreator::EditorType::Cuboid) {

			positionData.ThirdPosition = SAMSON::getWorldPositionFromViewportPosition(event->pos().x(), event->pos().y());

			if (!hasMiddleButton && !hasRightButton)
				event->accept();

		}

	}

	SAMSON::requestViewportUpdate();

}

void SEWireframeEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (isPressing) event->accept();

}

void SEWireframeEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SEWireframeEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (displayFlag) {

		if (event->key() == Qt::Key_Escape) {

			event->accept();

			resetData();

			SAMSON::requestViewportUpdate();

		}

	}

}

void SEWireframeEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

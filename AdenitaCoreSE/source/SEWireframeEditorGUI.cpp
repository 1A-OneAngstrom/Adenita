#include "SEWireframeEditorGUI.hpp"
#include "SEWireframeEditor.hpp"
#include "SEAdenitaCoreSEAppGUI.hpp"


SEWireframeEditorGUI::SEWireframeEditorGUI(SEWireframeEditor* editor) {

	this->editor = editor;
	ui.setupUi( this );

}

void SEWireframeEditorGUI::initializeUI() {

	if (uiInitializedFlag) return;

	uiInitializedFlag = true;

	const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/");

	QIcon tetrahedronIcon(QPixmap(iconPath + "tetrahedron.png"));
	ui.radioButtonTetrahedron->setIcon(tetrahedronIcon);

	QIcon cubeIcon(QPixmap(iconPath + "cube.png"));
	ui.radioButtonCube->setIcon(cubeIcon);

	QIcon cuboctahedronIcon(QPixmap(iconPath + "cuboctahedron.png"));
	ui.radioButtonCuboctahedron->setIcon(cuboctahedronIcon);

	QIcon dodecahedronIcon(QPixmap(iconPath + "dodecahedron.png"));
	ui.radioButtonDodecahedron->setIcon(dodecahedronIcon);

	QIcon icosahedronIcon(QPixmap(iconPath + "icosahedron.png"));
	ui.radioButtonIcosahedron->setIcon(icosahedronIcon);

	QIcon icosidodecahedronIcon(QPixmap(iconPath + "icosidodecahedron.png"));
	ui.radioButtonIcosidodecahedron->setIcon(icosidodecahedronIcon);

	QIcon octahedronIcon(QPixmap(iconPath + "octahedron.png"));
	ui.radioButtonOctahedron->setIcon(octahedronIcon);

	QIcon rhombicuboctahedronIcon(QPixmap(iconPath + "rhombicuboctahedron.png"));
	ui.radioButtonRhombicuboctahedron->setIcon(rhombicuboctahedronIcon);

	QIcon snubCubeIcon(QPixmap(iconPath + "snub_Cube.png"));
	ui.radioButtonSnub_cube->setIcon(snubCubeIcon);

	QIcon truncated_cubeIcon(QPixmap(iconPath + "truncated_cube.png"));
	ui.radioButtonTruncated_cube->setIcon(truncated_cubeIcon);

	QIcon truncated_cuboctahedronIcon(QPixmap(iconPath + "Truncated_cuboctahedron.png"));
	ui.radioButtonTruncated_cuboctahedron->setIcon(truncated_cuboctahedronIcon);

	ui.radioButtonCuboid->setIcon(cubeIcon);

#ifndef ADENITA_DEBUG
	ui.radioButtonHelix->setVisible(false);
	ui.radioButtonStickman->setVisible(false);
	ui.radioButtonBunny->setVisible(false);
#endif

	// connect signals

	for (QRadioButton* radioButton : findChildren<QRadioButton*>()) {

		radioButton->setMinimumHeight(50);
		connect(radioButton, &QRadioButton::clicked, this, &SEWireframeEditorGUI::onCurrentWireframeTemplateChanged);

	}

}

SEWireframeEditorGUI::~SEWireframeEditorGUI() {

}

SEWireframeEditor* SEWireframeEditorGUI::getEditor() const { return editor; }

void SEWireframeEditorGUI::loadSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEWireframeEditorGUI::saveSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEWireframeEditorGUI::showEvent(QShowEvent* event) {

	initializeUI();

	SBGWindowWidget::showEvent(event);

}

void SEWireframeEditorGUI::onCurrentWireframeTemplateChanged() {

	if (ui.radioButtonTetrahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Tetrahedron);
	else if (ui.radioButtonCube->isChecked()) editor->setWireframeType(DASCreator::EditorType::Cube);
	else if (ui.radioButtonCuboid->isChecked()) editor->setWireframeType(DASCreator::EditorType::Cuboid);
	else if (ui.radioButtonOctahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Octahedron);
	else if (ui.radioButtonDodecahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Dodecahedron);
	else if (ui.radioButtonCuboctahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Cuboctahedron);
	else if (ui.radioButtonIcosahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Icosahedron);
	else if (ui.radioButtonIcosidodecahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Icosidodecahedron);
	else if (ui.radioButtonRhombicuboctahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Rhombicuboctahedron);
	else if (ui.radioButtonSnub_cube->isChecked()) editor->setWireframeType(DASCreator::EditorType::Snub_cube);
	else if (ui.radioButtonTruncated_cube->isChecked()) editor->setWireframeType(DASCreator::EditorType::Truncated_cube);
	else if (ui.radioButtonTruncated_cuboctahedron->isChecked()) editor->setWireframeType(DASCreator::EditorType::Truncated_cuboctahedron);
	else if (ui.radioButtonHelix->isChecked()) editor->setWireframeType(DASCreator::EditorType::Helix);
	else if (ui.radioButtonStickman->isChecked()) editor->setWireframeType(DASCreator::EditorType::Stickman);
	else if (ui.radioButtonBunny->isChecked()) editor->setWireframeType(DASCreator::EditorType::Bunny);

}

SBCContainerUUID SEWireframeEditorGUI::getUUID() const { return SBCContainerUUID( "3A914FAF-691B-B3A1-3B91-58E77E7CCC6D" );}

QPixmap SEWireframeEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEWireframeEditorIcon.png"));

}

QString SEWireframeEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return "Wireframe Creator"; 

}

int SEWireframeEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SEWireframeEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

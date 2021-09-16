#include "SEWireframeEditorGUI.hpp"
#include "SEWireframeEditor.hpp"


SEWireframeEditorGUI::SEWireframeEditorGUI(SEWireframeEditor* editor) {

	ui.setupUi( this );
	this->editor = editor;

	const std::string iconPath = SB_ELEMENT_PATH + "/Resource/icons/";

	QIcon tetrahedronIcon;
	tetrahedronIcon.addFile(QString(std::string(iconPath + "tetrahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbTetrahedron->setIcon(tetrahedronIcon);

	QIcon cubeIcon;
	cubeIcon.addFile(QString(std::string(iconPath + "cube.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbCube->setIcon(cubeIcon);
  
	QIcon cuboctahedronIcon;
	cuboctahedronIcon.addFile(QString(std::string(iconPath + "cuboctahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbCuboctahedron->setIcon(cuboctahedronIcon);

	QIcon dodecahedronIcon;
	dodecahedronIcon.addFile(QString(std::string(iconPath + "dodecahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbDodecahedron->setIcon(dodecahedronIcon);

	QIcon icosahedronIcon;
	icosahedronIcon.addFile(QString(std::string(iconPath + "icosahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbIcosahedron->setIcon(icosahedronIcon);

	QIcon icosidodecahedronIcon;
	icosidodecahedronIcon.addFile(QString(std::string(iconPath + "icosidodecahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbIcosidodecahedron->setIcon(icosidodecahedronIcon);

	QIcon octahedronIcon;
	octahedronIcon.addFile(QString(std::string(iconPath + "octahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbOctahedron->setIcon(octahedronIcon);

	QIcon rhombicuboctahedronIcon;
	rhombicuboctahedronIcon.addFile(QString(std::string(iconPath + "rhombicuboctahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbRhombicuboctahedron->setIcon(rhombicuboctahedronIcon);

	QIcon snubCubeIcon;
	snubCubeIcon.addFile(QString(std::string(iconPath + "snub_Cube.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbSnub_cube->setIcon(snubCubeIcon);

	QIcon truncated_cubeIcon;
	truncated_cubeIcon.addFile(QString(std::string(iconPath + "truncated_cube.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbTruncated_cube->setIcon(truncated_cubeIcon);

	QIcon truncated_cuboctahedronIcon;
	truncated_cuboctahedronIcon.addFile(QString(std::string(iconPath + "Truncated_cuboctahedron.png").c_str()), QSize(), QIcon::Normal, QIcon::Off);
	ui.rdbTruncated_cuboctahedron->setIcon(truncated_cuboctahedronIcon);

	ui.rdbCuboid->setIcon(cubeIcon);

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

void SEWireframeEditorGUI::onTetrahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Tetrahedron);
}

void SEWireframeEditorGUI::onCubeClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Cube);
}

void SEWireframeEditorGUI::onCuboidClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Cuboid);
}

void SEWireframeEditorGUI::onOctahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Octahedron);
}

void SEWireframeEditorGUI::onDodecahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Dodecahedron);
}

void SEWireframeEditorGUI::onCubocahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Cubocahedron);
}

void SEWireframeEditorGUI::onIcosahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Icosahedron);
}

void SEWireframeEditorGUI::onIcosidodecahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Icosidodecahedron);
}

void SEWireframeEditorGUI::onRhombicuboctahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Rhombicuboctahedron);
}

void SEWireframeEditorGUI::onSnubCubeClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Snub_cube);
}

void SEWireframeEditorGUI::onTruncatedCubeClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Truncated_cube);
}

void SEWireframeEditorGUI::onTruncatedCuboctahedronClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Truncated_cuboctahedron);
}

void SEWireframeEditorGUI::onHelixClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Helix);
}

void SEWireframeEditorGUI::onStickmanClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Stickman);
}

void SEWireframeEditorGUI::onBunnyClicked()
{
	editor->setWireframeType(DASCreator::EditorType::Bunny);
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

	return "SEWireframeEditor"; 

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

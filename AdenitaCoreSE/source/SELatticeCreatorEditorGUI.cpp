#include "SELatticeCreatorEditorGUI.hpp"
#include "SELatticeCreatorEditor.hpp"


SELatticeCreatorEditorGUI::SELatticeCreatorEditorGUI(SELatticeCreatorEditor* editor) {

	ui.setupUi( this );
	this->editor = editor;

	ui.labelPatterns->setVisible(false);
	ui.labelPatternsX->setVisible(false);
	ui.labelPatternsY->setVisible(false);
	ui.labelPatternsZ->setVisible(false);
	ui.comboBoxXPattern->setVisible(false);
	ui.comboBoxYPattern->setVisible(false);
	ui.comboBoxZPattern->setVisible(false);

}

SELatticeCreatorEditorGUI::~SELatticeCreatorEditorGUI() {

}

SELatticeCreatorEditor* SELatticeCreatorEditorGUI::getEditor() const { return editor; }

void SELatticeCreatorEditorGUI::loadSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SELatticeCreatorEditorGUI::saveSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SELatticeCreatorEditorGUI::setHoneycomb() {

	getEditor()->setLatticeType(LatticeType::Honeycomb);
	ui.spbMaxXds->setMaximum(32);
	ui.spbMaxYds->setMaximum(30);

}

void SELatticeCreatorEditorGUI::setSquare() {

	getEditor()->setLatticeType(LatticeType::Square);
	ui.spbMaxXds->setMaximum(50);
	ui.spbMaxYds->setMaximum(50);

}

void SELatticeCreatorEditorGUI::onZPatternChanged(int index) {

	ZLatticePattern pattern = static_cast<ZLatticePattern>(index);
	getEditor()->setZPattern(pattern);

}

void SELatticeCreatorEditorGUI::onMaxXdsChanged(int val) {

	getEditor()->setMaxXDoubleStrands(val);

}

void SELatticeCreatorEditorGUI::onMaxYdsChanged(int val) {

	getEditor()->setMaxYDoubleStrands(val);

}

void SELatticeCreatorEditorGUI::onMaxZBpsChanged(int val) {

	getEditor()->setMaxZBasePairs(val);

}

SBCContainerUUID SELatticeCreatorEditorGUI::getUUID() const { return SBCContainerUUID( "A9C48A6E-8BD1-B387-6A63-1CB8A19C1948" );}

QPixmap SELatticeCreatorEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SELatticeCreatorEditorIcon.png"));

}

QString SELatticeCreatorEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return "Lattice Creator"; 

}

int SELatticeCreatorEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SELatticeCreatorEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

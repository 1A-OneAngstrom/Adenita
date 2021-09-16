#include <QInputDialog>
#include "SEDSDNACreatorEditorGUI.hpp"
#include "SEDSDNACreatorEditor.hpp"


SEDSDNACreatorEditorGUI::SEDSDNACreatorEditorGUI(SEDSDNACreatorEditor* editor) {

	ui.setupUi( this );
	this->editor = editor;

}

SEDSDNACreatorEditorGUI::~SEDSDNACreatorEditorGUI() {

}

SEDSDNACreatorEditor* SEDSDNACreatorEditorGUI::getEditor() const { return editor; }

void SEDSDNACreatorEditorGUI::loadSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEDSDNACreatorEditorGUI::saveSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

std::string SEDSDNACreatorEditorGUI::AskUserForSequence(int l) {

	bool ok;
	QString def = QString(l, 'N');
	const std::string title = "Base pairs: " + std::to_string(l);
	const QString seq = QInputDialog::getText(this, tr("Set sequence"), tr(title.c_str()), QLineEdit::Normal, def, &ok);
	std::string res = "";
	if (ok)
		res = seq.toStdString();

	return res;

}

void SEDSDNACreatorEditorGUI::onSetDSDNA(bool b) {
	getEditor()->setDoubleStrandMode(b);
}

void SEDSDNACreatorEditorGUI::onSetSSDNA(bool b) {
	getEditor()->setDoubleStrandMode(!b);
}

void SEDSDNACreatorEditorGUI::onSetCircular(bool c) {
	getEditor()->setCircularStrandsMode(c);
}

void SEDSDNACreatorEditorGUI::onSetManual(bool b) {
	getEditor()->setManualFlag(b);
}

void SEDSDNACreatorEditorGUI::onSetNumNucleotides(int n) {
	getEditor()->setNumberOfNucleotides(n);
}

void SEDSDNACreatorEditorGUI::onShowBox(bool s) {

	getEditor()->setShowBoxFlag(s);
	if (s)
		onChangeBoxSize();
	else
		SAMSON::requestViewportUpdate();

}

void SEDSDNACreatorEditorGUI::onChangeBoxSize() {

	const SBQuantity::nanometer height = SBQuantity::nanometer(ui.spnBoxHeight->value());
	const SBQuantity::nanometer width  = SBQuantity::nanometer(ui.spnBoxWidth->value());
	const SBQuantity::nanometer depth  = SBQuantity::nanometer(ui.spnBoxDepth->value());
	getEditor()->setBoxSize(height, width, depth);

	SAMSON::requestViewportUpdate();

}

void SEDSDNACreatorEditorGUI::onSetSequence(bool s) {
	getEditor()->setSequenceFlag(s);
}

SBCContainerUUID SEDSDNACreatorEditorGUI::getUUID() const { return SBCContainerUUID( "751903AE-14BC-F0B9-01D9-D2CF8412AEF9" );}

QPixmap SEDSDNACreatorEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEDSDNACreatorEditorIcon.png"));

}

QString SEDSDNACreatorEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return "DNA Creator"; 

}

int SEDSDNACreatorEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SEDSDNACreatorEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

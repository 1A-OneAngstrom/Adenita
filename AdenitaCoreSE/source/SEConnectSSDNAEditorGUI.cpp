#include "SEConnectSSDNAEditor.hpp"


SEConnectSSDNAEditorGUI::SEConnectSSDNAEditorGUI(SEConnectSSDNAEditor* editor) {

	this->editor = editor;
	ui.setupUi(this);
	setupHelpText();
	updateSequenceControls();

}

SEConnectSSDNAEditorGUI::~SEConnectSSDNAEditorGUI() {

}

SEConnectSSDNAEditor* SEConnectSSDNAEditorGUI::getEditor() const { return editor; }

void SEConnectSSDNAEditorGUI::loadSettings(SBGSettings *settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEConnectSSDNAEditorGUI::saveSettings(SBGSettings *settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEConnectSSDNAEditorGUI::onSetSequence(QString seq) {

	getEditor()->setSequence(seq.toStdString());

}

void SEConnectSSDNAEditorGUI::onInsert(bool e) {

	getEditor()->setConcatFlag(e);
	updateSequenceControls();

}

void SEConnectSSDNAEditorGUI::onAuto(bool e) {

	getEditor()->setAutoSequenceFlag(e);
	updateSequenceControls();

}

void SEConnectSSDNAEditorGUI::setupHelpText() {

	ui.groupBoxConnectionType->setToolTip(
		tr("Choose whether the connection should be a single-strand linker or a double-strand bridge."));
	ui.radioButtonSingleStrand->setToolTip(
		tr("Create a single-stranded connection between the selected nucleotides. Use this for ssDNA crossovers or flexible linkers between DNA arms."));
	ui.radioButtonDoubleStrand->setToolTip(
		tr("Create a double-stranded connection between the selected nucleotides. Use this when the bridge should preserve dsDNA geometry."));
	ui.groupBoxLinker->setToolTip(
		tr("When enabled, Adenita inserts new linker nucleotides between the selected endpoints. When disabled, Adenita only reconnects or merges the selected strands where possible."));
	ui.labelSequence->setToolTip(
		tr("Sequence used for inserted linker bases."));
	ui.lineSequence->setToolTip(
		tr("Sequence to use for the inserted linker. Use bases such as A, C, G, T, or N. This field is ignored when insertion is disabled or auto-fill is enabled."));
	ui.checkBoxAutoFill->setToolTip(
		tr("Estimate the number of inserted linker bases from the distance between selected endpoints, and fill the sequence with N. Use this when you care about linker length but not the exact bases."));

}

void SEConnectSSDNAEditorGUI::updateSequenceControls() {

	const bool insert = ui.groupBoxLinker->isChecked();
	const bool autoFill = ui.checkBoxAutoFill->isChecked();

	// Sequence text only affects inserted linkers; keep disabled states aligned with that modeling path.
	ui.checkBoxAutoFill->setEnabled(insert);
	ui.lineSequence->setEnabled(insert && !autoFill);

}

SBCContainerUUID SEConnectSSDNAEditorGUI::getUUID() const { return SBCContainerUUID("CDC75BAA-A7AD-F837-49F4-E0F14DF87181"); }

QPixmap SEConnectSSDNAEditorGUI::getLogo() const {
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/connectDS.png"));

}

QString SEConnectSSDNAEditorGUI::getName() const {

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return tr("Connect DNA Strands");

}

int SEConnectSSDNAEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SEConnectSSDNAEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

  return ADNAuxiliary::AdenitaCitation();

}

void SEConnectSSDNAEditorGUI::onSelectMode() {

	const bool xo = ui.radioButtonSingleStrand->isChecked();
	getEditor()->setConnectionMode(xo);

}

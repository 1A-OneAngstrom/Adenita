#include "SEDSDNACreatorEditorGUI.hpp"
#include "SEDSDNACreatorEditor.hpp"

#include <QInputDialog>


SEDSDNACreatorEditorGUI::SEDSDNACreatorEditorGUI(SEDSDNACreatorEditor* editor) {

	this->editor = editor;
	ui.setupUi(this);
	setupHelpText();

}

SEDSDNACreatorEditorGUI::~SEDSDNACreatorEditorGUI() {

}

SEDSDNACreatorEditor* SEDSDNACreatorEditorGUI::getEditor() const { return editor; }

void SEDSDNACreatorEditorGUI::loadSettings(SBGSettings *settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

	const bool isDSDNA = settings->loadBoolValue("isDSDNA", true);
	if (isDSDNA) ui.radioButtonDSDNA->setChecked(true);
	else ui.radioButtonSSDNA->setChecked(true);
	getEditor()->setDoubleStrandMode(isDSDNA);

	//ui.checkBoxSetScaffold->setChecked(settings->loadBoolValue("setScaffold", false));
	//onSetSequence(ui.checkBoxSetScaffold->isChecked());

	ui.checkBoxCircular->setChecked(settings->loadBoolValue("circular", false));
	onSetCircular(ui.checkBoxCircular->isChecked());
	ui.checkBoxManual->setChecked(settings->loadBoolValue("manual", false));
	onSetManual(ui.checkBoxManual->isChecked());
	ui.spinBoxNumberNucleotides->setValue(settings->loadIntValue("numberNucleotides", 12));
	onSetNumNucleotides(ui.spinBoxNumberNucleotides->value());

	ui.groupBoxShowBox->setChecked(settings->loadBoolValue("showBox", false));
	getEditor()->setShowBoxFlag(ui.groupBoxShowBox->isChecked());
	ui.doubleSpinBoxHeight->setValue(settings->loadDoubleValue("boxHeight", 100.0));
	ui.doubleSpinBoxWidth->setValue(settings->loadDoubleValue("boxWidth", 100.0));
	ui.doubleSpinBoxDepth->setValue(settings->loadDoubleValue("boxDepth", 100.0));
	updateBoxSize();

}

void SEDSDNACreatorEditorGUI::saveSettings(SBGSettings *settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

	settings->saveValue("isDSDNA", ui.radioButtonDSDNA->isChecked());
	//settings->saveValue("setScaffold", ui.checkBoxSetScaffold->isChecked());

	settings->saveValue("circular", ui.checkBoxCircular->isChecked());
	settings->saveValue("manual", ui.checkBoxManual->isChecked());
	settings->saveValue("numberNucleotides", ui.spinBoxNumberNucleotides->value());

	settings->saveValue("showBox", ui.groupBoxShowBox->isChecked());
	settings->saveValue("boxHeight", ui.doubleSpinBoxHeight->value());
	settings->saveValue("boxWidth", ui.doubleSpinBoxWidth->value());
	settings->saveValue("boxDepth", ui.doubleSpinBoxDepth->value());

}

void SEDSDNACreatorEditorGUI::setupHelpText() {

	ui.radioButtonSSDNA->setToolTip(
		tr("Create a single-strand DNA segment. Drag in the viewport to set direction and, unless Manual length is enabled, length."));
	ui.radioButtonDSDNA->setToolTip(
		tr("Create a double-strand DNA segment. Drag in the viewport to set direction and, unless Manual length is enabled, length."));
	ui.checkBoxCircular->setToolTip(
		tr("Create a circular DNA strand or double-strand ring. Drag sets the radius unless Manual length is enabled."));
	ui.checkBoxManual->setToolTip(
		tr("Use the numeric length instead of measuring length from the drag distance. The drag still sets direction for linear strands or radius orientation for circular strands."));
	ui.label_4->setToolTip(
		tr("Number of nucleotides for ssDNA, or base pairs for dsDNA."));
	ui.spinBoxNumberNucleotides->setToolTip(
		tr("Number of nucleotides for ssDNA, or base pairs for dsDNA. Used when Manual length is enabled."));
	ui.checkBoxSetScaffold->setToolTip(
		tr("Ask for a sequence after creating the strand, then apply it to the created strand."));
	ui.groupBoxShowBox->setToolTip(
		tr("Show a size guide box in the viewport. This is a visual guide, not a clipping volume."));
	ui.label->setToolTip(
		tr("Height of the viewport size guide box in nanometers."));
	ui.doubleSpinBoxHeight->setToolTip(
		tr("Height of the viewport size guide box in nanometers."));
	ui.label_2->setToolTip(
		tr("Width of the viewport size guide box in nanometers."));
	ui.doubleSpinBoxWidth->setToolTip(
		tr("Width of the viewport size guide box in nanometers."));
	ui.label_3->setToolTip(
		tr("Length/depth of the viewport size guide box in nanometers."));
	ui.doubleSpinBoxDepth->setToolTip(
		tr("Length/depth of the viewport size guide box in nanometers."));

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
		updateBoxSize();
	
	if (getEditor() == SAMSON::getActiveEditor())
		SAMSON::requestViewportUpdate();

}

void SEDSDNACreatorEditorGUI::updateBoxSize() {

	const SBQuantity::nanometer height = SBQuantity::nanometer(ui.doubleSpinBoxHeight->value());
	const SBQuantity::nanometer width = SBQuantity::nanometer(ui.doubleSpinBoxWidth->value());
	const SBQuantity::nanometer depth = SBQuantity::nanometer(ui.doubleSpinBoxDepth->value());
	getEditor()->setBoxSize(height, width, depth);

}

void SEDSDNACreatorEditorGUI::onChangeBoxSize() {

	updateBoxSize();

	if (getEditor() == SAMSON::getActiveEditor())
		SAMSON::requestViewportUpdate();

}

void SEDSDNACreatorEditorGUI::onSetSequence(bool s) {
	getEditor()->setSequenceFlag(s);
}

SBCContainerUUID SEDSDNACreatorEditorGUI::getUUID() const { return SBCContainerUUID("751903AE-14BC-F0B9-01D9-D2CF8412AEF9"); }

QPixmap SEDSDNACreatorEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEDSDNACreatorEditorIcon.png"));

}

QString SEDSDNACreatorEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return tr("DNA Strands Creator");

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

#include "SETaggingEditorGUI.hpp"
#include "SETaggingEditor.hpp"

#include <QSignalBlocker>


namespace {

DNABlocks targetBaseFromIndex(int index) {

	if (index == 0) return DNABlocks::DA;
	if (index == 1) return DNABlocks::DT;
	if (index == 2) return DNABlocks::DC;
	if (index == 3) return DNABlocks::DG;
	return DNABlocks::DI;

}

int targetBaseIndex(DNABlocks type) {

	if (type == DNABlocks::DA) return 0;
	if (type == DNABlocks::DT) return 1;
	if (type == DNABlocks::DC) return 2;
	if (type == DNABlocks::DG) return 3;
	return 4;

}

}


SETaggingEditorGUI::SETaggingEditorGUI(SETaggingEditor* editor) {

	this->editor = editor;
	ui.setupUi(this);
	setupHelpText();

	QObject::connect(ui.comboBoxTargetBase, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SETaggingEditorGUI::onTargetBaseChanged);

	if (getEditor() != nullptr)
		setTargetBaseFromEditor(getEditor()->getNucleotideType());
	updateTargetBaseControls();

}

SETaggingEditorGUI::~SETaggingEditorGUI() {

}

SETaggingEditor* SETaggingEditorGUI::getEditor() const { return editor; }

void SETaggingEditorGUI::setTargetBaseFromEditor(DNABlocks type) {

	const QSignalBlocker blocker(ui.comboBoxTargetBase);
	ui.comboBoxTargetBase->setCurrentIndex(targetBaseIndex(type));
	updateTargetBaseControls();

}

void SETaggingEditorGUI::loadSettings(SBGSettings *settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

	const bool isTags = settings->loadBoolValue("isTags", true);
	if (isTags) {

		ui.radioButtonTags->setChecked(true);
		getEditor()->setTaggingMode(SETaggingEditor::TaggingMode::Tags);

	}
	else {

		ui.radioButtonBase->setChecked(true);
		getEditor()->setTaggingMode(SETaggingEditor::TaggingMode::Base);

	}

	setTargetBaseIndex(settings->loadIntValue("targetBaseIndex", 4));
	updateTargetBaseControls();

}

void SETaggingEditorGUI::saveSettings(SBGSettings *settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

	settings->saveValue("isTags", ui.radioButtonTags->isChecked());
	settings->saveValue("targetBaseIndex", ui.comboBoxTargetBase->currentIndex());

}

SBCContainerUUID SETaggingEditorGUI::getUUID() const { return SBCContainerUUID("EA0EDF95-028E-4DE2-236F-49EB9D68BC71"); }

QPixmap SETaggingEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SETaggingEditorIcon.png"));

}

QString SETaggingEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return tr("Tag or Mutate Nucleotides");

}

int SETaggingEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SETaggingEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

void SETaggingEditorGUI::onModeChanged() {

	if (ui.radioButtonTags->isChecked())
		getEditor()->setTaggingMode(SETaggingEditor::TaggingMode::Tags);
	else
		getEditor()->setTaggingMode(SETaggingEditor::TaggingMode::Base);

	updateTargetBaseControls();

}

void SETaggingEditorGUI::onTargetBaseChanged(int index) {

	if (getEditor() == nullptr) return;
	getEditor()->setNucleotideType(targetBaseFromIndex(index));

}

void SETaggingEditorGUI::setupHelpText() {

	ui.labelInstruction->setToolTip(
		tr("Hover a nucleotide and click to apply the selected action. In Mutate base mode, choose the target base first."));
	ui.label->setToolTip(
		tr("Choose whether clicks set a text tag or mutate nucleotide bases."));
	ui.radioButtonTags->setToolTip(
		tr("Click a nucleotide to enter a text tag. Tags are stored on nucleotides and appear when exporting sequences."));
	ui.radioButtonBase->setToolTip(
		tr("Click a nucleotide to mutate it to the selected target base. Choose the target base in the combo box, or use Ctrl + mouse wheel."));
	ui.labelTargetBase->setToolTip(
		tr("Target base used in Mutate base mode."));
	ui.comboBoxTargetBase->setToolTip(
		tr("Target base used in Mutate base mode. Ctrl + mouse wheel can also cycle through A, T, C, G, and I."));

}

void SETaggingEditorGUI::updateTargetBaseControls() {

	const bool baseMode = ui.radioButtonBase->isChecked();
	ui.labelTargetBase->setEnabled(baseMode);
	ui.comboBoxTargetBase->setEnabled(baseMode);

}

void SETaggingEditorGUI::setTargetBaseIndex(int index) {

	if (index < 0 || index >= ui.comboBoxTargetBase->count())
		index = 4;

	{
		const QSignalBlocker blocker(ui.comboBoxTargetBase);
		ui.comboBoxTargetBase->setCurrentIndex(index);
	}

	// The combo mirrors editor state; loading settings updates both without emitting an extra GUI signal.
	onTargetBaseChanged(index);

}

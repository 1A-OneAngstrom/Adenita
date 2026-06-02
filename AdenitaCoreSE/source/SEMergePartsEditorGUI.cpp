#include "SEMergePartsEditorGUI.hpp"
#include "SEMergePartsEditor.hpp"
#include "ADNSamsonContext.hpp"

namespace {

bool ClearActiveSelection(const char* context) {

	SBDocument* document = ADNSamsonContext::GetActiveDocument(context);
	if (document == nullptr) return false;
	document->clearSelection();
	return true;

}

}

SEMergePartsEditorGUI::SEMergePartsEditorGUI(SEMergePartsEditor* editor) {

	this->editor = editor;
	ui.setupUi(this);
	setupHelpText();

	connect(ui.comboBoxMergeComponent1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SEMergePartsEditorGUI::onComboBoxMergeComponent1CurrentIndexChanged);
	connect(ui.comboBoxMergeComponent2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SEMergePartsEditorGUI::onComboBoxMergeComponent2CurrentIndexChanged);
	connect(ui.comboBoxMoveElement, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SEMergePartsEditorGUI::onComboBoxMoveElementCurrentIndexChanged);
	connect(ui.comboBoxMoveToComponent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SEMergePartsEditorGUI::onComboBoxMoveToComponentCurrentIndexChanged);

}

SEMergePartsEditorGUI::~SEMergePartsEditorGUI() {

}

SEMergePartsEditor* SEMergePartsEditorGUI::getEditor() const { return editor; }

void SEMergePartsEditorGUI::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEMergePartsEditorGUI::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEMergePartsEditorGUI::setupHelpText() {

	ui.pushButtonRefresh->setToolTip(
		tr("Refresh the component and element lists from the current document."));
	ui.groupBoxMergeComponents->setToolTip(
		tr("Merge two selected components into one component."));
	ui.label_2->setToolTip(
		tr("First component to merge."));
	ui.comboBoxMergeComponent1->setToolTip(
		tr("First component to merge. Refresh the list after changing the document."));
	ui.label_3->setToolTip(
		tr("Second component to merge."));
	ui.comboBoxMergeComponent2->setToolTip(
		tr("Second component to merge. It will be combined with the first selected component."));
	ui.pushButtonMerge->setToolTip(
		tr("Merge the two selected components."));
	ui.groupBoxMoveComponents->setToolTip(
		tr("Move one strand or component into another component."));
	ui.label_5->setToolTip(
		tr("Strand or component to move."));
	ui.comboBoxMoveElement->setToolTip(
		tr("Strand or component to move into the destination component."));
	ui.label_6->setToolTip(
		tr("Destination component."));
	ui.comboBoxMoveToComponent->setToolTip(
		tr("Destination component that receives the selected strand or component."));
	ui.pushButtonMove->setToolTip(
		tr("Move the selected strand or component into the destination component."));

}

void SEMergePartsEditorGUI::updatePartsList() {

	auto indexParts = getEditor()->getPartsList();
	auto indexElements = getEditor()->getElementsList();

	const int sel1 = ui.comboBoxMergeComponent1->currentIndex();
	const int sel2 = ui.comboBoxMergeComponent2->currentIndex();
	const int sel3 = ui.comboBoxMoveToComponent->currentIndex();
	const int sel4 = ui.comboBoxMoveElement->currentIndex();
	ui.comboBoxMergeComponent1->clear();
	ui.comboBoxMergeComponent2->clear();
	ui.comboBoxMoveToComponent->clear();
	ui.comboBoxMoveElement->clear();

	ui.comboBoxMergeComponent1->insertItem(0, QString::fromStdString("None"));
	ui.comboBoxMergeComponent2->insertItem(0, QString::fromStdString("None"));
	ui.comboBoxMoveToComponent->insertItem(0, QString::fromStdString("None"));
	ui.comboBoxMoveElement->insertItem(0, QString::fromStdString("None"));

	for (auto& pair : indexParts) {

		SBPointer<ADNPart> part = pair.second;
		std::string n = part->getName();
		ui.comboBoxMergeComponent1->insertItem(pair.first, QString::fromStdString(n));
		ui.comboBoxMergeComponent2->insertItem(pair.first, QString::fromStdString(n));
		ui.comboBoxMoveToComponent->insertItem(pair.first, QString::fromStdString(n));

	}

	for (auto& pair : indexElements) {

		auto element = pair.second;
		std::string n = element.GetName();

		ui.comboBoxMoveElement->insertItem(pair.first, QString::fromStdString(n));

	}

	if (indexParts.find(sel1) != indexParts.end()) ui.comboBoxMergeComponent1->setCurrentIndex(sel1);
	if (indexParts.find(sel2) != indexParts.end()) ui.comboBoxMergeComponent2->setCurrentIndex(sel2);
	if (indexParts.find(sel3) != indexParts.end()) ui.comboBoxMoveToComponent->setCurrentIndex(sel3);
	if (indexParts.find(sel4) != indexParts.end()) ui.comboBoxMoveElement->setCurrentIndex(sel4);

}

void SEMergePartsEditorGUI::onComboBoxMergeComponent1CurrentIndexChanged(int index) {

	if (!ClearActiveSelection(__func__)) return;
	getEditor()->selectComponent(index);

}

void SEMergePartsEditorGUI::onComboBoxMergeComponent2CurrentIndexChanged(int index) {

	if (!ClearActiveSelection(__func__)) return;
	getEditor()->selectComponent(index);

}

void SEMergePartsEditorGUI::onComboBoxMoveElementCurrentIndexChanged(int index) {

	if (!ClearActiveSelection(__func__)) return;
	getEditor()->selectElement(index);

}

void SEMergePartsEditorGUI::onComboBoxMoveToComponentCurrentIndexChanged(int index) {

	if (!ClearActiveSelection(__func__)) return;
	getEditor()->selectComponent(index);

}

void SEMergePartsEditorGUI::onMerge() {

	const int sel1 = ui.comboBoxMergeComponent1->currentIndex();
	const int sel2 = ui.comboBoxMergeComponent2->currentIndex();

	if (getEditor()->mergeParts(sel1, sel2))
		updatePartsList();

}

void SEMergePartsEditorGUI::onMove() {

	const int sel1 = ui.comboBoxMoveElement->currentIndex();
	const int sel2 = ui.comboBoxMoveToComponent->currentIndex();

	if (getEditor()->moveElement(sel1, sel2))
		updatePartsList();

}

SBCContainerUUID SEMergePartsEditorGUI::getUUID() const { return SBCContainerUUID("3F52AD7B-A478-D380-AA01-2041081D06CB"); }

QPixmap SEMergePartsEditorGUI::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/merge.png"));

}

QString SEMergePartsEditorGUI::getName() const {

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return "Merge Components Editor";

}

int SEMergePartsEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SEMergePartsEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

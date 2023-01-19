#include "SEMergePartsEditorGUI.hpp"
#include "SEMergePartsEditor.hpp"


SEMergePartsEditorGUI::SEMergePartsEditorGUI(SEMergePartsEditor* editor) {

	this->editor = editor;
	ui.setupUi( this );

	connect(ui.comboBoxMergeComponent1, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxMergeComponent1CurrentIndexChanged(int)));
	connect(ui.comboBoxMergeComponent2, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxMergeComponent2CurrentIndexChanged(int)));
	connect(ui.comboBoxMoveElement, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxMoveElementCurrentIndexChanged(int)));
	connect(ui.comboBoxMoveToComponent, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxMoveToComponentCurrentIndexChanged(int)));

}

SEMergePartsEditorGUI::~SEMergePartsEditorGUI() {

}

SEMergePartsEditor* SEMergePartsEditorGUI::getEditor() const { return editor; }

void SEMergePartsEditorGUI::loadSettings( SBGSettings *settings ) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEMergePartsEditorGUI::saveSettings( SBGSettings *settings ) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

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

		int i = pair.first;
		ADNPointer<ADNPart> part = pair.second;
		std::string n = part->getName();
		ui.comboBoxMergeComponent1->insertItem(i, QString::fromStdString(n));
		ui.comboBoxMergeComponent2->insertItem(i, QString::fromStdString(n));
		ui.comboBoxMoveToComponent->insertItem(i, QString::fromStdString(n));

	}

	for (auto& pair : indexElements) {

		int i = pair.first;
		auto element = pair.second;
		std::string n = element.GetName();
    
		ui.comboBoxMoveElement->insertItem(i, QString::fromStdString(n));

	}

	if (indexParts.find(sel1) != indexParts.end()) ui.comboBoxMergeComponent1->setCurrentIndex(sel1);
	if (indexParts.find(sel2) != indexParts.end()) ui.comboBoxMergeComponent2->setCurrentIndex(sel2);
	if (indexParts.find(sel3) != indexParts.end()) ui.comboBoxMoveToComponent->setCurrentIndex(sel3);
	if (indexParts.find(sel4) != indexParts.end()) ui.comboBoxMoveElement->setCurrentIndex(sel4);

}

void SEMergePartsEditorGUI::onComboBoxMergeComponent1CurrentIndexChanged(int index) {

	SAMSON::getActiveDocument()->clearSelection();
	getEditor()->selectComponent(index);

}

void SEMergePartsEditorGUI::onComboBoxMergeComponent2CurrentIndexChanged(int index) {

	SAMSON::getActiveDocument()->clearSelection();
	getEditor()->selectComponent(index);

}

void SEMergePartsEditorGUI::onComboBoxMoveElementCurrentIndexChanged(int index) {

	SAMSON::getActiveDocument()->clearSelection();
	getEditor()->selectElement(index);

}

void SEMergePartsEditorGUI::onComboBoxMoveToComponentCurrentIndexChanged(int index) {

	SAMSON::getActiveDocument()->clearSelection();
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

SBCContainerUUID SEMergePartsEditorGUI::getUUID() const { return SBCContainerUUID( "3F52AD7B-A478-D380-AA01-2041081D06CB" );}

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

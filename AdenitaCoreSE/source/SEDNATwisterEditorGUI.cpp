#include "SEDNATwisterEditorGUI.hpp"
#include "SEDNATwisterEditor.hpp"


SEDNATwisterEditorGUI::SEDNATwisterEditorGUI(SEDNATwisterEditor* editor) {

	this->editor = editor;
	ui.setupUi(this);
	setupHelpText();

	QObject::connect(ui.rdbUntwist, &QRadioButton::clicked, this, &SEDNATwisterEditorGUI::setUntwistTwist);
	QObject::connect(ui.rdbInvisible, &QRadioButton::clicked, this, &SEDNATwisterEditorGUI::setInvisibleVisible);
	QObject::connect(ui.btnResetUntwist, &QPushButton::clicked, this, &SEDNATwisterEditorGUI::resetUntwist);
	QObject::connect(ui.btnResetInvisible, &QPushButton::clicked, this, &SEDNATwisterEditorGUI::resetInvisible);

}

SEDNATwisterEditorGUI::~SEDNATwisterEditorGUI() {

}

SEDNATwisterEditor* SEDNATwisterEditorGUI::getEditor() const { return editor; }

void SEDNATwisterEditorGUI::loadSettings(SBGSettings *settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEDNATwisterEditorGUI::saveSettings(SBGSettings *settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SEDNATwisterEditorGUI::setupHelpText() {

	ui.rdbUntwist->setToolTip(
		tr("Brush mode: remove the helical twist from double-strand DNA inside the sphere, making the two strands easier to inspect as parallel lines. Hold Alt while applying to restore/twist back."));
	ui.rdbInvisible->setToolTip(
		tr("Brush mode: hide nucleotides inside the sphere. Hold Alt while applying to make affected nucleotides visible again."));
	ui.btnResetUntwist->setToolTip(
		tr("Reset is not implemented yet. This button is hidden until a full restore operation is added."));
	ui.btnResetInvisible->setToolTip(
		tr("Reset is not implemented yet. This button is hidden until a full show-all operation is added."));

	// Keep no-op reset controls out of the UI until their document-wide operations exist.
	ui.btnResetUntwist->setVisible(false);
	ui.btnResetInvisible->setVisible(false);

}

void SEDNATwisterEditorGUI::setUntwistTwist() {

	if (getEditor()) getEditor()->setBendingType(SEDNATwisterEditor::BendingType::UNTWIST);

}

void SEDNATwisterEditorGUI::setInvisibleVisible() {

	if (getEditor()) getEditor()->setBendingType(SEDNATwisterEditor::BendingType::SPHEREVISIBILITY);

}

void SEDNATwisterEditorGUI::resetUntwist() {

}

void SEDNATwisterEditorGUI::resetInvisible() {

}

SBCContainerUUID SEDNATwisterEditorGUI::getUUID() const { return SBCContainerUUID("B2208669-EAE5-DEC5-00EB-A241BBA72C29"); }

QPixmap SEDNATwisterEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEDNATwisterEditorIcon.png"));

}

QString SEDNATwisterEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return tr("DNA Twister / Visibility Brush");

}

int SEDNATwisterEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SEDNATwisterEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

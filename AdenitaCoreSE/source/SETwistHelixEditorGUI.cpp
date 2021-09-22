#include "SETwistHelixEditorGUI.hpp"
#include "SETwistHelixEditor.hpp"
#include "ADNConstants.hpp"
#include "ADNAuxiliary.hpp"


SETwistHelixEditorGUI::SETwistHelixEditorGUI(SETwistHelixEditor* editor) {

	ui.setupUi( this );
	this->editor = editor;

}

SETwistHelixEditorGUI::~SETwistHelixEditorGUI() {

}

SETwistHelixEditor* SETwistHelixEditorGUI::getEditor() const { return editor; }

void SETwistHelixEditorGUI::loadSettings( SBGSettings *settings ) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

	int currentMode = settings->loadIntValue("currentMode", 1);
	if (currentMode == 0) ui.radioButtonMinusBP->setChecked(true);
	else if (currentMode == 1) ui.radioButtonPlusBP->setChecked(true);
	else if (currentMode == 2) ui.radioButtonTwistAngle->setChecked(true);
	else if (currentMode == 3) ui.radioButtonTwistTurns->setChecked(true);

	ui.doubleSpinBoxAngle->setValue(settings->loadDoubleValue("angle", 0.0));
	ui.spinBoxTurns->setValue(settings->loadIntValue("nTurns", 0));

}

void SETwistHelixEditorGUI::saveSettings( SBGSettings *settings ) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

	int currentMode = 0;
	if (ui.radioButtonMinusBP->isChecked()) currentMode = 0;
	else if (ui.radioButtonPlusBP->isChecked()) currentMode = 1;
	else if (ui.radioButtonTwistAngle->isChecked()) currentMode = 2;
	else if (ui.radioButtonTwistTurns->isChecked()) currentMode = 3;
	settings->saveValue("currentMode", currentMode);

	settings->saveValue("angle", ui.doubleSpinBoxAngle->value());
	settings->saveValue("nTurns", ui.spinBoxTurns->value());

}

void SETwistHelixEditorGUI::onMinus(bool checked) {

	if (checked) {

		double angle = - ADNConstants::BP_ROT;
		getEditor()->setTwistAngle(angle);

	}

}

void SETwistHelixEditorGUI::checkPlusOrMinus(bool plus) {

	if (plus) {
		ui.radioButtonPlusBP->setChecked(true);
		//onPlus(true);
	}
	else {
		ui.radioButtonMinusBP->setChecked(true);
		//onMinus(true);
	}

}

void SETwistHelixEditorGUI::onPlus(bool checked) {

	if (checked) {

		double angle = ADNConstants::BP_ROT;
		getEditor()->setTwistAngle(angle);

	}

}

void SETwistHelixEditorGUI::onTwistAngle(bool checked) {

	if (checked) {

		double angle = ui.doubleSpinBoxAngle->value();
		getEditor()->setTwistAngle(angle);

	}

}

void SETwistHelixEditorGUI::onTwistTurns(bool checked) {

	if (checked) {

		double angle = ui.spinBoxTurns->value() * ADNConstants::BP_ROT;
		getEditor()->setTwistAngle(angle);

	}

}

void SETwistHelixEditorGUI::onTwistAngleChanged(double angle) {

	if (ui.radioButtonTwistAngle->isChecked()) {

		double angle = ui.doubleSpinBoxAngle->value();
		getEditor()->setTwistAngle(angle);

	}

}

void SETwistHelixEditorGUI::onTwistTurnsChanged(int turns) {

	if (ui.radioButtonTwistTurns->isChecked()) {

		double angle = ui.spinBoxTurns->value() * ADNConstants::BP_ROT;
		getEditor()->setTwistAngle(angle);

	}

}

SBCContainerUUID SETwistHelixEditorGUI::getUUID() const { return SBCContainerUUID( "8D1CB60B-F52C-14C2-F72C-DB72B6361777" );}

QPixmap SETwistHelixEditorGUI::getLogo() const {
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/twistDS.png"));

}

QString SETwistHelixEditorGUI::getName() const {

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return "Rotate DNA Editor"; 

}

int SETwistHelixEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SETwistHelixEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

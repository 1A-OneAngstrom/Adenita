#include "SENanotubeCreatorEditorGUI.hpp"
#include "SENanotubeCreatorEditor.hpp"


SENanotubeCreatorEditorGUI::SENanotubeCreatorEditorGUI(SENanotubeCreatorEditor* editor) {

	this->editor = editor;
	ui.setupUi( this );

}

SENanotubeCreatorEditorGUI::~SENanotubeCreatorEditorGUI() {

}

SENanotubeCreatorEditor* SENanotubeCreatorEditorGUI::getEditor() const { return editor; }

void SENanotubeCreatorEditorGUI::loadSettings( SBGSettings *settings ) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SENanotubeCreatorEditorGUI::saveSettings( SBGSettings *settings ) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your editor can save its GUI state from one session to the next

}

void SENanotubeCreatorEditorGUI::updateInfo(const SBQuantity::length& radius, int numberOfDSDNA, int numberOfBasePairs, bool clear) {

	if (!clear) {

		ui.txtInfo->setText("Radius (nm): " + QString::number(SBQuantity::nanometer(radius).getValue()));
		ui.txtInfo->append("Number of double strands: " + QString::number(numberOfDSDNA));
		ui.txtInfo->append("Length (base pairs): " + QString::number(numberOfBasePairs));

	}
	else {
		ui.txtInfo->clear();
	}

}

void SENanotubeCreatorEditorGUI::onPredefinedNanotube(bool predefined) {

	const SBQuantity::nanometer radius = SBQuantity::nanometer(ui.doubleSpinBoxRadius->value());
	const int numberOfBasePairs = ui.spinBoxNumberOfBasePairs->value();
	getEditor()->setPredefined(predefined, radius, numberOfBasePairs);

}

void SENanotubeCreatorEditorGUI::onRadiusChanged() {

	const SBQuantity::nanometer radius = SBQuantity::nanometer(ui.doubleSpinBoxRadius->value());
	const int numberOfDSDNA = ADNVectorMath::CalculateNanotubeDoubleStrands(radius);
	ui.spinBoxNumberOfDSDNA->setValue(numberOfDSDNA);
	getEditor()->setRadius(radius);

}

void SENanotubeCreatorEditorGUI::onNumberOfBasePairsChanged() {

	const int numberOfBasePairs = ui.spinBoxNumberOfBasePairs->value();
	getEditor()->setNumberOfBasePairs(numberOfBasePairs);

}

void SENanotubeCreatorEditorGUI::onNumberOfDSDNAChanged() {

	const int numberOfDSDNA = ui.spinBoxNumberOfDSDNA->value();
	const SBQuantity::nanometer radius = SBQuantity::nanometer(ADNVectorMath::CalculateNanotubeRadius(numberOfDSDNA));
	ui.doubleSpinBoxRadius->setValue(radius.getValue());

}

void SENanotubeCreatorEditorGUI::onChangeRouting() {

	RoutingType type = static_cast<RoutingType>(ui.comboBoxRouting->currentIndex());

	getEditor()->setRoutingType(type);

}

SBCContainerUUID SENanotubeCreatorEditorGUI::getUUID() const { return SBCContainerUUID( "23177033-1C9B-B441-1916-35EE442706CC" );}

QPixmap SENanotubeCreatorEditorGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SENanotubeCreatorEditorIcon.png"));

}

QString SENanotubeCreatorEditorGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your editor inside SAMSON

	return "DNA Nanotube Creator"; 

}

int SENanotubeCreatorEditorGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SENanotubeCreatorEditorGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

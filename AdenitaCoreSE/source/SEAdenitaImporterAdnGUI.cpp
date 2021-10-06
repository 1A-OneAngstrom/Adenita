#include "SEAdenitaImporterAdnGUI.hpp"
#include "SEAdenitaImporterAdn.hpp"
#include "ADNAuxiliary.hpp"

#include "SAMSON.hpp"
#include "SBGWindow.hpp"


SEAdenitaImporterAdnGUI::SEAdenitaImporterAdnGUI( SEAdenitaImporterAdn* t ) : SBGFileImporter( t ) {

	ui.setupUi( this );

}

SEAdenitaImporterAdnGUI::~SEAdenitaImporterAdnGUI() {

}

SEAdenitaImporterAdn* SEAdenitaImporterAdnGUI::getImporter() const { return static_cast<SEAdenitaImporterAdn*>(importer); }

void SEAdenitaImporterAdnGUI::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEAdenitaImporterAdnGUI::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

SBCContainerUUID SEAdenitaImporterAdnGUI::getUUID() const { return SBCContainerUUID( "6524E3A7-007E-D4E9-F0AC-E81142C244AA" );}

QPixmap SEAdenitaImporterAdnGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your importer.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEAdenitaImporterAdnIcon.png"));

}

QString SEAdenitaImporterAdnGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your importer inside SAMSON

	return "SEAdenitaImporterAdn"; 

}

int SEAdenitaImporterAdnGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Lockable);

}

QString SEAdenitaImporterAdnGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

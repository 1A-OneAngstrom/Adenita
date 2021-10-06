#include "SEAdenitaImporterAdnpartGUI.hpp"
#include "SEAdenitaImporterAdnpart.hpp"
#include "ADNAuxiliary.hpp"

#include "SAMSON.hpp"
#include "SBGWindow.hpp"


SEAdenitaImporterAdnpartGUI::SEAdenitaImporterAdnpartGUI( SEAdenitaImporterAdnpart* t ) : SBGFileImporter( t ) {

	ui.setupUi( this );

}

SEAdenitaImporterAdnpartGUI::~SEAdenitaImporterAdnpartGUI() {

}

SEAdenitaImporterAdnpart* SEAdenitaImporterAdnpartGUI::getImporter() const { return static_cast<SEAdenitaImporterAdnpart*>(importer); }

void SEAdenitaImporterAdnpartGUI::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEAdenitaImporterAdnpartGUI::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

SBCContainerUUID SEAdenitaImporterAdnpartGUI::getUUID() const { return SBCContainerUUID( "16FB03EB-5405-D9D1-358E-CDF0F373AE30" );}

QPixmap SEAdenitaImporterAdnpartGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your importer.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEAdenitaImporterAdnpartIcon.png"));

}

QString SEAdenitaImporterAdnpartGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your importer inside SAMSON

	return "SEAdenitaImporterAdnpart"; 

}

int SEAdenitaImporterAdnpartGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Lockable);

}

QString SEAdenitaImporterAdnpartGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

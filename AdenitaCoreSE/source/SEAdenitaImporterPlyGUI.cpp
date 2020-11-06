#include "SEAdenitaImporterPlyGUI.hpp"
#include "SEAdenitaImporterPly.hpp"
#include "SAMSON.hpp"
#include "SBGWindow.hpp"
#include <QShortcut>

SEAdenitaImporterPlyGUI::SEAdenitaImporterPlyGUI( SEAdenitaImporterPly* t ) : SBGFileImporter( t ) {

	ui.setupUi( this );

}

SEAdenitaImporterPlyGUI::~SEAdenitaImporterPlyGUI() {

}

SEAdenitaImporterPly* SEAdenitaImporterPlyGUI::getImporter() const { return static_cast<SEAdenitaImporterPly*>(importer); }

void SEAdenitaImporterPlyGUI::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEAdenitaImporterPlyGUI::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

SBCContainerUUID SEAdenitaImporterPlyGUI::getUUID() const { return SBCContainerUUID( "A3FF0AE0-DF09-733E-635A-676EB18DA5AE" );}

QPixmap SEAdenitaImporterPlyGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your importer.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEAdenitaImporterPlyIcon.png"));

}

QString SEAdenitaImporterPlyGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your importer inside SAMSON

	return "SEAdenitaImporterPly"; 

}

int SEAdenitaImporterPlyGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Lockable);

}

QString SEAdenitaImporterPlyGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return
		"If you use this importer in your work, please cite: <br/>"
		"<br/>"
		"[1] <a href=\"https://www.samson-connect.net\">https://www.samson-connect.net</a><br/>";

}

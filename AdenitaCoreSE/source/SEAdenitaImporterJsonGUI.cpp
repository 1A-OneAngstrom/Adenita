#include "SEAdenitaImporterJsonGUI.hpp"
#include "SEAdenitaImporterJson.hpp"
#include "SAMSON.hpp"
#include "SBGWindow.hpp"
#include <QShortcut>

SEAdenitaImporterJsonGUI::SEAdenitaImporterJsonGUI( SEAdenitaImporterJson* t ) : SBGFileImporter( t ) {

	ui.setupUi( this );

}

SEAdenitaImporterJsonGUI::~SEAdenitaImporterJsonGUI() {

}

SEAdenitaImporterJson* SEAdenitaImporterJsonGUI::getImporter() const { return static_cast<SEAdenitaImporterJson*>(importer); }

void SEAdenitaImporterJsonGUI::loadSettings(SBGSettings* settings) {

	if (settings == nullptr) return;
	
	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SEAdenitaImporterJsonGUI::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

SBCContainerUUID SEAdenitaImporterJsonGUI::getUUID() const { return SBCContainerUUID( "B1449F89-0013-7A4B-5710-8BC38D03B5FE" );}

QPixmap SEAdenitaImporterJsonGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your importer.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEAdenitaImporterJsonIcon.png"));

}

QString SEAdenitaImporterJsonGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your importer inside SAMSON

	return "SEAdenitaImporterJson"; 

}

int SEAdenitaImporterJsonGUI::getFormat() const {

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Lockable);

}

QString SEAdenitaImporterJsonGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return
		"If you use this importer in your work, please cite: <br/>"
		"<br/>"
		"[1] <a href=\"https://www.samson-connect.net\">https://www.samson-connect.net</a><br/>";

}

#include "SEAdenitaImporterJson.hpp"
#include "SEAdenitaImporterJsonGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SEAdenitaCoreSEAppGUI.hpp"
#include "SAMSON.hpp"

SEAdenitaImporterJson::SEAdenitaImporterJson() {

	//propertyDialog = new SEAdenitaImporterJsonGUI(this);
	//propertyDialog->loadDefaultSettings();

}

SEAdenitaImporterJson::~SEAdenitaImporterJson() {

	//propertyDialog->saveDefaultSettings();
	//delete propertyDialog;

}

std::string SEAdenitaImporterJson::getFilter() const {

	// SAMSON Element generator pro tip: modify this function to return the filter of the importer

	return std::string("Cadnano or legacy Adenita parts format (*.json)");

}

std::string SEAdenitaImporterJson::getExtension() const {

	// SAMSON Element generator pro tip: modify this function to return the extension of the importer

	return std::string("json");

}

bool SEAdenitaImporterJson::importFromFile(const std::string& fileName, const SBList<std::string>* parameters, SBDDocumentFolder* preferredFolder) {

	SEAdenitaCoreSEApp* adenitaApp = SEAdenitaCoreSEApp::getAdenitaApp();

	if (!adenitaApp) return false;

	const QString fn = QString::fromStdString(fileName);
	if (!QFileInfo::exists(fn)) return false;

	// load the parts in the Adenita App

	// either cadnano or old Adenita format
	const std::string format = SEAdenitaCoreSEAppGUI::isCadnanoJsonFormat(fn);
	if (format == "cadnano") {

		if (!adenitaApp->importFromCadnano(fn, preferredFolder)) {

			SAMSON::informUser("Adenita", "Sorry, could not load the cadnano file:\n" + QFileInfo(fn).fileName());
			return false;

		}

	}
	else if (format == "adenita") {

		if (!adenitaApp->loadPart(fn)) {

			SAMSON::informUser("Adenita", "Sorry, could not load the adenita file:\n" + QFileInfo(fn).fileName());
			return false;

		}

	}
	else {

		SAMSON::informUser("Adenita", "Unknown json format. Current supported formats include Cadnano and legacy Adenita parts.");
		return false;

	}

	// add a visual model or reset the existing one

	SEAdenitaCoreSEApp::resetVisualModel();

	SAMSON::getActiveCamera()->center();

	return true;

}


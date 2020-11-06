#include "SEAdenitaImporterAdnpart.hpp"
#include "SEAdenitaImporterAdnpartGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SAMSON.hpp"

SEAdenitaImporterAdnpart::SEAdenitaImporterAdnpart() {

	//propertyDialog = new SEAdenitaImporterAdnpartGUI(this);
	//propertyDialog->loadDefaultSettings();

}

SEAdenitaImporterAdnpart::~SEAdenitaImporterAdnpart() {

	//propertyDialog->saveDefaultSettings();
	//delete propertyDialog;

}

std::string SEAdenitaImporterAdnpart::getFilter() const {

	// SAMSON Element generator pro tip: modify this function to return the filter of the importer

	return std::string("Adenita parts format (*.adnpart)");

}

std::string SEAdenitaImporterAdnpart::getExtension() const {

	// SAMSON Element generator pro tip: modify this function to return the extension of the importer

	return std::string("adnpart");

}

bool SEAdenitaImporterAdnpart::importFromFile(const std::string& fileName, const SBList<std::string>* parameters, SBDDocumentFolder* preferredFolder) {

	SEAdenitaCoreSEApp* adenitaApp = SEAdenitaCoreSEApp::getAdenitaApp();

	if (!adenitaApp) return false;

	QString fn = QString::fromStdString(fileName);
	if (!QFileInfo::exists(fn)) return false;

	// load the parts in the Adenita App

	adenitaApp->LoadPart(fn);

	// add a visual model or reset the existing one

	SEAdenitaCoreSEApp::resetVisualModel();

	SAMSON::getActiveCamera()->center();

	return true;

}


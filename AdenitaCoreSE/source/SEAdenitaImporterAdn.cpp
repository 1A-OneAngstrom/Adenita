#include "SEAdenitaImporterAdn.hpp"
#include "SEAdenitaImporterAdnGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SAMSON.hpp"

SEAdenitaImporterAdn::SEAdenitaImporterAdn() {

	//propertyDialog = new SEAdenitaImporterAdnGUI(this);
	//propertyDialog->loadDefaultSettings();

}

SEAdenitaImporterAdn::~SEAdenitaImporterAdn() {

	//propertyDialog->saveDefaultSettings();
	//delete propertyDialog;

}

std::string SEAdenitaImporterAdn::getFilter() const {

	// SAMSON Element generator pro tip: modify this function to return the filter of the importer

	return std::string("Adenita format (*.adn)");

}

std::string SEAdenitaImporterAdn::getExtension() const {

	// SAMSON Element generator pro tip: modify this function to return the extension of the importer

	return std::string("adn");

}

bool SEAdenitaImporterAdn::importFromFile(const std::string& fileName, const SBList<std::string>* parameters, SBDDocumentFolder* preferredFolder) {

	SEAdenitaCoreSEApp* adenitaApp = SEAdenitaCoreSEApp::getAdenitaApp();

	if (!adenitaApp) return false;

	QString fn = QString::fromStdString(fileName);
	if (!QFileInfo::exists(fn)) return false;

	// load the parts in the Adenita App

	adenitaApp->loadParts(fn);

	// add a visual model or reset the existing one

	SEAdenitaCoreSEApp::resetVisualModel();

	SAMSON::getActiveCamera()->center();

	return true;

}


#include "SEAdenitaImporterPly.hpp"
#include "SEAdenitaImporterPlyGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SAMSON.hpp"

SEAdenitaImporterPly::SEAdenitaImporterPly() {

	//propertyDialog = new SEAdenitaImporterPlyGUI(this);
	//propertyDialog->loadDefaultSettings();

}

SEAdenitaImporterPly::~SEAdenitaImporterPly() {

	//propertyDialog->saveDefaultSettings();
	//delete propertyDialog;

}

std::string SEAdenitaImporterPly::getFilter() const {

	// SAMSON Element generator pro tip: modify this function to return the filter of the importer

	return std::string("Cadnano mesh (*.ply)");

}

std::string SEAdenitaImporterPly::getExtension() const {

	// SAMSON Element generator pro tip: modify this function to return the extension of the importer

	return std::string("ply");

}

bool SEAdenitaImporterPly::importFromFile(const std::string& fileName, const std::unordered_map<std::string, SBValue>* parameters, SBDDocumentFolder* preferredFolder) {

	SEAdenitaCoreSEApp* adenitaApp = SEAdenitaCoreSEApp::getAdenitaApp();

	if (!adenitaApp) return false;

	const QString fn = QString::fromStdString(fileName);
	if (!QFileInfo::exists(fn)) return false;

	// load in the Adenita App

	int i = 42;
	bool ok;
	//int i = QInputDialog::getInt(this, tr("Wireframe structure (Daedalus)"), tr("Minimum edge size (bp): "), 42, 31, 1050, 1, &ok);
	ok = SAMSON::getIntegerFromUser(QString("Wireframe structure (Daedalus)"), i, 31, 1050, 1, QString("Minimum edge size: "), QString(" bp"));
	if (ok) {

		div_t d = div(i, 10.5);
		int minSize = floor(d.quot * 10.5);
		adenitaApp->LoadPartWithDaedalus(fn, minSize);

	}
	else return false;

	// add a visual model or reset the existing one

	SEAdenitaCoreSEApp::resetVisualModel();

	SAMSON::getActiveCamera()->center();

	return true;

}


#include "SEAdenitaImporterPly.hpp"
#include "SEAdenitaImporterPlyGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SAMSON.hpp"

#include <cmath>

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

bool SEAdenitaImporterPly::importFromFile(const std::string& fileName, const SBValueMap& parameters, SBDDocumentFolder* preferredFolder) {

	SEAdenitaCoreSEApp* adenitaApp = SEAdenitaCoreSEApp::getAdenitaApp();

	if (!adenitaApp) return false;

	const QString fn = QString::fromStdString(fileName);
	if (!QFileInfo::exists(fn)) return false;

	if (!DASPolyhedron::isPLYFile(fileName)) {

		SAMSON::informUser("Adenita", "The provided file is not in the PLY format.\nPlease check the file.");
		return false;

	}

	// load in the Adenita App

	int i = 42;
	if (SAMSON::getIntegerFromUser(QString("Wireframe structure (Daedalus)"), i, 31, 1050, 1, QString("Minimum edge size: "), QString(" bp"))) {

		// Daedalus edge sizes are quantized in 10.5 bp turns; keep that step explicitly
		// instead of relying on integer division, which would silently turn 10.5 into 10.
		const double turns = std::round(static_cast<double>(i) / 10.5);
		int minSize = static_cast<int>(std::floor(turns * 10.5));
		adenitaApp->LoadPartWithDaedalus(fn, minSize);

	}
	else return false;

	// add a visual model or reset the existing one

	SEAdenitaCoreSEApp::resetVisualModel();

	adenitaApp->centerCameraOnLoadedSystemWithTimer();

	return true;

}

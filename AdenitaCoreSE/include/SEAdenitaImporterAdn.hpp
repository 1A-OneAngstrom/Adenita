#pragma once 

#include "SBIFileImporter.hpp"
#include "SEAdenitaImporterAdnGUI.hpp"


/// This class implements the functionality of the importer

// SAMSON Element generator pro tip: add non-GUI functionality in this class. The GUI functionality should go in the SEAdenitaImporterAdnGUI class

class SEAdenitaImporterAdn : public SBIFileImporter {

	SB_CLASS

public :

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterAdn();																													///< Constructs an importer
	virtual ~SEAdenitaImporterAdn();																											///< Destructs the importer

	//@}

	/// \name Properties
	//@{

	virtual std::string											getFilter() const;														///< Returns the filter of the importer
	virtual std::string											getExtension() const;													///< Returns the extension of the importer

	//@}

	/// \name Parsing
	//@{

	virtual bool												importFromFile(const std::string& fileName, const SBList<std::string>* parameters = 0, SBDDocumentFolder* preferredFolder = 0);			///< Imports data into SAMSON from a file

	//@}

};


SB_REGISTER_TARGET_TYPE(SEAdenitaImporterAdn, "SEAdenitaImporterAdn", "B1AA7F6F-6A31-0EEB-B158-3AD28EC53043");
SB_DECLARE_BASE_TYPE(SEAdenitaImporterAdn, SBIFileImporter);

#pragma once 

#include "SBIFileImporter.hpp"
#include "SEAdenitaImporterJsonGUI.hpp"


/// This class implements the functionality of the importer

// SAMSON Element generator pro tip: add non-GUI functionality in this class. The GUI functionality should go in the SEAdenitaImporterJsonGUI class

class SB_EXPORT SEAdenitaImporterJson : public SBIFileImporter {

	SB_CLASS

public :

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterJson();																													///< Constructs an importer
	virtual ~SEAdenitaImporterJson();																											///< Destructs the importer

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


SB_REGISTER_TARGET_TYPE(SEAdenitaImporterJson, "SEAdenitaImporterJson", "6D09F24D-E24B-6E74-F08C-2436CA4D2C06");
SB_DECLARE_BASE_TYPE(SEAdenitaImporterJson, SBIFileImporter);

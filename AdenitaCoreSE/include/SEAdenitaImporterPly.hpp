#pragma once 

#include "SBIFileImporter.hpp"
#include "SEAdenitaImporterPlyGUI.hpp"


/// This class implements the functionality of the importer

// SAMSON Element generator pro tip: add non-GUI functionality in this class. The GUI functionality should go in the SEAdenitaImporterPlyGUI class

class SB_EXPORT SEAdenitaImporterPly : public SBIFileImporter {

	SB_CLASS

public :

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterPly();																													///< Constructs an importer
	virtual ~SEAdenitaImporterPly();																											///< Destructs the importer

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


SB_REGISTER_TARGET_TYPE(SEAdenitaImporterPly, "SEAdenitaImporterPly", "0F7F4BBC-8CFF-4502-6283-C1958334EEDB");
SB_DECLARE_BASE_TYPE(SEAdenitaImporterPly, SBIFileImporter);

#pragma once 

#include "SBIFileImporter.hpp"
#include "SEAdenitaImporterAdnpartGUI.hpp"


/// This class implements the functionality of the importer

// SAMSON Element generator pro tip: add non-GUI functionality in this class. The GUI functionality should go in the SEAdenitaImporterAdnpartGUI class

class SB_EXPORT SEAdenitaImporterAdnpart : public SBIFileImporter {

	SB_CLASS

public :

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterAdnpart();																													///< Constructs an importer
	virtual ~SEAdenitaImporterAdnpart();																										///< Destructs the importer

	//@}

	/// \name Properties
	//@{

	virtual std::string											getFilter() const override;														///< Returns the filter of the importer
	virtual std::string											getExtension() const override;													///< Returns the extension of the importer

	//@}

	/// \name Parsing
	//@{

	virtual bool												importFromFile(const std::string& fileName, const SBList<std::string>* parameters = 0, SBDDocumentFolder* preferredFolder = 0) override;			///< Imports data into SAMSON from a file

	//@}

};


SB_REGISTER_TARGET_TYPE(SEAdenitaImporterAdnpart, "SEAdenitaImporterAdnpart", "8EFFD6D2-5FE0-5696-5EDB-7358270ABC9A");
SB_DECLARE_BASE_TYPE(SEAdenitaImporterAdnpart, SBIFileImporter);

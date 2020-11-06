#pragma once


#include "SBGFileImporter.hpp" 
#include "ui_SEAdenitaImporterPlyGUI.h"

class SEAdenitaImporterPly;

/// This class implements the GUI of the importer

// SAMSON Element generator pro tip: add GUI functionality in this class. The non-GUI functionality should go in the SEAdenitaImporterPly class

class SEAdenitaImporterPlyGUI : public SBGFileImporter {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterPlyGUI(SEAdenitaImporterPly* t);																					///< Constructs a GUI for the importer
	~SEAdenitaImporterPlyGUI();																											///< Destructs the GUI of the importer

	//@}

	/// \name Importer
	//@{

	SEAdenitaImporterPly*										getImporter() const;													///< Returns a pointer to the importer

	//@}

	/// \name Identity
	//@{

	virtual SBCContainerUUID									getUUID() const;														///< Returns the widget UUID
	virtual QString												getName() const;														///< Returns the widget name (used as a title for the embedding window)
	virtual QPixmap												getLogo() const;														///< Returns the widget logo
	int															getFormat() const;														///< Returns the widget format
	virtual QString												getCitation() const;													///< Returns the citation information

	//@}

	///\name Settings
	//@{

	void														loadSettings(SBGSettings* settings);									///< Load GUI settings
	void														saveSettings(SBGSettings* settings);									///< Save GUI settings

	//@}

private:

	Ui::SEAdenitaImporterPlyGUIClass							ui;

};


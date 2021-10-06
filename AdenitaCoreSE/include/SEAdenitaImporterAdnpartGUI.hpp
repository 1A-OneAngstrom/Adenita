#pragma once


#include "SBGFileImporter.hpp" 
#include "ui_SEAdenitaImporterAdnpartGUI.h"

class SEAdenitaImporterAdnpart;

/// This class implements the GUI of the importer

// SAMSON Element generator pro tip: add GUI functionality in this class. The non-GUI functionality should go in the SEAdenitaImporterAdnpart class

class SB_EXPORT SEAdenitaImporterAdnpartGUI : public SBGFileImporter {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterAdnpartGUI(SEAdenitaImporterAdnpart* t);																			///< Constructs a GUI for the importer
	~SEAdenitaImporterAdnpartGUI();																										///< Destructs the GUI of the importer

	//@}

	/// \name Importer
	//@{

	SEAdenitaImporterAdnpart*									getImporter() const;													///< Returns a pointer to the importer

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

	Ui::SEAdenitaImporterAdnpartGUIClass						ui;

};


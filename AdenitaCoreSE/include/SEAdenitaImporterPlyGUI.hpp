#pragma once


#include "SBGFileImporter.hpp" 
#include "ui_SEAdenitaImporterPlyGUI.h"

class SEAdenitaImporterPly;

/// This class implements the GUI of the importer

// SAMSON Element generator pro tip: add GUI functionality in this class. The non-GUI functionality should go in the SEAdenitaImporterPly class

class SB_EXPORT SEAdenitaImporterPlyGUI : public SBGFileImporter {

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

	virtual SBCContainerUUID									getUUID() const override;												///< Returns the widget UUID
	virtual QString												getName() const override;												///< Returns the widget name (used as a title for the embedding window)
	virtual QPixmap												getLogo() const override;												///< Returns the widget logo
	virtual int													getFormat() const override;												///< Returns the widget format
	QString														getCitation() const;													///< Returns the citation information

	//@}

	///\name Settings
	//@{

	virtual void												loadSettings(SBGSettings* settings) override;							///< Load GUI settings
	virtual void												saveSettings(SBGSettings* settings) override;							///< Save GUI settings

	//@}

private:

	Ui::SEAdenitaImporterPlyGUIClass							ui;

};


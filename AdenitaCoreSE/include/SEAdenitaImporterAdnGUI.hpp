#pragma once


#include "SBGFileImporter.hpp" 
#include "ui_SEAdenitaImporterAdnGUI.h"

class SEAdenitaImporterAdn;

/// This class implements the GUI of the importer

// SAMSON Element generator pro tip: add GUI functionality in this class. The non-GUI functionality should go in the SEAdenitaImporterAdn class

class SB_EXPORT SEAdenitaImporterAdnGUI : public SBGFileImporter {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEAdenitaImporterAdnGUI(SEAdenitaImporterAdn* t);																					///< Constructs a GUI for the importer
	~SEAdenitaImporterAdnGUI();																											///< Destructs the GUI of the importer

	//@}

	/// \name Importer
	//@{

	SEAdenitaImporterAdn*										getImporter() const;													///< Returns a pointer to the importer

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

	Ui::SEAdenitaImporterAdnGUIClass							ui;

};


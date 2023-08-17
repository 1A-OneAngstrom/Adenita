#pragma once

#include "SBCHeapExport.hpp"
#include "SBGWindowWidget.hpp" 
#include "ui_SEDeleteEditorGUI.h"

class SEDeleteEditor;

/// This class implements the GUI of the editor

// SAMSON Element generator pro tip: add GUI functionality in this class. 
// The non-GUI functionality (and the mouse and keyboard event handling methods that are specific to the editor) should go in the SEDeleteEditor class

class SB_EXPORT SEDeleteEditorGUI : public SBGWindowWidget {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEDeleteEditorGUI(SEDeleteEditor* editor);																							///< Constructs a GUI for the editor
	virtual ~SEDeleteEditorGUI();																										///< Destructs the GUI of the editor

	//@}

	/// \name Editor
	//@{

	SEDeleteEditor*												getEditor() const;														///< Returns a pointer to the editor

	//@}

	/// \name Identity
	//@{

	virtual SBCContainerUUID									getUUID() const override;												///< Returns the widget UUID
	virtual QString												getName() const override;												///< Returns the widget name (used as a title for the embedding window)
	virtual QPixmap												getLogo() const override;												///< Returns the widget logo
	virtual int													getFormat() const override;												///< Returns the widget format
	virtual QString												getCitation() const override;											///< Returns the citation information

	//@}

	///\name Settings
	//@{

	virtual void												loadSettings(SBGSettings* settings) override;							///< Load GUI settings
	virtual void												saveSettings(SBGSettings* settings) override;							///< Save GUI settings

	//@}

private:

	Ui::SEDeleteEditorGUIClass									ui;
	SEDeleteEditor*												editor{ nullptr };

};


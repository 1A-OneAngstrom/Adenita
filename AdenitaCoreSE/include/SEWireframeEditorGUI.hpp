#pragma once

#include "SBCHeapExport.hpp"
#include "SBGWindowWidget.hpp" 
#include "ui_SEWireframeEditorGUI.h"

class SEWireframeEditor;

/// This class implements the GUI of the editor

// SAMSON Element generator pro tip: add GUI functionality in this class. 
// The non-GUI functionality (and the mouse and keyboard event handling methods that are specific to the editor) should go in the SEWireframeEditor class

class SB_EXPORT SEWireframeEditorGUI : public SBGWindowWidget {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEWireframeEditorGUI(SEWireframeEditor* editor);																					///< Constructs a GUI for the editor
	virtual ~SEWireframeEditorGUI();																									///< Destructs the GUI of the editor

	//@}

	/// \name Editor
	//@{

	SEWireframeEditor*											getEditor() const;														///< Returns a pointer to the editor

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

public slots:

	void														onCurrentWireframeTemplateChanged();

protected:

	void														showEvent(QShowEvent* event) override;

private:

	Ui::SEWireframeEditorGUIClass								ui;
	SEWireframeEditor*											editor{ nullptr };

	bool														uiInitializedFlag = false;												///< True if the UI has been initialized on the first show
	void														initializeUI();

};


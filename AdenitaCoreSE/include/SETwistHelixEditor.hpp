#pragma once 

#include "SBGEditor.hpp"
#include "SETwistHelixEditorGUI.hpp"

#include "SBAction.hpp"


/// This class implements an editor

class SB_EXPORT SETwistHelixEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public :

	/// \name Constructors and destructors
	//@{

	SETwistHelixEditor();																												///< Builds an editor					
	virtual ~SETwistHelixEditor();																										///< Destructs the editor

	//@}

	/// \name Identity
	//@{

	virtual SBCContainerUUID									getUUID() const override;												///< Returns the widget UUID
	virtual QString												getName() const override;												///< Returns the widget name (used as a title for the embedding window)
	virtual QString												getDescription() const override;										///< Returns the menu item text
	virtual QPixmap												getLogo() const override;												///< Returns the widget logo
	virtual int													getFormat() const override;												///< Returns the widget format
	virtual QKeySequence										getShortcut() const override;											///< Returns the shortcut
	virtual QString												getToolTip() const override;											///< Returns the tool tip

	//@}

	///\name Settings
	//@{

	virtual void												loadSettings(SBGSettings* settings) override;							///< Load GUI settings
	virtual void												saveSettings(SBGSettings* settings) override;							///< Save GUI settings

	//@}

	/// \name Editing
	//@{

	virtual void												beginEditing() override;												///< Called when editing is about to begin
	virtual void												endEditing() override;													///< Called when editing is about to end

	//@}

	/// \name Actions
	//@{

	virtual void												getContextMenuActions(SBVector<SBAction*>& actionVector) override;		///< Gets the editor's actions

	//@}

	/// \name Rendering
	//@{

	virtual void												display() override;														///< Displays the editor
	virtual void												displayForShadow() override;											///< Displays the editor for shadow purposes

	//@}

	/// \name GUI Events
	//@{

	virtual void												mousePressEvent(QMouseEvent* event);									///< Handles mouse press event
	virtual void												mouseReleaseEvent(QMouseEvent* event);									///< Handles mouse release event
	virtual void												mouseMoveEvent(QMouseEvent* event);										///< Handles mouse move event
	virtual void												mouseDoubleClickEvent(QMouseEvent* event);								///< Handles mouse double click event

	virtual void												wheelEvent(QWheelEvent* event);											///< Handles wheel event

	virtual void												keyPressEvent(QKeyEvent* event);										///< Handles key press event
	virtual void												keyReleaseEvent(QKeyEvent* event);										///< Handles key release event

	//@}

	/// \name GUI
	//@{

	SETwistHelixEditorGUI*										getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setTwistAngle(double angle);
	void														updateCursor();

	//! For use from Adenita App, to keep coherence in the GUIs
	void														setMode(bool t);

private:

	double														twistAngle = 34.3;

	std::string													previousSelectionFilter;

};


SB_REGISTER_TYPE(SETwistHelixEditor, "SETwistHelixEditor", "4B60FECA-2A79-680F-F289-B4908A924409");
SB_DECLARE_BASE_TYPE(SETwistHelixEditor, SBGEditor);

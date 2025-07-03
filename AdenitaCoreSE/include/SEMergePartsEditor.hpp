#pragma once 

#include "SBGEditor.hpp"
#include "SEMergePartsEditorGUI.hpp"

#include "SBBaseEvent.hpp"
#include "SBAction.hpp"

#include "ADNPart.hpp"


/// This class implements an editor

class SB_EXPORT SEMergePartsEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEMergePartsEditor();																													///< Builds an editor					
	virtual ~SEMergePartsEditor();																											///< Destructs the editor

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

	virtual void												mousePressEvent(QMouseEvent* event) override;							///< Handles mouse press event
	virtual void												mouseReleaseEvent(QMouseEvent* event) override;							///< Handles mouse release event
	virtual void												mouseMoveEvent(QMouseEvent* event) override;							///< Handles mouse move event
	virtual void												mouseDoubleClickEvent(QMouseEvent* event) override;						///< Handles mouse double click event

	virtual void												wheelEvent(QWheelEvent* event) override;								///< Handles wheel event

	virtual void												keyPressEvent(QKeyEvent* event) override;								///< Handles key press event
	virtual void												keyReleaseEvent(QKeyEvent* event) override;								///< Handles key release event

	//@}

	/// \name GUI
	//@{

	SEMergePartsEditorGUI*										getPropertyWidget() const;												///< Returns the property widget of the editor

	struct Element {
		int type = -1;
		ADNPointer<ADNSingleStrand> ss;
		ADNPointer<ADNDoubleStrand> ds;

		std::string GetName() const {
			std::string n = "";
			if (type == 0)
				n = ds->getName();
			else if (type == 1)
				n = ss->getName();
			return n;
		}
	};

	std::map<int, ADNPointer<ADNPart>>							getPartsList();
	std::map<int, Element>										getElementsList();

	bool														mergeParts(int idx, int jdx);
	bool														moveElement(int edx, int pdx);

	void														selectComponent(int idx) const;
	void														selectElement(int idx) const;

private:

	std::map<int, ADNPointer<ADNPart>>							mapOfParts;
	std::map<int, Element>										mapOfElements;

};


SB_REGISTER_TYPE(SEMergePartsEditor, "SEMergePartsEditor", "EB812444-8EA8-BD83-988D-AFF5987461D8");
SB_DECLARE_BASE_TYPE(SEMergePartsEditor, SBGEditor);

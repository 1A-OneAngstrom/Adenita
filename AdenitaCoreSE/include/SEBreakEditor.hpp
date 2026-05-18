#pragma once 

#include "SBGEditor.hpp"
#include "SEBreakEditorGUI.hpp"

#include "SBAction.hpp"

class ADNNucleotide;
class ADNSingleStrand;

/// This class implements an editor

class SB_EXPORT SEBreakEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEBreakEditor();																													///< Builds an editor					
	virtual ~SEBreakEditor();																											///< Destructs the editor

	//@}

	/// \name Identity
	//@{

	[[nodiscard]] virtual SBCContainerUUID						getUUID() const override;												///< Returns the widget UUID
	[[nodiscard]] virtual QString								getName() const override;												///< Returns the widget name (used as a title for the embedding window)
	[[nodiscard]] virtual QString								getDescription() const override;										///< Returns the menu item text
	[[nodiscard]] virtual QPixmap								getLogo() const override;												///< Returns the widget logo
	[[nodiscard]] virtual int									getFormat() const override;												///< Returns the widget format
	[[nodiscard]] virtual QKeySequence							getShortcut() const override;											///< Returns the shortcut
	[[nodiscard]] virtual QString								getToolTip() const override;											///< Returns the tool tip

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

	virtual void												display(SBNode::RenderingPass renderingPass) override;					///< Display elements related to the editor

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

	SEBreakEditorGUI*											getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setFivePrimeModeFlag(bool fivePrimeModeFlag);

private:

	struct BreakTarget {
		SBPointer<ADNNucleotide>								clickedNucleotide{ nullptr };
		SBPointer<ADNNucleotide>								firstNucleotide{ nullptr };
		SBPointer<ADNNucleotide>								secondNucleotide{ nullptr };
		SBPointer<ADNSingleStrand>								singleStrand{ nullptr };
		bool													valid{ false };
	};

	[[nodiscard]] BreakTarget									getBreakTarget(SBPointer<ADNNucleotide> nucleotide) const;
	void														updateCursor();
	void														restoreCursor();
	void														setCursor(QMouseEvent* event);
	void														setCursor(QKeyEvent* event);

	bool														fivePrimeModeFlag = true;
	QCursor														breakCursor_;
	double														viewportPixelRatio_{ -1.0 };

	std::string													previousSelectionFilter;

};


SB_REGISTER_TYPE(SEBreakEditor, "SEBreakEditor", "CFACD1E5-FCD1-916F-2CF7-4B60979F1A77");
SB_DECLARE_BASE_TYPE(SEBreakEditor, SBGEditor);

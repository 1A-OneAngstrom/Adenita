#pragma once 

#include "SBGEditor.hpp"
#include "SETaggingEditorGUI.hpp"

#include "SBAction.hpp"

#include "DASCreator.hpp"

/// This class implements an editor

class SB_EXPORT SETaggingEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	//enum class TaggingShape {
	//  Sphere,
	//  Rod,
	//  HisTag
	//};
	enum class TaggingMode {
		Tags = 0,
		Base = 1
	};

	/// \name Constructors and destructors
	//@{

	SETaggingEditor();																													///< Builds an editor					
	virtual ~SETaggingEditor();																											///< Destructs the editor

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

	SETaggingEditorGUI*											getPropertyWidget() const;												///< Returns the property widget of the editor
	ADNPointer<ADNNucleotide>									GetHighlightedNucleotide() const;

	//@}

	void														setTaggingMode(TaggingMode mode);

private:

	DNABlocks													getNucleotideType(const QPoint& numSteps) const;

	//TaggingShape												shape_{ TaggingShape::Sphere };
	TaggingMode													taggingMode{ TaggingMode::Tags };
	DNABlocks													nucleotideType{ DNABlocks::DI };

	std::string													previousSelectionFilter;

	static const std::map<int, DNABlocks>						values;
	static const std::map<DNABlocks, int>						indices;

};


SB_REGISTER_TYPE(SETaggingEditor, "SETaggingEditor", "473D2F88-5D06-25F5-EB58-053661504C43");
SB_DECLARE_BASE_TYPE(SETaggingEditor, SBGEditor);

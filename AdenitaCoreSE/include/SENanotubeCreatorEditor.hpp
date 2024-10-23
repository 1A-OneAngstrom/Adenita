#pragma once 

#include "SBGEditor.hpp"
#include "SENanotubeCreatorEditorGUI.hpp"

#include "SBAction.hpp"

#include "ADNPart.hpp"
#include "DASRouter.hpp"


/// This class implements an editor

class SB_EXPORT SENanotubeCreatorEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SENanotubeCreatorEditor();																											///< Builds an editor					
	virtual ~SENanotubeCreatorEditor();																									///< Destructs the editor

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

	SENanotubeCreatorEditorGUI*									getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setRoutingType(RoutingType t);
	void														setPredefined(bool predefined, const SBQuantity::length& radius, int numBp);
	void														setRadius(const SBQuantity::length& radius);
	void														setNumberOfBasePairs(int bp);

	void														resetData();

private:

	ADNPointer<ADNPart>											generateNanotube(bool mock = false);

	void														sendPartToAdenita(ADNPointer<ADNPart> nanotube);
	void														updateGUI(SBQuantity::length radius, int numBp, bool clear = false);

	SBPosition3													firstPosition;
	SBPosition3													secondPosition;
	SBPosition3													thirdPosition;
	bool														displayFlag = false;
	bool														isPressing = false;
	bool														lengthSelected = false;

	ADNPointer<ADNPart>											tempPart = nullptr;
	RoutingType													routingType = RoutingType::None;

	//! for manual setting of base pairs and double strands
	bool														predefinedFlag = false;
	int															numberOfBasePairs = 0;
	SBQuantity::length											radius = SBQuantity::length(0.0);

};


SB_REGISTER_TYPE(SENanotubeCreatorEditor, "SENanotubeCreatorEditor", "4B6A0B18-48B5-233A-28A4-BA3EF3D56AB8");
SB_DECLARE_BASE_TYPE(SENanotubeCreatorEditor, SBGEditor);

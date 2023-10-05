#pragma once 

#include "SBGEditor.hpp"
#include "SEDNATwisterEditorGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"

#include "SBAction.hpp"

#include "ADNNanorobot.hpp"
#include "DASBackToTheAtom.hpp"

/// This class implements an editor

class SB_EXPORT SEDNATwisterEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	enum class BendingType {
		UNTWIST,
		SPHEREVISIBILITY
	};

	/// \name Constructors and destructors
	//@{

	SEDNATwisterEditor();																												///< Builds an editor					
	virtual ~SEDNATwisterEditor();																										///< Destructs the editor

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

	SEDNATwisterEditorGUI*										getPropertyWidget() const;												///< Returns the property widget of the editor
	void														setBendingType(BendingType type);

	//@}

private:

	void														untwisting();
	void														makeInvisible();
	SBPosition3													getSnappedPosition(const SBPosition3& currentPosition);

	void														updateEditorText();
	void														updateForwardReverseState();

	bool														altPressed = false;
	SBPosition3													spherePosition;
	std::string													editorText = "Untwisting";
	BendingType													bendingType = BendingType::UNTWIST;
	SBQuantity::length											sphereRadius = SBQuantity::nanometer(2.0);
	bool														forwardActionSphereActive = false;
	bool														reverseActionSphereActive = false;
	bool														snappingIsActive = true;

	/// \name Display
	//@{

	float														positionData[3] = { 0.0f, 0.0f, 0.0f };
	float														radiusData[1] = { 0.0f };
	float														colorData[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	unsigned int												flagData[1] = { 0 };

	//@}

};


SB_REGISTER_TYPE(SEDNATwisterEditor, "SEDNATwisterEditor", "677B1667-7856-12E6-5901-E8EAC729501A");
SB_DECLARE_BASE_TYPE(SEDNATwisterEditor, SBGEditor);

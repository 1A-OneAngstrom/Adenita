#pragma once 

#include "SBGEditor.hpp"
#include "SEWireframeEditorGUI.hpp"

#include "SBAction.hpp"

#include "ADNPart.hpp"
#include "DASCreator.hpp"
#include "DASPolyhedron.hpp"

/// This class implements an editor

class SB_EXPORT SEWireframeEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEWireframeEditor();																												///< Builds an editor					
	virtual ~SEWireframeEditor();																										///< Destructs the editor

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

	SEWireframeEditorGUI*										getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setWireframeType(DASCreator::EditorType type);

	void														resetData();

private:

	ADNPointer<ADNPart>											generateCuboid(const SBPosition3& currentPosition, bool mock = false);
	ADNPointer<ADNPart>											generateWireframe(bool mock = false);
	void														sendPartToAdenita(ADNPointer<ADNPart> part);

	//! Creates a ADNPart containing only the high-level model (double strands) of a daedalus wireframe design
	/*!
	\param the polyhedron containing the PLY information
	\param position of the center of the model
	\param minimum edge length
	*/
	ADNPointer<ADNPart>											createMockDaedalusWireframe(DASPolyhedron& polyhedron, int min_edge_length);

	bool														isPressing = false;

	DASCreatorEditors::UIData									positionData;
	bool														displayFlag = false;
	ADNPointer<ADNPart>											tempPart = nullptr;
	DASCreator::EditorType										wireframeType = DASCreator::EditorType::Tetrahedron;

};


SB_REGISTER_TYPE(SEWireframeEditor, "SEWireframeEditor", "F1F29042-3D87-DA61-BC5C-D3348EB2E1FA");
SB_DECLARE_BASE_TYPE(SEWireframeEditor, SBGEditor);

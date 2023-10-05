#pragma once 

#include "SBGEditor.hpp"
#include "SELatticeCreatorEditorGUI.hpp"

#include "SBAction.hpp"

#include "ADNConstants.hpp"
#include "ADNPart.hpp"
#include "DASCadnano.hpp"
#include "DASCreator.hpp"

/// This class implements an editor

class SB_EXPORT SELatticeCreatorEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

	enum class ZLatticePattern {
		ALLZ,
		TRIANGLE,
		CIRCLE
	};

	/// \name Constructors and destructors
	//@{

	SELatticeCreatorEditor();																											///< Builds an editor					
	virtual ~SELatticeCreatorEditor();																									///< Destructs the editor

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

	void														setLatticeType(LatticeType type);
	void														setZPattern(ZLatticePattern pattern);

	/// \name GUI
	//@{

	SELatticeCreatorEditorGUI*									getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setMaxXDoubleStrands(int val);
	void														setMaxYDoubleStrands(int val);
	void														setMaxZBasePairs(int val);

	void														resetData();

private:

	ADNPointer<ADNPart>											generateLattice(bool mock = false);

	void														sendPartToAdenita(ADNPointer<ADNPart> lattice);

	LatticeType													latticeType = LatticeType::Honeycomb;
	VGrid														vGrid;

	bool														isPressing = false;
	bool														lengthSelected = false;
	bool														heightSelected = false;

	SBPosition3													firstPosition;
	SBPosition3													secondPosition;
	SBPosition3													thirdPosition;
	bool														displayFlag = false;
	ADNPointer<ADNPart>											tempPart = nullptr;

	int															maxXDoubleStrands = 32;
	int															maxYDoubleStrands = 30;
	int															maxZBasePairs = 400;

	std::string													xyText = "";
	std::string													zText = "";

	ZLatticePattern												zPattern = ZLatticePattern::ALLZ;

};


SB_REGISTER_TYPE(SELatticeCreatorEditor, "SELatticeCreatorEditor", "EA67625E-89B5-2EEA-156D-FC836214B0E4");
SB_DECLARE_BASE_TYPE(SELatticeCreatorEditor, SBGEditor);

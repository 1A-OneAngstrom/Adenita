#pragma once 

#include <cmath>

#include "SBGEditor.hpp"
#include "SEDSDNACreatorEditorGUI.hpp"

#include "SBAction.hpp"

#include "ADNPart.hpp"
#include "DASCreator.hpp"

/// This class implements an editor

class SB_EXPORT SEDSDNACreatorEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public :

	/// \name Constructors and destructors
	//@{

	SEDSDNACreatorEditor();																												///< Builds an editor					
	virtual ~SEDSDNACreatorEditor();																									///< Destructs the editor

	//@}

	/// \name Identity
	//@{

	virtual SBCContainerUUID									getUUID() const;														///< Returns the widget UUID
	virtual QString												getName() const;														///< Returns the class name
	virtual QString												getDescription() const;													///< Returns the menu item text
	virtual QPixmap												getLogo() const;														///< Returns the pixmap logo
	virtual int													getFormat() const;														///< Returns the format
	virtual QKeySequence										getShortcut() const;													///< Returns the shorcut
	virtual QString												getToolTip() const;														///< Returns the tool tip

	//@}

	///\name Settings
	//@{

	virtual void												loadSettings(SBGSettings* settings);									///< Loads \p settings
	virtual void												saveSettings(SBGSettings* settings);									///< Saves \p settings

	//@}

	/// \name Editing
	//@{

	virtual void												beginEditing();															///< Called when editing is about to begin
	virtual void												endEditing();															///< Called when editing is about to end

	//@}

	/// \name Actions
	//@{

	virtual void												getActions(SBVector<SBAction*>& actionVector);							///< Gets the editor's actions

	//@}

	/// \name Rendering
	//@{

	virtual void												display();																///< Displays the editor
	virtual void												displayForShadow();														///< Displays the editor for shadow purposes
	virtual void												displayInterface();														///< Displays the 2D interface in the viewport

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

	SEDSDNACreatorEditorGUI*									getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setDoubleStrandMode(bool m);
	void														setShowBoxFlag(bool s);
	void														setBoxSize(SBQuantity::nanometer height, SBQuantity::nanometer width, SBQuantity::nanometer depth);
	void														setCircularStrandsMode(bool c);
	void														setManualFlag(bool m);
	void														setNumberOfNucleotides(int n);
	void														setSequenceFlag(bool s);

private:

	ADNPointer<ADNPart>											generateStrand(bool mock = false);
	ADNPointer<ADNPart>											generateCircularStrand(bool mock = false);
	void														displayStrand();
	void														displayBox();
	void														sendPartToAdenita(ADNPointer<ADNPart> nanotube);
	void														setSequence(ADNPointer<ADNPart> nanotube);
	SBPosition3													getSnappedPosition(const SBPosition3& currentPosition);

	void														resetData();

	bool														doubleStrandMode = true;  // true for dsDNA, false for ssDNA
	bool														circularStrandsMode = false;  // if we are creating circular strands
	bool														manualFlag = false;
	int															numberOfNucleotides = 12;

	bool														isPressing = false;
	DASCreatorEditors::UIData									positionData;
	bool														displayFlag = false;
	ADNPointer<ADNPart>											tempPart = nullptr;
	bool														showBoxFlag = false;
	SBQuantity::length											boxHeight = SBQuantity::length(0.0);
	SBQuantity::length											boxWidth = SBQuantity::length(0.0);
	SBQuantity::length											boxDepth = SBQuantity::length(0.0);
	bool														sequenceFlag = false;
	bool														snappingIsActive = true;

	float														opaqueness = 0.5f;
	float														basePairRadius = 1000.0f;

};


SB_REGISTER_TYPE(SEDSDNACreatorEditor, "SEDSDNACreatorEditor", "86204A08-DFD6-97A8-2BE2-4CFC8B4169A3");
SB_DECLARE_BASE_TYPE(SEDSDNACreatorEditor, SBGEditor);

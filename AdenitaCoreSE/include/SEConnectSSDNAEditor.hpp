#pragma once 

#include "SBGEditor.hpp"
#include "SEConnectSSDNAEditorGUI.hpp"

#include "SBBaseEvent.hpp"
#include "SBDocumentEvent.hpp"
#include "SBDynamicalEvent.hpp"
#include "SBStructuralEvent.hpp"
#include "SBAction.hpp"

#include "ADNModel.hpp"
#include "ADNMixins.hpp"

/// This class implements an editor

class SB_EXPORT SEConnectSSDNAEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public :

	enum class ConnectionMode {
		Single,
		Double
	};

	/// \name Constructors and destructors
	//@{

	SEConnectSSDNAEditor();																												///< Builds an editor					
	virtual ~SEConnectSSDNAEditor();																									///< Destructs the editor

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

	SEConnectSSDNAEditorGUI*									getPropertyWidget() const;												///< Returns the property widget of the editor

	//@}

	void														setConnectionMode(bool xo);
	void														setSequence(std::string seq);
	void														setAutoSequenceFlag(bool s);
	void														setConcatFlag(bool c);

	void														resetData();

protected:

	/// \name SAMSON Events
	//@{

	class Observer : public SBCReferenceTarget {

		Observer(SEConnectSSDNAEditor* editor) { this->editor = editor; }
		virtual ~Observer() {}

		friend class SEConnectSSDNAEditor;

		void														onBaseEvent(SBBaseEvent* event);

		SEConnectSSDNAEditor*										editor{ nullptr };

	};

	SBPointer<Observer>											observer;

	//@}

private:

	bool														displayFlag = false;
	ADNPointer<ADNNucleotide>									selectedStartNucleotide = nullptr;

	ConnectionMode												connectionMode = ConnectionMode::Single;
	std::string													sequence = "";
	bool														concatFlag = false;
	bool														autoSequenceFlag = false;

	std::string													previousSelectionFilter;

};


SB_REGISTER_TYPE(SEConnectSSDNAEditor, "SEConnectSSDNAEditor", "48FDCE78-A55E-FDA2-237E-319202E56080");
SB_DECLARE_BASE_TYPE(SEConnectSSDNAEditor, SBGEditor);

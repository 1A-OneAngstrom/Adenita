#pragma once 

#include "SBGEditor.hpp"
#include "SEConnectSSDNAEditorGUI.hpp"

#include "SBBaseEvent.hpp"
#include "SBDocumentEvent.hpp"
#include "SBDynamicalEvent.hpp"
#include "SBStructuralEvent.hpp"
#include "SBAction.hpp"

#include "ADNModel.hpp"
#include "ADNNucleotide.hpp"
#include "ADNMixins.hpp"

/// This class implements an editor

class SB_EXPORT SEConnectSSDNAEditor : public SBGEditor {

	SB_CLASS
	Q_OBJECT

public:

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

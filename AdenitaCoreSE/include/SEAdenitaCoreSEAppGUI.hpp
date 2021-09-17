#pragma once


#include "SBGApp.hpp"
#include "ui_SEAdenitaCoreSEAppGUI.h"

#include <QToolButton>
#include <QPushButton>

#ifdef NDEBUG
#define ADENITA_DEBUG
#endif
#ifdef _DEBUG
#define ADENITA_DEBUG
#endif

class SEAdenitaCoreSEApp;

/// This class implements the GUI of the app

// SAMSON Element generator pro tip: add GUI functionality in this class. The non-GUI functionality should go in the SEAdenitaCoreSEApp class

class SB_EXPORT SEAdenitaCoreSEAppGUI : public SBGApp {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEAdenitaCoreSEAppGUI(SEAdenitaCoreSEApp* t);																						///< Constructs a GUI for the app
	virtual ~SEAdenitaCoreSEAppGUI();																									///< Destructs the GUI of the app

	//@}

	/// \name App
	//@{

	SEAdenitaCoreSEApp*											getApp() const;															///< Returns a pointer to the app

	//@}

	/// \name Identity
	//@{

	virtual SBCContainerUUID									getUUID() const;														///< Returns the widget UUID
	virtual QString												getName() const;														///< Returns the widget name (used as a title for the embedding window)
	virtual QPixmap												getLogo() const;														///< Returns the widget logo
	int															getFormat() const;														///< Returns the widget format
	virtual QString												getCitation() const;													///< Returns the citation information

	//@}

	virtual void												keyPressEvent(QKeyEvent *event);

	///\name Settings
	//@{

	void														loadSettings(SBGSettings* settings);									///< Load GUI settings
	void														saveSettings(SBGSettings* settings);									///< Save GUI settings

	//@}

	static std::string											getScaffoldFilename();													///< Returns the selected scaffold

	static std::string											isCadnanoJsonFormat(QString filename);

	void														clearHighlightEditor();

public slots:

	// Main

	void														onLoadFile();
	void														onSaveAll();
	void														onSaveSelection();
	void														onExport();
	void														onResetVisualModel();
	void														onSettings();

	// Edit sequences

	void														onSetScaffold();
	void														onGenerateSequence();
	void														onCalculateBindingProperties();
	void														onSetStart();
	void														onTaggingEditor();

	// Modeling

	void														onBreakEditor();
	void														onConnectEditor();
	void														onDeleteEditor();
	void														onTwisterEditor();
	void														onDNATwistEditor();
	void														onMergePartsEditor();

	// Creators

	void														onCreateBasePair();
	void														onCreateStrandEditor();
	void														onNanotubeCreatorEditor();
	void														onLatticeCreatorEditor();
	void														onWireframeEditor();

	// Debug

	void														onAddNtThreeP();
	void														onCenterPart();
	void														onCatenanes();
	void														onKinetoplast();
	void														onTestNeighbors();
	void														onOxDNAImport();
	void														onFromDatagraph();
	void														onHighlightXOs();
	void														onHighlightPosXOs();
	void														onExportToCanDo();
	void														onFixDesigns();

private slots:

	void														checkForLoadedParts();

private:

	void														setupUI();

	void														setHighlightEditor(QToolButton* button = nullptr);

	std::vector<QToolButton*>									menuButtons_;
	std::vector<QToolButton*>									editSequencesButtons_;
	std::vector<QToolButton*>									modelingButtons_;
	std::vector<QToolButton*>									creatorsButtons_;
	std::vector<QPushButton*>									debugButtons_;

	std::vector<QToolButton*>									getMenuButtons();
	std::vector<QToolButton*>									getEditSequencesButtons();
	std::vector<QToolButton*>									getModelingButtons();
	std::vector<QToolButton*>									getCreatorsButtons();
	std::vector<QPushButton*>									getDebugButtons();

	static void													setToolButtonStyleSheet(QToolButton* button);

	Ui::SEAdenitaCoreSEAppGUIClass								ui;

	QToolButton*												highlightedEditorButton_ = nullptr;

	QString														workingDirectory = QString();

};

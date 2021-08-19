#pragma once

#include "ui_SEAdenitaCoreSettings.h"

#include "SAMSON.hpp"
#include "SBGWindow.hpp"

#include <QDialog>


class SB_EXPORT SEAdenitaCoreSettingsGUI : public QDialog {

	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SEAdenitaCoreSettingsGUI(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());													///< Constructs a GUI for the app
	virtual ~SEAdenitaCoreSettingsGUI();																										///< Destructs the GUI of the app

	void														showDialog();

public slots:

	void onChangeScaffold(int idx);
	void onSetPathNtthal();
	void accept();
	void reject() { QDialog::reject(); };

private:

	Ui_SEAdenitaCoreSettingsGUI									ui;

	SBGWindow*													dialogWindow{ nullptr };

};


#include "SEAdenitaCoreSettingsGUI.hpp"
#include "ADNConfig.hpp"

#include <QFileDialog>


SEAdenitaCoreSettingsGUI::SEAdenitaCoreSettingsGUI(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f) {

	ui.setupUi( this );

	populateUi();

	dialogWindow = SAMSON::addDialog(this, "Adenita settings", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);

	QObject::connect(ui.comboBoxScaffold, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeScaffold(int)));
	QObject::connect(ui.pushButtonOK, SIGNAL(released()), this, SLOT(accept()));
	QObject::connect(ui.pushButtonCancel, SIGNAL(released()), this, SLOT(reject()));
	//QObject::connect(ui.pushButtonReset, SIGNAL(released()), this, SLOT(onResetToDefault()));
	QObject::connect(ui.pushButtonSetPathNtthal, SIGNAL(released()), this, SLOT(onSetPathNtthal()));

	ui.pushButtonReset->setVisible(false);

}

SEAdenitaCoreSettingsGUI::~SEAdenitaCoreSettingsGUI() {}

void SEAdenitaCoreSettingsGUI::populateUi() {

	SEConfig& c = SEConfig::GetInstance();

	ui.checkBoxXODisplay->setChecked(c.display_possible_crossovers);
	ui.checkBoxOverlay->setChecked(c.show_overlay);
	ui.checkBoxInterpolateDim->setChecked(c.interpolate_dimensions);
	ui.checkBoxGenerateAtomicDetails->setChecked(c.use_atomic_details);
	ui.checkBoxAutoScaffold->setChecked(c.auto_set_scaffold_sequence);
	ui.lineEditNtthal->setText(QString::fromStdString(c.ntthal));
	ui.lineEditCustomScaffold->setText(QString::fromStdString(c.scaffCustomFilename));
	ui.comboBoxScaffold->setCurrentIndex(c.scaffType);
	ui.radioButtonDaedalusMesh->setChecked(!c.custom_mesh_model);
	ui.radioButtonIterativeMesh->setChecked(c.custom_mesh_model);

}


void SEAdenitaCoreSettingsGUI::onResetToDefault() {

}

void SEAdenitaCoreSettingsGUI::showDialog() {

	populateUi();

	dialogWindow->show();
	exec();

}

void SEAdenitaCoreSettingsGUI::onChangeScaffold(int idx) {

	if (idx == 2) {

		// custom scaffold

		QString filename;// = QFileDialog::getOpenFileName(this, tr("Choose scaffold"), QDir::currentPath(), tr("(Sequences *.fasta)"));
		if (!SAMSON::getFileNameFromUser(tr("Choose scaffold"), filename, QDir::currentPath(), tr("(Sequences *.fasta)"))) return;
		
		ui.lineEditCustomScaffold->setText(filename);

	}

}

void SEAdenitaCoreSettingsGUI::onSetPathNtthal() {

	QString filename;// = QFileDialog::getOpenFileName(this, tr("Set path to ntthal executable"), QDir::currentPath(), tr("(ntthal.exe)"));
	if (!SAMSON::getFileNameFromUser(tr("Set path to ntthal executable"), filename, QDir::currentPath(), tr("(ntthal.exe)"))) return;
	ui.lineEditNtthal->setText(filename);

}

void SEAdenitaCoreSettingsGUI::accept() {

	QDialog::accept();

	SEConfig& c = SEConfig::GetInstance();

	c.setScaffCustomFilename(ui.lineEditCustomScaffold->text().toStdString());
	c.setDisplayPossibleCrossovers(ui.checkBoxXODisplay->isChecked());
	c.setShowOverlay(ui.checkBoxOverlay->isChecked());
	c.setInterpolateDimensions(ui.checkBoxInterpolateDim->isChecked());
	c.setUseAtomicDetailsFlag(ui.checkBoxGenerateAtomicDetails->isChecked());
	c.setAutoSetScaffoldSequence(ui.checkBoxAutoScaffold->isChecked());
	c.setScaffType(ui.comboBoxScaffold->currentIndex());
	c.setNtthalExe(ui.lineEditNtthal->text().toStdString());
	c.setCustomMeshModel(ui.radioButtonIterativeMesh->isChecked());

}

void SEAdenitaCoreSettingsGUI::reject() {

	QDialog::reject();

}

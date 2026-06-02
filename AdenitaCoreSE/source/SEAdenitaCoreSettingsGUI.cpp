#include "SEAdenitaCoreSettingsGUI.hpp"
#include "ADNConfig.hpp"


SEAdenitaCoreSettingsGUI::SEAdenitaCoreSettingsGUI(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {

	ui.setupUi(this);
	setupHelpText();

	populateUi();

	dialogWindow = SAMSON::addDialog(this, "Adenita settings", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);

	QObject::connect(ui.comboBoxScaffold, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SEAdenitaCoreSettingsGUI::onChangeScaffold);
	QObject::connect(ui.pushButtonOK, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::accept);
	QObject::connect(ui.pushButtonCancel, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::reject);
	//QObject::connect(ui.pushButtonReset, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::onResetToDefault);
	QObject::connect(ui.pushButtonSetPathNtthal, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::onSetPathNtthal);

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

void SEAdenitaCoreSettingsGUI::setupHelpText() {

	ui.label->setToolTip(
		tr("Scaffold sequence used for DNA origami designs. Choose m13mp18, p7249, or Custom."));
	ui.comboBoxScaffold->setToolTip(
		tr("Scaffold sequence used for DNA origami designs. Choose Custom to provide a sequence file."));
	ui.label_2->setToolTip(
		tr("Path to the custom scaffold sequence used when Custom scaffold is selected."));
	ui.lineEditCustomScaffold->setToolTip(
		tr("Path to the custom scaffold sequence used when Custom scaffold is selected."));
	ui.checkBoxAutoScaffold->setToolTip(
		tr("Automatically assign the selected scaffold sequence to new compatible Adenita structures."));
	ui.checkBoxOverlay->setToolTip(
		tr("Show Adenita viewport overlays for editor previews and design guidance."));
	ui.checkBoxXODisplay->setToolTip(
		tr("Display possible crossover positions in the viewport."));
	ui.checkBoxInterpolateDim->setToolTip(
		tr("Interpolate displayed dimensions for smoother Adenita visual geometry."));
	ui.checkBoxGenerateAtomicDetails->setToolTip(
		tr("Generate atomic models when creating DNA structures. This is slower because all atoms must be created."));
	ui.radioButtonDaedalusMesh->setToolTip(
		tr("Use the Daedalus mesh processing model for wireframe DNA generation."));
	ui.radioButtonIterativeMesh->setToolTip(
		tr("Use the iterative mesh processing model for wireframe DNA generation."));
	ui.label_3->setToolTip(
		tr("Path to the ntthal executable used for melting temperature and Gibbs free energy calculations."));
	ui.lineEditNtthal->setToolTip(
		tr("Path to the ntthal executable used for melting temperature and Gibbs free energy calculations."));
	ui.pushButtonSetPathNtthal->setToolTip(
		tr("Browse for the ntthal executable."));

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

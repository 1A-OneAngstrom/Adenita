#include "SEAdenitaCoreSettingsGUI.hpp"
#include "ADNAuxiliary.hpp"
#include "ADNConfig.hpp"

#include <initializer_list>

#include <QDir>
#include <QFileInfo>

namespace {

void applyToolTip(const QString& text, std::initializer_list<QWidget*> widgets) {

	for (QWidget* widget : widgets)
		if (widget) widget->setToolTip(text);

}

QString getSelectionDirectory(const QString& currentPath) {

	if (currentPath.isEmpty()) return QDir::currentPath();

	QFileInfo currentPathInfo(currentPath);
	if (currentPathInfo.exists()) {
		if (currentPathInfo.isDir()) return currentPathInfo.absoluteFilePath();
		return currentPathInfo.absolutePath();
	}

	const QFileInfo parentDirectoryInfo(currentPathInfo.absolutePath());
	if (parentDirectoryInfo.exists() && parentDirectoryInfo.isDir())
		return parentDirectoryInfo.absoluteFilePath();

	return QDir::currentPath();

}

}


SEAdenitaCoreSettingsGUI::SEAdenitaCoreSettingsGUI(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {

	ui.setupUi(this);
	setupHelpText();

	populateUi();

	dialogWindow = SAMSON::addDialog(this, "Adenita settings", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);

	QObject::connect(ui.comboBoxScaffold, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SEAdenitaCoreSettingsGUI::onChangeScaffold);
	QObject::connect(ui.pushButtonSetCustomScaffold, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::onSetCustomScaffold);
	QObject::connect(ui.pushButtonOK, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::accept);
	QObject::connect(ui.pushButtonCancel, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::reject);
	QObject::connect(ui.pushButtonSetPathNtthal, &QPushButton::released, this, &SEAdenitaCoreSettingsGUI::onSetPathNtthal);

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
	ui.checkBoxOverlay->setEnabled(false);
	ui.checkBoxXODisplay->setEnabled(false);
	ui.checkBoxInterpolateDim->setEnabled(false);

	updateCustomScaffoldControls();

}

void SEAdenitaCoreSettingsGUI::setupHelpText() {

	const QString scaffoldToolTip = tr(
		"<qt><p>Choose the scaffold sequence used by Adenita's scaffold-assignment tools.</p>"
		"<p>Adenita uses this sequence when assigning bases to scaffold strands, for example through the Set scaffold command or automatic scaffold assignment. "
		"m13mp18 and p7249 are built in. Choose Custom to use your own FASTA sequence file.</p></qt>");
	applyToolTip(scaffoldToolTip, { ui.groupBoxScaffoldSequence, ui.label, ui.comboBoxScaffold });

	const QString customScaffoldToolTip = tr(
		"<qt><p>FASTA file used when the scaffold sequence is set to Custom.</p>"
		"<p>Choose a FASTA file containing the scaffold sequence to assign to scaffold strands. "
		"This path is used only when the default scaffold sequence is Custom.</p></qt>");
	applyToolTip(customScaffoldToolTip, { ui.label_2, ui.lineEditCustomScaffold });
	ui.pushButtonSetCustomScaffold->setToolTip(
		tr("<qt><p>Browse for a custom scaffold FASTA file.</p>"
		   "<p>Select the FASTA file Adenita should use when the default scaffold sequence is Custom.</p></qt>"));

	ui.checkBoxAutoScaffold->setToolTip(
		tr("<qt><p>Automatically assign the selected scaffold sequence when Adenita creates or loads compatible designs.</p>"
		   "<p>When enabled, Adenita tries to assign the currently selected scaffold sequence to scaffold strands automatically when appropriate. "
		   "Existing designs can still be updated manually with Set scaffold.</p></qt>"));

	const QString viewportGroupToolTip = tr(
		"<qt><p>Viewport guidance options for Adenita's multiscale display.</p>"
		"<p>The three controls in this section are shown for transparency, but they are currently not active in Adenita's runtime behavior.</p></qt>");
	applyToolTip(viewportGroupToolTip, { ui.groupBoxViewportDisplay });

	ui.checkBoxOverlay->setToolTip(
		tr("<qt><p>Reserved for a future viewport option for Adenita overlays and editor guides.</p>"
		   "<p>This setting is stored in Adenita's configuration, but it is not currently used at runtime.</p></qt>"));
	ui.checkBoxXODisplay->setToolTip(
		tr("<qt><p>Reserved for a future viewport option for possible crossover positions.</p>"
		   "<p>This setting is stored in Adenita's configuration, but it is not currently used at runtime.</p></qt>"));
	ui.checkBoxInterpolateDim->setToolTip(
		tr("<qt><p>Reserved for a future visual-model option for 1D, 2D, and 3D view transitions.</p>"
		   "<p>This setting is stored in Adenita's configuration, but it currently has no effect.</p></qt>"));

	ui.checkBoxGenerateAtomicDetails->setToolTip(
		tr("<qt><p>Also create atom-level DNA geometry when Adenita creates new structures.</p>"
		   "<p>When enabled, Adenita creates atom-level details for newly generated DNA structures. "
		   "This is useful for atomistic inspection or downstream workflows, but generation is slower and the document becomes heavier.</p></qt>"));

	const QString meshAlgorithmToolTip = tr(
		"<qt><p>Choose the algorithm used to create DNA wireframe models from mesh input.</p></qt>");
	applyToolTip(meshAlgorithmToolTip, { ui.groupBoxModelGeneration, ui.labelMeshAlgorithm });
	ui.radioButtonDaedalusMesh->setToolTip(
		tr("<qt><p>Use the Daedalus-based wireframe generation path.</p>"
		   "<p>This remains the default unless you specifically need the iterative method.</p></qt>"));
	ui.radioButtonIterativeMesh->setToolTip(
		tr("<qt><p>Use Adenita's iterative wireframe generation path.</p>"
		   "<p>Try this when the Daedalus-based path is not appropriate for the mesh or when you intentionally want Adenita's iterative behavior.</p></qt>"));

	const QString ntthalToolTip = tr(
		"<qt><p>Path to the ntthal executable used for binding-region thermodynamic calculations.</p>"
		"<p>Adenita can use ntthal from Primer3 to calculate melting temperatures and Gibbs free energies for binding regions. "
		"On Windows this is typically an ntthal.exe file.</p></qt>");
	applyToolTip(ntthalToolTip, { ui.groupBoxThermodynamicCalculations, ui.label_3, ui.lineEditNtthal });
	ui.pushButtonSetPathNtthal->setToolTip(
		tr("<qt><p>Browse for the ntthal executable.</p>"
		   "<p>Adenita uses this path only when calculating binding-region properties; it is not required for basic editing or visualization.</p></qt>"));

}

void SEAdenitaCoreSettingsGUI::updateCustomScaffoldControls() {

	const bool useCustomScaffold = ui.comboBoxScaffold->currentIndex() == static_cast<int>(ADNAuxiliary::ScaffoldSeq::Custom);
	ui.pushButtonSetCustomScaffold->setEnabled(useCustomScaffold);

}

void SEAdenitaCoreSettingsGUI::showDialog() {

	populateUi();

	dialogWindow->show();
	exec();

}

void SEAdenitaCoreSettingsGUI::onChangeScaffold(int idx) {

	Q_UNUSED(idx);

	updateCustomScaffoldControls();

}

void SEAdenitaCoreSettingsGUI::onSetCustomScaffold() {

	QString filename;
	const QString initialDirectory = getSelectionDirectory(ui.lineEditCustomScaffold->text());
	if (!SAMSON::getFileNameFromUser(
			tr("Choose custom scaffold FASTA"),
			filename,
			initialDirectory,
			tr("FASTA files (*.fasta *.fa *.fas);;All files (*.*)"))) return;

	ui.lineEditCustomScaffold->setText(filename);

}

void SEAdenitaCoreSettingsGUI::onSetPathNtthal() {

	QString filename;
	const QString initialDirectory = getSelectionDirectory(ui.lineEditNtthal->text());
	if (!SAMSON::getFileNameFromUser(
			tr("Choose ntthal executable"),
			filename,
			initialDirectory,
			tr("Executable files (*.exe);;All files (*.*)"))) return;
	ui.lineEditNtthal->setText(filename);

}

void SEAdenitaCoreSettingsGUI::accept() {

	SEConfig& c = SEConfig::GetInstance();
	const bool useCustomScaffold = ui.comboBoxScaffold->currentIndex() == static_cast<int>(ADNAuxiliary::ScaffoldSeq::Custom);
	const QString customScaffoldPath = ui.lineEditCustomScaffold->text().trimmed();
	const QFileInfo customScaffoldInfo(customScaffoldPath);

	if (useCustomScaffold && (customScaffoldPath.isEmpty() || !customScaffoldInfo.exists() || !customScaffoldInfo.isFile())) {

		SAMSON::informUser(
			tr("Adenita settings"),
			tr("Choose a valid custom scaffold FASTA file or switch back to one of the built-in scaffold sequences before saving."));
		return;

	}

	c.setScaffCustomFilename(customScaffoldPath.toStdString());
	c.setDisplayPossibleCrossovers(ui.checkBoxXODisplay->isChecked());
	c.setShowOverlay(ui.checkBoxOverlay->isChecked());
	c.setInterpolateDimensions(ui.checkBoxInterpolateDim->isChecked());
	c.setUseAtomicDetailsFlag(ui.checkBoxGenerateAtomicDetails->isChecked());
	c.setAutoSetScaffoldSequence(ui.checkBoxAutoScaffold->isChecked());
	c.setScaffType(ui.comboBoxScaffold->currentIndex());
	c.setNtthalExe(ui.lineEditNtthal->text().toStdString());
	c.setCustomMeshModel(ui.radioButtonIterativeMesh->isChecked());

	QDialog::accept();

}

void SEAdenitaCoreSettingsGUI::reject() {

	QDialog::reject();

}

#include "SEAdenitaCoreSEAppGUI.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SEAdenitaCoreSettingsGUI.hpp"
#include "SAMSON.hpp"
#include "SBGWindow.hpp"
#include "SBGWindowDialog.hpp"

#include <QInputDialog>
#include <QFormLayout>
#include <QToolButton>

#include "SEWireframeEditor.hpp"
#include "SEBreakEditor.hpp"
#include "SEConnectSSDNAEditor.hpp"
#include "SEDeleteEditor.hpp"
#include "SETwistHelixEditor.hpp"
#include "SEDNATwisterEditor.hpp"
#include "SETaggingEditor.hpp"
#include "SELatticeCreatorEditor.hpp"
#include "SENanotubeCreatorEditor.hpp"
#include "SEDSDNACreatorEditor.hpp"
#include "SEMergePartsEditor.hpp"

#include <QPixmap>
#include <QTimer>
#include <QGroupBox>
#include <QMessageBox>

#include "SEAdenitaCoreSEAppGUIFlowLayout.hpp"

#ifdef NDEBUG
#define ADENITA_DEBUG
#endif
#ifdef _DEBUG
#define ADENITA_DEBUG
#endif

SEAdenitaCoreSEAppGUI::SEAdenitaCoreSEAppGUI( SEAdenitaCoreSEApp* t ) : SBGApp( t ) {

	setupUI();

	//QTimer *timer = new QTimer(this);
	//connect(timer, SIGNAL(timeout()), this, SLOT(checkForLoadedParts()));
	//timer->start();

}

SEAdenitaCoreSEAppGUI::~SEAdenitaCoreSEAppGUI() {

}

SEAdenitaCoreSEApp* SEAdenitaCoreSEAppGUI::getApp() const { return static_cast<SEAdenitaCoreSEApp*>(SBGApp::getApp()); }

void SEAdenitaCoreSEAppGUI::loadSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;
	
	// SAMSON Element generator pro tip: complete this function so your app can save its GUI state from one session to the next

}

void SEAdenitaCoreSEAppGUI::saveSettings( SBGSettings *settings ) {

	if ( settings == nullptr ) return;

	// SAMSON Element generator pro tip: complete this function so your app can save its GUI state from one session to the next

}

std::string SEAdenitaCoreSEAppGUI::getScaffoldFilename() {

	SEConfig& c = SEConfig::GetInstance();

	std::string filename = "";
	ADNAuxiliary::ScaffoldSeq type = ADNAuxiliary::ScaffoldSeq(c.scaffType);
	if (type == ADNAuxiliary::m13mp18) {
		filename = SB_ELEMENT_PATH + "/Data/m13mp18.fasta";
	}
	else if (type == ADNAuxiliary::p7249) {
		filename = SB_ELEMENT_PATH + "/Data/p7249.fasta";
	}
	else if (type == ADNAuxiliary::Custom) {
		filename = c.scaffCustomFilename;
	}

	return filename;

}

void SEAdenitaCoreSEAppGUI::onLoadFile() {

	bool ok;
	QString filename;// = QFileDialog::getOpenFileName(this, tr("Open document: caDNAno, mesh (ply), Adenita document (adn, adnpart)"), QDir::currentPath(), tr("(Documents *.json *.ply *.adn *.adnpart)"));
	ok = SAMSON::getFileNameFromUser(tr("Open document: caDNAno, mesh (ply, json), Adenita document (adn, adnpart, json)"), filename, workingDirectory, tr("(Documents *.json *.ply *.adn *.adnpart)"));
	if (!ok) return;
	if (filename.isEmpty()) return;

	workingDirectory = QFileInfo(filename).absolutePath();	// get the absolute path to the filename

	SEAdenitaCoreSEApp* t = getApp();

	if (filename.endsWith(".json")) {

		// either cadnano or old Adenita format
		std::string format = SEAdenitaCoreSEAppGUI::isCadnanoJsonFormat(filename);
		if (format == "cadnano") {

			t->ImportFromCadnano(filename);

		}
		else if (format == "adenita") {

			t->LoadPart(filename);

		}
		else {

			SAMSON::informUser("Adenita", "Unknown json format. Current supported formats include Cadnano and legacy Adenita parts.");
			return;

		}

	}
	else if (filename.endsWith(".ply")) {

		int i = 42;
		//int i = QInputDialog::getInt(this, tr("Wireframe structure (Daedalus)"), tr("Minimum edge size (bp): "), 42, 31, 1050, 1, &ok);
		ok = SAMSON::getIntegerFromUser(QString("Wireframe structure (Daedalus)"), i, 31, 1050, 1, QString("Minimum edge size: "), QString(" bp"));
		if (ok) {

			div_t d = div(i, 10.5);
			int minSize = floor(d.quot * 10.5);
			t->LoadPartWithDaedalus(filename, minSize);

		}
		else return;

	}
	else if (filename.endsWith(".adnpart")) {

		t->LoadPart(filename);

	}
	else if (filename.endsWith(".adn")) {

		t->LoadParts(filename);

	}
	else return;

	// add a visual model or reset the existing one

	SEAdenitaCoreSEApp::resetVisualModel();

	SAMSON::getActiveCamera()->center();

}

void SEAdenitaCoreSEAppGUI::onSaveAll() {

	SEAdenitaCoreSEApp* t = getApp();

	QString filename;// = QFileDialog::getSaveFileName(this, tr("Save the workspace"), QDir::currentPath(), tr("Adenita workspace (*.adn)"));
	if (SBGWindowDialog::getSaveFileNameFromUser(tr("Save the workspace"), filename, workingDirectory, tr("Adenita workspace (*.adn)"))) {

		QFileInfo fileInfo(filename);
		workingDirectory = fileInfo.absolutePath();	// get the absolute path to the filename
		if (fileInfo.exists()) if (!SAMSON::askUser("Save the workspace", "The file already exists. Do you want to overwrite it?")) return;

		if (!filename.isEmpty()) t->SaveFile(filename);

	}

}

void SEAdenitaCoreSEAppGUI::onExport() {

	SEAdenitaCoreSEApp* t = getApp();

	QDialog* dialog = new QDialog();

	QComboBox* typeSelection = new QComboBox();
  
	auto nr = t->GetNanorobot();
	auto parts = nr->GetParts();
	int i = 0;
	std::map<int, ADNPointer<ADNPart>> indexParts;
	SB_FOR(ADNPointer<ADNPart> p, parts) {

		std::string n = p->GetName();
		typeSelection->insertItem(i, QString::fromStdString(n));
		indexParts.insert(std::make_pair(i, p));
		++i;

	}
	typeSelection->insertItem(i, QString::fromStdString("Selected Component(s)"));
	int sel_idx = i;
	typeSelection->insertItem(i+1, QString::fromStdString("Workspace"));
	int all_idx = i + 1;

	QStringList itemsExportType;
	itemsExportType << "Sequence list" << "oxDNA";
	QComboBox* exportType = new QComboBox();
	exportType->addItems(itemsExportType);

	QPushButton* acceptButton = new QPushButton(tr("Export"));
	acceptButton->setDefault(true);
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(acceptButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

	QObject::connect(cancelButton, SIGNAL(released()), dialog, SLOT(close()));
	QObject::connect(acceptButton, SIGNAL(released()), dialog, SLOT(accept()));

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addWidget(typeSelection, 0, 0);
	mainLayout->addWidget(exportType, 1, 0);
	mainLayout->addWidget(buttonBox, 2, 0);

	dialog->setLayout(mainLayout);
	//dialog->setWindowTitle(tr("Export design"));

	// make it a SAMSON dialog
	SBGWindow* dialogWindow = SAMSON::addDialog(dialog, "Adenita: Export design", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);
	dialogWindow->show();

	int dialogCode = dialog->exec();

	if (dialogCode == QDialog::Accepted) {
    
		auto val = typeSelection->currentIndex();

		CollectionMap<ADNPart> selectedParts;
		std::pair<SBPosition3, SBPosition3> boundingBox;

		if (val == sel_idx) {

			selectedParts = nr->GetSelectedParts();

		}
		else if (val != all_idx) {

			ADNPointer<ADNPart> part = indexParts.at(val);
			selectedParts.addReferenceTarget(part());

		}
		else {

			selectedParts = nr->GetParts();

		}
		boundingBox = nr->GetBoundingBox(selectedParts);

		QString eType = exportType->currentText();

		if (eType == "Sequence list") {

			// export sequences
			QString filename;// = QFileDialog::getSaveFileName(this, tr("Sequence List"), QDir::currentPath(), tr("Sequence List (*.csv)"));
			if (SBGWindowDialog::getSaveFileNameFromUser(tr("Sequence List"), filename, workingDirectory, tr("Sequence List (*.csv)"))) {

				QFileInfo fileInfo(filename);
				workingDirectory = fileInfo.absolutePath();	// get the absolute path to the filename
				if (fileInfo.exists()) if (!SAMSON::askUser("Sequence List", "The file already exists. Do you want to overwrite it?")) return;

				t->ExportToSequenceList(filename, selectedParts);

			}

		}
		else if (eType == "oxDNA") {

			auto bbSize = boundingBox.second - boundingBox.first;

			ADNAuxiliary::OxDNAOptions options;
			double sysX = bbSize[0].getValue() * 0.001;  // nm
			double sysY = bbSize[1].getValue() * 0.001;  // nm
			double sysZ = bbSize[2].getValue() * 0.001;  // nm

			double refVal = std::max(sysX, sysY);
			refVal = std::max(refVal, sysZ);

			// oxDNA dialog
			QDialog* dialogOxDNA = new QDialog();
			QFormLayout *oxDNALayout = new QFormLayout;
			QLabel* info = new QLabel;
			info->setText("System size (nm): " + QString::number(sysX, 'g',2) + " x " + QString::number(sysY, 'g', 2) + " x " + QString::number(sysZ, 'g', 2));
			QDoubleSpinBox* boxX = new QDoubleSpinBox();
			boxX->setRange(0.0, 99999.9);
			boxX->setValue(refVal * 3);
			boxX->setDecimals(2);
			QDoubleSpinBox* boxY = new QDoubleSpinBox();
			boxY->setRange(0.0, 99999.9);
			boxY->setValue(refVal * 3);
			boxY->setDecimals(2);
			QDoubleSpinBox* boxZ = new QDoubleSpinBox();
			boxZ->setRange(0.0, 99999.9);
			boxZ->setValue(refVal * 3);
			boxZ->setDecimals(2);

			QPushButton* accButton = new QPushButton(tr("Continue"));
			accButton->setDefault(true);
			QPushButton* cButton = new QPushButton(tr("Cancel"));

			QDialogButtonBox* bttBox = new QDialogButtonBox(Qt::Horizontal);
			bttBox->addButton(accButton, QDialogButtonBox::ActionRole);
			bttBox->addButton(cButton, QDialogButtonBox::ActionRole);

			QObject::connect(cButton, SIGNAL(released()), dialogOxDNA, SLOT(close()));
			QObject::connect(accButton, SIGNAL(released()), dialogOxDNA, SLOT(accept()));

			oxDNALayout->addRow(info);
			oxDNALayout->addRow(QString("Box size X (nm)"), boxX);
			oxDNALayout->addRow(QString("Box size Y (nm)"), boxY);
			oxDNALayout->addRow(QString("Box size Z (nm)"), boxZ);
			oxDNALayout->addRow(bttBox);

			dialogOxDNA->setLayout(oxDNALayout);
			//dialogOxDNA->setWindowTitle(tr("Set bounding box size"));

			// make it a SAMSON dialog
			SBGWindow* dialogOxDNAWindow = SAMSON::addDialog(dialogOxDNA, "Adenita: Set bounding box size", SBGWindow::NoOptions);
			dialogOxDNAWindow->QWidget::setWindowFlags(dialogOxDNAWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
			dialogOxDNAWindow->setWindowModality(Qt::ApplicationModal);
			dialogOxDNAWindow->show();

			int dCode = dialogOxDNA->exec();

			if (dCode == QDialog::Accepted) {

				options.boxSizeX_ = boxX->value();
				options.boxSizeY_ = boxY->value();
				options.boxSizeZ_ = boxZ->value();

				QString folder = QFileDialog::getExistingDirectory(this, tr("Choose an existing directory"), workingDirectory, QFileDialog::DontUseNativeDialog);
				if (!folder.isEmpty()) t->ExportToOxDNA(folder, options, selectedParts);

			}

			dialogOxDNAWindow->deleteLater();

		}

	}

	dialogWindow->deleteLater();

}

void SEAdenitaCoreSEAppGUI::onSaveSelection() {

	SEAdenitaCoreSEApp* t = getApp();

	QDialog* dialog = new QDialog();

	QComboBox* typeSelection = new QComboBox();

	auto nr = t->GetNanorobot();
	auto parts = nr->GetParts();
	int i = 0;
	std::map<int, ADNPointer<ADNPart>> indexParts;
	SB_FOR(ADNPointer<ADNPart> p, parts) {

		std::string n = p->GetName();
		typeSelection->insertItem(i, QString::fromStdString(n));
		indexParts.insert(std::make_pair(i, p));
		++i;

	}
	typeSelection->insertItem(i, QString::fromStdString("Selected Component"));
	int sel_idx = i;

	QPushButton* acceptButton = new QPushButton(tr("Save"));
	acceptButton->setDefault(true);
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(acceptButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

	QObject::connect(cancelButton, SIGNAL(released()), dialog, SLOT(close()));
	QObject::connect(acceptButton, SIGNAL(released()), dialog, SLOT(accept()));

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addWidget(typeSelection, 0, 0);
	mainLayout->addWidget(buttonBox, 1, 0);

	dialog->setLayout(mainLayout);
	//dialog->setWindowTitle(tr("Export design"));

	// make it a SAMSON dialog
	SBGWindow* dialogWindow = SAMSON::addDialog(dialog, "Adenita: Export design", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);
	dialogWindow->show();

	int dialogCode = dialog->exec();

	if (dialogCode == QDialog::Accepted) {

		auto val = typeSelection->currentIndex();
		ADNPointer<ADNPart> part = nullptr;
		if (val == sel_idx) {
			part = nr->GetSelectedParts()[0];
		}
		else {
			part = indexParts.at(val);
		}

		QString filename;// = QFileDialog::getSaveFileName(this, tr("Save a part"), QDir::currentPath(), tr("Adenita part (*.adnpart)"));
		if (SBGWindowDialog::getSaveFileNameFromUser(tr("Save a part"), filename, QString(), tr("Adenita part (*.adnpart)"))) {

			QFileInfo fileInfo(filename);
			workingDirectory = fileInfo.absolutePath();	// get the absolute path to the filename
			if (fileInfo.exists()) if (!SAMSON::askUser("Save a part", "The file already exists. Do you want to overwrite it?")) return;

			if (!filename.isEmpty()) t->SaveFile(filename, part);

		}
	
	}

	dialogWindow->deleteLater();

}

void SEAdenitaCoreSEAppGUI::onSetScaffold() {

	std::string filename = SEAdenitaCoreSEAppGUI::getScaffoldFilename();
	SEAdenitaCoreSEApp *t = getApp();
	t->SetScaffoldSequence(filename);

}

void SEAdenitaCoreSEAppGUI::onAddNtThreeP() {

	SEAdenitaCoreSEApp *t = getApp();
	int numNt = 1;
	t->AddNtThreeP(numNt);

}

void SEAdenitaCoreSEAppGUI::onCenterPart() {

	SEAdenitaCoreSEApp *t = getApp();
	t->CenterPart();
	SAMSON::getActiveCamera()->center();

}

void SEAdenitaCoreSEAppGUI::onCatenanes() {

	QDialog* dialog = new QDialog();

	QLabel* numberLabel = new QLabel();
	numberLabel->setText("Number of catenanes");
	QSpinBox* number = new QSpinBox();
	number->setRange(1, 9999);
	number->setValue(2);

	QLabel* radiusLabel = new QLabel();
	radiusLabel->setText("Radius (nm)");
	QDoubleSpinBox* radius = new QDoubleSpinBox();
	radius->setRange(0.0, 99999.9);
	radius->setValue(20.0);
	radius->setDecimals(2);

	QPushButton* acceptButton = new QPushButton(tr("Create catenanes"));
	acceptButton->setDefault(true);
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	QDialogButtonBox* buttonBox_ = new QDialogButtonBox(Qt::Horizontal);
	buttonBox_->addButton(acceptButton, QDialogButtonBox::ActionRole);
	buttonBox_->addButton(cancelButton, QDialogButtonBox::ActionRole);

	QObject::connect(cancelButton, SIGNAL(released()), dialog, SLOT(reject()));
	QObject::connect(acceptButton, SIGNAL(released()), dialog, SLOT(accept()));

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addWidget(numberLabel, 0, 0);
	mainLayout->addWidget(number, 0, 1);
	mainLayout->addWidget(radiusLabel, 1, 0);
	mainLayout->addWidget(radius, 1, 1);
	mainLayout->addWidget(buttonBox_, 2, 0);

	dialog->setLayout(mainLayout);
	//dialog->setWindowTitle(tr("Create Catenanes"));

	// make it a SAMSON dialog
	SBGWindow* dialogWindow = SAMSON::addDialog(dialog, "Adenita: Create Catenanes", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);
	dialogWindow->show();

	int dialogCode = dialog->exec();

	if (dialogCode == QDialog::Accepted ) {

		int num = number->value();
		SBQuantity::length R = SBQuantity::nanometer(radius->value());
		SBVector3 normal = SBVector3();
		normal[0] = 0.0;
		normal[1] = 0.0;
		normal[2] = 1.0;
		SBPosition3 center = SBPosition3();
		SEAdenitaCoreSEApp* t = getApp();
		t->LinearCatenanes(R, center, normal, num);

	}

	dialogWindow->deleteLater();

}

void SEAdenitaCoreSEAppGUI::onKinetoplast() {

	QDialog* dialog = new QDialog();

	QLabel* rowsLabel = new QLabel();
	rowsLabel->setText("Number of rows");
	QSpinBox* rows = new QSpinBox();
	rows->setRange(1, 9999);
	rows->setValue(2);

	QLabel* colsLabel = new QLabel();
	colsLabel->setText("Number of columns");
	QSpinBox* cols = new QSpinBox();
	cols->setRange(1, 9999);
	cols->setValue(3);

	QLabel* radiusLabel = new QLabel();
	radiusLabel->setText("Radius (nm)");
	QDoubleSpinBox* radius = new QDoubleSpinBox();
	radius->setRange(0.0, 99999.9);
	radius->setValue(20.0);
	radius->setDecimals(2);

	QPushButton* acceptButton = new QPushButton(tr("Create catenanes"));
	acceptButton->setDefault(true);
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	QDialogButtonBox* buttonBox_ = new QDialogButtonBox(Qt::Horizontal);
	buttonBox_->addButton(acceptButton, QDialogButtonBox::ActionRole);
	buttonBox_->addButton(cancelButton, QDialogButtonBox::ActionRole);

	QObject::connect(cancelButton, SIGNAL(released()), dialog, SLOT(reject()));
	QObject::connect(acceptButton, SIGNAL(released()), dialog, SLOT(accept()));

	QGridLayout *mainLayout = new QGridLayout();
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addWidget(rowsLabel, 0, 0);
	mainLayout->addWidget(rows, 0, 1);
	mainLayout->addWidget(colsLabel, 1, 0);
	mainLayout->addWidget(cols, 1, 1);
	mainLayout->addWidget(radiusLabel, 2, 0);
	mainLayout->addWidget(radius, 2, 1);
	mainLayout->addWidget(buttonBox_, 3, 0);

	dialog->setLayout(mainLayout);
	//dialog->setWindowTitle(tr("Create Kinetoplast"));

	// make it a SAMSON dialog
	SBGWindow* dialogWindow = SAMSON::addDialog(dialog, "Adenita: Create Kinetoplast", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);
	dialogWindow->show();

	int dialogCode = dialog->exec();

	if (dialogCode == QDialog::Accepted) {

		int r = rows->value();
		int c = cols->value();
		SBQuantity::length R = SBQuantity::nanometer(radius->value());
		SBVector3 normal = SBVector3();
		normal[0] = 0.0;
		normal[1] = 0.0;
		normal[2] = 1.0;
		SBPosition3 center = SBPosition3();
		SEAdenitaCoreSEApp* t = getApp();
		t->Kinetoplast(R, center, normal, r, c);

	}

	dialogWindow->deleteLater();

}

void SEAdenitaCoreSEAppGUI::onCalculateBindingProperties() {

	QDialog dialog(this);
	QFormLayout form(&dialog);

	form.addRow(new QLabel("Parameters for Calculating the Binding Region Properties"));

	//oligo conc
	QLineEdit *oligoConcText = new QLineEdit(&dialog);
	oligoConcText->setText("100");
	form.addRow(new QLabel("Oligo Conc"), oligoConcText);

	QLineEdit *monovalentConcText = new QLineEdit(&dialog);
	monovalentConcText->setText("5");
	form.addRow(new QLabel("Concentration of monovalent cations"), monovalentConcText);

	QLineEdit *divalentConcText = new QLineEdit(&dialog);
	divalentConcText->setText("16");
	form.addRow(new QLabel("Concentration of divalent cations"), divalentConcText);

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);
	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	// make it a SAMSON dialog
	SBGWindow* dialogWindow = SAMSON::addDialog(&dialog, "Adenita: Calculate Binding Properties", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);
	dialogWindow->show();

	if (dialog.exec() == QDialog::Accepted) {

		bool oligoConcOk = false;
		int oligoConc = oligoConcText->text().toInt(&oligoConcOk);

		bool monovalentConcOk = false;
		int monovalentConc = monovalentConcText->text().toInt(&monovalentConcOk);

		bool divalentConcOk = false;
		int divalentConc = divalentConcText->text().toInt(&divalentConcOk);

		if (oligoConcOk && monovalentConcOk && divalentConcOk) {

			SEAdenitaCoreSEApp* t = getApp();

			bool res = t->CalculateBindingRegions(oligoConc, monovalentConc, divalentConc);

			if (res) {

				SEAdenitaVisualModel* adenitaVm = static_cast<SEAdenitaVisualModel*>(SEAdenitaCoreSEApp::getVisualModel());
				if (adenitaVm != nullptr) {

					adenitaVm->changePropertyColors(1, 0);
					SAMSON::requestViewportUpdate();

				}

			}

		}
		else {

			QMessageBox messageBox;
			messageBox.critical(0, "Error", "Check whether the entered values are numeric! Check if the Structural Model is selected");
			messageBox.setFixedSize(500, 200);

		}

	}

	dialogWindow->deleteLater();

}

void SEAdenitaCoreSEAppGUI::onSetStart() {

	SEAdenitaCoreSEApp* t = getApp();
	t->SetStart();

}

void SEAdenitaCoreSEAppGUI::onTestNeighbors() {

	SEAdenitaCoreSEApp* t = getApp();
	t->TestNeighbors();

}

void SEAdenitaCoreSEAppGUI::onOxDNAImport() {

	QString topoFile;// = QFileDialog::getOpenFileName(this, tr("Select a topology file"), QDir::currentPath(), tr("(OxDNA topology *.*)"));
	if (!SAMSON::getFileNameFromUser(tr("Select a topology file"), topoFile, workingDirectory, tr("(OxDNA topology *.*)"))) return;

	workingDirectory = QFileInfo(topoFile).absolutePath();

	QString configFile;// = QFileDialog::getOpenFileName(this, tr("Select a configuration file"), QDir::currentPath(), tr("(OxDNA configuration *.*)"));
	if (!SAMSON::getFileNameFromUser(tr("Select a configuration file"), configFile, workingDirectory, tr("(OxDNA configuration *.*)"))) return;

	workingDirectory = QFileInfo(configFile).absolutePath();

	if (!topoFile.isEmpty() || !configFile.isEmpty()) {
		
		SEAdenitaCoreSEApp* t = getApp();
		t->ImportFromOxDNA(topoFile.toStdString(), configFile.toStdString());

	}

}

void SEAdenitaCoreSEAppGUI::onFromDatagraph() {

	SEAdenitaCoreSEApp* t = getApp();
	t->FromDatagraph();

}

void SEAdenitaCoreSEAppGUI::onHighlightXOs() {

	SEAdenitaCoreSEApp* t = getApp();
	t->HighlightXOs();

}

void SEAdenitaCoreSEAppGUI::onHighlightPosXOs() {

	SEAdenitaCoreSEApp* t = getApp();
	t->HighlightPosXOs();

}

void SEAdenitaCoreSEAppGUI::onExportToCanDo() {

	QString filename;// = QFileDialog::getSaveFileName(this, tr("Choose a filename"), QDir::currentPath(), tr("(CanDo .cndo)"));
	if (SBGWindowDialog::getSaveFileNameFromUser(tr("Choose a filename"), filename, QString(), tr("CanDo (*.cndo)"))) {

		QFileInfo fileInfo(filename);
		workingDirectory = fileInfo.absolutePath();	// get the absolute path to the filename
		if (fileInfo.exists()) if (!SAMSON::askUser("Export to CanDo", "The file already exists. Do you want to overwrite it?")) return;

		if (!filename.isEmpty()) {

			SEAdenitaCoreSEApp* t = getApp();
			t->ExportToCanDo(filename);

		}

	}

}

void SEAdenitaCoreSEAppGUI::onFixDesigns() {

	SEAdenitaCoreSEApp* t = getApp();
	t->FixDesigns();

}

void SEAdenitaCoreSEAppGUI::onCreateBasePair() {

	SEAdenitaCoreSEApp* t = getApp();
	t->CreateBasePair();

}

void SEAdenitaCoreSEAppGUI::onGenerateSequence() {

	QDialog dialog(this);
	QFormLayout form(&dialog);

	form.addRow(new QLabel("Generate Sequence"));

	// gc content
	QDoubleSpinBox *gcCont = new QDoubleSpinBox(&dialog);
	gcCont->setMaximum(100.0);
	gcCont->setMinimum(0.0);
	gcCont->setValue(40.0);
	form.addRow(new QLabel("GC Content"), gcCont);

	QSpinBox *contigousGs = new QSpinBox(&dialog);
	contigousGs->setMinimum(0);
	contigousGs->setMaximum(100);
	form.addRow(new QLabel("Maximum amount of contiguous Gs"), contigousGs);

	QCheckBox* chk = new QCheckBox(&dialog);
	chk->setChecked(false);
	form.addRow(new QLabel("Overwrite existing base pairs"), chk);

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);

	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	// make it a SAMSON dialog
	SBGWindow* dialogWindow = SAMSON::addDialog(&dialog, "Adenita: Generate Sequence", SBGWindow::NoOptions);
	dialogWindow->QWidget::setWindowFlags(dialogWindow->QWidget::windowFlags() | Qt::WindowStaysOnTopHint);
	dialogWindow->setWindowModality(Qt::ApplicationModal);
	dialogWindow->show();

	if (dialog.exec() == QDialog::Accepted) {

		double gc100 = gcCont->value() / 100;
		int maxContGs = contigousGs->value();
		bool overwrite = chk->isChecked();
		SEAdenitaCoreSEApp* t = getApp();
		t->GenerateSequence(gc100, maxContGs, overwrite);

	}

	dialogWindow->deleteLater();

}

void SEAdenitaCoreSEAppGUI::onResetVisualModel() { getApp()->resetVisualModel(); }

void SEAdenitaCoreSEAppGUI::onSettings() {

	SEAdenitaCoreSettingsGUI diag(this);
	diag.showDialog();
	//diag.exec();

}

void SEAdenitaCoreSEAppGUI::onBreakEditor() {

	SEBreakEditor* be = static_cast<SEBreakEditor*>(SAMSON::getEditor(SBCContainerUUID("CFACD1E5-FCD1-916F-2CF7-4B60979F1A77"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(be);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onConnectEditor() {

	SEConnectSSDNAEditor* c = static_cast<SEConnectSSDNAEditor*>(SAMSON::getEditor(SBCContainerUUID("48FDCE78-A55E-FDA2-237E-319202E56080"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onDeleteEditor() {

	SEDeleteEditor* c = static_cast<SEDeleteEditor*>(SAMSON::getEditor(SBCContainerUUID("592B8158-15E9-B621-0BCB-D7DA210FF149"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onDNATwistEditor() {

	SETwistHelixEditor* c = static_cast<SETwistHelixEditor*>(SAMSON::getEditor(SBCContainerUUID("4B60FECA-2A79-680F-F289-B4908A924409"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onMergePartsEditor() {

	SEMergePartsEditor* c = static_cast<SEMergePartsEditor*>(SAMSON::getEditor(SBCContainerUUID("EB812444-8EA8-BD83-988D-AFF5987461D8"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onCreateStrandEditor() {

	SEDSDNACreatorEditor* c = static_cast<SEDSDNACreatorEditor*>(SAMSON::getEditor(SBCContainerUUID("86204A08-DFD6-97A8-2BE2-4CFC8B4169A3"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onNanotubeCreatorEditor() {

	SENanotubeCreatorEditor* c = static_cast<SENanotubeCreatorEditor*>(SAMSON::getEditor(SBCContainerUUID("4B6A0B18-48B5-233A-28A4-BA3EF3D56AB8"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onLatticeCreatorEditor() {

	SELatticeCreatorEditor* c = static_cast<SELatticeCreatorEditor*>(SAMSON::getEditor(SBCContainerUUID("EA67625E-89B5-2EEA-156D-FC836214B0E4"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onWireframeEditor() {

	SEWireframeEditor* c = static_cast<SEWireframeEditor*>(SAMSON::getEditor(SBCContainerUUID("F1F29042-3D87-DA61-BC5C-D3348EB2E1FA"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onTaggingEditor() {

	SETaggingEditor* c = static_cast<SETaggingEditor*>(SAMSON::getEditor(SBCContainerUUID("473D2F88-5D06-25F5-EB58-053661504C43"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

void SEAdenitaCoreSEAppGUI::onTwisterEditor() {

	SEDNATwisterEditor* c = static_cast<SEDNATwisterEditor*>(SAMSON::getEditor(SBCContainerUUID("677B1667-7856-12E6-5901-E8EAC729501A"), SBUUID(SB_ELEMENT_UUID)));
	SAMSON::setActiveEditor(c);
	setHighlightEditor(qobject_cast<QToolButton*>(QObject::sender()));

}

std::string SEAdenitaCoreSEAppGUI::isCadnanoJsonFormat(QString filename) {

	FILE* fp = fopen(filename.toStdString().c_str(), "rb");
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document d;
	d.ParseStream(is);

	if (d.HasMember("vstrands"))
		return "cadnano";
	else if (d.HasMember("doubleStrands"))
		return "adenita";

	return "unknown";

}

void SEAdenitaCoreSEAppGUI::setHighlightEditor(QToolButton* b) {

	// remove current
	if (highlightedEditor_ != nullptr) highlightedEditor_->setStyleSheet(QString("border: none"));
	b->setStyleSheet(QString("border: 2px solid #FFFFFF"));
	highlightedEditor_ = b;

}

void SEAdenitaCoreSEAppGUI::checkForLoadedParts() {

	SEAdenitaCoreSEApp* adenita = getApp();
	SBNodeIndexer nodeIndexer;
	SAMSON::getActiveDocument()->getNodes(nodeIndexer, (SBNode::GetClass() == std::string("ADNPart")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));

	SB_FOR(SBNode* n, nodeIndexer) {

		ADNPointer<ADNPart> part = static_cast<ADNPart*>(n);
		adenita->AddLoadedPartToNanorobot(part);

	}

}

void SEAdenitaCoreSEAppGUI::keyPressEvent(QKeyEvent* event) {

	if (event->modifiers() == Qt::ControlModifier) {
	
		SBProxy* ep = nullptr;

		//if (event->key() == Qt::Key_D) ep = SAMSON::getProxy("SEDeleteEditor", SBUUID(SB_ELEMENT_UUID));
		//if (event->key() == Qt::Key_B) ep = SAMSON::getProxy("SEBreakEditor", SBUUID(SB_ELEMENT_UUID));
		//if (event->key() == Qt::Key_G) ep = SAMSON::getProxy("SEConnectSSDNAEditor", SBUUID(SB_ELEMENT_UUID));

		if (ep) {

			SBEditor * e = SAMSON::getEditor(ep->getUUID(), ep->getElementUUID());
			SAMSON::setActiveEditor(e);

		}

	}

}


SBCContainerUUID SEAdenitaCoreSEAppGUI::getUUID() const { return SBCContainerUUID( "386506A7-DD8B-69DD-4599-F136C1B91610" );}

QPixmap SEAdenitaCoreSEAppGUI::getLogo() const { 
	
	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your app.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/SEAdenitaCoreSEAppIcon.png"));

}

QString SEAdenitaCoreSEAppGUI::getName() const { 

	// SAMSON Element generator pro tip: this string will be the GUI title. 
	// Modify this function to have a user-friendly description of your app inside SAMSON

	return "Adenita (alpha)"; 

}

int SEAdenitaCoreSEAppGUI::getFormat() const { 
	
	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)
	
	return (SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QString SEAdenitaCoreSEAppGUI::getCitation() const {

	// SAMSON Element generator pro tip: modify this function to add citation information

	return ADNAuxiliary::AdenitaCitation();

}

void SEAdenitaCoreSEAppGUI::setupUI() {

	ui.setupUi(this);

	QGroupBox* groupBoxMenu = new QGroupBox("General");
	QGroupBox* groupBoxEditSequences = new QGroupBox("Edit sequences");
	QGroupBox* groupBoxModeling = new QGroupBox("Modeling");
	QGroupBox* groupBoxCreators = new QGroupBox("Creators");

	ui.verticalLayout->addWidget(groupBoxMenu);
	ui.verticalLayout->addWidget(groupBoxEditSequences);
	ui.verticalLayout->addWidget(groupBoxModeling);
	ui.verticalLayout->addWidget(groupBoxCreators);

	QHBoxLayout* layoutMenu = new QHBoxLayout;
	QHBoxLayout* layoutEditSequences = new QHBoxLayout;
	QHBoxLayout* layoutModeling = new QHBoxLayout;
	QHBoxLayout* layoutCreators = new QHBoxLayout;

	groupBoxMenu->setLayout(layoutMenu);
	groupBoxEditSequences->setLayout(layoutEditSequences);
	groupBoxModeling->setLayout(layoutModeling);
	groupBoxCreators->setLayout(layoutCreators);

	const std::vector<QToolButton*> menuButtons = getMenuButtons();
	const std::vector<QToolButton*> editSequencesButtons = getEditSequencesButtons();
	const std::vector<QToolButton*> modelingButtons = getModelingButtons();
	const std::vector<QToolButton*> creatorsButtons = getCreatorsButtons();
	
	for (auto b : menuButtons) layoutMenu->addWidget(b);
	for (auto b : editSequencesButtons) layoutEditSequences->addWidget(b);
	for (auto b : modelingButtons) layoutModeling->addWidget(b);
	for (auto b : creatorsButtons) layoutCreators->addWidget(b);

	layoutMenu->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed));
	layoutEditSequences->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed));
	layoutModeling->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed));
	layoutCreators->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed));

#ifdef ADENITA_DEBUG
	QGroupBox* groupBoxDebug = new QGroupBox("Debug");
	groupBoxDebug->setMinimumHeight(150);
	ui.verticalLayout->addWidget(groupBoxDebug);
	FlowLayout* layoutDebug = new FlowLayout;
	groupBoxDebug->setLayout(layoutDebug);
	const std::vector<QPushButton*> debugButtons = getDebugButtons();
	for (auto b : debugButtons) layoutDebug->addWidget(b);
#endif

}

std::vector<QToolButton*> SEAdenitaCoreSEAppGUI::getMenuButtons() {

	if (menuButtons_.size() == 0) {

		auto btnLoad = new QToolButton(this);
		btnLoad->setObjectName(QStringLiteral("btnLoad"));
		btnLoad->setText("Load\n");
		btnLoad->setToolTip("Load structure");
		btnLoad->setIconSize(QSize(24, 24));
		btnLoad->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnLoad->setAutoRaise(true);
		menuButtons_.push_back(btnLoad);

		auto btnSaveSelection = new QToolButton(this);
		btnSaveSelection->setObjectName(QStringLiteral("btnSaveSelection"));
		btnSaveSelection->setText("Save\nselection");
		btnSaveSelection->setToolTip("Save selection");
		btnSaveSelection->setIconSize(QSize(24, 24));
		btnSaveSelection->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnSaveSelection->setAutoRaise(true);
		menuButtons_.push_back(btnSaveSelection);

		auto btnSaveAll = new QToolButton(this);
		btnSaveAll->setObjectName(QStringLiteral("btnSaveAll"));
		btnSaveAll->setText("Save all\n");
		btnSaveAll->setToolTip("Save all");
		btnSaveAll->setIconSize(QSize(24, 24));
		btnSaveAll->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnSaveAll->setAutoRaise(true);
		menuButtons_.push_back(btnSaveAll);

		auto btnExport = new QToolButton(this);
		btnExport->setObjectName(QStringLiteral("btnExport"));
		btnExport->setText("Export\n");
		btnExport->setToolTip("Export");
		btnExport->setIconSize(QSize(24, 24));
		btnExport->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnExport->setAutoRaise(true);
		menuButtons_.push_back(btnExport);

		auto btnResetVisualModel = new QToolButton(this);
		btnResetVisualModel->setObjectName(QStringLiteral("btnResetVisualModel"));
		btnResetVisualModel->setText("Reset\nvisual model");
		btnResetVisualModel->setToolTip("Reset visual model");
		btnResetVisualModel->setIconSize(QSize(24, 24));
		btnResetVisualModel->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnResetVisualModel->setAutoRaise(true);
		menuButtons_.push_back(btnResetVisualModel);

		auto btnSettings = new QToolButton(this);
		btnSettings->setObjectName(QStringLiteral("btnSettings"));
		btnSettings->setText("Settings\n");
		btnSettings->setToolTip("Settings");
		btnSettings->setIconSize(QSize(24, 24));
		btnSettings->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnSettings->setAutoRaise(true);
		menuButtons_.push_back(btnSettings);

		QObject::connect(btnLoad, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onLoadFile, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnSaveSelection, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onSaveSelection, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnSaveAll, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onSaveAll, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnSettings, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onSettings, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnExport, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onExport, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnResetVisualModel, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onResetVisualModel, Qt::ConnectionType::UniqueConnection);

		//change icons
		std::string iconsPath = SB_ELEMENT_PATH + "/Resource/icons/";

		QIcon loadIcon;
		loadIcon.addFile(std::string(iconsPath + "load.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnLoad->setIcon(loadIcon);

		QIcon saveAllIcon;
		saveAllIcon.addFile(std::string(iconsPath + "saveAll.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnSaveAll->setIcon(saveAllIcon);

		QIcon saveSelectionIcon;
		saveSelectionIcon.addFile(std::string(iconsPath + "saveSelection.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnSaveSelection->setIcon(saveSelectionIcon);

		QIcon exportIcon;
		exportIcon.addFile(std::string(iconsPath + "export.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnExport->setIcon(exportIcon);

		QIcon resetVM;
		resetVM.addFile(std::string(iconsPath + "resetVisualModel.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnResetVisualModel->setIcon(resetVM);

		QIcon settings;
		settings.addFile(std::string(iconsPath + "settings.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnSettings->setIcon(settings);

	}

	return menuButtons_;

}

std::vector<QToolButton*> SEAdenitaCoreSEAppGUI::getEditSequencesButtons() {

	if (editSequencesButtons_.size() == 0) {

		auto btnSetScaff = new QToolButton(this);
		btnSetScaff->setObjectName(QStringLiteral("btnSetScaff"));
		btnSetScaff->setText("Set\nscaffold\n");
		btnSetScaff->setToolTip("Set scaffold");
		btnSetScaff->setIconSize(QSize(24, 24));
		btnSetScaff->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnSetScaff->setAutoRaise(true);
		editSequencesButtons_.push_back(btnSetScaff);

		auto btnGenerateSequence = new QToolButton(this);
		btnGenerateSequence->setObjectName(QStringLiteral("btnGenerateSequence"));
		btnGenerateSequence->setText("Generate\nsequence\n");
		btnGenerateSequence->setToolTip("Generate sequence");
		btnGenerateSequence->setIconSize(QSize(24, 24));
		btnGenerateSequence->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnGenerateSequence->setAutoRaise(true);
		editSequencesButtons_.push_back(btnGenerateSequence);

		auto btnSetStart = new QToolButton(this);
		btnSetStart->setObjectName(QStringLiteral("btnSetStart"));
		btnSetStart->setText("Set\nstart (5\')\n");
		btnSetStart->setToolTip("Set start (5\')");
		btnSetStart->setIconSize(QSize(24, 24));
		btnSetStart->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnSetStart->setAutoRaise(true);
		editSequencesButtons_.push_back(btnSetStart);

		auto btnCalculateBindingProperties = new QToolButton(this);
		btnCalculateBindingProperties->setObjectName(QStringLiteral("btnCalculateBindingProperties"));
		btnCalculateBindingProperties->setText("Calculate\nbinding\nproperties");
		btnCalculateBindingProperties->setToolTip("Calculate binding properties");
		btnCalculateBindingProperties->setIconSize(QSize(24, 24));
		btnCalculateBindingProperties->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnCalculateBindingProperties->setAutoRaise(true);
		editSequencesButtons_.push_back(btnCalculateBindingProperties);

		auto btnTaggingEditor = new QToolButton(this);
		btnTaggingEditor->setObjectName(QStringLiteral("btnTaggingEditor"));
		btnTaggingEditor->setText("Tagging\neditor\n");
		btnTaggingEditor->setToolTip("Tagging editor");
		btnTaggingEditor->setIconSize(QSize(24, 24));
		btnTaggingEditor->setCheckable(true);
		btnTaggingEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnTaggingEditor->setAutoRaise(true);
		editSequencesButtons_.push_back(btnTaggingEditor);

		QObject::connect(btnSetScaff, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onSetScaffold, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnGenerateSequence, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onGenerateSequence, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnCalculateBindingProperties, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onCalculateBindingProperties, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnSetStart, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onSetStart, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnTaggingEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onTaggingEditor, Qt::ConnectionType::UniqueConnection);

		//change icons
		std::string iconsPath = SB_ELEMENT_PATH + "/Resource/icons/";

		QIcon setScaffIcon;
		setScaffIcon.addFile(std::string(iconsPath + "setScaffold.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnSetScaff->setIcon(setScaffIcon);

		QIcon calcProperties;
		calcProperties.addFile(std::string(iconsPath + "calculate.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnCalculateBindingProperties->setIcon(calcProperties);

		QIcon setStart;
		setStart.addFile(std::string(iconsPath + "set5Prime.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnSetStart->setIcon(setStart);

		QIcon generateSequences;
		generateSequences.addFile(std::string(iconsPath + "generateSeqs.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnGenerateSequence->setIcon(generateSequences);

		QIcon tagEditor;
		tagEditor.addFile(std::string(iconsPath + "SETaggingEditorIcon.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnTaggingEditor->setIcon(tagEditor);

	}
  
	return editSequencesButtons_;

}

std::vector<QToolButton*> SEAdenitaCoreSEAppGUI::getModelingButtons() {

	if (modelingButtons_.size() == 0) {

		auto btnBreakEditor = new QToolButton(this);
		btnBreakEditor->setObjectName(QStringLiteral("btnBreakEditor"));
		btnBreakEditor->setText("Break");
		btnBreakEditor->setToolTip("Break editor");
		btnBreakEditor->setIconSize(QSize(24, 24));
		btnBreakEditor->setCheckable(true);
		btnBreakEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnBreakEditor->setAutoRaise(true);
		modelingButtons_.push_back(btnBreakEditor);

		auto btnDeleteEditor = new QToolButton(this);
		btnDeleteEditor->setObjectName(QStringLiteral("btnDeleteEditor"));
		btnDeleteEditor->setText("Delete");
		btnDeleteEditor->setToolTip("Delete editor");
		btnDeleteEditor->setIconSize(QSize(24, 24));
		btnDeleteEditor->setCheckable(true);
		btnDeleteEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnDeleteEditor->setAutoRaise(true);
		modelingButtons_.push_back(btnDeleteEditor);

		auto btnConnectEditor = new QToolButton(this);
		btnConnectEditor->setObjectName(QStringLiteral("btnConnectEditor"));
		btnConnectEditor->setText("Connect");
		btnConnectEditor->setToolTip("Connect editor");
		btnConnectEditor->setIconSize(QSize(24, 24));
		btnConnectEditor->setCheckable(true);
		btnConnectEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnConnectEditor->setAutoRaise(true);
		modelingButtons_.push_back(btnConnectEditor);

		auto btnMergePartsEditor = new QToolButton(this);
		btnMergePartsEditor->setObjectName(QStringLiteral("btnMergePartsEditor"));
		btnMergePartsEditor->setText("Merge parts");
		btnMergePartsEditor->setToolTip("Merge parts editor");
		btnMergePartsEditor->setIconSize(QSize(24, 24));
		btnMergePartsEditor->setCheckable(true);
		btnMergePartsEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnMergePartsEditor->setAutoRaise(true);
		modelingButtons_.push_back(btnMergePartsEditor);

		auto btnDNATwisterEditor = new QToolButton(this);
		btnDNATwisterEditor->setObjectName(QStringLiteral("btnDNATwisterEditor"));
		btnDNATwisterEditor->setText("DNA twister");
		btnDNATwisterEditor->setToolTip("DNA twister editor");
		btnDNATwisterEditor->setIconSize(QSize(24, 24));
		btnDNATwisterEditor->setCheckable(true);
		btnDNATwisterEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnDNATwisterEditor->setAutoRaise(true);
		modelingButtons_.push_back(btnDNATwisterEditor);

		auto btnTwisterEditor = new QToolButton(this);
		btnTwisterEditor->setObjectName(QStringLiteral("btnTwisterEditor"));
		btnTwisterEditor->setText("Twister");
		btnTwisterEditor->setToolTip("Twister editor");
		btnTwisterEditor->setIconSize(QSize(24, 24));
		btnTwisterEditor->setCheckable(true);
		btnTwisterEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnTwisterEditor->setAutoRaise(true);
		modelingButtons_.push_back(btnTwisterEditor);

		QObject::connect(btnBreakEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onBreakEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnConnectEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onConnectEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnDeleteEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onDeleteEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnTwisterEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onTwisterEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnDNATwisterEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onDNATwistEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnMergePartsEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onMergePartsEditor, Qt::ConnectionType::UniqueConnection);

		//change icons
		std::string iconsPath = SB_ELEMENT_PATH + "/Resource/icons/";

		QIcon breakEditor;
		breakEditor.addFile(std::string(iconsPath + "break.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnBreakEditor->setIcon(breakEditor);

		QIcon deleteEditor;
		deleteEditor.addFile(std::string(iconsPath + "delete.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnDeleteEditor->setIcon(deleteEditor);

		QIcon connectEditor;
		connectEditor.addFile(std::string(iconsPath + "connectDS.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnConnectEditor->setIcon(connectEditor);

		QIcon twistDsDNA;
		twistDsDNA.addFile(std::string(iconsPath + "twistDS.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnDNATwisterEditor->setIcon(twistDsDNA);

		QIcon mergePartsEditor;
		mergePartsEditor.addFile(std::string(iconsPath + "merge.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnMergePartsEditor->setIcon(mergePartsEditor);

		QIcon dnaTwister;
		dnaTwister.addFile(std::string(iconsPath + "SEDNATwisterEditorIcon.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnTwisterEditor->setIcon(dnaTwister);

	}

	return modelingButtons_;
}

std::vector<QToolButton*> SEAdenitaCoreSEAppGUI::getCreatorsButtons() {

	if (creatorsButtons_.size() == 0) {

		QToolButton* btnCreateBasePair = new QToolButton;
		btnCreateBasePair->setObjectName(QStringLiteral("btnCreateBasePair"));
		btnCreateBasePair->setText("Create\nbase pair");
		btnCreateBasePair->setToolTip("Create base pair");
		btnCreateBasePair->setIconSize(QSize(24, 24));
		btnCreateBasePair->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnCreateBasePair->setAutoRaise(true);
		creatorsButtons_.push_back(btnCreateBasePair);

		QToolButton* btnDsDNACreatorEditor = new QToolButton;
		btnDsDNACreatorEditor->setObjectName(QStringLiteral("btnDsDNACreatorEditor"));
		btnDsDNACreatorEditor->setText("DsDNA\ncreator");
		btnDsDNACreatorEditor->setToolTip("DsDNA creator editor");
		btnDsDNACreatorEditor->setIconSize(QSize(24, 24));
		btnDsDNACreatorEditor->setCheckable(true);
		btnDsDNACreatorEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnDsDNACreatorEditor->setAutoRaise(true);
		creatorsButtons_.push_back(btnDsDNACreatorEditor);

		QToolButton* btnNanotubeCreator = new QToolButton;
		btnNanotubeCreator->setObjectName(QStringLiteral("btnNanotubeCreator"));
		btnNanotubeCreator->setText("Nanotube\ncreator");
		btnNanotubeCreator->setToolTip("Nanotube creator");
		btnNanotubeCreator->setIconSize(QSize(24, 24));
		btnNanotubeCreator->setCheckable(true);
		btnNanotubeCreator->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnNanotubeCreator->setAutoRaise(true);
		creatorsButtons_.push_back(btnNanotubeCreator);

		QToolButton* btnLatticeCreatorEditor = new QToolButton;
		btnLatticeCreatorEditor->setObjectName(QStringLiteral("btnLatticeCreatorEditor"));
		btnLatticeCreatorEditor->setText("Lattice\ncreator");
		btnLatticeCreatorEditor->setToolTip("Lattice creator editor");
		btnLatticeCreatorEditor->setIconSize(QSize(24, 24));
		btnLatticeCreatorEditor->setCheckable(true);
		btnLatticeCreatorEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnLatticeCreatorEditor->setAutoRaise(true);
		creatorsButtons_.push_back(btnLatticeCreatorEditor);

		QToolButton* btnWireframeEditor = new QToolButton;
		btnWireframeEditor->setObjectName(QStringLiteral("btnWireframeEditor"));
		btnWireframeEditor->setText("Wireframe\n");
		btnWireframeEditor->setToolTip("Wireframe editor");
		btnWireframeEditor->setIconSize(QSize(24, 24));
		btnWireframeEditor->setCheckable(true);
		btnWireframeEditor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		btnWireframeEditor->setAutoRaise(true);
		creatorsButtons_.push_back(btnWireframeEditor);

		QObject::connect(btnCreateBasePair, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onCreateBasePair, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnDsDNACreatorEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onCreateStrandEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnNanotubeCreator, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onNanotubeCreatorEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnLatticeCreatorEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onLatticeCreatorEditor, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnWireframeEditor, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onWireframeEditor, Qt::ConnectionType::UniqueConnection);

		//change icons
		std::string iconsPath = SB_ELEMENT_PATH + "/Resource/icons/";

		QIcon createBp;
		createBp.addFile(std::string(iconsPath + "createBasePair.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnCreateBasePair->setIcon(createBp);

		QIcon strandCreator;
		strandCreator.addFile(std::string(iconsPath + "SEDSDNACreatorEditorIcon.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnDsDNACreatorEditor->setIcon(strandCreator);

		QIcon nanotubeEditor;
		nanotubeEditor.addFile(std::string(iconsPath + "SENanotubeCreatorEditorIcon.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnNanotubeCreator->setIcon(nanotubeEditor);

		QIcon latticeEditor;
		latticeEditor.addFile(std::string(iconsPath + "SELatticeCreatorEditorIcon.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnLatticeCreatorEditor->setIcon(latticeEditor);

		QIcon wireframeEditor;
		wireframeEditor.addFile(std::string(iconsPath + "SEWireframeEditorIcon.png").c_str(), QSize(), QIcon::Normal, QIcon::Off);
		btnWireframeEditor->setIcon(wireframeEditor);

	}

	return creatorsButtons_;

}

std::vector<QPushButton*> SEAdenitaCoreSEAppGUI::getDebugButtons() {

	if (debugButtons_.size() == 0) {

		//change icons
		std::string iconsPath = SB_ELEMENT_PATH + "/Resource/icons/";

		auto btnDataGraph = new QPushButton(this);
		btnDataGraph->setObjectName(QStringLiteral("btnDataGraph"));
		btnDataGraph->setText("From Datagraph");
		debugButtons_.push_back(btnDataGraph);

		auto btnTestNeighbors = new QPushButton(this);
		btnTestNeighbors->setObjectName(QStringLiteral("btnTestNeighbors"));
		btnTestNeighbors->setText("Test Neighbors");
		debugButtons_.push_back(btnTestNeighbors);

		auto btnOxDNAImport = new QPushButton(this);
		btnOxDNAImport->setObjectName(QStringLiteral("btnOxDNAImport"));
		btnOxDNAImport->setText("oxDNA Import");
		debugButtons_.push_back(btnOxDNAImport);

		auto btnCenterPart = new QPushButton(this);
		btnCenterPart->setObjectName(QStringLiteral("btnCenterPart"));
		btnCenterPart->setText("Center Part");
		debugButtons_.push_back(btnCenterPart);

		auto btnCatenanes = new QPushButton(this);
		btnCatenanes->setObjectName(QStringLiteral("btnCatenanes"));
		btnCatenanes->setText("Catenanes");
		debugButtons_.push_back(btnCatenanes);

		auto btnKinetoplast = new QPushButton(this);
		btnKinetoplast->setObjectName(QStringLiteral("btnKinetoplast"));
		btnKinetoplast->setText("Kinetoplast");
		debugButtons_.push_back(btnKinetoplast);

		auto btnHighlightCrossovers = new QPushButton(this);
		btnHighlightCrossovers->setObjectName(QStringLiteral("btnHighlightCrossovers"));
		btnHighlightCrossovers->setText("Highlight Crossovers");
		debugButtons_.push_back(btnHighlightCrossovers);

		auto btnHighlightPosXOs = new QPushButton(this);
		btnHighlightPosXOs->setObjectName(QStringLiteral("btnHighlightPosXOs"));
		btnHighlightPosXOs->setText("Possible Crossovers");
		debugButtons_.push_back(btnHighlightPosXOs);

		auto btnAddNtThreeP = new QPushButton(this);
		btnAddNtThreeP->setObjectName(QStringLiteral("btnAddNtThreeP"));
		btnAddNtThreeP->setText("Add nt 3'");
		debugButtons_.push_back(btnAddNtThreeP);

		auto btnExportCanDo = new QPushButton(this);
		btnExportCanDo->setObjectName(QStringLiteral("btnExportCanDo"));
		btnExportCanDo->setText("Export to CanDo");
		debugButtons_.push_back(btnExportCanDo);

		auto btnFix = new QPushButton(this);
		btnFix->setObjectName(QStringLiteral("btnFix"));
		btnFix->setText("Fix");
		debugButtons_.push_back(btnFix);

		QObject::connect(btnCenterPart, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onCenterPart, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnOxDNAImport, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onOxDNAImport, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnKinetoplast, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onKinetoplast, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnTestNeighbors, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onTestNeighbors, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnExportCanDo, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onExportToCanDo, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnHighlightPosXOs, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onHighlightPosXOs, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnHighlightCrossovers, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onHighlightXOs, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnCatenanes, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onCatenanes, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnAddNtThreeP, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onAddNtThreeP, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnFix, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onFixDesigns, Qt::ConnectionType::UniqueConnection);
		QObject::connect(btnDataGraph, &QPushButton::released, this, &SEAdenitaCoreSEAppGUI::onFromDatagraph, Qt::ConnectionType::UniqueConnection);

	}

	return debugButtons_;

}

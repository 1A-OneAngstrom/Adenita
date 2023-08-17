#include "SETaggingEditor.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "MSVDisplayHelper.hpp"

#include "SAMSON.hpp"

#include <QInputDialog>


SETaggingEditor::SETaggingEditor() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.

	propertyWidget = new SETaggingEditorGUI(this);
	propertyWidget->loadDefaultSettings();

}

SETaggingEditor::~SETaggingEditor() {

	// SAMSON Element generator pro tip: disconnect from signals you might have connected to.

	propertyWidget->saveDefaultSettings();
	delete propertyWidget;

}

SETaggingEditorGUI* SETaggingEditor::getPropertyWidget() const { return static_cast<SETaggingEditorGUI*>(propertyWidget); }

SBCContainerUUID SETaggingEditor::getUUID() const { return SBCContainerUUID("2A1B195E-3E38-BD20-1B61-70A3CA6761C8"); }

QString SETaggingEditor::getName() const { 

	// SAMSON Element generator pro tip: this name should not be changed

	return "SETaggingEditor"; 

}

QPixmap SETaggingEditor::getLogo() const {

	// SAMSON Element generator pro tip: this icon will be visible in the GUI title bar. 
	// Modify it to better reflect the purpose of your editor.

	return QPixmap(QString::fromStdString(SB_ELEMENT_PATH + "/Resource/Icons/SETaggingEditorIcon.png"));

}

int SETaggingEditor::getFormat() const
{

	// SAMSON Element generator pro tip: modify these default settings to configure the window
	//
	// SBGWindow::Savable : let users save and load interface settings (implement loadSettings and saveSettings)
	// SBGWindow::Lockable : let users lock the window on top
	// SBGWindow::Resizable : let users resize the window
	// SBGWindow::Citable : let users obtain citation information (implement getCitation)

	return (SBGWindow::Savable | SBGWindow::Lockable | SBGWindow::Resizable | SBGWindow::Citable);

}

QKeySequence SETaggingEditor::getShortcut() const { 
	
	// SAMSON Element generator pro tip: modify this function to associate a tentative shortcut to your editor

	return QKeySequence(""); 

}

QString SETaggingEditor::getToolTip() const { 
	
	// SAMSON Element generator pro tip: modify this function to have your editor display a tool tip in the SAMSON GUI when the mouse hovers the editor's icon

	return QObject::tr("Tag nucleotides or change their base. The tag will appear when exporting sequences."); 

}

void SETaggingEditor::loadSettings(SBGSettings * settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

void SETaggingEditor::saveSettings(SBGSettings* settings) {

	if (settings == nullptr) return;

	// SAMSON Element generator pro tip: complete this function so your importer can save its GUI state from one session to the next

}

QString SETaggingEditor::getDescription() const {

	return QObject::tr("Adenita | Tag Nucleotides");

}

void SETaggingEditor::beginEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function when your editor becomes active. 
	// Implement this function if you need to prepare some data structures in order to be able to handle GUI or SAMSON events.

	const QString iconPath = QString::fromStdString(SB_ELEMENT_PATH + "/Resource/icons/cursor_tagging.png");
	SAMSON::setViewportCursor(QCursor(QPixmap(iconPath)));

	previousSelectionFilter = SAMSON::getActiveSelectionFilterName();
	SAMSON::setActiveSelectionFilterByName("Residues");

}

void SETaggingEditor::endEditing() {

	// SAMSON Element generator pro tip: SAMSON calls this function immediately before your editor becomes inactive (for example when another editor becomes active). 
	// Implement this function if you need to clean some data structures.

	SEAdenitaCoreSEApp::getAdenitaApp()->getGUI()->clearHighlightEditor();

	if (SAMSON::getActiveSelectionFilterName() == "Residues")
		SAMSON::setActiveSelectionFilterByName(previousSelectionFilter);

	SAMSON::unsetViewportCursor();

}

void SETaggingEditor::getContextMenuActions(SBVector<SBAction*>& actionVector) {

	// SAMSON Element generator pro tip: SAMSON calls this function to show the user actions associated to your editor in context menus.
	// Append actions to the actionVector if necessary.
	// Please refer to tutorials for examples.

}

void SETaggingEditor::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	if (taggingMode == TaggingMode::Base) {

		const std::string base(1, ADNModel::GetResidueName(nucleotideType));
		const SBPosition3 position = SAMSON::getWorldPositionFromViewportPosition(SAMSON::getMousePositionInViewport());
		ADNDisplayHelper::displayText(position, base);

	}

}


void SETaggingEditor::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function if your editor displays things in viewports, so that your editor can cast shadows
	// to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

}

void SETaggingEditor::mousePressEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETaggingEditor::mouseReleaseEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	auto nucleotide = GetHighlightedNucleotide();

	if (nucleotide != nullptr) {

		if (taggingMode == TaggingMode::Tags) {

			//bool ok;
			//const QString tagText = QInputDialog::getText(this, tr("Enter Nucleotide Tag"), tr("Tag:"), QLineEdit::Normal, QString(), &ok);
			QString tagText;
			if (SAMSON::getStringFromUser("Enter Nucleotide Tag", tagText))
			if (!tagText.isEmpty()) {

				nucleotide->setTag(tagText.toStdString());
				nucleotide->setSelectionFlag(true);

			}

		}
		else if (taggingMode == TaggingMode::Base) {

			ADNBasicOperations::MutateNucleotide(nucleotide, nucleotideType);

		}

		event->accept();

		SEAdenitaCoreSEApp::resetVisualModel();

	}

}

void SETaggingEditor::mouseMoveEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.


}

void SETaggingEditor::mouseDoubleClickEvent(QMouseEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETaggingEditor::wheelEvent(QWheelEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

	if (event->modifiers() == Qt::ControlModifier) {

		QPoint numDegrees = event->angleDelta() / 8;
		if (!numDegrees.isNull()) {

			QPoint numSteps = numDegrees / 15;
			nucleotideType = getNucleotideType(numSteps);
			event->accept();

		}

	}

}

void SETaggingEditor::keyPressEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

void SETaggingEditor::keyReleaseEvent(QKeyEvent* event) {

	// SAMSON Element generator pro tip: SAMSON redirects Qt events to the active editor. 
	// Implement this function to handle this event with your editor.

}

ADNPointer<ADNNucleotide> SETaggingEditor::GetHighlightedNucleotide() {

	ADNPointer<ADNNucleotide> nucleotide = nullptr;
	auto highlightedNucleotides = SEAdenitaCoreSEApp::getAdenitaApp()->GetNanorobot()->GetHighlightedNucleotides();

	if (highlightedNucleotides.size() == 1)
		nucleotide = highlightedNucleotides[0];

	return nucleotide;

}

void SETaggingEditor::setTaggingMode(TaggingMode mode) {
	
	this->taggingMode = mode;

}

DNABlocks SETaggingEditor::getNucleotideType(QPoint numSteps) {

	DNABlocks t = nucleotideType;

	std::map<int, DNABlocks> values = {
		{0, DNABlocks::DA},
		{1, DNABlocks::DT},
		{2, DNABlocks::DC},
		{3, DNABlocks::DG},
		{4, DNABlocks::DI}
	};

	std::map<DNABlocks, int> indices = {
		{DNABlocks::DA, 0},
		{DNABlocks::DT, 1},
		{DNABlocks::DC, 2},
		{DNABlocks::DG, 3},
		{DNABlocks::DI, 4}
	};

	const int currIndex = indices[t];
	int newIndex = currIndex + numSteps.y();
	// keep new index between 0 and 4
	const int numValues = static_cast<int>(values.size());
	if (newIndex < 0) {

		int turns = abs(newIndex);
		std::div_t divresult;
		divresult = std::div(turns, numValues);
		newIndex = numValues - divresult.rem + 1;

	}
	else if (newIndex > 4) {

		int turns = newIndex + 1; // take into account 0
		std::div_t divresult;
		divresult = std::div(turns, numValues);
		newIndex = divresult.rem;

	}

	t = values[newIndex];

	return t;

}


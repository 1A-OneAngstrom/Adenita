#include "SEAdenitaVisualModel.hpp"
#include "SEAdenitaCoreSEApp.hpp"
#include "SAMSON.hpp"
#include "ADNLogger.hpp"
#include "ADNAuxiliary.hpp"
#include "ADNConfig.hpp"
#include "ADNNanorobot.hpp"

#include "PICrossovers.hpp"
#include "PIPrimer3.hpp"

#include "MSVColors.hpp"
#include "MSVDisplayHelper.hpp"

#include <QOpenGLFunctions_4_3_Core>

#include <cmath>

SEAdenitaVisualModel::SEAdenitaVisualModel() {

	// SAMSON Element generator pro tip: this default constructor is called when unserializing the node, so it should perform all default initializations.
	
	init();
	setName("Adenita Visual Model");

}

SEAdenitaVisualModel::SEAdenitaVisualModel(const SBNodeIndexer& nodeIndexer) {

	// SAMSON Element generator pro tip: implement this function if you want your visual model to be applied to a set of data graph nodes.
	// You might want to connect to various signals and handle the corresponding events. For example, if your visual model represents a sphere positioned at
	// the center of mass of a group of atoms, you might want to connect to the atoms' base signals (e.g. to update the center of mass when an atom is erased) and
	// the atoms' structural signals (e.g. to update the center of mass when an atom is moved).

	SEAdenitaCoreSEApp::getAdenitaApp()->FromDatagraph();

	init();
	setName("Adenita Visual Model");

}

SEAdenitaVisualModel::~SEAdenitaVisualModel() {

	ADNLogger::Log(std::string("Adenita Visual Model got destroyed"));

}

 bool SEAdenitaVisualModel::isSerializable() const {

	// SAMSON Element generator pro tip: serialization is used in SAMSON to e.g. save documents, copy nodes, etc. 
	// Please refer to the SDK documentation for more information.
	// Modify the line below to "return true;" if you want this visual model be serializable (hence copyable, savable, etc.)

	return false;
	
}

void SEAdenitaVisualModel::serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) const {

	SBMVisualModel::serialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);

	// SAMSON Element generator pro tip: serialization is used in SAMSON to e.g. save documents, copy nodes, etc. 
	// Please refer to the SDK documentation for more information.
	// Complete this function to serialize your visual model.

}

void SEAdenitaVisualModel::unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber, const SBVersionNumber& classVersionNumber) {

	SBMVisualModel::unserialize(serializer, nodeIndexer, sdkVersionNumber, classVersionNumber);
	
	// SAMSON Element generator pro tip: serialization is used in SAMSON to e.g. save documents, copy nodes, etc. 
	// Please refer to the SDK documentation for more information.
	// Complete this function to unserialize your visual model.

}

void SEAdenitaVisualModel::eraseImplementation() {

	// SAMSON Element generator pro tip: modify this function when you need to perform special tasks when your visual model is erased (e.g. disconnect from nodes you are connected to).
	// Important: this function must be undoable (i.e. only call undoable functions or add an undo command to the undo stack)

}

float		SEAdenitaVisualModel::getScale() const { return scale_; }
void		SEAdenitaVisualModel::setScale(float scale) {

	if      (hasScaleRange() && scale < getMinimumScale()) this->scale_ = getMinimumScale();
	else if (hasScaleRange() && scale > getMaximumScale()) this->scale_ = getMaximumScale();
	else this->scale_ = scale;

	if (nanorobot_ == nullptr) return;

	if (this->scale_ > static_cast<float>(Scale::OBJECTS)) this->scale_ = static_cast<float>(Scale::OBJECTS);

	const float interpolated = 1.0f - (floor(this->scale_ + 1.0f) - this->scale_);

	if (scale_ < static_cast<float>(Scale::ATOMS_STICKS)) {
		//prepareSticksToBalls(interpolated);
	}
	else if (scale_ < static_cast<float>(Scale::ATOMS_BALLS)) {
	}
	else if (scale_ < static_cast<float>(Scale::NUCLEOTIDES)) {
		prepareBallsToNucleotides(interpolated);
	}
	else if (scale_ < static_cast<float>(Scale::SINGLE_STRANDS)) {
		prepareNucleotidesToSingleStrands(interpolated);
		setDimension(dim_);
	}
	else if (scale_ < static_cast<float>(Scale::DOUBLE_STRANDS)) {
		prepareSingleStrandsToDoubleStrands(interpolated);
	}
	else if (scale_ < static_cast<float>(Scale::OBJECTS)) {
		prepareDoubleStrandsToObjects(interpolated);
	}

	SAMSON::requestViewportUpdate();

}
bool		SEAdenitaVisualModel::hasScaleRange() const { return true; }
float		SEAdenitaVisualModel::getDefaultScale() const { return static_cast<float>(Scale::SINGLE_STRANDS); }
float		SEAdenitaVisualModel::getMinimumScale() const { return 0.0f; }
float		SEAdenitaVisualModel::getMaximumScale() const { return static_cast<float>(Scale::OBJECTS); }
float		SEAdenitaVisualModel::getScaleSingleStep() const { return 0.1f; }
std::string	SEAdenitaVisualModel::getScaleSuffix() const { return ""; }

int			SEAdenitaVisualModel::getDiscreteScaleCount() const { return 6; }
int			SEAdenitaVisualModel::getDiscreteScaleCurrentIndex() const {
	
	int discreteScale = static_cast<int>(std::ceil(scale_));
	if (discreteScale > 5) discreteScale = 5;
	return discreteScale;

}
void		SEAdenitaVisualModel::setDiscreteScaleCurrentIndex(const int index) { setScale(static_cast<float>(index));  }
std::string	SEAdenitaVisualModel::getDiscreteScaleItemText(const int index) const {

	if (index >= 0 && index <= 5) {

		const Scale s = static_cast<Scale>(index);
		if (s == Scale::ATOMS_STICKS) return "Atoms - sticks";
		if (s == Scale::ATOMS_BALLS) return "Atoms - balls";
		if (s == Scale::NUCLEOTIDES) return "Nucleotides";
		if (s == Scale::SINGLE_STRANDS) return "Single strands";
		if (s == Scale::DOUBLE_STRANDS) return "Double strands";
		if (s == Scale::OBJECTS) return "Objects";

	}

	return "";

}

float		SEAdenitaVisualModel::getDimension() const { return dim_; }
void		SEAdenitaVisualModel::setDimension(float dimension) {

	if      (hasDimensionRange() && dimension < getMinimumDimension()) this->dim_ = getMinimumDimension();
	else if (hasDimensionRange() && dimension > getMaximumDimension()) this->dim_ = getMaximumDimension();
	else this->dim_ = dimension;

	if (nanorobot_ == nullptr) return;

	if (scale_ < static_cast<float>(Scale::NUCLEOTIDES) || scale_ > static_cast<float>(Scale::SINGLE_STRANDS)) return;

	const float interpolated = 1.0f - (floor(this->dim_ + 1.0f) - this->dim_);

	if      (dim_ <  2.0f) prepare1Dto2D(interpolated);
	else if (dim_ <  3.0f) prepare2Dto3D(interpolated);
	else if (dim_ >= 3.0f) prepare3D(interpolated);

	SAMSON::requestViewportUpdate();

}
bool		SEAdenitaVisualModel::hasDimensionRange() const { return true; }
float		SEAdenitaVisualModel::getDefaultDimension() const { return 3.0f; }
float		SEAdenitaVisualModel::getMinimumDimension() const { return 1.0f; }
float		SEAdenitaVisualModel::getMaximumDimension() const { return 3.0f; }
float		SEAdenitaVisualModel::getDimensionSingleStep() const { return 0.1f; }
std::string	SEAdenitaVisualModel::getDimensionSuffix() const { return ""; }

int			SEAdenitaVisualModel::getDiscreteDimensionCount() const { return 3; }
int			SEAdenitaVisualModel::getDiscreteDimensionCurrentIndex() const {

	int discreteDimension = static_cast<int>(std::ceil(dim_));
	if (discreteDimension > 3) discreteDimension = 3;
	return discreteDimension - 1;

}
void		SEAdenitaVisualModel::setDiscreteDimensionCurrentIndex(const int index) { setDimension(static_cast<float>(index + 1)); }
std::string SEAdenitaVisualModel::getDiscreteDimensionItemText(const int index) const {
	
	if (index == 0) return "1D";
	if (index == 1) return "2D";
	if (index == 2) return "3D";
	return "";

}

double			SEAdenitaVisualModel::getVisibility() const { return visibility_; }
bool			SEAdenitaVisualModel::hasVisibilityRange() const { return true; }
double			SEAdenitaVisualModel::getDefaultVisibility() const { return 0.9999; }
double			SEAdenitaVisualModel::getMinimumVisibility() const { return 0.0; }
double			SEAdenitaVisualModel::getMaximumVisibility() const { return 0.9999; }
double			SEAdenitaVisualModel::getVisibilitySingleStep() const { return 0.01; }
std::string		SEAdenitaVisualModel::getVisibilitySuffix() const { return ""; }
void			SEAdenitaVisualModel::setVisibility(double layer) {

	this->visibility_ = layer;

	auto parts = nanorobot_->GetParts();
	SEConfig& config = SEConfig::GetInstance();

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			auto nucleotides = ss->GetNucleotides();
			auto ssDist = sortedSingleStrandsByDist_[ss()];

			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

				auto index = ntMap_[nt()];

				if (index >= nPositions_) return;
				if (ssDist > layer) {

					radiiV_(index) = 0.0f;
					radiiE_(index) = 0.0f;

				}
				else {

					radiiV_(index) = config.nucleotide_V_radius;
					radiiE_(index) = config.nucleotide_V_radius;

				}

				auto ntDist = sortedNucleotidesByDist_[nt()];
				if (ntDist > layer) {

					//nt->setVisibilityFlag(false);
					colorsV_(index, 3) = 0.0f;
					colorsE_(index, 3) = 0.0f;
					if (true) {

						if (ss->IsScaffold()) {

							radiiV_(index) = 0.0f;
							radiiE_(index) = 0.0f;
						}

					}

				}
				else {

					//nt->setVisibilityFlag(true);
					colorsV_(index, 3) = 1.0f;
					colorsE_(index, 3) = 1.0f;

				}
        
			}

		}

	}

	SAMSON::requestViewportUpdate();

}

void SEAdenitaVisualModel::init() {
	
	SBDocument* document = getDocument();
	if (document) nanorobot_ = SEAdenitaCoreSEApp::getAdenitaApp()->getNanorobot(document);
	else nanorobot_ = SEAdenitaCoreSEApp::getAdenitaApp()->GetNanorobot();

	SEConfig& config = SEConfig::GetInstance();

	MSVColors* regularColors = new MSVColors();
	MSVColors* meltTempColors = new MSVColors();
	MSVColors* gibbsColors = new MSVColors();
	colors_[ColorType::REGULAR] = regularColors;
	colors_[ColorType::MELTTEMP] = meltTempColors;
	colors_[ColorType::GIBBS] = gibbsColors;

	//setup the display properties
	nucleotideEColor_ = ADNArray<float>(4);
	nucleotideEColor_(0) = config.nucleotide_E_Color[0];
	nucleotideEColor_(1) = config.nucleotide_E_Color[1];
	nucleotideEColor_(2) = config.nucleotide_E_Color[2];
	nucleotideEColor_(3) = config.nucleotide_E_Color[3];

	update();

	setupPropertyColors();

	updateEnabledFlagForHighlightAttributes();

	//orderVisibility();

}

void SEAdenitaVisualModel::initAtoms(bool createIndex /*= true*/) {

	unsigned int nPositions = 0;

	auto parts = nanorobot_->GetParts();

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			auto nucleotides = ss->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

				nPositions += nt->getNumberOfAtoms();

			}

		}

	}

	unsigned int nCylinders = 0;

	nPositionsAtom_ = nPositions;
	nCylindersAtom_ = nCylinders;
	positionsAtom_ = ADNArray<float>(3, nPositions);
	radiiVAtom_ = ADNArray<float>(nPositions);
	radiiEAtom_ = ADNArray<float>(nPositions);
	colorsVAtom_ = ADNArray<float>(4, nPositions);
	colorsEAtom_ = ADNArray<float>(4, nPositions);
	flagsAtom_ = ADNArray<unsigned int>(nPositions);
	nodeIndicesAtom_ = ADNArray<unsigned int>(nPositions);
  
	if (createIndex) indicesAtom_ = getAtomIndices();

}

void SEAdenitaVisualModel::requestUpdate() {

	isUpdateRequested = true;
	SAMSON::requestViewportUpdate();

}

void SEAdenitaVisualModel::requestVisualModelUpdate() {

	SEAdenitaCoreSEApp::requestVisualModelUpdate();

}

void SEAdenitaVisualModel::update() {

	auto parts = nanorobot_->GetParts();

	SB_FOR(auto part, parts) part->connectBaseSignalToSlot(this, SB_SLOT(&SEAdenitaVisualModel::onBaseEvent));

	SB_FOR(auto part, parts) 
		if (!part->structuralSignalIsConnectedToSlot(this, SB_SLOT(&SEAdenitaVisualModel::onStructuralEvent)))
			part->connectStructuralSignalToSlot(this, SB_SLOT(&SEAdenitaVisualModel::onStructuralEvent));

	SBDocument* document = getDocument();
	if (!document) document = SAMSON::getActiveDocument();

	if (document) if (!document->documentSignalIsConnectedToSlot(this, SB_SLOT(&SEAdenitaVisualModel::onDocumentEvent)))
		document->connectDocumentSignalToSlot(this, SB_SLOT(&SEAdenitaVisualModel::onDocumentEvent));

	initDisplayIndices();

	initAtoms(true);
	initNucleotidesAndSingleStrands(true);
	initDoubleStrands(true);

	prepareDiscreteScalesDim();

	if (highlightType_ != HighlightType::NONE) highlightNucleotides();
	setScale(scale_);

	isPrepareDiscreteScalesDimRequested = false;
	isUpdateRequested = false;

}

unsigned int	SEAdenitaVisualModel::getHighlightMinLength() const { return highlightMinLen_; }
void			SEAdenitaVisualModel::setHighlightMinLength(unsigned int min) {

	this->highlightMinLen_ = min;
	setHighlight(HighlightType::LENGTH);

	SAMSON::requestViewportUpdate();

}

unsigned int	SEAdenitaVisualModel::getHighlightMaxLength() const { return highlightMaxLen_; }
void			SEAdenitaVisualModel::setHighlightMaxLength(unsigned int max) {

	this->highlightMaxLen_ = max;
	setHighlight(HighlightType::LENGTH);

	SAMSON::requestViewportUpdate();

}

bool SEAdenitaVisualModel::getNotWithinRange() const { return notWithin_; }
void SEAdenitaVisualModel::setNotWithinRange(bool c) {

	this->notWithin_ = c;
	setHighlight(HighlightType::LENGTH);

	SAMSON::requestViewportUpdate();

}
bool SEAdenitaVisualModel::getDefaultNotWithinRange() const { return false; }

bool SEAdenitaVisualModel::getNotScaffold() const { return notScaffold_; }
void SEAdenitaVisualModel::setNotScaffold(bool c) {

	this->notScaffold_ = c;
	setHighlight(HighlightType::LENGTH);

	SAMSON::requestViewportUpdate();

}
bool SEAdenitaVisualModel::getDefaultNotScaffold() const { return true; }

void SEAdenitaVisualModel::initNucleotidesAndSingleStrands(bool createIndex /* = true */) {

	const int nSingleStrands = nanorobot_->GetNumberOfSingleStrands();
	const unsigned int nPositions = nanorobot_->GetNumberOfNucleotides();
	const unsigned int nCylinders = boost::numeric_cast<unsigned int>(nPositions - nSingleStrands);

	nPositionsNt_ = nPositions;
	nCylindersNt_ = nCylinders;
	positionsNt_ = ADNArray<float>(3, nPositions);
	radiiVNt_ = ADNArray<float>(nPositions);
	radiiENt_ = ADNArray<float>(nPositions);
	colorsVNt_ = ADNArray<float>(4, nPositions);
	colorsENt_ = ADNArray<float>(4, nPositions);
	capDataNt_ = ADNArray<unsigned int>(nPositions);
	flagsNt_ = ADNArray<unsigned int>(nPositions);
	nodeIndicesNt_ = ADNArray<unsigned int>(nPositions);

	colorsVSS_ = ADNArray<float>(4, nPositions);
	colorsESS_ = ADNArray<float>(4, nPositions);
	radiiVSS_ = ADNArray<float>(nPositions);
	radiiESS_ = ADNArray<float>(nPositions);
  
	positionsNt2D_ = ADNArray<float>(3, nPositions);
	positionsNt1D_ = ADNArray<float>(3, nPositions);

	if (createIndex)
		indicesNt_ = getNucleotideIndices();

}

void SEAdenitaVisualModel::initDisplayIndices() {

	auto parts = nanorobot_->GetParts();

	bsMap_.clear();

	unsigned int indexDS = 0;

	SB_FOR(auto part, parts) {

		auto doubleStrands = part->GetDoubleStrands();
		SB_FOR(ADNPointer<ADNDoubleStrand> ds, doubleStrands) {

			auto baseSegments = ds->GetBaseSegments();
			SB_FOR(ADNPointer<ADNBaseSegment> bs, baseSegments) {

				bsMap_.insert(std::make_pair(bs(), indexDS));
				++indexDS;

			}

		}

	}

	unsigned int indexAtom = 0;
	unsigned int indexNt = 0;

	ntMap_.clear();
	atomMap_.clear();
	ntBsIndexMap_.clear();
	atomNtIndexMap_.clear();
	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			auto nucleotides = ss->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

				ntMap_.insert(std::make_pair(nt(), indexNt));
				auto atoms = nt->GetAtoms();
				SB_FOR(ADNPointer<ADNAtom> a, atoms) {

					atomMap_.insert(std::make_pair(a(), indexAtom));

					//associate atom to nt index
					atomNtIndexMap_.insert(std::make_pair(indexAtom, indexNt));
          
					++indexAtom;

				}

				//associate nt to bs index
				auto bs = nt->GetBaseSegment();
				ntBsIndexMap_.insert(std::make_pair(indexNt, bsMap_[bs()]));

				++indexNt;

			}

		}

	}

}

void SEAdenitaVisualModel::initDoubleStrands(bool createIndex /*= true*/) {

	const unsigned int nPositions = nanorobot_->GetNumberOfBaseSegments();

	nPositionsDS_ = nPositions;
	positionsDS_ = ADNArray<float>(3, nPositions);
	radiiVDS_ = ADNArray<float>(nPositions);
	colorsVDS_ = ADNArray<float>(4, nPositions);
	flagsDS_ = ADNArray<unsigned int>(nPositions);
	nodeIndicesDS_ = ADNArray<unsigned int>(nPositions);

	if (createIndex) {
	}

}

ADNArray<unsigned int> SEAdenitaVisualModel::getAtomIndices() {

	ADNArray<unsigned int> indices = ADNArray<unsigned int>(0);
  
	return indices;

}

ADNArray<unsigned int> SEAdenitaVisualModel::getNucleotideIndices() {

	auto singleStrands = nanorobot_->GetSingleStrands();
	auto parts = nanorobot_->GetParts();

	// check the correspondance of the number of single strands

	SBDocument* document = getDocument();
	if (!document) document = SAMSON::getActiveDocument();

	const int numberOfSingleStrandsUsingSAMSON = document->countNodes((SBNode::GetClass() == std::string("ADNSingleStrand")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
	int numberOfSingleStrandsInAllParts = 0;
	SB_FOR(auto part, parts) numberOfSingleStrandsInAllParts += part->GetSingleStrands().size();
	if (singleStrands.size() != numberOfSingleStrandsInAllParts || numberOfSingleStrandsUsingSAMSON != numberOfSingleStrandsInAllParts) {

		std::cerr << "ERROR: The number of single strands in nanorobot does not correspond to their number in the data graph. " << 
			"The total number in nanorobot is " << singleStrands.size() << " (the number in parts in nanorobot it is " << numberOfSingleStrandsInAllParts << ") and using SAMSON it is " << numberOfSingleStrandsUsingSAMSON << std::endl;

	}

	// check the correspondance of the number of nucleotides

	const int numberOfNucleotidesUsingSAMSON = document->countNodes((SBNode::GetClass() == std::string("ADNNucleotide")) && (SBNode::GetElementUUID() == SBUUID(SB_ELEMENT_UUID)));
	const int numberOfNucleotidesInNanorobot = nanorobot_->GetNumberOfNucleotides();
	if (numberOfNucleotidesUsingSAMSON != numberOfNucleotidesInNanorobot) {

		std::cerr << "ERROR: The number of nucleotides in nanorobot does not correspond to their number in the data graph. " <<
			"The total number in nanorobot is " << numberOfNucleotidesInNanorobot << " and using SAMSON it is " << numberOfNucleotidesUsingSAMSON << std::endl;

	}

	const unsigned int nPositions = nanorobot_->GetNumberOfNucleotides();
	const unsigned int nCylinders = boost::numeric_cast<unsigned int>(nPositions - singleStrands.size());

	ADNArray<unsigned int> indices = ADNArray<unsigned int>(nCylinders * 2);

	//std::cout << "nPositions: " << nPositions << "\tnCylinders: " << nCylinders << "\tnCylinders x 2: " << nCylinders * 2 << std::endl;

	//this init can be optimized in the future

	size_t sumNumEdges = 0;

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();

		SB_FOR(ADNPointer<ADNSingleStrand> singleStrand, singleStrands) {

			auto nucleotides = singleStrand->GetNucleotides();
			ADNPointer<ADNNucleotide> currentNucleotide = singleStrand->GetFivePrime();
			const size_t curNCylinders = nucleotides.size() - 1; //todo fix this
			ADNArray<unsigned int> curIndices = ADNArray<unsigned int>(2 * curNCylinders);

			//looping using the next_ member variable of nucleotides
			unsigned int j = 0;
			while (currentNucleotide->GetNext() != nullptr) {

				unsigned int currentIndex = ntMap_[currentNucleotide()];
				//nucleotides.getIndex(currentNucleotide(), currentIndex);
				ADNNucleotide* nextNucleotide = currentNucleotide->GetNext()();
				unsigned int nextIndex = ntMap_[nextNucleotide];
				//nucleotides.getIndex(nextNucleotide, nextIndex);

				curIndices(2 * j) = currentIndex;
				curIndices(2 * j + 1) = nextIndex;
				j++;

				currentNucleotide = currentNucleotide->GetNext();

			}

			for (int k = 0; k < curNCylinders * 2; ++k) {

				if (sumNumEdges + k >= nCylinders * 2) {

					std::cerr << "ERROR: \tindex is out of range: " << sumNumEdges + k << std::endl;
					break;

				}

				indices(sumNumEdges + k) = curIndices(k);

			}

			sumNumEdges += (2 * curNCylinders);

		}

	}

	return indices;

}

ADNArray<unsigned int> SEAdenitaVisualModel::getBaseSegmentIndices() {

	//not clear yet whether the indices are needed
	//for now this function is just used to construct the bsMap_

	auto parts = nanorobot_->GetParts();

	const unsigned int nDs = bsMap_.size();
  
	const unsigned int nPositions = nanorobot_->GetNumberOfBaseSegments();
	const unsigned int nCylinders = boost::numeric_cast<unsigned int>(nPositions - nDs);
 
	ADNArray<unsigned int> indices = ADNArray<unsigned int>(nCylinders * 2);

	return indices;

}

void SEAdenitaVisualModel::prepareDiscreteScalesDim() {

	isPrepareDiscreteScalesDimRequested = false;

	prepareAtoms();
	prepareNucleotides();
	prepareSingleStrands();
	prepareDoubleStrands();
	prepareDimensions();

}

void SEAdenitaVisualModel::prepareDimensions() {

	auto parts = nanorobot_->GetParts();
	auto conformations = nanorobot_->GetConformations();

	if (conformations.size() <= 1) return;

	// TODO: fix getting and determining 1D, 2D, 3D conformations
#if 1
	CollectionMap<ADNConformation> conformations1D;
	CollectionMap<ADNConformation> conformations2D;

	for (auto conf : conformations) {

		const QString confName = QString::fromStdString(conf->getName());
		if (confName.endsWith(" 1D")) conformations1D.addReferenceTarget(conf);
		else if (confName.endsWith(" 2D")) conformations2D.addReferenceTarget(conf);

	}
#else
	ADNConformation* conf2D = nullptr;
	ADNConformation* conf1D = nullptr;
	const QString conf1Name = QString::fromStdString(conformations[1]->getName());
	const QString conf2Name = QString::fromStdString(conformations[2]->getName());
	if (conf1Name.endsWith(" 1D")) conf1D = conformations[1];
	else if (conf1Name.endsWith(" 2D")) conf2D = conformations[1];
	
	if (conformations.size() > 2) {
		
		if (conf2Name.endsWith(" 1D")) conf1D = conformations[2];
		else if (conf2Name.endsWith(" 2D")) conf2D = conformations[2];

	}
#endif

	for (auto it = ntMap_.begin(); it != ntMap_.end(); it++) {

		auto nt = it->first;
		auto index = it->second;

		if (nt == nullptr) continue;
		if (index >= positionsNt2D_.GetNumElements()) continue;
		if (index >= positionsNt1D_.GetNumElements()) continue;

#if 1
		SBPosition3 pos2D, pos1D;

		for (auto conf : conformations2D) {

			if (conf->getPosition(nt->GetBackboneCenterAtom()(), pos2D)) {

				positionsNt2D_(index, 0) = pos2D[0].getValue();
				positionsNt2D_(index, 1) = pos2D[1].getValue();
				positionsNt2D_(index, 2) = pos2D[2].getValue();
				break;

			}

		}

		for (auto conf : conformations1D) {

			if (conf->getPosition(nt->GetBackboneCenterAtom()(), pos1D)) {

				positionsNt1D_(index, 0) = pos1D[0].getValue();
				positionsNt1D_(index, 1) = pos1D[1].getValue();
				positionsNt1D_(index, 2) = pos1D[2].getValue();
				break;

			}

		}
#else
		if (conf2D) {

			SBPosition3 pos2D;
			if (conf2D->getPosition(nt->GetBackboneCenterAtom()(), pos2D)) {

				positionsNt2D_(index, 0) = pos2D[0].getValue();
				positionsNt2D_(index, 1) = pos2D[1].getValue();
				positionsNt2D_(index, 2) = pos2D[2].getValue();

			}

		}

		if (conf1D) {

			SBPosition3 pos1D;
			if (conf1D->getPosition(nt->GetBackboneCenterAtom()(), pos1D)) {

				positionsNt1D_(index, 0) = pos1D[0].getValue();
				positionsNt1D_(index, 1) = pos1D[1].getValue();
				positionsNt1D_(index, 2) = pos1D[2].getValue();

			}

		}
#endif

	}

}

void SEAdenitaVisualModel::displayTransition(bool forSelection) {

	ADNArray<unsigned int> capData = ADNArray<unsigned int>(nodeIndices_.GetNumElements());
	if (nCylinders_ > 0) {

		for (int i = 0; i < capData.GetNumElements(); ++i)
			capData(i) = 1;

	}

	if (forSelection) {
		if (nCylinders_ > 0) {
			SAMSON::displayCylindersSelection(
				nCylinders_,
				nPositions_,
				indices_.GetArray(),
				positions_.GetArray(),
				radiiE_.GetArray(),
				capData.GetArray(),
				nodeIndices_.GetArray());
		}

		SAMSON::displaySpheresSelection(
			nPositions_,
			positions_.GetArray(),
			radiiV_.GetArray(),
			nodeIndices_.GetArray()
			);

	}
	else {

		if (nCylinders_ > 0) {
			SAMSON::displayCylinders(
				nCylinders_,
				nPositions_,
				indices_.GetArray(),
				positions_.GetArray(),
				radiiE_.GetArray(),
				capData.GetArray(),
				colorsE_.GetArray(),
				flags_.GetArray());
		}

		SAMSON::displaySpheres(
			nPositions_,
			positions_.GetArray(),
			radiiV_.GetArray(),
			colorsV_.GetArray(),
			flags_.GetArray());

		displayCircularDNAConnection();
		if (showBasePairing_) displayBasePairConnections(false);
		if (highlightType_ == HighlightType::TAGGED) displayTags();
		displayForDebugging();

	}

}

void SEAdenitaVisualModel::prepareSticksToBalls(double iv) {
  
}

void SEAdenitaVisualModel::prepareBallsToNucleotides(double iv) {

	nPositions_ = nPositionsAtom_;
	nCylinders_ = nCylindersAtom_;

	positions_ = ADNArray<float>(3, nPositions_);
	radiiV_ = ADNArray<float>(nPositions_);
	radiiE_ = ADNArray<float>(0);
	colorsV_ = ADNArray<float>(4, nPositions_);
	colorsE_ = ADNArray<float>(0);
	//flags_ = ADNArray<unsigned int>(nPositions_);
	//nodeIndices_ = ADNArray<unsigned int>(nPositions_);
	indices_ = ADNArray<unsigned int>(0);

	//positions_ = positionsAtom_;
	//radiiV_ = radiiVAtom_;
	//radiiE_ = radiiEAtom_;
	flags_ = flagsAtom_;
	nodeIndices_ = nodeIndicesAtom_;
	//indices_ = indicesAtom_;
	//colorsV_ = colorsVAtom_;
	//colorsE_ = colorsEAtom_;

	for (auto it = atomMap_.begin(); it != atomMap_.end(); it++) {

		auto indexAtom = it->second;
		auto indexNt = atomNtIndexMap_[indexAtom];

		if (indexAtom >= nPositions_) continue;
		if (indexAtom >= positionsAtom_.GetNumElements()) continue;
		if (indexAtom >= colorsVAtom_.GetNumElements()) continue;
		if (indexAtom >= radiiVAtom_.GetNumElements()) continue;
		if (indexNt >= positionsNt_.GetNumElements()) continue;
		if (indexNt >= colorsVNt_.GetNumElements()) continue;
		if (indexNt >= radiiVNt_.GetNumElements()) continue;

		positions_(indexAtom, 0) = positionsAtom_(indexAtom, 0) + iv * (positionsNt_(indexNt, 0) - positionsAtom_(indexAtom, 0));
		positions_(indexAtom, 1) = positionsAtom_(indexAtom, 1) + iv * (positionsNt_(indexNt, 1) - positionsAtom_(indexAtom, 1));
		positions_(indexAtom, 2) = positionsAtom_(indexAtom, 2) + iv * (positionsNt_(indexNt, 2) - positionsAtom_(indexAtom, 2));

		colorsV_(indexAtom, 0) = colorsVAtom_(indexAtom, 0) + iv * (colorsVNt_(indexNt, 0) - colorsVAtom_(indexAtom, 0));
		colorsV_(indexAtom, 1) = colorsVAtom_(indexAtom, 1) + iv * (colorsVNt_(indexNt, 1) - colorsVAtom_(indexAtom, 1));
		colorsV_(indexAtom, 2) = colorsVAtom_(indexAtom, 2) + iv * (colorsVNt_(indexNt, 2) - colorsVAtom_(indexAtom, 2));
		colorsV_(indexAtom, 3) = colorsVAtom_(indexAtom, 3) + iv * (colorsVNt_(indexNt, 3) - colorsVAtom_(indexAtom, 3));

		radiiV_(indexAtom) = radiiVAtom_(indexAtom) + iv * (radiiVNt_(indexNt) - radiiVAtom_(indexAtom));

	}

}

void SEAdenitaVisualModel::prepareNucleotidesToSingleStrands(double iv) {

	nPositions_ = nPositionsNt_;
	nCylinders_ = nCylindersNt_;

	//positions_ = ADNArray<float>(3, nPositions_);
	//radiiV_ = ADNArray<float>(nPositions_);
	radiiE_ = ADNArray<float>(nPositions_);
	colorsV_ = ADNArray<float>(4, nPositions_);
	//colorsE_ = ADNArray<float>(4, nPositions_);
	//flags_ = ADNArray<unsigned int>(nPositions_);
	//nodeIndices_ = ADNArray<unsigned int>(nPositions_);
	//indices_ = ADNArray<unsigned int>(nCylinders_ * 2);

	positions_ = positionsNt_;
	radiiV_ = radiiVNt_;
	flags_ = flagsNt_;
	nodeIndices_ = nodeIndicesNt_;
	indices_ = indicesNt_;
  
	for (auto it = ntMap_.begin(); it != ntMap_.end(); it++) {

		auto index = it->second;

		if (index >= nPositions_) continue;
		if (index >= radiiENt_.GetNumElements()) continue;
		if (index >= radiiESS_.GetNumElements()) continue;
		if (index >= colorsVNt_.GetNumElements()) continue;
		if (index >= colorsVSS_.GetNumElements()) continue;

		radiiE_(index) = radiiENt_(index) + iv * (radiiESS_(index) - radiiENt_(index));
		colorsV_(index, 0) = colorsVNt_(index, 0) + iv * (colorsVSS_(index, 0) - colorsVNt_(index, 0));
		colorsV_(index, 1) = colorsVNt_(index, 1) + iv * (colorsVSS_(index, 1) - colorsVNt_(index, 1));
		colorsV_(index, 2) = colorsVNt_(index, 2) + iv * (colorsVSS_(index, 2) - colorsVNt_(index, 2));
		colorsV_(index, 3) = colorsVNt_(index, 3) + iv * (colorsVSS_(index, 3) - colorsVNt_(index, 3));

	}

	colorsE_ = colorsV_;

}

void SEAdenitaVisualModel::prepareSingleStrandsToDoubleStrands(double iv) {

	nPositions_ = nPositionsNt_;
	nCylinders_ = nCylindersNt_;

	positions_ = ADNArray<float>(3, nPositions_);
	radiiV_ = ADNArray<float>(nPositions_);
	radiiE_ = ADNArray<float>(nPositions_);
	colorsV_ = ADNArray<float>(4, nPositions_);
	//colorsE_ = ADNArray<float>(4, nPositions_);
	//flags_ = ADNArray<unsigned int>(nPositions_);
	//nodeIndices_ = ADNArray<unsigned int>(nPositions_);
	//indices_ = ADNArray<unsigned int>(nCylinders_ * 2);


	//positions_ = positionsNt_;
	//radiiV_ = radiiVSS_;
	//radiie_ = radiiESS_;
	flags_ = flagsNt_;
	nodeIndices_ = nodeIndicesNt_;
	indices_ = indicesNt_;
	//colorsV_ = colorsVNt_;
	colorsE_ = colorsESS_;

	for (auto it = ntMap_.begin(); it != ntMap_.end(); it++) {

		auto nt = it->first;
		auto indexSS = it->second;
		auto indexDS = ntBsIndexMap_[indexSS];

		if (nt == nullptr) continue;
		if (indexSS >= nPositions_) continue;
		if (indexSS >= radiiVSS_.GetNumElements()) continue;
		if (indexDS >= radiiVDS_.GetNumElements()) continue;
		if (indexSS >= positionsNt_.GetNumElements()) continue;
		if (indexDS >= positionsDS_.GetNumElements()) continue;
		if (indexSS >= colorsVSS_.GetNumElements()) continue;
		if (indexDS >= colorsVDS_.GetNumElements()) continue;

		radiiV_(indexSS) = radiiVSS_(indexSS) + iv * (radiiVDS_(indexDS) - radiiVSS_(indexSS));
		radiiE_(indexSS) = radiiESS_(indexSS) - iv * radiiESS_(indexSS);

		positions_(indexSS, 0) = positionsNt_(indexSS, 0) + iv * (positionsDS_(indexDS, 0) - positionsNt_(indexSS, 0));
		positions_(indexSS, 1) = positionsNt_(indexSS, 1) + iv * (positionsDS_(indexDS, 1) - positionsNt_(indexSS, 1));
		positions_(indexSS, 2) = positionsNt_(indexSS, 2) + iv * (positionsDS_(indexDS, 2) - positionsNt_(indexSS, 2));

		colorsV_(indexSS, 0) = colorsVSS_(indexSS, 0) + iv * (colorsVDS_(indexDS, 0) - colorsVSS_(indexSS, 0));
		colorsV_(indexSS, 1) = colorsVSS_(indexSS, 1) + iv * (colorsVDS_(indexDS, 1) - colorsVSS_(indexSS, 1));
		colorsV_(indexSS, 2) = colorsVSS_(indexSS, 2) + iv * (colorsVDS_(indexDS, 2) - colorsVSS_(indexSS, 2));
		colorsV_(indexSS, 3) = colorsVSS_(indexSS, 3) + iv * (colorsVDS_(indexDS, 3) - colorsVSS_(indexSS, 3));

	}

}

void SEAdenitaVisualModel::prepareDoubleStrandsToObjects(double iv) {

	nPositions_ = nPositionsDS_;
	nCylinders_ = 0;

	positions_ = positionsDS_;
	radiiV_ = radiiVDS_;
	flags_ = flagsDS_;
	nodeIndices_ = nodeIndicesDS_;
	colorsV_ = colorsVDS_;

}

void SEAdenitaVisualModel::prepare1Dto2D(double iv) {

	for (auto it = ntMap_.begin(); it != ntMap_.end(); it++) {

		auto nt = it->first;
		auto index = it->second;

		if (nt == nullptr) continue;
		if (index >= positions_.GetNumElements()) continue;
		if (index >= positionsNt1D_.GetNumElements()) continue;
		if (index >= positionsNt2D_.GetNumElements()) continue;

		positions_(index, 0) = positionsNt1D_(index, 0) + iv * (positionsNt2D_(index, 0) - positionsNt1D_(index, 0));
		positions_(index, 1) = positionsNt1D_(index, 1) + iv * (positionsNt2D_(index, 1) - positionsNt1D_(index, 1));
		positions_(index, 2) = positionsNt1D_(index, 2) + iv * (positionsNt2D_(index, 2) - positionsNt1D_(index, 2));

	}

}

void SEAdenitaVisualModel::prepare2Dto3D(double iv) {

	for (auto it = ntMap_.begin(); it != ntMap_.end(); it++) {

		auto nt = it->first;
		auto index = it->second;

		if (nt == nullptr) continue;
		if (index >= positions_.GetNumElements()) continue;
		if (index >= positionsNt_.GetNumElements()) continue;
		if (index >= positionsNt2D_.GetNumElements()) continue;
    
		positions_(index, 0) = positionsNt2D_(index, 0) + iv * (positionsNt_(index, 0) - positionsNt2D_(index, 0));
		positions_(index, 1) = positionsNt2D_(index, 1) + iv * (positionsNt_(index, 1) - positionsNt2D_(index, 1));
		positions_(index, 2) = positionsNt2D_(index, 2) + iv * (positionsNt_(index, 2) - positionsNt2D_(index, 2));

	}

}

void SEAdenitaVisualModel::prepare3D(double iv) {

	positions_ = positionsNt_;

}

void SEAdenitaVisualModel::emphasizeColors(ADNArray<float> & colors, std::vector<unsigned int> & indices, float r, float g, float b, float a) {

	for (int i = 0; i < indices.size(); i++) {

		auto index = indices[i];

		if (index >= colors.GetNumElements()) continue;

		colors(index, 0) *= r;
		colors(index, 1) *= g;
		colors(index, 2) *= b;
		colors(index, 3) *= a;

		if (colors(index, 0) > 1.0f) colors(index, 0) = 1.0f;
		if (colors(index, 1) > 1.0f) colors(index, 1) = 1.0f;
		if (colors(index, 2) > 1.0f) colors(index, 2) = 1.0f;
		if (colors(index, 3) > 1.0f) colors(index, 3) = 1.0f;

	}

}

void SEAdenitaVisualModel::replaceColors(ADNArray<float> & colors, std::vector<unsigned int> & indices, float * color) {

	for (int i = 0; i < indices.size(); i++) {

		auto index = indices[i];

		if (index >= colors.GetNumElements()) continue;

		colors(index, 0) = color[0];
		colors(index, 1) = color[1];
		colors(index, 2) = color[2];
		colors(index, 3) = color[3];
    
	}

}

void SEAdenitaVisualModel::changeHighlightFlag() {

	isHighlightFlagChangeRequested = false;

	auto parts = nanorobot_->GetParts();

	if (scale_ < static_cast<float>(Scale::NUCLEOTIDES)) {

		for (auto it = atomMap_.begin(); it != atomMap_.end(); it++) {

			auto a = it->first;
			auto index = it->second;

			if (a == nullptr) continue;
			if (index >= flagsAtom_.GetNumElements()) continue;

			flagsAtom_(index) = a->getInheritedFlags();

		}

	}
	else if (scale_ >= static_cast<float>(Scale::NUCLEOTIDES) && scale_ < static_cast<float>(Scale::DOUBLE_STRANDS)) {

		for (auto it = ntMap_.begin(); it != ntMap_.end(); it++) {

			auto nt = it->first;
			auto index = it->second;

			if (nt == nullptr) continue;
			if (index >= flagsNt_.GetNumElements()) continue;

			flagsNt_(index) = nt->getInheritedFlags();

		}

	}
	else if (scale_ >= static_cast<float>(Scale::DOUBLE_STRANDS)) {

		for (auto it = bsMap_.begin(); it != bsMap_.end(); it++) {

			auto bs = it->first;
			auto index = it->second;

			if (bs == nullptr) continue;
			if (index >= flagsDS_.GetNumElements()) continue;

			flagsDS_(index) = bs->getInheritedFlags();

		}

	}

	setScale(scale_);

}

void SEAdenitaVisualModel::orderVisibility() {

	unsigned int order = 1;

	SEConfig& config = SEConfig::GetInstance();
  
	auto parts = nanorobot_->GetParts();

	std::vector<std::pair<ADNNucleotide*, float>> nucleotidesSorted;
	std::vector<std::pair<ADNSingleStrand*, float>> singleStrandsSorted;

	//ordered by
	if (order == 1) {

		SB_FOR(auto part, parts) {

			auto scaffolds = part->GetScaffolds();

			if (scaffolds.empty()) return;
        
			SB_FOR(ADNPointer<ADNSingleStrand> ss, scaffolds) {

				auto nucleotides = ss->GetNucleotides();
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

					auto pair = nt->GetPair();
					nucleotidesSorted.push_back(std::make_pair(nt(), float(nt->getNodeIndex())));
					if (pair != nullptr)
					nucleotidesSorted.push_back(std::make_pair(pair(), float(nt->getNodeIndex()))); //the staple nucleotide should get the same order as the scaffold nucleotide

				}

			}

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				auto nucleotides = ss->GetNucleotides();
				unsigned int minIdx = UINT_MAX;
				if (ss->IsScaffold()) {
					minIdx = 0;
				}
				else {

					SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

						auto pair = nt->GetPair();
						if (pair != nullptr) {

							unsigned int idx = pair->getNodeIndex();
							if (idx < minIdx) minIdx = idx;

						}

					}

				}

				singleStrandsSorted.push_back(std::make_pair(ss(), boost::numeric_cast<float>(minIdx)));

			}
      
		}

	}
	else if (order == 1) {

		SBPosition3 center;
		SB_FOR(auto part, parts) {

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				auto nucleotides = ss->GetNucleotides();
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) center += nt->GetPosition();

			}

		}

		center /= nanorobot_->GetNumberOfNucleotides();

		SB_FOR(auto part, parts) {
			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {
				auto nucleotides = ss->GetNucleotides();
				SBPosition3 strandPosition;
				float minDist = FLT_MAX;
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

					SBPosition3 diff = nt->GetPosition() - center;
					float dist = diff.norm().getValue();
					nucleotidesSorted.push_back(std::make_pair(nt(), dist));

					if (dist < minDist) minDist = dist;

				}

				singleStrandsSorted.push_back(std::make_pair(ss(), minDist));

			}

		}

	}
	else if (order == 2) {

	}

	if (nucleotidesSorted.size() == 0 || singleStrandsSorted.size() == 0) return;

	sort(nucleotidesSorted.begin(), nucleotidesSorted.end(), [=](std::pair<ADNNucleotide*, float>& a, std::pair<ADNNucleotide*, float>& b) { return a.second < b.second; });

	sort(singleStrandsSorted.begin(), singleStrandsSorted.end(), [=](std::pair<ADNSingleStrand*, float>& a, std::pair<ADNSingleStrand*, float>& b) { return a.second < b.second; });

	sortedNucleotidesByDist_.clear();
	sortedSingleStrandsByDist_.clear();

	float max = nucleotidesSorted.back().second;

	for (int i = 0; i < nucleotidesSorted.size(); i++) {
		sortedNucleotidesByDist_.insert(std::make_pair(nucleotidesSorted[i].first, nucleotidesSorted[i].second / max));
		//logger.Log(nucleotidesSorted[i].second);
	}

	for (int i = 0; i < singleStrandsSorted.size(); i++) {
		sortedSingleStrandsByDist_.insert(std::make_pair(singleStrandsSorted[i].first, singleStrandsSorted[i].second / max));
		//logger.Log(singleStrandsSorted[i].second);
	}

}

int			SEAdenitaVisualModel::getSingleStrandColorsCount() const { return 4; }
int			SEAdenitaVisualModel::getSingleStrandColorsCurrentIndex() const { return singleStrandColorsCurrentIndex; }
std::string	SEAdenitaVisualModel::getSingleStrandColorsItemText(const int index) const {

	if (index == 0) return "13 Colors Standard (Customizable)";
	if (index == 1) return "12 Colors Pastel";
	if (index == 2) return "8 Colors Dark";
	if (index == 3) return "8 Colors Pastel";
	return "";

}
void		SEAdenitaVisualModel::setSingleStrandColorsCurrentIndex(const int index) {

	auto regularColors = colors_[ColorType::REGULAR];

	if (index == 0) {
		
		regularColors->SetStandardSingleStrandColorScheme();

	}
	else if (index == 1) {

		ADNArray<float> pastel12 = ADNArray<float>(4, 12);

		pastel12(0, 0) = 141.0f / 255.0f;
		pastel12(0, 1) = 211.0f / 255.0f;
		pastel12(0, 2) = 199.0f / 255.0f;
		pastel12(0, 3) = 1.0f;

		pastel12(1, 0) = 255.0f / 255.0f;
		pastel12(1, 1) = 255.0f / 255.0f;
		pastel12(1, 2) = 179.0f / 255.0f;
		pastel12(1, 3) = 1.0f;

		pastel12(2, 0) = 190.0f / 255.0f;
		pastel12(2, 1) = 186.0f / 255.0f;
		pastel12(2, 2) = 218.0f / 255.0f;
		pastel12(2, 3) = 1.0f;

		pastel12(3, 0) = 251.0f / 255.0f;
		pastel12(3, 1) = 128.0f / 255.0f;
		pastel12(3, 2) = 114.0f / 255.0f;
		pastel12(3, 3) = 1.0f;

		pastel12(4, 0) = 128.0f / 255.0f;
		pastel12(4, 1) = 177.0f / 255.0f;
		pastel12(4, 2) = 211.0f / 255.0f;
		pastel12(4, 3) = 1.0f;

		pastel12(5, 0) = 253.0f / 255.0f;
		pastel12(5, 1) = 180.0f / 255.0f;
		pastel12(5, 2) =  98.0f / 255.0f;
		pastel12(5, 3) = 1.0f;

		pastel12(6, 0) = 179.0f / 255.0f;
		pastel12(6, 1) = 222.0f / 255.0f;
		pastel12(6, 2) = 105.0f / 255.0f;
		pastel12(6, 3) = 1.0f;

		pastel12(7, 0) = 252.0f / 255.0f;
		pastel12(7, 1) = 205.0f / 255.0f;
		pastel12(7, 2) = 229.0f / 255.0f;
		pastel12(7, 3) = 1.0f;

		pastel12(8, 0) = 217.0f / 255.0f;
		pastel12(8, 1) = 217.0f / 255.0f;
		pastel12(8, 2) = 217.0f / 255.0f;
		pastel12(8, 3) = 1.0f;

		pastel12(9, 0) = 188.0f / 255.0f;
		pastel12(9, 1) = 128.0f / 255.0f;
		pastel12(9, 2) = 189.0f / 255.0f;
		pastel12(9, 3) = 1.0f;

		pastel12(10, 0) = 204.0f / 255.0f;
		pastel12(10, 1) = 235.0f / 255.0f;
		pastel12(10, 2) = 197.0f / 255.0f;
		pastel12(10, 3) = 1.0f;

		pastel12(11, 0) = 255.0f / 255.0f;
		pastel12(11, 1) = 237.0f / 255.0f;
		pastel12(11, 2) = 111.0f / 255.0f;
		pastel12(11, 3) = 1.0f;

		regularColors->SetSingleStrandColorScheme(pastel12);

	}
	else if (index == 2) {

		ADNArray<float> dark8 = ADNArray<float>(4, 8);

		dark8(0, 0) = 102.0f / 255.0f;
		dark8(0, 1) = 102.0f / 255.0f;
		dark8(0, 2) = 102.0f / 255.0f;
		dark8(0, 3) = 1.0f;

		dark8(1, 0) =  27.0f / 255.0f;
		dark8(1, 1) = 158.0f / 255.0f;
		dark8(1, 2) = 119.0f / 255.0f;
		dark8(1, 3) = 1.0f;

		dark8(2, 0) = 217.0f / 255.0f;
		dark8(2, 1) =  95.0f / 255.0f;
		dark8(2, 2) =   2.0f / 255.0f;
		dark8(2, 3) = 1.0f;

		dark8(3, 0) = 117.0f / 255.0f;
		dark8(3, 1) = 112.0f / 255.0f;
		dark8(3, 2) = 179.0f / 255.0f;
		dark8(3, 3) = 1.0f;

		dark8(4, 0) = 231.0f / 255.0f;
		dark8(4, 1) =  41.0f / 255.0f;
		dark8(4, 2) = 138.0f / 255.0f;
		dark8(4, 3) = 1.0f;

		dark8(5, 0) = 102.0f / 255.0f;
		dark8(5, 1) = 166.0f / 255.0f;
		dark8(5, 2) =  30.0f / 255.0f;
		dark8(5, 3) = 1.0f;

		dark8(6, 0) = 230.0f / 255.0f;
		dark8(6, 1) = 171.0f / 255.0f;
		dark8(6, 2) =   2.0f / 255.0f;
		dark8(6, 3) = 1.0f;

		dark8(7, 0) = 166.0f / 255.0f;
		dark8(7, 1) = 118.0f / 255.0f;
		dark8(7, 2) =  29.0f / 255.0f;
		dark8(7, 3) = 1.0f;
    
		regularColors->SetSingleStrandColorScheme(dark8);

	}
	else if (index == 3) {

		ADNArray<float> pastel8 = ADNArray<float>(4, 8);

		pastel8(0, 0) = 179.0f / 255.0f;
		pastel8(0, 1) = 226.0f / 255.0f;
		pastel8(0, 2) = 205.0f / 255.0f;
		pastel8(0, 3) = 1.0f;

		pastel8(1, 0) = 253.0f / 255.0f;
		pastel8(1, 1) = 205.0f / 255.0f;
		pastel8(1, 2) = 172.0f / 255.0f;
		pastel8(1, 3) = 1.0f;

		pastel8(2, 0) = 203.0f / 255.0f;
		pastel8(2, 1) = 213.0f / 255.0f;
		pastel8(2, 2) = 232.0f / 255.0f;
		pastel8(2, 3) = 1.0f;

		pastel8(3, 0) = 244.0f / 255.0f;
		pastel8(3, 1) = 202.0f / 255.0f;
		pastel8(3, 2) = 228.0f / 255.0f;
		pastel8(3, 3) = 1.0f;

		pastel8(4, 0) = 230.0f / 255.0f;
		pastel8(4, 1) = 245.0f / 255.0f;
		pastel8(4, 2) = 201.0f / 255.0f;
		pastel8(4, 3) = 1.0f;

		pastel8(5, 0) = 255.0f / 255.0f;
		pastel8(5, 1) = 242.0f / 255.0f;
		pastel8(5, 2) = 174.0f / 255.0f;
		pastel8(5, 3) = 1.0f;

		pastel8(6, 0) = 241.0f / 255.0f;
		pastel8(6, 1) = 226.0f / 255.0f;
		pastel8(6, 2) = 204.0f / 255.0f;
		pastel8(6, 3) = 1.0f;

		pastel8(7, 0) = 204.0f / 255.0f; 
		pastel8(7, 1) = 204.0f / 255.0f; 
		pastel8(7, 2) = 204.0f / 255.0f; 
		pastel8(7, 3) = 1.0f;

		regularColors->SetSingleStrandColorScheme(pastel8);

	}

	prepareDiscreteScalesDim();
	setScale(scale_);// , false);

}

int			SEAdenitaVisualModel::getNucleotideColorsCount() const { return 4; }
int			SEAdenitaVisualModel::getNucleotideColorsCurrentIndex() const { return nucleotideColorSchemeCurrentIndex; }
std::string	SEAdenitaVisualModel::getNucleotideColorsItemText(const int index) const {

	if (index == 0) return "4 Colors Standard";
	if (index == 1) return "4 Colors Dark";
	if (index == 2) return "4 Colors Paired";
	if (index == 3) return "2 Colors Pastel (Pyrimidine, Purine)";
	return "";

}
void		SEAdenitaVisualModel::setNucleotideColorsCurrentIndex(const int index) {

	this->nucleotideColorSchemeCurrentIndex = index;

	auto regularColors = colors_[ColorType::REGULAR];

	if (index == 0) {
	
		regularColors->SetStandardNucleotideColorScheme();

	}
	else if (index == 1) {

		ADNArray<float> dark4 = ADNArray<float>(4, 4);

		dark4(0, 0) =  27.0f / 255.0f;  
		dark4(0, 1) = 158.0f / 255.0f;
		dark4(0, 2) = 119.0f / 255.0f;
		dark4(0, 3) = 1.0f;

		dark4(1, 0) = 217.0f / 255.0f;
		dark4(1, 1) =  95.0f / 255.0f;
		dark4(1, 2) =   2.0f / 255.0f;
		dark4(1, 3) = 1.0f;

		dark4(2, 0) = 117.0f / 255.0f;
		dark4(2, 1) = 112.0f / 255.0f;
		dark4(2, 2) = 179.0f / 255.0f;
		dark4(2, 3) = 1.0f;

		dark4(3, 0) = 231.0f / 255.0f;
		dark4(3, 1) =  41.0f / 255.0f;
		dark4(3, 2) = 138.0f / 255.0f;
		dark4(3, 3) = 1.0f;

		regularColors->SetNucleotideColorScheme(dark4);

	}
	else if (index == 2) {

		ADNArray<float> pastel4 = ADNArray<float>(4, 4);
    
		pastel4(0, 0) = 166.0f / 255.0f; 
		pastel4(0, 1) = 206.0f / 255.0f;
		pastel4(0, 2) = 227.0f / 255.0f;
		pastel4(0, 3) = 1.0f;

		pastel4(1, 0) =  31.0f / 255.0f;
		pastel4(1, 1) = 120.0f / 255.0f;
		pastel4(1, 2) = 180.0f / 255.0f;
		pastel4(1, 3) = 1.0f;

		pastel4(2, 0) = 178.0f / 255.0f;
		pastel4(2, 1) = 223.0f / 255.0f;
		pastel4(2, 2) = 138.0f / 255.0f;
		pastel4(2, 3) = 1.0f;

		pastel4(3, 0) =  51.0f / 255.0f;
		pastel4(3, 1) = 160.0f / 255.0f;
		pastel4(3, 2) =  44.0f / 255.0f;
		pastel4(3, 3) = 1.0f;

		regularColors->SetNucleotideColorScheme(pastel4);

	}
	else if (index == 3) {

		ADNArray<float> pastel2 = ADNArray<float>(4, 4);

		pastel2(0, 0) = 127.0f / 255.0f; 
		pastel2(0, 1) = 201.0f / 255.0f; 
		pastel2(0, 2) = 127.0f / 255.0f; 
		pastel2(0, 3) = 1.0f;

		pastel2(1, 0) = 253.0f / 255.0f;
		pastel2(1, 1) = 192.0f / 255.0f;
		pastel2(1, 2) = 134.0f / 255.0f;
		pastel2(1, 3) = 1.0f;

		pastel2(2, 0) = 253.0f / 255.0f;
		pastel2(2, 1) = 192.0f / 255.0f;
		pastel2(2, 2) = 134.0f / 255.0f;
		pastel2(2, 3) = 1.0f;

		pastel2(3, 0) = 127.0f / 255.0f;
		pastel2(3, 1) = 201.0f / 255.0f;
		pastel2(3, 2) = 127.0f / 255.0f;
		pastel2(3, 3) = 1.0f;

		regularColors->SetNucleotideColorScheme(pastel2);

	}

	prepareDiscreteScalesDim();
	setScale(scale_);// , false);

}

int			SEAdenitaVisualModel::getDoubleStrandColorsCount() const { return 5; }
int			SEAdenitaVisualModel::getDoubleStrandColorsCurrentIndex() const { return doubleStrandColorsCurrentIndex; }
std::string	SEAdenitaVisualModel::getDoubleStrandColorsItemText(const int index) const {

	if (index == 0) return "1 Color Standard";
	if (index == 1) return "2 Colors Pastel";
	if (index == 2) return "4 Colors Dark";
	if (index == 3) return "6 Colors Pastel";
	if (index == 4) return "12 Colors Pastel";
	return "";

}
void		SEAdenitaVisualModel::setDoubleStrandColorsCurrentIndex(const int index) {

	this->doubleStrandColorsCurrentIndex = index;

	auto regularColors = colors_[ColorType::REGULAR];

	if (index == 0) {
	
		regularColors->SetStandardDoubleStrandColorScheme();

	}
	else if (index == 1) {

		ADNArray<float> pastel2 = ADNArray<float>(4, 2);

		pastel2(0, 0) = 179.0f / 255.0f;
		pastel2(0, 1) = 226.0f / 255.0f;
		pastel2(0, 2) = 205.0f / 255.0f;
		pastel2(0, 3) = 1.0f;

		pastel2(1, 0) = 253.0f / 255.0f;
		pastel2(1, 1) = 205.0f / 255.0f;
		pastel2(1, 2) = 172.0f / 255.0f;
		pastel2(1, 3) = 1.0f;

		regularColors->SetDoubleStrandColorScheme(pastel2);

	}
	else if (index == 2) {

		ADNArray<float> pastel4 = ADNArray<float>(4, 4);

		pastel4(0, 0) = 141.0f / 255.0f;
		pastel4(0, 1) = 211.0f / 255.0f;
		pastel4(0, 2) = 199.0f / 255.0f;
		pastel4(0, 3) = 1.0f;

		pastel4(1, 0) = 255.0f / 255.0f;
		pastel4(1, 1) = 255.0f / 255.0f;
		pastel4(1, 2) = 179.0f / 255.0f;
		pastel4(1, 3) = 1.0f;

		pastel4(2, 0) = 190.0f / 255.0f;
		pastel4(2, 1) = 186.0f / 255.0f;
		pastel4(2, 2) = 218.0f / 255.0f;
		pastel4(2, 3) = 1.0f;

		pastel4(3, 0) = 251.0f / 255.0f;
		pastel4(3, 1) = 128.0f / 255.0f;
		pastel4(3, 2) = 114.0f / 255.0f;
		pastel4(3, 3) = 1.0f;

		regularColors->SetDoubleStrandColorScheme(pastel4);

	}
	else if (index == 3) {

		ADNArray<float> pastel6 = ADNArray<float>(4, 6);

		pastel6(0, 0) = 251.0f / 255.0f; 
		pastel6(0, 1) = 180.0f / 255.0f; 
		pastel6(0, 2) = 174.0f / 255.0f; 
		pastel6(0, 3) = 1.0f;

		pastel6(1, 0) = 179.0f / 255.0f; 
		pastel6(1, 1) = 205.0f / 255.0f; 
		pastel6(1, 2) = 227.0f / 255.0f; 
		pastel6(1, 3) = 1.0f;

		pastel6(2, 0) = 204.0f / 255.0f;
		pastel6(2, 1) = 235.0f / 255.0f;
		pastel6(2, 2) = 197.0f / 255.0f;
		pastel6(2, 3) = 1.0f;

		pastel6(3, 0) = 222.0f / 255.0f;
		pastel6(3, 1) = 203.0f / 255.0f;
		pastel6(3, 2) = 228.0f / 255.0f;
		pastel6(3, 3) = 1.0f;

		pastel6(4, 0) = 254.0f / 255.0f; 
		pastel6(4, 1) = 217.0f / 255.0f; 
		pastel6(4, 2) = 166.0f / 255.0f; 
		pastel6(4, 3) = 1.0f;

		pastel6(5, 0) = 255.0f / 255.0f; 
		pastel6(5, 1) = 255.0f / 255.0f; 
		pastel6(5, 2) = 204.0f / 255.0f; 
		pastel6(5, 3) = 1.0f;

		regularColors->SetDoubleStrandColorScheme(pastel6);

	}
	else if (index == 4) {

		ADNArray<float> pastel12 = ADNArray<float>(4, 12);

		pastel12(0, 0) = 141.0f / 255.0f;
		pastel12(0, 1) = 211.0f / 255.0f;
		pastel12(0, 2) = 199.0f / 255.0f;
		pastel12(0, 3) = 1.0f;

		pastel12(1, 0) = 255.0f / 255.0f;
		pastel12(1, 1) = 255.0f / 255.0f;
		pastel12(1, 2) = 179.0f / 255.0f;
		pastel12(1, 3) = 1.0f;

		pastel12(2, 0) = 190.0f / 255.0f;
		pastel12(2, 1) = 186.0f / 255.0f;
		pastel12(2, 2) = 218.0f / 255.0f;
		pastel12(2, 3) = 1.0f;

		pastel12(3, 0) = 251.0f / 255.0f;
		pastel12(3, 1) = 128.0f / 255.0f;
		pastel12(3, 2) = 114.0f / 255.0f;
		pastel12(3, 3) = 1.0f;

		pastel12(4, 0) = 128.0f / 255.0f;
		pastel12(4, 1) = 177.0f / 255.0f;
		pastel12(4, 2) = 211.0f / 255.0f;
		pastel12(4, 3) = 1.0f;

		pastel12(5, 0) = 253.0f / 255.0f;
		pastel12(5, 1) = 180.0f / 255.0f;
		pastel12(5, 2) =  98.0f / 255.0f;
		pastel12(5, 3) = 1.0f;

		pastel12(6, 0) = 179.0f / 255.0f;
		pastel12(6, 1) = 222.0f / 255.0f;
		pastel12(6, 2) = 105.0f / 255.0f;
		pastel12(6, 3) = 1.0f;

		pastel12(7, 0) = 252.0f / 255.0f;
		pastel12(7, 1) = 205.0f / 255.0f;
		pastel12(7, 2) = 229.0f / 255.0f;
		pastel12(7, 3) = 1.0f;

		pastel12(8, 0) = 217.0f / 255.0f;
		pastel12(8, 1) = 217.0f / 255.0f;
		pastel12(8, 2) = 217.0f / 255.0f;
		pastel12(8, 3) = 1.0f;

		pastel12(9, 0) = 188.0f / 255.0f;
		pastel12(9, 1) = 128.0f / 255.0f;
		pastel12(9, 2) = 189.0f / 255.0f;
		pastel12(9, 3) = 1.0f;

		pastel12(10, 0) = 204.0f / 255.0f;
		pastel12(10, 1) = 235.0f / 255.0f;
		pastel12(10, 2) = 197.0f / 255.0f;
		pastel12(10, 3) = 1.0f;

		pastel12(11, 0) = 255.0f / 255.0f;
		pastel12(11, 1) = 237.0f / 255.0f;
		pastel12(11, 2) = 111.0f / 255.0f;
		pastel12(11, 3) = 1.0f;

		regularColors->SetDoubleStrandColorScheme(pastel12);

	}

	prepareDiscreteScalesDim();
	setScale(scale_);// , false);

}

void SEAdenitaVisualModel::setupPropertyColors() {

	ADNArray<float> purpleBlueYellow = ADNArray<float>(4, 3);
	purpleBlueYellow(0, 0) = 68.0f / 255.0f;
	purpleBlueYellow(0, 1) = 3.0f / 255.0f;
	purpleBlueYellow(0, 2) = 84.0f / 255.0f;
	purpleBlueYellow(0, 3) = 1.0f;

	purpleBlueYellow(1, 0) = 38.0f / 255.0f;
	purpleBlueYellow(1, 1) = 148.0f / 255.0f;
	purpleBlueYellow(1, 2) = 139.0f / 255.0f;
	purpleBlueYellow(1, 3) = 1.0f;

	purpleBlueYellow(2, 0) = 252.0f / 255.0f;
	purpleBlueYellow(2, 1) = 230.0f / 255.0f;
	purpleBlueYellow(2, 2) = 59.0f / 255.0f;
	purpleBlueYellow(2, 3) = 1.0f;

	propertyColorSchemes_.push_back(purpleBlueYellow);

	ADNArray<float> divergingPurpleOrange = ADNArray<float>(4, 4);
	divergingPurpleOrange(0, 0) = 94 / 255.0f;
	divergingPurpleOrange(0, 1) = 60 / 255.0f;
	divergingPurpleOrange(0, 2) = 153 / 255.0f;
	divergingPurpleOrange(0, 3) = 1.0f;

	divergingPurpleOrange(1, 0) = 178 / 255.0f;
	divergingPurpleOrange(1, 1) = 171 / 255.0f;
	divergingPurpleOrange(1, 2) = 210 / 255.0f;
	divergingPurpleOrange(1, 3) = 1.0f;

	divergingPurpleOrange(2, 0) = 253 / 255.0f;
	divergingPurpleOrange(2, 1) = 184 / 255.0f;
	divergingPurpleOrange(2, 2) = 99 / 255.0f;
	divergingPurpleOrange(2, 3) = 1.0f;

	divergingPurpleOrange(3, 0) = 230 / 255.0f;
	divergingPurpleOrange(3, 1) = 97 / 255.0f;
	divergingPurpleOrange(3, 2) = 1 / 255.0f;
	divergingPurpleOrange(3, 3) = 1.0f;

	propertyColorSchemes_.push_back(divergingPurpleOrange);

	//this is a color scheme that is especially hard for red-green blind people to distinguish differences
	//our PI is red-green blind
	ADNArray<float> magicColors = ADNArray<float>(4, 4);
	magicColors(0, 0) = 0 / 255.0f;
	magicColors(0, 1) = 255 / 255.0f;
	magicColors(0, 2) = 0 / 255.0f;
	magicColors(0, 3) = 1.0f;

	magicColors(1, 0) = 255 / 255.0f;
	magicColors(1, 1) = 255 / 255.0f;
	magicColors(1, 2) = 0 / 255.0f;
	magicColors(1, 3) = 1.0f;

	magicColors(2, 0) = 255 / 255.0f;
	magicColors(2, 1) = 150 / 255.0f;
	magicColors(2, 2) = 0 / 255.0f;
	magicColors(2, 3) = 1.0f;

	magicColors(3, 0) = 255 / 255.0f;
	magicColors(3, 1) = 0 / 255.0f;
	magicColors(3, 2) = 0 / 255.0f;
	magicColors(3, 3) = 1.0f;

	propertyColorSchemes_.push_back(magicColors);

}

void SEAdenitaVisualModel::changePropertyColors(const int propertyIdx, const int colorSchemeIdx) {

	this->curColorType_ = static_cast<ColorType>(propertyIdx);
	this->colorSchemeCurrentIndex = colorSchemeIdx;

	if (this->curColorType_ == ColorType::MELTTEMP || this->curColorType_ == ColorType::GIBBS) {

		auto meltingTempColors = colors_.at(ColorType::MELTTEMP);
		auto gibbsColors = colors_.at(ColorType::GIBBS);

		SEConfig& config = SEConfig::GetInstance();

		auto p = PIPrimer3::GetInstance();

		auto parts = nanorobot_->GetParts();

		SB_FOR(auto part, parts) {

			auto regions = p.GetBindingRegions(part);

			SB_FOR(auto region, regions) {

				auto mt = region->getTemp();
				auto gibbs = region->getGibbs();
				auto groupNodes = region->getGroupNodes();

				for (unsigned i = 0; i < groupNodes->size(); i++) {

					auto node = groupNodes->getReferenceTarget(i);
					ADNPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(node);
					auto baseSegment = nt->GetBaseSegment();

					ADNArray<float> meltTempColor = calcPropertyColor(colorSchemeIdx, config.min_melting_temp, config.max_melting_temp, mt);
					meltingTempColors->SetColor(meltTempColor, nt);
					meltingTempColors->SetColor(meltTempColor, baseSegment);

					ADNArray<float> gibbsColor = calcPropertyColor(colorSchemeIdx, config.min_gibbs_free_energy, config.max_gibbs_free_energy, gibbs);
					gibbsColors->SetColor(gibbsColor, nt);
					gibbsColors->SetColor(gibbsColor, baseSegment);

				}

			}

		}

	}

	SAMSON::requestViewportUpdate();

}

int			SEAdenitaVisualModel::getColorTypeCount() const { return 3; }
int			SEAdenitaVisualModel::getColorTypeCurrentIndex() const { return static_cast<int>(curColorType_); }
void		SEAdenitaVisualModel::setColorTypeCurrentIndex(const int index) { changePropertyColors(index, colorSchemeCurrentIndex); }
std::string SEAdenitaVisualModel::getColorTypeItemText(const int index) const {

	if (index == 0) return "Standard";
	if (index == 1) return "Melting temperature";
	if (index == 2) return "Gibbs free energy";
	return "";

}

int			SEAdenitaVisualModel::getPropertyColorSchemeCount() const { return 3; }
int			SEAdenitaVisualModel::getPropertyColorSchemeCurrentIndex() const { return colorSchemeCurrentIndex; }
void		SEAdenitaVisualModel::setPropertyColorSchemeCurrentIndex(const int index) { changePropertyColors(static_cast<int>(curColorType_), index); }
std::string SEAdenitaVisualModel::getPropertyColorSchemeItemText(const int index) const {

	if (index == 0) return "Sequential plasma";
	if (index == 1) return "Diverging autumn";
	if (index == 2) return "Magic colors";
	return "";

}

void		SEAdenitaVisualModel::updateEnabledFlagForHighlightAttributes() {

	// enable/disable attributes in the Inspector

	const bool enabled = (highlightType_ == HighlightType::LENGTH);
	SBCClassInterface const* classInterface = getProxy()->getInterface();
	if (classInterface) {

		if (classInterface->getAttribute("NotScaffold")) classInterface->getAttribute("NotScaffold")->setEnabled(enabled);
		if (classInterface->getAttribute("NotWithinRange")) classInterface->getAttribute("NotWithinRange")->setEnabled(enabled);
		if (classInterface->getAttribute("HighlightMinLength")) classInterface->getAttribute("HighlightMinLength")->setEnabled(enabled);
		if (classInterface->getAttribute("HighlightMaxLength")) classInterface->getAttribute("HighlightMaxLength")->setEnabled(enabled);

	}

}

void		SEAdenitaVisualModel::setHighlight(const HighlightType highlightType) {

	this->highlightType_ = highlightType;

	updateEnabledFlagForHighlightAttributes();

	highlightNucleotides();
	setScale(scale_);
	SAMSON::requestViewportUpdate();

}
int			SEAdenitaVisualModel::getHighlightCount() const { return 7; }
int			SEAdenitaVisualModel::getHighlightCurrentIndex() const { return static_cast<int>(highlightType_); }
void		SEAdenitaVisualModel::setHighlightCurrentIndex(const int index) { setHighlight(static_cast<HighlightType>(index)); }
std::string SEAdenitaVisualModel::getHighlightItemText(const int index) const {

	HighlightType ht = static_cast<HighlightType>(index);

	if (ht == HighlightType::NONE) return "None";
	if (ht == HighlightType::CROSSOVERS) return "Crossovers";
	if (ht == HighlightType::GC) return "GC content";
	if (ht == HighlightType::TAGGED) return "Tagged";
	if (ht == HighlightType::LENGTH) return "Length";
	if (ht == HighlightType::NOBASE) return "No base";
	if (ht == HighlightType::UNPAIRED) return "Unpaired nucleotides";
	return "";

}

void SEAdenitaVisualModel::display() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop. This is the main function of your visual model. 
	// Implement this function to display things in SAMSON, for example thanks to the utility functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	if (isUpdateRequested) update();
	if (isHighlightFlagChangeRequested) changeHighlightFlag();

	//ADNLogger& logger = ADNLogger::GetLogger();

	SBEditor* activeEditor = SAMSON::getActiveEditor();
	/*logger.Log(activeEditor->getName());*/
	if (isPrepareDiscreteScalesDimRequested || activeEditor->getName() == "SEERotation" || activeEditor->getName() == "SEETranslation") {

		prepareDiscreteScalesDim();
		setScale(scale_);

	}

	displayTransition(false);
  
}


void SEAdenitaVisualModel::prepareNucleotides() {

	SEConfig& config = SEConfig::GetInstance();

	auto parts = nanorobot_->GetParts();

	MSVColors * curColors = colors_[curColorType_];

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> singleStrand, singleStrands) {

			auto nucleotides = singleStrand->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

				if (nt == nullptr) continue;
				unsigned int index = ntMap_[nt()];

				const Position3D nucleotideBackbonePosition = nt->GetBackbonePosition();
				positionsNt_(index, 0) = nucleotideBackbonePosition[0].getValue();
				positionsNt_(index, 1) = nucleotideBackbonePosition[1].getValue();
				positionsNt_(index, 2) = nucleotideBackbonePosition[2].getValue();

				// fill in position arrays for 1D and 2D
				positionsNt2D_(index, 0) = nucleotideBackbonePosition[0].getValue();
				positionsNt2D_(index, 1) = nucleotideBackbonePosition[1].getValue();
				positionsNt2D_(index, 2) = nucleotideBackbonePosition[2].getValue();
				positionsNt1D_(index, 0) = nucleotideBackbonePosition[0].getValue();
				positionsNt1D_(index, 1) = nucleotideBackbonePosition[1].getValue();
				positionsNt1D_(index, 2) = nucleotideBackbonePosition[2].getValue();

				colorsENt_.SetRow(index, nucleotideEColor_);
				nodeIndicesNt_(index) = nt->getNodeIndex();
				flagsNt_(index) = nt->getInheritedFlags();
				capDataNt_(index) = 1;

				auto baseColor = curColors->GetColor(nt);
				colorsVNt_.SetRow(index, baseColor);
				radiiVNt_(index) = config.nucleotide_V_radius;
				radiiENt_(index) = config.nucleotide_E_radius;

				if (nt->GetBaseSegment() != nullptr) {

					auto type = nt->GetBaseSegment()->GetCellType();
					if (type == CellType::LoopPair)
						radiiVNt_(index) = radiiVNt_(index) * 0.7f;

				}
        
				//strand direction
				if (nt->getEndType() == ADNNucleotide::EndType::ThreePrime) {
					radiiENt_(index) = config.nucleotide_E_radius;
				}

				if (!singleStrand->isVisible()) {

					colorsVNt_(index, 3) = 0.0f;
					radiiVNt_(index) = 0.0f;
					radiiENt_(index) = 0.0f;
					colorsENt_(index, 3) = 0.0f;

				}
				else if (!nt->isVisible()) {

					colorsVNt_(index, 3) = 0.0f;
					colorsENt_(index, 3) = 0.0f;
					radiiVNt_(index) = 0.0f;
					radiiENt_(index) = 0.0f;

				}

			}

		}

	}

}

void SEAdenitaVisualModel::prepareSingleStrands() {

	SEConfig& config = SEConfig::GetInstance();
	auto parts = nanorobot_->GetParts();
  
	MSVColors * curColors = colors_[curColorType_];

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			auto nucleotides = ss->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

				if (nt == nullptr) continue;
				unsigned int index = ntMap_[nt()];

				if (curColorType_ == ColorType::REGULAR) {

					auto color = curColors->GetColor(ss);
					colorsVSS_.SetRow(index, color);

				}
				else if (curColorType_ == ColorType::MELTTEMP || curColorType_ == ColorType::GIBBS) {

					auto color = curColors->GetColor(ss);
					colorsVSS_.SetRow(index, color);

				}

				colorsESS_(index, 0) = colorsVSS_(index, 0);
				colorsESS_(index, 1) = colorsVSS_(index, 1);
				colorsESS_(index, 2) = colorsVSS_(index, 2);
				colorsESS_(index, 3) = colorsVSS_(index, 3);

				radiiVSS_(index) = config.nucleotide_V_radius;

				radiiESS_(index) = config.nucleotide_V_radius;

				//strand direction
				if (nt->getEndType() == ADNNucleotide::EndType::ThreePrime) {
					radiiESS_(index) = config.nucleotide_E_radius;
				}

				if (!ss->isVisible()) {

					radiiVSS_(index) = 0.0f;
					radiiESS_(index) = 0.0f;

				}
				else if (!nt->isVisible()) {

					colorsVSS_(index, 3) = 0.0f;
					colorsESS_(index, 3) = 0.0f;
					radiiVSS_(index) = 0.0f;
					radiiESS_(index) = 0.0f;

				}

			}

		}

	}

}

void SEAdenitaVisualModel::prepareDoubleStrands() {

	SEConfig& config = SEConfig::GetInstance();

	auto parts = nanorobot_->GetParts();

	MSVColors * curColors = colors_[curColorType_];
	positionsDS_ = ADNArray<float>(3, nPositionsDS_);
	radiiVDS_ = ADNArray<float>(nPositionsDS_);
	flagsDS_ = ADNArray<unsigned int>(nPositionsDS_);
	colorsVDS_ = ADNArray<float>(4, nPositionsDS_);
	nodeIndicesDS_ = ADNArray<unsigned int>(nPositionsDS_);
  
	SB_FOR(auto part, parts) {

		if (part == nullptr) continue;

		auto doubleStrands = part->GetDoubleStrands();
		//for now also the base segments that have loops and skips are displayed as sphere
		SB_FOR(auto doubleStrand, doubleStrands) {

			if (doubleStrand == nullptr) continue;

			auto baseSegments = doubleStrand->GetBaseSegments();
			SB_FOR(auto baseSegment, baseSegments) {

				if (baseSegment == nullptr) continue;

				auto index = bsMap_[baseSegment];
				//auto cell = baseSegment->GetCell();
				if (index >= positionsDS_.GetNumElements()) continue;

				const Position3D pos = baseSegment->GetPosition();
				positionsDS_(index, 0) = static_cast<float>(pos.v[0].getValue());
				positionsDS_(index, 1) = static_cast<float>(pos.v[1].getValue());
				positionsDS_(index, 2) = static_cast<float>(pos.v[2].getValue());

				auto color = curColors->GetColor(baseSegment);
				colorsVDS_.SetRow(index, color);
				radiiVDS_(index) = config.base_pair_radius;

				auto type = baseSegment->GetCellType();
				if (type == CellType::SkipPair) {
					colorsVDS_(index, 1) = 0.0f;
					colorsVDS_(index, 2) = 0.0f;
					colorsVDS_(index, 3) = 0.3f;
				}

				if (type == CellType::LoopPair) {
					radiiVDS_(index) = config.base_pair_radius * 1.2f;
					colorsVDS_(index, 0) = 0.0f;
					colorsVDS_(index, 3) = 0.3f;
				}

				flagsDS_(index) = baseSegment->getInheritedFlags();

				nodeIndicesDS_(index) = baseSegment->getNodeIndex();

				if (!doubleStrand->isVisible()) {
					colorsVDS_(index, 3) = 0.0f;
					radiiVDS_(index) = 0.0f;
				}
				else if (!baseSegment->isVisible()) {
					colorsVDS_(index, 3) = 0.0f;
				}

				++index;

			}

		}

	}

}

void SEAdenitaVisualModel::displayNucleotides(bool forSelection) {

	if (forSelection) {

		if (nCylindersNt_ > 0) {
			SAMSON::displayCylindersSelection(
				nCylindersNt_,
				nPositionsNt_,
				indicesNt_.GetArray(),
				positionsNt_.GetArray(),
				radiiENt_.GetArray(),
				capDataNt_.GetArray(),
				nodeIndicesNt_.GetArray());
		}

		SAMSON::displaySpheresSelection(
			nPositionsNt_,
			positionsNt_.GetArray(),
			radiiVNt_.GetArray(),
			nodeIndicesNt_.GetArray()
		);

	}
	else {

		if (nCylindersNt_ > 0) {
			SAMSON::displayCylinders(
				nCylindersNt_,
				nPositionsNt_,
				indicesNt_.GetArray(),
				positionsNt_.GetArray(),
				radiiENt_.GetArray(),
				capDataNt_.GetArray(),
				colorsENt_.GetArray(),
				flagsNt_.GetArray());
		}

		SAMSON::displaySpheres(
			nPositionsNt_,
			positionsNt_.GetArray(),
			radiiVNt_.GetArray(),
			colorsVNt_.GetArray(),
			flagsNt_.GetArray());

	}

}

void SEAdenitaVisualModel::displaySingleStrands(bool forSelection) {

	if (forSelection) {

		if (nCylindersNt_ > 0) {
			SAMSON::displayCylindersSelection(
				nCylindersNt_,
				nPositionsNt_,
				indicesNt_.GetArray(),
				positionsNt_.GetArray(),
				radiiESS_.GetArray(),
				capDataNt_.GetArray(),
				nodeIndicesNt_.GetArray());
		}

		SAMSON::displaySpheresSelection(
			nPositionsNt_,
			positionsNt_.GetArray(),
			radiiVSS_.GetArray(),
			nodeIndicesNt_.GetArray()
		);

	}
	else {

		if (nCylindersNt_ > 0) {
			SAMSON::displayCylinders(
				nCylindersNt_,
				nPositionsNt_,
				indicesNt_.GetArray(),
				positionsNt_.GetArray(),
				radiiESS_.GetArray(),
				capDataNt_.GetArray(),
				colorsESS_.GetArray(),
				flagsNt_.GetArray());
		}

		SAMSON::displaySpheres(
			nPositionsNt_,
			positionsNt_.GetArray(),
			radiiVSS_.GetArray(),
			colorsVSS_.GetArray(),
			flagsNt_.GetArray());

	}

}

void SEAdenitaVisualModel::displayDoubleStrands(bool forSelection) {

	if (forSelection) {
		SAMSON::displaySpheresSelection(
			nPositionsDS_,
			positionsDS_.GetArray(),
			radiiVDS_.GetArray(),
			nodeIndicesDS_.GetArray()
		);
	}
	else {
		SAMSON::displaySpheres(
			nPositionsDS_,
			positionsDS_.GetArray(),
			radiiVDS_.GetArray(),
			colorsVDS_.GetArray(),
			flagsDS_.GetArray());
	}

}

void SEAdenitaVisualModel::displayForShadow() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to compute shadows. 
	// Implement this function so that your visual model can cast shadows to other objects in SAMSON, for example thanks to the utility
	// functions provided by SAMSON (e.g. displaySpheres, displayTriangles, etc.)

	display();

}

void SEAdenitaVisualModel::displayForSelection() {

	// SAMSON Element generator pro tip: this function is called by SAMSON during the main rendering loop in order to perform object picking.
	// Instead of rendering colors, your visual model is expected to write the index of a data graph node (obtained with getIndex()).
	// Implement this function so that your visual model can be selected (if you render its own index) or can be used to select other objects (if you render 
	// the other objects' indices), for example thanks to the utility functions provided by SAMSON (e.g. displaySpheresSelection, displayTrianglesSelection, etc.)

	if (isUpdateRequested) update();

	displayTransition(true);

}

void SEAdenitaVisualModel::displayBasePairConnections(bool onlySelected) {

	if (scale_ < static_cast<float>(Scale::NUCLEOTIDES) && scale_ > static_cast<float>(Scale::SINGLE_STRANDS)) return;

	SEConfig& config = SEConfig::GetInstance();

	auto baseColors = colors_.at(ColorType::REGULAR);
	auto parts = nanorobot_->GetParts();

	unsigned int numPairedNucleotides = 0;
	std::map<ADNNucleotide*, unsigned int> ntMap;

	//determine how many nucleotides have pairs 

	SB_FOR(auto part, parts) {

		if (part == nullptr) continue;

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> singleStrand, singleStrands) {

			if (singleStrand == nullptr) continue;

			auto nucleotides = singleStrand->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nucleotide, nucleotides) {

				if (nucleotide == nullptr) continue;
				// skip if at least one of the paired nucleotides is not visible
				if (!nucleotide->isVisible() || !nucleotide->GetPair()->isVisible()) continue;

				if (nucleotide->GetPair() != nullptr) {

					ntMap.insert(std::make_pair(nucleotide(), numPairedNucleotides));
					++numPairedNucleotides;

				}

			}

		}

	}

	const unsigned int nPositions = boost::numeric_cast<unsigned int>(ntMap.size());
	const unsigned int nCylinders = nPositions / 2;

	ADNArray<float> positions = ADNArray<float>(3, nPositions);
	ADNArray<unsigned int> indices = ADNArray<unsigned int>(nCylinders * 2);
	ADNArray<float> radii = ADNArray<float>(nPositions);
	ADNArray<float> colors = ADNArray<float>(4, nPositions);
	ADNArray<unsigned int> caps = ADNArray<unsigned int>(nPositions);
	ADNArray<unsigned int> flags = ADNArray<unsigned int>(nPositions);

	unsigned int j = 0;
	std::vector<unsigned int> registerIndices;
	for (auto &p : ntMap) {

		ADNPointer<ADNNucleotide> nt = p.first;
		if (nt == nullptr) continue;
		ADNPointer<ADNNucleotide> nucleotidePair = nt->GetPair();
		if (nucleotidePair == nullptr) continue;
		unsigned int index = p.second;
    
		const SBPosition3 pos = nt->GetBackbonePosition();
   
		positions(index, 0) = static_cast<float>(pos[0].getValue());
		positions(index, 1) = static_cast<float>(pos[1].getValue());
		positions(index, 2) = static_cast<float>(pos[2].getValue());

		radii(index) = config.nucleotide_E_radius;
		colors.SetRow(index, baseColors->GetColor(nt));
		flags(index) = 0;
		caps(index) = 1;

		if (std::find(registerIndices.begin(), registerIndices.end(), ntMap[nucleotidePair()]) == registerIndices.end()) {

			// we only need to insert the indices once per pair
			indices(2 * j) = index;
			indices(2 * j + 1) = ntMap[nucleotidePair()];
			registerIndices.push_back(index);
      
			++j;

		}

		if (onlySelected) {
			if (!nt->isSelected() && !nucleotidePair->isSelected()) {
				radii(index) = 0;
			}
		}

	}

	SAMSON::displayCylinders(
		nCylinders,
		nPositions,
		indices.GetArray(),
		positions.GetArray(),
		radii.GetArray(),
		caps.GetArray(),
		colors.GetArray(),
		flags.GetArray()
		);

}

void SEAdenitaVisualModel::displayForDebugging() {

	return;

	SEConfig& config = SEConfig::GetInstance();

	if (config.debugOptions.display_nucleotide_basis) {
		SB_FOR(auto pair, ntMap_) {
			ADNPointer<ADNNucleotide> nt = pair.first;
			unsigned int idx = pair.second;
			if (nt->isSelected()) {
				SBPosition3 currPos = SBPosition3(SBQuantity::picometer(positionsNt_(idx, 0)),
					SBQuantity::picometer(positionsNt_(idx, 1)), SBQuantity::picometer(positionsNt_(idx, 2)));
				ADNDisplayHelper::displayBaseVectors(nt, currPos);
			}
		}
	}
	if (config.debugOptions.display_base_pairing) {
		displayBasePairConnections(true);
	}

}

void SEAdenitaVisualModel::displayCircularDNAConnection() {

	if (scale_ < static_cast<float>(Scale::NUCLEOTIDES) || scale_ > static_cast<float>(Scale::SINGLE_STRANDS)) return;

	auto parts = nanorobot_->GetParts();

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();

		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			if (ss->IsCircular()) {

				SEConfig& config = SEConfig::GetInstance();

				auto startNt = ss->GetFivePrime();
				auto endNt = ss->GetThreePrime();
				int startIdx = ntMap_[startNt()];
				int endIdx = ntMap_[endNt()];

				float * startPos = new float[3];
				startPos[0] = positions_(startIdx, 0);
				startPos[1] = positions_(startIdx, 1);
				startPos[2] = positions_(startIdx, 2);
        
				float * endPos = new float[3];
				endPos[0] = positions_(endIdx, 0);
				endPos[1] = positions_(endIdx, 1);
				endPos[2] = positions_(endIdx, 2);

				float * color = new float[4];
				color[0] = colorsE_(endIdx, 0);
				color[1] = colorsE_(endIdx, 1);
				color[2] = colorsE_(endIdx, 2);
				color[3] = colorsE_(endIdx, 3);
        
				auto radius = radiiE_(startIdx);

				ADNDisplayHelper::displayDirectedCylinder(startPos, endPos, color, radius);
        
				delete[] startPos;
				delete[] endPos;
				delete[] color;

			}

		}

	}

}

void SEAdenitaVisualModel::displayTags() {

	//tagged nucleotides should be saved in a list
	auto parts = nanorobot_->GetParts();

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			auto nucleotides = ss->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {
				ADNDisplayHelper::displayText(nt->GetBackbonePosition(), nt->getTag());
			}

		}

	}

}

void SEAdenitaVisualModel::prepareAtoms() {

	SEConfig& config = SEConfig::GetInstance();
	MSVColors* curColors = colors_[curColorType_];

	auto parts = nanorobot_->GetParts();

	SB_FOR(auto part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

			auto nucleotides = ss->GetNucleotides();
			SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

				auto atoms = nt->GetAtoms();
				SB_FOR(ADNPointer<ADNAtom> a, atoms) {

					auto index = atomMap_[a()];

					positionsAtom_(index, 0) = a->getPosition()[0].getValue();
					positionsAtom_(index, 1) = a->getPosition()[1].getValue();
					positionsAtom_(index, 2) = a->getPosition()[2].getValue();

					auto color = curColors->GetColor(a);
					colorsVAtom_.SetRow(index, color);

					nodeIndicesAtom_(index) = a->getNodeIndex();
					flagsAtom_(index) = a->getInheritedFlags();

					radiiVAtom_(index) = a->getVanDerWaalsRadius().getValue();

					if (!ss->isVisible()) {
						colorsVAtom_(index, 3) = 0.0f;
						radiiVAtom_(index) = 0.0f;
					}
					else if (!nt->isVisible()) {
						radiiVAtom_(index) = 0.0f;
						colorsVAtom_(index, 3) = 0.0f;
					}
					/*else if (!a->isVisible()) {
					  colorsVAtom_(index, 3) = 0.0f;
					  radiiVAtom_(index) = 0.0f;
					}*/

				}

			}

		}

	}

}

void SEAdenitaVisualModel::highlightNucleotides() {

	prepareDiscreteScalesDim();

	float * colorHighlight = new float[4];
	colorHighlight[0] = 0.2f;
	colorHighlight[1] = 0.8f;
	colorHighlight[2] = 0.2f;
	colorHighlight[3] = 1.0f;

	auto parts = nanorobot_->GetParts();
	std::vector<unsigned int> ntHighlight;
	std::vector<unsigned int> ntContext;
	std::vector<unsigned int> bsHighlight;
	std::vector<unsigned int> bsContext;

	if (highlightType_ == HighlightType::NONE) {
    
	}
	else if (highlightType_ == HighlightType::CROSSOVERS) {

		for (auto p : ntMap_) {
			auto index = p.second;
			ntContext.push_back(index);
		}

		for (auto p : bsMap_) {
			auto index = p.second;
			bsContext.push_back(index);
		}

		SB_FOR(auto part, parts) {

			auto xos = PICrossovers::GetCrossovers(part);
			for (auto xo : xos) {

				auto startNt = xo.first;
				auto endNt = xo.second;
				auto startBs = startNt->GetBaseSegment();
				auto endBs = endNt->GetBaseSegment();

				auto startNtIdx = ntMap_[startNt()];
				auto endNtIdx = ntMap_[endNt()];
				auto startBsIdx = bsMap_[startBs()];
				auto endBsIdx = bsMap_[endBs()];

				ntHighlight.push_back(startNtIdx);
				ntHighlight.push_back(endNtIdx);
				bsHighlight.push_back(startBsIdx);
				bsHighlight.push_back(endBsIdx);

			}

		}

	}
	else if (highlightType_ == HighlightType::GC) {

		for (auto p : ntMap_) {
			auto index = p.second;
			ntContext.push_back(index);
		}

		for (auto p : bsMap_) {
			auto index = p.second;
			bsContext.push_back(index);
		}

		SB_FOR(auto part, parts) {

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				auto nucleotides = ss->GetNucleotides();
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

					auto indexNt = ntMap_[nt()];
					if (nt->getNucleotideType() == DNABlocks::DC || nt->getNucleotideType() == DNABlocks::DG) {
						ntHighlight.push_back(indexNt);

						auto bs = nt->GetBaseSegment();
						auto indexDs = bsMap_[bs()];
						bsHighlight.push_back(indexDs);

					}

				}

			}

		}

	}
	else if (highlightType_ == HighlightType::TAGGED) {

		for (auto p : ntMap_) {
			auto index = p.second;
			ntContext.push_back(index);
		}

		for (auto p : bsMap_) {
			auto index = p.second;
			bsContext.push_back(index);
		}

		SB_FOR(auto part, parts) {

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				auto nucleotides = ss->GetNucleotides();
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

					auto indexNt = ntMap_[nt()];
					if (nt->hasTag()) {

						ntHighlight.push_back(indexNt);
						auto bs = nt->GetBaseSegment();
						auto indexDs = bsMap_[bs()];
						bsHighlight.push_back(indexDs);

					}

				}

			}

		}

	}
	else if (highlightType_ == HighlightType::LENGTH) {

		for (auto p : ntMap_) {
			auto index = p.second;
			ntContext.push_back(index);
		}

		for (auto p : bsMap_) {
			auto index = p.second;
			bsContext.push_back(index);
		}

		SB_FOR(auto part, parts) {

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				bool inRange = ss->getNumberOfNucleotides() > highlightMinLen_ && ss->getNumberOfNucleotides() < highlightMaxLen_;
				if (notWithin_) inRange = !inRange;
				if (notScaffold_) inRange = ss->IsScaffold() ? false : inRange;
				if (inRange) {

					auto nucleotides = ss->GetNucleotides();
					SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

						auto indexNt = ntMap_[nt()];
						ntHighlight.push_back(indexNt);
						auto bs = nt->GetBaseSegment();
						auto indexDs = bsMap_[bs()];
						bsHighlight.push_back(indexDs);

					}

				}

			}

		}

	}
	else if (highlightType_ == HighlightType::NOBASE) {

		for (auto p : ntMap_) {
			auto index = p.second;
			ntContext.push_back(index);
		}

		for (auto p : bsMap_) {
			auto index = p.second;
			bsContext.push_back(index);
		}

		SB_FOR(auto part, parts) {

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				auto nucleotides = ss->GetNucleotides();
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

					auto indexNt = ntMap_[nt()];
					if (nt->getNucleotideType() == DNABlocks::DI) {

						ntHighlight.push_back(indexNt);
						auto bs = nt->GetBaseSegment();
						auto indexDs = bsMap_[bs()];
						bsHighlight.push_back(indexDs);

					}

				}

			}

		}
	}
	else if (highlightType_ == HighlightType::UNPAIRED) {

		for (auto p : ntMap_) {
			auto index = p.second;
			ntContext.push_back(index);
		}

		for (auto p : bsMap_) {
			auto index = p.second;
			bsContext.push_back(index);
		}

		SB_FOR(auto part, parts) {

			auto singleStrands = part->GetSingleStrands();
			SB_FOR(ADNPointer<ADNSingleStrand> ss, singleStrands) {

				auto nucleotides = ss->GetNucleotides();
				SB_FOR(ADNPointer<ADNNucleotide> nt, nucleotides) {

					auto indexNt = ntMap_[nt()];
					if (nt->GetPair() == nullptr) {

						ntHighlight.push_back(indexNt);
						auto bs = nt->GetBaseSegment();
						auto indexDs = bsMap_[bs()];
						bsHighlight.push_back(indexDs);

					}

				}

			}

		}

	}

	emphasizeColors(colorsVNt_, ntContext, 0.4f, 0.4f, 0.4f, 1.0f);
	emphasizeColors(colorsENt_, ntContext, 0.4f, 0.4f, 0.4f, 1.0f);
	emphasizeColors(colorsVSS_, ntContext, 0.4f, 0.4f, 0.4f, 1.0f);
	emphasizeColors(colorsESS_, ntContext, 0.4f, 0.4f, 0.4f, 1.0f);
	emphasizeColors(colorsVDS_, bsContext, 0.4f, 0.4f, 0.4f, 1.0f);

	replaceColors(colorsVNt_, ntHighlight, colorHighlight);
	replaceColors(colorsENt_, ntHighlight, colorHighlight);
	replaceColors(colorsVSS_, ntHighlight, colorHighlight);
	replaceColors(colorsESS_, ntHighlight, colorHighlight);
	replaceColors(colorsVDS_, bsHighlight, colorHighlight);

	delete[] colorHighlight;

}

ADNArray<float> SEAdenitaVisualModel::calcPropertyColor(int colorSchemeIdx, float min, float max, float val) {

	ADNArray<float> color = ADNArray<float>(4);

	auto colorScheme = propertyColorSchemes_[colorSchemeIdx];

	if (val == FLT_MAX) { //if region is unbound

		color(0) = colorScheme(0, 0);
		color(1) = colorScheme(0, 1);
		color(2) = colorScheme(0, 2);
		color(3) = colorScheme(0, 3);
		
		return color;

	}

	auto numColors = colorScheme.GetNumElements();

	int idx1;
	int idx2;
	float fractBetween = 0;

	//Y = (X - A) / (B - A) * (D - C) + C
	double mappedVal = ADNAuxiliary::mapRange(val, min, max, 0, 1);

	if (mappedVal <= 0) {
		idx1 = idx2 = 0;
	}
	else if (mappedVal >= 1) {
		idx1 = idx2 = numColors - 1;
	}
	else {

		mappedVal = mappedVal * (numColors - 1);
		idx1 = int(mappedVal);
		idx2 = idx1 + 1;
		fractBetween = mappedVal - float(idx1);

	}

	color(0) = (colorScheme(idx2, 0) - colorScheme(idx1, 0)) * fractBetween + colorScheme(idx1, 0);
	color(1) = (colorScheme(idx2, 1) - colorScheme(idx1, 1)) * fractBetween + colorScheme(idx1, 1);
	color(2) = (colorScheme(idx2, 2) - colorScheme(idx1, 2)) * fractBetween + colorScheme(idx1, 2);
	color(3) = 1.0f;

	return color;

}


bool SEAdenitaVisualModel::getShowBasePairingFlag() const { return showBasePairing_; }
void SEAdenitaVisualModel::setShowBasePairingFlag(bool show) {
	
	if (this->showBasePairing_ == show) return;

	this->showBasePairing_ = show;

	SAMSON::requestViewportUpdate();

}

void SEAdenitaVisualModel::expandBounds(SBIAPosition3& bounds) const {

	// SAMSON Element generator pro tip: this function is called by SAMSON to determine the model's spatial bounds. 
	// When this function returns, the bounds interval vector should contain the visual model. 

}

void SEAdenitaVisualModel::collectAmbientOcclusion(const SBPosition3& boxOrigin, const SBPosition3& boxSize, unsigned int nCellsX, unsigned int nCellsY, unsigned int nCellsZ, float* ambientOcclusionData) {

	// SAMSON Element generator pro tip: this function is called by SAMSON to determine your model's influence on ambient occlusion.
	// Implement this function if you want your visual model to occlude other objects in ambient occlusion calculations.
	//
	// The ambientOcclusionData represents a nCellsX x nCellsY x nCellsZ grid of occlusion densities over the spatial region (boxOrigin, boxSize).
	// If your model represents geometry at position (x, y, z), then the occlusion density in corresponding grid nodes should be increased.
	//
	// Assuming x, y and z are given in length units (SBQuantity::length, SBQuantity::angstrom, etc.), the grid coordinates are:
	// SBQuantity::dimensionless xGrid = nCellsX * (x - boxOrigin.v[0]) / boxSize.v[0];
	// SBQuantity::dimensionless yGrid = nCellsY * (x - boxOrigin.v[1]) / boxSize.v[1];
	// SBQuantity::dimensionless zGrid = nCellsZ * (x - boxOrigin.v[2]) / boxSize.v[2];
	//
	// The corresponding density can be found at ambientOcclusionData[((int)zGrid.getValue() + 0)*nCellsY*nCellsX + ((int)yGrid.getValue() + 0)*nCellsX + ((int)xGrid.getValue() + 0)] (beware of grid bounds).
	// For higher-quality results, the influence of a point can be spread over neighboring grid nodes.

}

void SEAdenitaVisualModel::onBaseEvent(SBBaseEvent* baseEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle base events (e.g. when a node for which you provide a visual representation emits a base signal, such as when it is erased)

	const SBBaseEvent::Type eventType = baseEvent->getType();

	if (eventType == SBBaseEvent::HighlightingFlagChanged || eventType == SBBaseEvent::SelectionFlagChanged) {

		isHighlightFlagChangeRequested = true;
		//changeHighlightFlag();

	}
	else if (eventType == SBBaseEvent::VisibilityFlagChanged || eventType == SBBaseEvent::MaterialChanged) {

		isPrepareDiscreteScalesDimRequested = true;
		//prepareDiscreteScalesDim();
		//setScale(scale_);// , false);

	}

	//ADNLogger& logger = ADNLogger::GetLogger();
	//logger.Log(QString("onBaseEvent"));

}

void SEAdenitaVisualModel::onDocumentEvent(SBDocumentEvent* documentEvent) {

	// SAMSON Element generator pro tip: implement this function if you need to handle document events 

	const SBDocumentEvent::Type eventType = documentEvent->getType();
	SBNode* node = documentEvent->getAuxiliaryNode();
	if (!node) return;
	if (node->getProxy()->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return;

	// handle additiona and deletion of ADN nodes for updating the Adenita Visual Model

	if (eventType == SBDocumentEvent::StructuralModelAdded || eventType == SBDocumentEvent::StructuralModelRemoved) {

		if (node->getProxy()->getName() == "ADNPart") {

			if (eventType == SBDocumentEvent::StructuralModelRemoved)
				static_cast<ADNPart*>(node)->disconnectStructuralSignalFromSlot(this, SB_SLOT(&SEAdenitaVisualModel::onStructuralEvent));

			requestUpdate();

		}

	}

}

void SEAdenitaVisualModel::onStructuralEvent(SBStructuralEvent* structuralEvent) {
	
	// SAMSON Element generator pro tip: implement this function if you need to handle structural events (e.g. when a structural node for which you provide a visual representation is updated)
	
	const SBStructuralEvent::Type eventType = structuralEvent->getType();
	if (eventType == SBStructuralEvent::ParticlePositionChanged) {

		requestUpdate();
		return;

	}

	SBNode* node = structuralEvent->getAuxiliaryNode();
	if (!node) return;
	if (node->getProxy()->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return;

	const std::string nodeClassName = node->getProxy()->getName();

	// handle additiona and deletion of ADN nodes for updating the Adenita Visual Model

	if (eventType == SBStructuralEvent::ChainAdded || eventType == SBStructuralEvent::ChainRemoved) {

		if (nodeClassName == "ADNSingleStrand")
			requestUpdate();

	}
	else if (eventType == SBStructuralEvent::ResidueAdded || eventType == SBStructuralEvent::ResidueRemoved) {

		if (nodeClassName == "ADNNucleotide")
			requestUpdate();

	}
	else if (eventType == SBStructuralEvent::BackboneAdded || eventType == SBStructuralEvent::BackboneRemoved) {

		if (nodeClassName == "ADNBackbone")
			requestUpdate();

	}
	else if (eventType == SBStructuralEvent::SideChainAdded || eventType == SBStructuralEvent::SideChainRemoved) {

		if (nodeClassName == "ADNSidechain")
			requestUpdate();

	}
	else if (eventType == SBStructuralEvent::StructuralGroupAdded || eventType == SBStructuralEvent::StructuralGroupRemoved) {

		if (nodeClassName == "ADNBaseSegment" || nodeClassName == "ADNDoubleStrand" || nodeClassName == "ADNLoop" ||
			nodeClassName == "ADNCell" || nodeClassName == "ADNBasePair" || nodeClassName == "ADNSkipPair" || nodeClassName == "ADNLoopPair")
			requestUpdate();

	}

	//ADNLogger& logger = ADNLogger::GetLogger();
	//logger.Log(QString("onStructuralEvent"));
	//if (structuralEvent->getType() == SBStructuralEvent::MobilityFlagChanged) {
	//  prepareArraysNoTranstion();
	//  //ADNLogger& logger = ADNLogger::GetLogger();
	//  //logger.Log(QString("MobilityFlagChanged"));
	//}

	//logger.Log(structuralEvent->getTypeString(structuralEvent->getType()));

}

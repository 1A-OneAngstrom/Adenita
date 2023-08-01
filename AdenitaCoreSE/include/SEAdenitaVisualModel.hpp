#pragma once 

#include "SBMVisualModel.hpp"
#include "SBBaseEvent.hpp"
#include "SBDocumentEvent.hpp"
#include "SBStructuralEvent.hpp"

#include "ADNArray.hpp"
#include "ADNMixins.hpp"
#include "ADNPart.hpp"


class MSVColors;
class ADNNanorobot;

/// This class implements a visual model

class SB_EXPORT SEAdenitaVisualModel : public SBMVisualModel {

	SB_CLASS

public :

	/// \name Constructors and destructors
	//@{

	SEAdenitaVisualModel();																												///< Builds a visual model					
	SEAdenitaVisualModel(const SBNodeIndexer& nodeIndexer);																				///< Builds a visual model 
	virtual ~SEAdenitaVisualModel();																									///< Destructs the visual model

	//@}

	/// \name Serialization
	//@{

	virtual bool												isSerializable() const override;										///< Returns true when the class is serializable

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	//@}

	/// \name Topology
	//@{

	virtual	void												onErase() override;														///< Erases the visual model

	//@}

	/// \name Rendering options
	//@{

	enum class Scale {
		ATOMS_STICKS = 0,
		ATOMS_BALLS = 1,
		NUCLEOTIDES = 2,
		SINGLE_STRANDS = 3,
		DOUBLE_STRANDS = 4,
		OBJECTS = 5
	};

	float       												getScale() const;
	void												        setScale(float scale);
	bool														hasScaleRange() const;
	float														getDefaultScale() const;
	float														getMinimumScale() const;
	float														getMaximumScale() const;
	float														getScaleSingleStep() const;
	std::string													getScaleSuffix() const;

	int															getDiscreteScaleCount() const;
	int															getDiscreteScaleCurrentIndex() const;
	void														setDiscreteScaleCurrentIndex(const int index);
	std::string													getDiscreteScaleItemText(const int index) const;

	float       												getDimension() const;
	void												        setDimension(float dimension);
	bool														hasDimensionRange() const;
	float														getDefaultDimension() const;
	float														getMinimumDimension() const;
	float														getMaximumDimension() const;
	float														getDimensionSingleStep() const;
	std::string													getDimensionSuffix() const;

	int															getDiscreteDimensionCount() const;
	int															getDiscreteDimensionCurrentIndex() const;
	void														setDiscreteDimensionCurrentIndex(const int index);
	std::string													getDiscreteDimensionItemText(const int index) const;

	void														setVisibility(double layer);
	double														getVisibility() const;
	bool														hasVisibilityRange() const;
	double														getDefaultVisibility() const;
	double														getMinimumVisibility() const;
	double														getMaximumVisibility() const;
	double														getVisibilitySingleStep() const;
	std::string													getVisibilitySuffix() const;

	void														changePropertyColors(const int propertyIdx, const int colorSchemeIdx);

	int															getColorTypeCount() const;
	int															getColorTypeCurrentIndex() const;
	void														setColorTypeCurrentIndex(const int index);
	std::string													getColorTypeItemText(const int index) const;

	int															getPropertyColorSchemeCount() const;
	int															getPropertyColorSchemeCurrentIndex() const;
	void														setPropertyColorSchemeCurrentIndex(const int index);
	std::string													getPropertyColorSchemeItemText(const int index) const;

	void														setSingleStrandColorsCurrentIndex(const int index);
	int															getSingleStrandColorsCount() const;
	int															getSingleStrandColorsCurrentIndex() const;
	std::string													getSingleStrandColorsItemText(const int index) const;

	void														setNucleotideColorsCurrentIndex(const int index);
	int															getNucleotideColorsCount() const;
	int															getNucleotideColorsCurrentIndex() const;
	std::string													getNucleotideColorsItemText(const int index) const;

	void														setDoubleStrandColorsCurrentIndex(const int index);
	int															getDoubleStrandColorsCount() const;
	int															getDoubleStrandColorsCurrentIndex() const;
	std::string													getDoubleStrandColorsItemText(const int index) const;

	int															getHighlightCount() const;
	int															getHighlightCurrentIndex() const;
	void														setHighlightCurrentIndex(const int index);
	std::string													getHighlightItemText(const int index) const;

	unsigned int												getHighlightMinLength() const;
	void														setHighlightMinLength(unsigned int min);

	unsigned int												getHighlightMaxLength() const;
	void														setHighlightMaxLength(unsigned int max);

	bool														getNotWithinRange() const;
	void														setNotWithinRange(bool c);
	bool														getDefaultNotWithinRange() const;

	bool														getNotScaffold() const;
	void														setNotScaffold(bool c);
	bool														getDefaultNotScaffold() const;

	bool														getShowBasePairingFlag() const;
	void														setShowBasePairingFlag(bool show);

	//@}

	/// \name Update
	//@{

	void														requestUpdate();
	void														update();

	static void													requestVisualModelUpdate();												///< Requests an update of the Adenita visual model in the active document on the next display call

	//@}

	/// \name Display
	//@{

	virtual void												display() override;														///< Displays the visual model
	virtual void												displayForShadow() override;											///< Displays the visual model for shadow purposes
	virtual void												displayForSelection() override;											///< Displays the visual model for selection purposes

	virtual void												highlightNucleotides();

	virtual void												expandBounds(SBIAPosition3& bounds) const override;						///< Expands the bounds to make sure the visual model fits inside them
	virtual void												collectAmbientOcclusion(const SBPosition3& boxOrigin, const SBPosition3& boxSize, unsigned int nCellsX, unsigned int nCellsY, unsigned int nCellsZ, float* ambientOcclusionData) override;		///< To collect ambient occlusion data

	//@}

	/// \name Events
	//@{

	virtual void												onBaseEvent(SBBaseEvent* baseEvent);									///< Handles base events
	virtual void												onDocumentEvent(SBDocumentEvent* documentEvent);						///< Handles document events
	virtual void												onStructuralEvent(SBStructuralEvent* structuralEvent);					///< Handles structural events

	//@}

private:

	void														init();
	void														initAtoms(bool createIndex = true);
	void														initNucleotidesAndSingleStrands(bool createIndex = true);
	void														initDoubleStrands(bool createIndex = true);
	void														initDisplayIndices();

	ADNArray<unsigned int>										getAtomIndices();
	ADNArray<unsigned int>										getNucleotideIndices();
	ADNArray<unsigned int>										getBaseSegmentIndices();

	void														changeHighlightFlag();													///< scale 9: display polyhedron
	bool														isHighlightFlagChangeRequested = true;

	void														orderVisibility();

	void														setupPropertyColors();
	ADNArray<float>												calcPropertyColor(int colorSchemeIdx, float min, float max, float val);
	void														displayBasePairConnections(bool onlySelected);
	void														displayForDebugging();
	void														displayCircularDNAConnection();
	void														displayTags();

	void														prepareAtoms();
	void														prepareNucleotides();
	void														prepareSingleStrands();
	void														prepareDoubleStrands();

	void														displayNucleotides(bool forSelection = false);
	void														displaySingleStrands(bool forSelection = false);
	void														displayDoubleStrands(bool forSelection = false);

	void														prepareDiscreteScalesDim();
	bool														isPrepareDiscreteScalesDimRequested = true;

	void														prepareDimensions();

	void														displayTransition(bool forSelection); 

	void														prepareSticksToBalls(double iv);
	void														prepareBallsToNucleotides(double iv);
	void														prepareNucleotidesToSingleStrands(double iv);
	void														prepareSingleStrandsToDoubleStrands(double iv);
	void														prepareDoubleStrandsToObjects(double iv);

	void														prepare1Dto2D(double iv);
	void														prepare2Dto3D(double iv);
	void														prepare3D(double iv);

	void														emphasizeColors(ADNArray<float> & colors, std::vector<unsigned int> & indices, float r, float g, float b, float a);
	void														replaceColors(ADNArray<float> & colors, std::vector<unsigned int> & indices, float * color);

	// general display properties 

	ADNArray<float>												nucleotideEColor_;
  
	float														scale_ = 3.0f;
	float														dim_ = 3.0f;

	double														visibility_ = 0.99;

	ADNNanorobot*												nanorobot_{ nullptr };

	bool														isUpdateRequested = true;

	/// \name Transitional scale
	//@{

	unsigned int												nPositions_ = 0;
	unsigned int												nCylinders_ = 0;
	ADNArray<float>												colorsV_;
	ADNArray<float>												colorsE_;
	ADNArray<float>												positions_;
	ADNArray<float>												radiiV_;
	ADNArray<float>												radiiE_;
	ADNArray<unsigned int>										flags_;
	ADNArray<unsigned int>										nodeIndices_;
	ADNArray<unsigned int>										indices_;

	//@}

	/// \name Atom scale
	//@{

	unsigned int												nPositionsAtom_ = 0;
	unsigned int												nCylindersAtom_ = 0;
	ADNArray<float>												colorsVAtom_;
	ADNArray<float>												colorsEAtom_;
	ADNArray<float>												positionsAtom_;
	ADNArray<float>												radiiVAtom_;
	ADNArray<float>												radiiEAtom_;
	ADNArray<unsigned int>										flagsAtom_;
	ADNArray<unsigned int>										nodeIndicesAtom_;
	ADNArray<unsigned int>										indicesAtom_;

	//@}

	/// \name Nucleotide scale
	//@{

	unsigned int												nPositionsNt_ = 0;
	unsigned int												nCylindersNt_ = 0;
	ADNArray<float>												colorsVNt_;
	ADNArray<float>												colorsENt_;
	ADNArray<float>												positionsNt_;
	ADNArray<float>												radiiVNt_;
	ADNArray<float>												radiiENt_;
	ADNArray<unsigned int>										capDataNt_;
	ADNArray<unsigned int>										flagsNt_;
	ADNArray<unsigned int>										nodeIndicesNt_;
	ADNArray<unsigned int>										indicesNt_;

	//@}

	/// \name Single strand scale
	//@{

	ADNArray<float>												colorsVSS_;
	ADNArray<float>												colorsESS_;
	ADNArray<float>												radiiVSS_;
	ADNArray<float>												radiiESS_;

	//@}

	/// \name Double strand scale
	//@{

	unsigned int												nPositionsDS_ = 0;
	unsigned int												nCylindersDS_ = 0;
	ADNArray<float>												colorsVDS_;
	ADNArray<float>												positionsDS_;
	ADNArray<float>												radiiVDS_;
	ADNArray<unsigned int>										flagsDS_;
	ADNArray<unsigned int>										nodeIndicesDS_;

	//@}

	//2D
	ADNArray<float>												positionsNt2D_;
	ADNArray<float>												positionsNt1D_;

	std::map<ADNAtom*, unsigned int>							atomMap_;
	std::map<ADNNucleotide*, unsigned int>						ntMap_;
	std::map<ADNBaseSegment*, unsigned int>						bsMap_;
	std::map<unsigned int, unsigned>							atomNtIndexMap_;
	std::map<unsigned int, unsigned>							ntBsIndexMap_;

	std::map<ADNNucleotide*, float>								sortedNucleotidesByDist_;
	std::map<ADNSingleStrand*, float>							sortedSingleStrandsByDist_;
 
	// current arrays for being displayed (only spheres and cylinders)

	/// \name Property colors
	//@{

	std::vector<ADNArray<float>>								propertyColorSchemes_;
  
	enum class ColorType {
		REGULAR = 0,	///< default color map
		MELTTEMP = 1,	///< melting temperatures color map
		GIBBS = 2		///< Gibbs free energy color map
	};

	ColorType													curColorType_ = ColorType::REGULAR;

	std::map<ColorType, MSVColors*>								colors_;

	int															colorSchemeCurrentIndex = 0;
	int															nucleotideColorSchemeCurrentIndex = 0;
	int															singleStrandColorsCurrentIndex = 0;
	int															doubleStrandColorsCurrentIndex = 0;

	//@}

	/// \name Highlight options
	//@{

	enum class HighlightType {
		NONE = 0,
		CROSSOVERS = 1,
		GC = 2,
		TAGGED = 3,
		LENGTH = 4,
		NOBASE = 5,
		UNPAIRED = 6
	};

	void														setHighlight(const HighlightType highlightType);
	HighlightType												highlightType_ = HighlightType::NONE;
	void														updateEnabledFlagForHighlightAttributes();

	bool														showBasePairing_ = false;
	unsigned int												highlightMinLen_ = 0;
	unsigned int												highlightMaxLen_ = UINT_MAX;
	bool														notWithin_ = false;
	bool														notScaffold_ = true;

	//@}

};


SB_REGISTER_TARGET_TYPE(SEAdenitaVisualModel, "SEAdenitaVisualModel", "315CE7EA-009E-CE93-E00C-E921865953AB");
SB_DECLARE_BASE_TYPE(SEAdenitaVisualModel, SBMVisualModel);

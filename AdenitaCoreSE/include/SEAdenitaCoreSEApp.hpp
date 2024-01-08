#pragma once 

#include "SBDApp.hpp"

#include "SEAdenitaCoreSEAppGUI.hpp"
#include "ADNNanorobot.hpp"
#include "ADNSaveAndLoad.hpp"
#include "ADNNeighbors.hpp"
#include "DASDaedalus.hpp"
#include "DASBackToTheAtom.hpp"
#include "DASCadnano.hpp"
#include "DASComplexOperations.hpp"
#include "ADNConfig.hpp"
#include "SEAdenitaVisualModel.hpp"
#include "PIPrimer3.hpp"


/// This class implements the functionality of the app

// SAMSON Element generator pro tip: add non-GUI functionality in this class. The GUI functionality should go in the SEAdenitaCoreSEAppGUI class

class SB_EXPORT SEAdenitaCoreSEApp : public SBDApp {

  SB_CLASS

public:

	/// \name Constructors and destructors
	//@{

	SEAdenitaCoreSEApp();																												///< Constructs an app
	virtual ~SEAdenitaCoreSEApp();																										///< Destructs the app

	//@}

	/// \name GUI
	//@{

	SEAdenitaCoreSEAppGUI*										getGUI() const;															///< Returns a pointer to the GUI of the app

	//@}

	/// \name App
	//@{
	
	static SEAdenitaCoreSEApp*									adenitaApp;

	static SEAdenitaCoreSEApp*									getAdenitaApp();														///< Returns a pointer to the app's instance

	//@}

	/// \name Import/export
	//@{

	bool														loadPart(const QString& filename, SBDDocumentFolder* preferredFolder = nullptr);
	void														loadParts(const QString& filename, SBDDocumentFolder* preferredFolder = nullptr);

	void														SaveFile(QString filename, ADNPointer<ADNPart> part = nullptr);
	void														LoadPartWithDaedalus(QString filename, int minEdgeSize);
	bool														importFromCadnano(const QString& filename, SBDDocumentFolder* preferredFolder = nullptr);
	void														ExportToOxDNA(QString folder, ADNAuxiliary::OxDNAOptions options, CollectionMap<ADNPart> parts);
	void														ExportToSequenceList(QString filename, CollectionMap<ADNPart> parts);

	//@}

	void														SetScaffoldSequence(std::string seq);
	void														GenerateSequence(double gcCont, int maxContGs, bool overwrite = true);

	/// \name Visual model
	//@{

	static void													requestVisualModelUpdate();												///< Requests an update of the Adenita visual model in the active document on the next display call
	static void													resetVisualModel();														///< Resets the Adenita visual model in the active document if any, else creates one
	static void													resetVisualModel(SBNode* parent);										///< Resets the Adenita visual model in the active document if any, else creates one
	static SEAdenitaVisualModel*								getVisualModel();														///< Returns an Adenita visual model from the active document if any, else returns nullptr
	static SEAdenitaVisualModel*								getVisualModel(SBNode* parent);											///< Returns an Adenita visual model from the parent node if any, else returns nullptr
	static bool													addVisualModel(SBNode* parent);

	//@}

	void														setStartNucleotide();

	// Modifications

	void														BreakSingleStrand(bool fivePrimeMode = false);
	void														TwistDoubleHelix(CollectionMap<ADNDoubleStrand> dss, double angle);
	bool														CalculateBindingRegions(int oligoConc, int monovalentConc, int divalentConc);
	void														MergeComponents(ADNPointer<ADNPart> p1, ADNPointer<ADNPart> p2);
	void														MoveDoubleStrand(ADNPointer<ADNDoubleStrand> ds, ADNPointer<ADNPart> p);
	void														MoveSingleStrand(ADNPointer<ADNSingleStrand> ss, ADNPointer<ADNPart> p);
	void														CreateBasePair();

	// Debug

	void														AddNtThreeP(int numNt);
	void														CenterPart();
	void														LinearCatenanes(SBQuantity::length radius, SBPosition3 center, SBVector3 normal, int num);
	void														Kinetoplast(SBQuantity::length radius, SBPosition3 center, SBVector3 normal, int rows, int cols);
	void														TwistDoubleHelix();
	void														TestNeighbors();
	void														ImportFromOxDNA(const std::string& topoFile, const std::string& configFile);
	void														FromDataGraph(bool resetVisualModel = true);
	void														HighlightXOs();
	void														HighlightPosXOs();
	void														ExportToCanDo(const QString& filename);
	void														FixDesigns();

	virtual void												onDocumentEvent(SBDocumentEvent* documentEvent);						///< Handles document events
	virtual void												onStructuralEvent(SBStructuralEvent* structuralEvent);					///< Handles structural events
	void														ConnectToDocument();
	void														ConnectToDocument(SBDocument* doc);

	// Helper functions

	ADNNanorobot*												getNanorobot(SBDocument* document);
	ADNNanorobot*												GetNanorobot();
	static std::string											readScaffoldFilename(const std::string& filename);

	QStringList													getListOfPartNames();
	std::string													getUniquePartName(const std::string& partName);

	SBPosition3													getSnappedPosition(const SBPosition3& currentPosition);

	// Adding things to data graph

	void														addPartToDocument(ADNPointer<ADNPart> part, bool positionsData = false, SBFolder* preferredFolder = nullptr);
	void														addConformationToDocument(ADNPointer<ADNConformation> conf, SBFolder* preferredFolder = nullptr);
	void														AddLoadedPartToNanorobot(ADNPointer<ADNPart> part);

	virtual void												keyPressEvent(QKeyEvent* event);

	void														SetMod(bool m);

private:

	void														ConnectStructuralSignalSlots(ADNPointer<ADNPart> part);

	std::map<SBDocument*, ADNNanorobot*>						nanorobotMap;

	bool														mod_ = false;

};

SB_REGISTER_TARGET_TYPE(SEAdenitaCoreSEApp, "SEAdenitaCoreSEApp", "85DB7CE6-AE36-0CF1-7195-4A5DF69B1528");
SB_DECLARE_BASE_TYPE(SEAdenitaCoreSEApp, SBDApp);

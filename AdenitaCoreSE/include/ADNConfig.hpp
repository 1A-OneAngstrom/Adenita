#pragma once

#include <SAMSON.hpp>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"

#include <fstream>
#include <string>
#include <iostream>

#include <QFileInfo>
#include <QFileSystemWatcher>


//using namespace rapidjson;

typedef rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson::CrtAllocator> Val;

enum class SEConfigMode {

	DEBUG_LOG													= 0,
	DEBUG_NO_LOG												= 1

};

struct DebugOptions {

	double														minCutOff{ 0.0 };								///< [nm]
	double														maxCutOff{ 0.345 };								///< [nm]
	bool														display_nucleotide_basis{ false };
	bool														display_base_pairing{ false };
	//! custom values to use while developing
	bool														customBool{ false };
	double														customDouble{ 0.0 };
	int															customInt{ 0 };

};

class SB_EXPORT SEConfig : public QObject {

	Q_OBJECT

public:

	// singleton pattern
	~SEConfig() = default;
	// deleted functions should be public for better error message handling
	SEConfig(SEConfig const& c) = delete;
	void														operator=(SEConfig const& c) = delete;

	static SEConfig&											GetInstance();

	// visual_model settings
	float														min_melting_temp{ 25.0f };
	float														max_melting_temp{ 80.0f };
	float														min_gibbs_free_energy{ 0.0f };
	float														max_gibbs_free_energy{ -10000.0f };
	bool														interpolate_dimensions{ true };
	float														animation_step_size{ 0.0f };

	double														double_helix_V_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	double														nucleotide_E_Color[4]   = { 0.59f, 0.63f, 0.8f, 1.0f };
	double														double_strand_color[4]  = { 1.0f, 1.0f, 1.0f, 1.0f};
  
	double														adenine_color[4]  = { 102.0f / 255.0f, 194.0f / 255.0f, 165.0f / 255.0f, 1.0f };
	double														thymine_color[4]  = { 252.0f / 255.0f, 141.0f / 255.0f,  98.0f / 255.0f, 1.0f };
	double														guanine_color[4]  = { 231.0f / 255.0f, 138.0f / 255.0f, 195.0f / 255.0f, 1.0f };
	double														cytosine_color[4] = { 229.0f / 255.0f, 196.0f / 255.0f, 148.0f / 255.0f, 1.0f };

	float														nucleotide_V_radius{ 160.0f };
	float														nucleotide_E_radius{ 32.0f };
	float														base_pair_radius{ 1000.0f };
	unsigned int												num_staple_colors{ 12 };
	double														staple_colors[48] = {
		0.65f, 0.80f, 0.89f, 1.0f, 
		0.12f, 0.47f, 0.71f, 1.0f,
		0.70f, 0.87f, 0.54f, 1.0f,
		0.20f, 0.63f, 0.17f, 1.0f,
		0.98f, 0.60f, 0.60f, 1.0f,
		0.89f, 0.10f, 0.11f, 1.0f,
		0.99f, 0.75f, 0.44f, 1.0f,
		1.00f, 0.50f, 0.00f, 1.0f,
		0.79f, 0.70f, 0.84f, 1.0f,
		0.42f, 0.24f, 0.60f, 1.0f,
		1.00f, 1.00f, 0.60f, 1.0f,
		0.69f, 0.35f, 0.16f, 1.0f,
	};

	bool														automatic_camera{ true };					// adjusting camera according to dimension 
	bool														preview_editor{ true };						// adjusting camera according to dimension
	// structure prediction and algorithms
	bool														use_atomic_details{ false };				// generate atoms when creating DNA structures
	bool														detect_possible_crossovers{ false };
	float														crossover_distance_threshold{ 15.0f };		// angstroms
	float														crossover_angle_threshold{ 25.0f };			// angstroms
	float														dh_dist{ 4.0f };							// distance between double helices belonging to the same edge (angstroms)
	// logging and debugging
	bool														clear_log_file{ false };
	// toggle overlays and display options
	bool														display_possible_crossovers{ true };
	bool														show_overlay{ false };
	// toggle to automatically set the scaffold sequence when loading a part
	bool														auto_set_scaffold_sequence{ true };
	// scaffold config
	int															scaffType{ 0 };
	std::string													scaffCustomFilename;

	std::string													ntthal;										// the path to ntthal.exe from Primer

	// group general
	SEConfigMode												mode{ SEConfigMode::DEBUG_NO_LOG };			// which mode of the software active. debug_log, debug_no_log
	// algorithm to create a model from a mesh
	bool														custom_mesh_model{ false };

	// debug
	DebugOptions												debugOptions;

	// setters
	void														setAutoSetScaffoldSequence(bool b);
	void														setShowOverlay(bool b);
	void														setDisplayPossibleCrossovers(bool b);
	void														setClearLogFile(bool b);
	void														setInterpolateDimensions(bool b);
	void														setNtthalExe(const std::string& filename, bool write = true);
	void														setScaffType(int typ);
	void														setScaffCustomFilename(const std::string& filename, bool write = true);
	void														setCustomMeshModel(bool b);
	void														setUseAtomicDetailsFlag(bool b);

public slots:

	void														updateConfig();
	void														updateDebugConfig();

private:

	// private constructors to implement singleton
	SEConfig();
  
	const std::string											SAMSON_VERSION = std::to_string(SAMSON::getVersionNumber().getMajorVersionNumber()) + "." 
		+ std::to_string(SAMSON::getVersionNumber().getMinorVersionNumber()) + "." + std::to_string(SAMSON::getVersionNumber().getPatchVersionNumber());
	const std::string											DEBUG_CONFIGPATH = SAMSON::getUserDataPath() + "/" + SAMSON_VERSION + "/adenita_debug_settings.json";
	const std::string											DEFAULT_CONFIGPATH = SAMSON::getUserDataPath() + "/" + SAMSON_VERSION + "/adenita_settings.json";
	rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> setting_;
	rapidjson::Document											debugSetting_;
	QFileSystemWatcher											configFileWatcher_;
	QFileSystemWatcher											debugConfigFileWatcher_;

	void														loadConfig();
	void														loadDebugConfig();
	void														writeDoubleArray(rapidjson::Writer<rapidjson::StringBuffer> & writer, std::string key, double * arr, int length) const;
	void														readDoubleArray(Val & val, double * arr, int length);
	void														readFloatArray(Val& val, float* arr, int length);
	void														writeDocumentToJson() const;

};

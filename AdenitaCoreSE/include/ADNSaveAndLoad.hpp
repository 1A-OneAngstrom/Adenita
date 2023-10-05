#pragma once

#include "ADNNanorobot.hpp"
#include "ADNBasicOperations.hpp"
#include "ADNNeighbors.hpp"

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"

#include "SBCHeapExport.hpp"


namespace ADNLoader {

	// json
	SB_EXPORT ADNPointer<ADNPart> LoadPartFromJson(const std::string& filename);
	SB_EXPORT ADNPointer<ADNPart> LoadPartFromJson(rapidjson::Value& val, double versionValue);
	SB_EXPORT std::vector<ADNPointer<ADNPart>> LoadPartsFromJson(std::string filename);
	SB_EXPORT ADNPointer<ADNPart> LoadPartFromJsonLegacy(const std::string& filename);
	SB_EXPORT void SavePartToJson(ADNPointer<ADNPart> p, const std::string& filename);
	//! Writes a part to a string buffer for rapidjson
	SB_EXPORT void SavePartToJson(ADNPointer<ADNPart> p, rapidjson::Writer<rapidjson::StringBuffer>& s);
	SB_EXPORT void SaveNanorobotToJson(ADNNanorobot* nr, const std::string& filename);

	// pdb
	//SB_EXPORT ADNPointer<ADNPart> LoadPartFromPDB(const std::string& filename, int id = -1);

	// samson
	SB_EXPORT ADNPointer<ADNPart> GenerateModelFromDataGraph(SBNode* sn);
	SB_EXPORT ADNPointer<ADNPart> GenerateModelFromDataGraphParametrized(SBNode* sn, const SBQuantity::length& maxCutOff, const SBQuantity::length& minCutOff, double maxAngle);

	// oxdna
	SB_EXPORT void OutputToOxDNA(ADNPointer<ADNPart> part, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options);
	SB_EXPORT void OutputToOxDNA(CollectionMap<ADNPart> parts, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options);
	SB_EXPORT void SingleStrandsToOxDNA(CollectionMap<ADNSingleStrand> singleStrands, std::ofstream& outConf, std::ofstream& outTopo, const ADNAuxiliary::OxDNAOptions& options);
	SB_EXPORT void SignOutputFile(std::ofstream& output);
	SB_EXPORT std::pair<bool, ADNPointer<ADNPart>> InputFromOxDNA(const std::string& topoFile, const std::string& configFile);

	// CanDo
	SB_EXPORT void OutputToCanDo(ADNPointer<ADNPart> part, const std::string& filename);
	SB_EXPORT void OutputToCanDo(ADNNanorobot* nanorobot, const std::string& filename);

	// sequence list
	SB_EXPORT void OutputToCSV(CollectionMap<ADNPart> parts, const std::string& fname, const std::string& folder);

	// generic functions
	//! Populates base segments and double strands from nucleotides and single strands
	SB_EXPORT void BuildTopScales(ADNPointer<ADNPart> part);

	// generic functions
	//! Populates base segments and double strands from residues
	SB_EXPORT void BuildTopScalesParametrized(ADNPointer<ADNPart> part, const SBQuantity::length& maxCutOff, const SBQuantity::length& minCutOff, double maxAngle);

	template <typename T>
	struct Wrap {
		ADNPointer<T> elem_;
		int id_ = -1;
		int strandId_ = -1;
	};

	using NucleotideWrap = Wrap<ADNNucleotide>;

	template <class T>
	class SB_EXPORT ElementMap {

	public:

		std::pair<bool, ADNPointer<T>> Get(int idx) const {
			ADNPointer<T> nt = nullptr;
			bool success = false;
			if (ids_.find(idx) != ids_.end()) {
				nt = ids_.at(idx).elem_;
				success = true;
			}
			return std::make_pair(success, nt);
		};

		int GetIndex(ADNPointer<T> nt, int sId = -1) {
			int idx = -1;
			if (pointers_.find(nt()) != pointers_.end()) {
				Wrap<T> w = pointers_.at(nt());
				idx = w.id_;
			}
			else {
				idx = Insert(nt, sId);
			}
			return idx;
		};

		int Store(ADNPointer<T> elem, int id = -1, int sId = -1) {
			return Insert(elem, sId, id);
		}

	private:

		int Insert(ADNPointer<T> nt, int sId, int id = -1) {
			auto key = GetNextKey();
			if (id != -1) key = id;
			Wrap<T> w;
			w.elem_ = nt;
			w.id_ = key;
			w.strandId_ = sId;
			ids_[key] = w;
			pointers_.insert(std::make_pair(nt(), w));
			return key;
		};

		int GetNextKey() const {
			int lkey = 0;
			if (!ids_.empty()) lkey = ids_.rbegin()->first + 1;
			return lkey;
		};

		std::map<int, Wrap<T>> ids_;
		std::map<T*, Wrap<T>> pointers_;

	};

}

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
	SB_EXPORT [[nodiscard]] SBPointer<ADNPart> LoadPartFromJson(const std::string& filename);
	SB_EXPORT [[nodiscard]] SBPointer<ADNPart> LoadPartFromJson(rapidjson::Value& val, double versionValue);
	SB_EXPORT [[nodiscard]] std::vector<SBPointer<ADNPart>> LoadPartsFromJson(std::string filename);
	SB_EXPORT [[nodiscard]] SBPointer<ADNPart> LoadPartFromJsonLegacy(const std::string& filename);
	SB_EXPORT void SavePartToJson(SBPointer<ADNPart> p, const std::string& filename);
	//! Writes a part to a string buffer for rapidjson
	SB_EXPORT void SavePartToJson(SBPointer<ADNPart> p, rapidjson::Writer<rapidjson::StringBuffer>& s);
	SB_EXPORT void SaveNanorobotToJson(ADNNanorobot* nr, const std::string& filename);

	// pdb
	//SB_EXPORT SBPointer<ADNPart> LoadPartFromPDB(const std::string& filename, int id = -1);

	// samson
	SB_EXPORT [[nodiscard]] SBPointer<ADNPart> GenerateModelFromDataGraph(SBNode* sn);
	SB_EXPORT [[nodiscard]] SBPointer<ADNPart> GenerateModelFromDataGraphParametrized(SBNode* sn, const SBQuantity::length& maxCutOff, const SBQuantity::length& minCutOff, double maxAngle);

	// oxdna
	SB_EXPORT void OutputToOxDNA(SBPointer<ADNPart> part, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options);
	SB_EXPORT void OutputToOxDNA(SBPointerIndexer<ADNPart> parts, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options);
	SB_EXPORT void SingleStrandsToOxDNA(SBPointerIndexer<ADNSingleStrand> singleStrands, std::ofstream& outConf, std::ofstream& outTopo, const ADNAuxiliary::OxDNAOptions& options);
	SB_EXPORT void SignOutputFile(std::ofstream& output);
	SB_EXPORT std::pair<bool, SBPointer<ADNPart>> InputFromOxDNA(const std::string& topoFile, const std::string& configFile);

	// CanDo
	SB_EXPORT void OutputToCanDo(SBPointer<ADNPart> part, const std::string& filename);
	SB_EXPORT void OutputToCanDo(ADNNanorobot* nanorobot, const std::string& filename);
	SB_EXPORT void OutputToCanDo(const SBPointerIndexer<ADNSingleStrand>& singleStrands, const std::vector<SBPointerIndexer<ADNBaseSegment>>& baseSegmentsVector, const std::string& filename);

	// sequence list
	SB_EXPORT void OutputToCSV(SBPointerIndexer<ADNPart> parts, const std::string& fname, const std::string& folder);

	// generic functions
	//! Populates base segments and double strands from nucleotides and single strands
	SB_EXPORT void BuildTopScales(SBPointer<ADNPart> part);

	// generic functions
	//! Populates base segments and double strands from residues
	SB_EXPORT void BuildTopScalesParametrized(SBPointer<ADNPart> part, const SBQuantity::length& maxCutOff, const SBQuantity::length& minCutOff, double maxAngle);

	template <typename T>
	struct Wrap {
		SBPointer<T> elem_;
		int id_{ -1 };
		int strandId_{ -1 };
	};

	using NucleotideWrap = Wrap<ADNNucleotide>;

	template <class T>
	class SB_EXPORT ElementMap {

	public:

		[[nodiscard]] std::pair<bool, SBPointer<T>> Get(int idx) const {

			SBPointer<T> nt = nullptr;
			bool success = false;
			if (ids_.find(idx) != ids_.end()) {

				nt = ids_.at(idx).elem_;
				success = true;

			}
			return std::make_pair(success, nt);

		};

		[[nodiscard]] int GetIndex(SBPointer<T> nt, int sId = -1) {

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

		int Store(SBPointer<T> elem, int id = -1, int sId = -1) {

			return Insert(elem, sId, id);

		}

	private:

		int Insert(SBPointer<T> nt, int sId, int id = -1) {

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

		[[nodiscard]] int GetNextKey() const {

			int lkey = 0;
			if (!ids_.empty()) lkey = ids_.rbegin()->first + 1;
			return lkey;

		};

		std::map<int, Wrap<T>> ids_;
		std::map<T*, Wrap<T>> pointers_;

	};

}

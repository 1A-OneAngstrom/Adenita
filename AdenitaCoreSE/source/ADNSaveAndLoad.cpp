#include "ADNSaveAndLoad.hpp"

#include "ADNFrameAdapters.hpp"
#include "ADNGeometrySynchronization.hpp"
#include "ADNJsonValidation.hpp"
#include "ADNLogger.hpp"

#include "SBBackbone.hpp"
#include "SBSideChain.hpp"

#include <filesystem>

#undef foreach
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

SBPointer<ADNPart> ADNLoader::LoadPartFromJson(const std::string& filename) {

	FILE* fp = nullptr;
	try {
		std::filesystem::path filepath = std::filesystem::u8path(filename);
#ifdef _WIN32
		// convert to a wide string (UTF-8) to take care of special characters
		fp = _wfopen(filepath.c_str(), L"rb");
#else
		fp = fopen(filepath.c_str(), "rb");
#endif
	}
	catch (...) {
		return nullptr;
	}

	if (fp == nullptr) return nullptr;

	char readBuffer[131072];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document d;
	d.ParseStream(is);

	if (d.HasParseError() || !d.IsObject()) {

		fclose(fp);
		return nullptr;

	}

	// check for save version
	double versionValue = 0.0;
	if (rapidjson::Value* version = rapidjson::Pointer("/version").Get(d)) {
		if (!version->IsNumber()) {

			fclose(fp);
			return nullptr;

		}
		versionValue = version->GetDouble();
	}

	if (versionValue < 0.4) {

		fclose(fp);
		return LoadPartFromJsonLegacy(filename);

	}

	if (!ADNLoader::JsonValidation::isValidModernPart(d, versionValue)) {

		fclose(fp);
		return nullptr;

	}

	SBPointer<ADNPart> part = LoadPartFromJson(d, versionValue);

	fclose(fp);

	return part;

}

static void print_json_value(const rapidjson::Value& value) {

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	value.Accept(writer);

	std::cout << buffer.GetString() << std::endl;

}

namespace {

bool hasThreePrimeEndMetadata(ADNNucleotide::EndType endType) {

	return endType == ADNNucleotide::EndType::ThreePrime ||
		endType == ADNNucleotide::EndType::FiveAndThreePrime;

}

bool hasFivePrimeEndMetadata(ADNNucleotide::EndType endType) {

	return endType == ADNNucleotide::EndType::FivePrime ||
		endType == ADNNucleotide::EndType::FiveAndThreePrime;

}

ublas::vector<double> frameVectorToUblas(const ADNFrameUtils::Vec3& vector) {

	ublas::vector<double> result(3, 0.0);
	result[0] = vector.x;
	result[1] = vector.y;
	result[2] = vector.z;
	return result;

}

void logBrokenTopScaleTopology(const std::string& relation, SBPointer<ADNNucleotide> nucleotide) {

	ADNLogger::LogError("Top-scale construction found a nucleotide marked as " +
		nucleotide->getEndTypeString() + " with no " + relation + " link.");

}

bool shouldBreakTopScaleDoubleStrand(SBPointer<ADNNucleotide> nt,
	SBPointer<ADNNucleotide> pair,
	const ublas::vector<double>& e3,
	double turningThreshold) {

	bool breakDs = false;

	const SBPointer<ADNNucleotide> ntNext = nt->GetNext();
	if (ntNext != nullptr) {

		const auto& e3Next = ntNext->GetE3();
		const auto theta = ublas::inner_prod(e3, e3Next);
		if (theta < turningThreshold) breakDs = true;

	}
	else if (!hasThreePrimeEndMetadata(nt->getEndType())) {

		logBrokenTopScaleTopology("next", nt);
		breakDs = true;

	}

	if (pair != nullptr) {

		const SBPointer<ADNNucleotide> pairPrev = pair->GetPrev();
		if (pairPrev != nullptr) {

			const auto& e3Prev = pairPrev->GetE3();
			const auto theta = ublas::inner_prod(pair->GetE3(), e3Prev);
			if (theta < turningThreshold) breakDs = true;

		}
		else if (!hasFivePrimeEndMetadata(pair->getEndType())) {

			logBrokenTopScaleTopology("previous", pair);
			breakDs = true;

		}

	}

	return breakDs;

}

}

SBPointer<ADNPart> ADNLoader::LoadPartFromJson(rapidjson::Value& val, double versionValue) {

	if (!ADNLoader::JsonValidation::isValidModernPart(val, versionValue)) return nullptr;

	SBPointer<ADNPart> part = new ADNPart();

	rapidjson::Value& d = val;

	const std::string name = d["name"].GetString();
	part->setName(name);

	ElementMap<ADNNucleotide> nts;
	std::map<int, int> nexts;
	std::map<int, int> prevs;
	std::map<int, int> pairs;
	rapidjson::Value& strands = d["singleStrands"];
	for (rapidjson::Value::ConstMemberIterator itr = strands.MemberBegin(); itr != strands.MemberEnd(); ++itr) {

		SBPointer<ADNSingleStrand> ss = SBPointer<ADNSingleStrand>(new ADNSingleStrand());
		ss->setName(itr->value["chainName"].GetString());
		ss->setScaffoldFlag(itr->value["isScaffold"].GetBool());
		if (itr->value.HasMember("isCircular"))
			ss->setCircularFlag(itr->value["isCircular"].GetBool());

		const rapidjson::Value& val_nucleotides = itr->value["nucleotides"];
		for (rapidjson::Value::ConstMemberIterator itr2 = val_nucleotides.MemberBegin(); itr2 != val_nucleotides.MemberEnd(); ++itr2) {

			SBPointer<ADNNucleotide> nt = new ADNNucleotide();
			nt->Init();
			const std::string type = itr2->value["type"].GetString();
			nt->setNucleotideType(ADNModel::ResidueNameToType(type));
			nt->SetPosition(ADNAuxiliary::StringToSBPosition(itr2->value["position"].GetString()));
			nt->SetBackbonePosition(ADNAuxiliary::StringToSBPosition(itr2->value["backboneCenter"].GetString()));
			nt->SetSidechainPosition(ADNAuxiliary::StringToSBPosition(itr2->value["sidechainCenter"].GetString()));
			nt->SetE1(ADNAuxiliary::StringToUblasVector(itr2->value["e1"].GetString()));
			nt->SetE2(ADNAuxiliary::StringToUblasVector(itr2->value["e2"].GetString()));
			nt->SetE3(ADNAuxiliary::StringToUblasVector(itr2->value["e3"].GetString()));

			if (versionValue >= 0.5) {
				nt->setTag(itr2->value["tag"].GetString());
			}

			nts.Store(nt, std::stoi(itr2->name.GetString()));
			nexts.insert(std::make_pair(std::stoi(itr2->name.GetString()), itr2->value["next"].GetInt()));
			prevs.insert(std::make_pair(std::stoi(itr2->name.GetString()), itr2->value["prev"].GetInt()));
			pairs.insert(std::make_pair(std::stoi(itr2->name.GetString()), itr2->value["pair"].GetInt()));

		}

		const int f_id = itr->value["fivePrimeId"].GetInt();

		SBPointer<ADNNucleotide> nt = nts.Get(f_id).second;
		int currId = f_id;
		do {

			// pairing is done when parsing base segments
			nt = nts.Get(currId).second;
			part->RegisterNucleotideThreePrime(ss, nt);

			if (nexts.find(currId) != nexts.end()) {

				int nextId = nexts.at(currId);
				currId = nextId;

			}
			else break;

		} while (currId != -1);

		part->RegisterSingleStrand(ss);

	}

	ElementMap<ADNBaseSegment> bss;
	std::map<int, int> nextsBs;
	std::map<int, int> prevsBs;
	rapidjson::Value& doubleStrands = d["doubleStrands"];
	for (rapidjson::Value::ConstMemberIterator itr = doubleStrands.MemberBegin(); itr != doubleStrands.MemberEnd(); ++itr) {

		SBPointer<ADNDoubleStrand> ds = SBPointer<ADNDoubleStrand>(new ADNDoubleStrand());
		ds->SetInitialTwistAngle(itr->value["initialTwistAngle"].GetDouble());

		const rapidjson::Value& bases = itr->value["bases"];
		for (rapidjson::Value::ConstMemberIterator itr2 = bases.MemberBegin(); itr2 != bases.MemberEnd(); ++itr2) {

			SBPointer<ADNBaseSegment> bs = SBPointer<ADNBaseSegment>(new ADNBaseSegment());
			bs->SetPosition(ADNAuxiliary::StringToSBPosition(itr2->value["position"].GetString()));
			bs->SetE1(ADNAuxiliary::StringToUblasVector(itr2->value["e1"].GetString()));
			bs->SetE2(ADNAuxiliary::StringToUblasVector(itr2->value["e2"].GetString()));
			bs->SetE3(ADNAuxiliary::StringToUblasVector(itr2->value["e3"].GetString()));
			bs->SetNumber(itr2->value["number"].GetInt());

			// Load cells
			const rapidjson::Value& c = itr2->value["cell"];
			CellType type = CellType(c["type"].GetInt());
			if (type == CellType::BasePair) {

				SBPointer<ADNBasePair> bp_cell = new ADNBasePair();
				const rapidjson::Value& left = c["left"];
				const rapidjson::Value& right = c["right"];
				const int nt_id_left = left.GetInt();
				const int nt_id_right = right.GetInt();

				SBPointer<ADNNucleotide> ntLeft = nullptr;
				if (nt_id_left > -1) {

					ntLeft = nts.Get(nt_id_left).second;
					if (ntLeft != nullptr)
						ntLeft->SetBaseSegment(bs);

				}

				SBPointer<ADNNucleotide> ntRight = nullptr;
				if (nt_id_right > -1) {

					ntRight = nts.Get(nt_id_right).second;
					if (ntRight != nullptr)
						ntRight->SetBaseSegment(bs);

				}

				//if (!ntLeft.isValid()) {
				//
				//	const std::string msg = "[Adenita] Loading adn part: the left nucleotide (id=" + std::to_string(nt_id_left) + ") is NULL for right nucleotide id=" + std::to_string(nt_id_right) + ".";
				//	std::cerr << msg << std::endl;
				//	print_json_value(c);
				//
				//}
				//if (!ntRight.isValid()) {
				//
				//	const std::string msg = "[Adenita] Loading adn part: the right nucleotide (id=" + std::to_string(nt_id_right) + ") is NULL for left nucleotide id=" + std::to_string(nt_id_left) + ".";
				//	std::cerr << msg << std::endl;
				//	print_json_value(c);
				//
				//}

				bp_cell->AddPair(ntLeft, ntRight);
				bs->SetCell(bp_cell());

			}
			else if (type == CellType::SkipPair) {

				SBPointer<ADNSkipPair> sk_cell = new ADNSkipPair();
				bs->SetCell(sk_cell());

			}
			else if (type == CellType::LoopPair) {

				SBPointer<ADNLoopPair> lp_cell = new ADNLoopPair();

				const rapidjson::Value& left = c["leftLoop"];
				const rapidjson::Value& right = c["rightLoop"];

				if (left.HasMember("startNt")) {

					SBPointer<ADNLoop> leftLoop = SBPointer<ADNLoop>(new ADNLoop());
					lp_cell->SetLeftLoop(leftLoop);

					const int startNtId = left["startNt"].GetInt();
					const int endNtId = left["endNt"].GetInt();

					SBPointer<ADNNucleotide> startNt = nts.Get(startNtId).second;
					SBPointer<ADNNucleotide> lastNt = nts.Get(endNtId).second;
					if (startNt != nullptr) {

						SBPointer<ADNSingleStrand> ss = startNt->GetStrand();
						leftLoop->SetStart(startNt);
						leftLoop->SetEnd(lastNt);

						std::string nucleotides = left["nucleotides"].GetString();
						std::vector<int> ntVec = ADNAuxiliary::StringToVector(nucleotides);
						for (const auto& i : ntVec) {

							SBPointer<ADNNucleotide> nt = nts.Get(i).second;
							if (nt != nullptr) {

								leftLoop->AddNucleotide(nt);
								nt->SetBaseSegment(bs);

							}

						}

					}

				}

				if (right.HasMember("startNt")) {

					SBPointer<ADNLoop> rightLoop = SBPointer<ADNLoop>(new ADNLoop());
					lp_cell->SetRightLoop(rightLoop);

					const int startNtId = right["startNt"].GetInt();
					const int endNtId = right["endNt"].GetInt();

					SBPointer<ADNNucleotide> startNt = nts.Get(startNtId).second;
					SBPointer<ADNNucleotide> lastNt = nts.Get(endNtId).second;
					if (startNt != nullptr) {

						SBPointer<ADNSingleStrand> ss = startNt->GetStrand();
						rightLoop->SetStart(startNt);
						rightLoop->SetEnd(lastNt);

						std::string nucleotides = right["nucleotides"].GetString();
						std::vector<int> ntVec = ADNAuxiliary::StringToVector(nucleotides);
						for (const auto& i : ntVec) {

							SBPointer<ADNNucleotide> nt = nts.Get(i).second;
							if (nt != nullptr) {

								rightLoop->AddNucleotide(nt);
								nt->SetBaseSegment(bs);

							}

						}

					}

				}

				bs->SetCell(lp_cell());

			}

			bss.Store(bs, std::stoi(itr2->name.GetString()));
			nextsBs.insert(std::make_pair(std::stoi(itr2->name.GetString()), itr2->value["next"].GetInt()));
			prevsBs.insert(std::make_pair(std::stoi(itr2->name.GetString()), itr2->value["previous"].GetInt()));

		}

		const int bsStartId = itr->value["firstBaseSegment"].GetInt();
		const int bsEndId = itr->value["lastBaseSegment"].GetInt();

		SBPointer<ADNBaseSegment> bs = bss.Get(bsStartId).second;
		int currId = bsStartId;
		do {

			bs = bss.Get(currId).second;
			part->RegisterBaseSegmentEnd(ds, bs);

			if (nextsBs.find(currId) != nextsBs.end()) {

				int nextId = nextsBs.at(currId);
				currId = nextId;

			}
			else break;

		} while (currId != -1);

		part->RegisterDoubleStrand(ds);

	}

	ADNGeometrySynchronization::syncPartFramesFromGeometry(*part,
		ADNGeometrySynchronization::SyncReason::AfterUnserialization);

	return part;

}

std::vector<SBPointer<ADNPart>> ADNLoader::LoadPartsFromJson(std::string filename) {

	std::vector<SBPointer<ADNPart>> parts;

	FILE* fp = nullptr;
	try {
		std::filesystem::path filepath = std::filesystem::u8path(filename);
#ifdef _WIN32
		// convert to a wide string (UTF-8) to take care of special characters
		fp = _wfopen(filepath.c_str(), L"rb");
#else
		fp = fopen(filepath.c_str(), "rb");
#endif
	}
	catch (...) {
		return parts;
	}

	if (fp == nullptr) return parts;

	char readBuffer[131072];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document d;
	d.ParseStream(is);

	if (d.HasParseError() || !d.IsObject()) {

		fclose(fp);
		return parts;

	}

	// check for save version
	double versionValue = 0.0;
	if (rapidjson::Value* version = rapidjson::Pointer("/version").Get(d)) {
		if (!version->IsNumber()) {

			fclose(fp);
			return parts;

		}
		versionValue = version->GetDouble();
	}

	if (versionValue < 0.4) {

		fclose(fp);
		return parts;

	}

	if (!ADNLoader::JsonValidation::isValidPartsDocument(d, versionValue)) {

		fclose(fp);
		return parts;

	}

	rapidjson::Value& partList = d["parts"];
	for (rapidjson::Value::MemberIterator itr = partList.MemberBegin(); itr != partList.MemberEnd(); ++itr) {

		rapidjson::Value& v = itr->value;
		SBPointer<ADNPart> part = LoadPartFromJson(v, versionValue);
		if (part != nullptr) parts.push_back(part);

	}

	fclose(fp);
	return parts;

}

SBPointer<ADNPart> ADNLoader::LoadPartFromJsonLegacy(const std::string& filename) {

	// ids are reset since old format didn't use unique ids

	FILE* fp = nullptr;
	try {
		std::filesystem::path filepath = std::filesystem::u8path(filename);
#ifdef _WIN32
		// convert to a wide string (UTF-8) to take care of special characters
		fp = _wfopen(filepath.c_str(), L"rb");
#else
		fp = fopen(filepath.c_str(), "rb");
#endif
	}
	catch (...) {
		return nullptr;
	}

	if (fp == nullptr) return nullptr;

	char readBuffer[131072];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document d;
	d.ParseStream(is);

	if (d.HasParseError() || !d.IsObject()) {

		fclose(fp);
		return nullptr;

	}

	// check for save version
	double versionValue = 0.0;
	if (rapidjson::Value* version = rapidjson::Pointer("/version").Get(d)) {

		if (!version->IsNumber()) {

			fclose(fp);
			return nullptr;

		}
		versionValue = version->GetDouble();

	}
	else {

		fclose(fp);
		std::string msg = "Format is too old and can't be loaded with the current Adenita version";
		ADNLogger::LogError(msg);

		return nullptr;

	}

	if (!ADNLoader::JsonValidation::isValidLegacyPart(d, versionValue)) {

		fclose(fp);
		return nullptr;

	}

	SBPointer<ADNPart> part = new ADNPart();

	std::string name = d["name"].GetString();
	part->setName(name);
	//std::string position = d["position"].GetString();
	//part->SetPosition(ADNAuxiliary::StringToSBPosition(position));

	rapidjson::Value& strands = d["strands"];
	std::map<int, SBPointer<ADNSingleStrand>> origSingleStrandId;
	std::map<std::pair<int, int>, SBPointer<ADNNucleotide>> origNucleotideId;

	for (rapidjson::Value::ConstMemberIterator itr = strands.MemberBegin(); itr != strands.MemberEnd(); ++itr) {

		const int strandId = itr->value["id"].GetInt();
		SBPointer<ADNSingleStrand> ss = SBPointer<ADNSingleStrand>(new ADNSingleStrand());
		part->RegisterSingleStrand(ss);

		ss->setName(itr->value["chainName"].GetString());
		ss->setScaffoldFlag(itr->value["isScaffold"].GetBool());
		if (itr->value.HasMember("isCircular"))
			ss->setCircularFlag(itr->value["isCircular"].GetBool());

		const int fivePrimeId = itr->value["fivePrimeId"].GetInt();
		const int threePrimeId = itr->value["fivePrimeId"].GetInt();

		const rapidjson::Value& val_nucleotides = itr->value["nucleotides"];
		for (rapidjson::Value::ConstMemberIterator itr2 = val_nucleotides.MemberBegin(); itr2 != val_nucleotides.MemberEnd(); ++itr2) {

			SBPointer<ADNNucleotide> nt = new ADNNucleotide();
			nt->Init();
			part->RegisterNucleotideThreePrime(ss, nt);
			const std::string type = itr2->value["type"].GetString();
			nt->setNucleotideType(ADNModel::ResidueNameToType(type));
			nt->SetE1(ADNAuxiliary::StringToUblasVector(itr2->value["e1"].GetString()));
			nt->SetE2(ADNAuxiliary::StringToUblasVector(itr2->value["e2"].GetString()));
			nt->SetE3(ADNAuxiliary::StringToUblasVector(itr2->value["e3"].GetString()));
			nt->SetPosition(ADNAuxiliary::StringToSBPosition(itr2->value["position"].GetString()));

			nt->SetBackbonePosition(ADNAuxiliary::StringToSBPosition(itr2->value["backboneCenter"].GetString()));
			nt->SetSidechainPosition(ADNAuxiliary::StringToSBPosition(itr2->value["sidechainCenter"].GetString()));

			const int nucleotideId = itr2->value["id"].GetInt();

			if (nucleotideId == fivePrimeId) {

				nt->setEndType(ADNNucleotide::EndType::FivePrime);
				ss->SetFivePrime(nt);

			}
			if (nucleotideId == threePrimeId) {

				nt->setEndType(ADNNucleotide::EndType::ThreePrime);
				ss->SetThreePrime(nt);

			}

			std::pair<int, int> key = std::make_pair(strandId, nucleotideId);
			origNucleotideId.insert(std::make_pair(key, nt));

		}

		origSingleStrandId.insert(std::make_pair(strandId, ss));

	}

	// pair nucleotides
	for (rapidjson::Value::ConstMemberIterator itr = strands.MemberBegin(); itr != strands.MemberEnd(); ++itr) {

		int old_strand_id = itr->value["id"].GetInt();
		const rapidjson::Value& val_nucleotides = itr->value["nucleotides"];

		for (rapidjson::Value::ConstMemberIterator itr2 = val_nucleotides.MemberBegin(); itr2 != val_nucleotides.MemberEnd(); ++itr2) {

			int curr_id = itr2->value["id"].GetInt();
			int next_id = itr2->value["next"].GetInt();
			int prev_id = itr2->value["prev"].GetInt();

			const rapidjson::Value& val_pair_info = itr2->value["pair"];
			int ss_pair_id = val_pair_info["strandId"].GetInt();
			int nt_pair_id = val_pair_info["pairId"].GetInt();

			std::pair<int, int> keyCurr = std::make_pair(old_strand_id, curr_id);
			std::pair<int, int> keyPair = std::make_pair(ss_pair_id, nt_pair_id);
			std::pair<int, int> keyNext = std::make_pair(old_strand_id, next_id);
			std::pair<int, int> keyPrev = std::make_pair(old_strand_id, prev_id);

			SBPointer<ADNNucleotide> pairNt = nullptr;
			SBPointer<ADNNucleotide> nextNt = nullptr;
			SBPointer<ADNNucleotide> prevNt = nullptr;
			SBPointer<ADNNucleotide> nt = origNucleotideId.at(keyCurr);

			if (origNucleotideId.find(keyPair) != origNucleotideId.end()) {
				pairNt = origNucleotideId.at(keyPair);
			}
			if (origNucleotideId.find(keyNext) != origNucleotideId.end()) {
				nextNt = origNucleotideId.at(keyNext);
			}

			if (origNucleotideId.find(keyPrev) != origNucleotideId.end()) {
				prevNt = origNucleotideId.at(keyPrev);
			}

			nt->SetPair(pairNt);

		}

	}

	rapidjson::Value& doubleStrands = d["doubleStrands"];
	std::map<std::pair<int, int>, SBPointer<ADNBaseSegment>> origBssId;
	std::map<int, SBPointer<ADNDoubleStrand>> origDoubleStrandId;
	for (rapidjson::Value::ConstMemberIterator itr = doubleStrands.MemberBegin(); itr != doubleStrands.MemberEnd(); ++itr) {

		SBPointer<ADNDoubleStrand> ds = SBPointer<ADNDoubleStrand>(new ADNDoubleStrand());
		part->RegisterDoubleStrand(ds);

		int dsId = itr->value["id"].GetInt();
		ds->SetInitialTwistAngle(itr->value["initialTwistAngle"].GetDouble());

		const int size = itr->value["size"].GetInt();
		int bsStartId = itr->value["bsStartId"].GetInt();

		SBPointer<ADNBaseSegment> startBs = nullptr;
		for (int i = 0; i < size; ++i) {

			SBPointer<ADNBaseSegment> bs = new ADNBaseSegment();
			part->RegisterBaseSegmentEnd(ds, bs);
			if (i == 0) {
				startBs = bs;
			}

		}

		std::pair<int, int> bsKey = std::make_pair(dsId, bsStartId);
		origBssId.insert(std::make_pair(bsKey, startBs));
		origDoubleStrandId.insert(std::make_pair(dsId, ds));

	}

	rapidjson::Value& joints = d["joints"];
	std::map<int, ublas::vector<double>> jointPositions;
	for (rapidjson::Value::ConstMemberIterator itr = joints.MemberBegin(); itr != joints.MemberEnd(); ++itr) {

		int jId = itr->value["id"].GetInt();
		ublas::vector<double> pos = ADNAuxiliary::StringToUblasVector(itr->value["position"].GetString());
		jointPositions.insert(std::make_pair(jId, pos));

	}

	rapidjson::Value& bases = d["bases"];
	for (rapidjson::Value::ConstMemberIterator itr = bases.MemberBegin(); itr != bases.MemberEnd(); ++itr) {

		const int doubleStrandId = itr->value["double_strand"].GetInt();
		const auto& ds = origDoubleStrandId.at(doubleStrandId);

		const int sourceId = itr->value["source"].GetInt();
		const int targetId = itr->value["target"].GetInt();
		const auto& sourcePos = jointPositions.at(sourceId);
		const auto& targetPos = jointPositions.at(targetId);
		const int number = itr->value["number"].GetInt();

		SBPointer<ADNBaseSegment> bs = ds->GetNthBaseSegment(number);

		bs->SetNumber(number);
		bs->SetE2(ADNAuxiliary::StringToUblasVector(itr->value["normal"].GetString()));
		bs->SetE3(ADNAuxiliary::StringToUblasVector(itr->value["direction"].GetString()));
		bs->SetE1(ADNAuxiliary::StringToUblasVector(itr->value["u"].GetString()));
		bs->SetPosition(ADNAuxiliary::UblasVectorToSBPosition(sourcePos + targetPos) * 0.5);

		if (versionValue > 0.1) {

			// Load cells
			const rapidjson::Value& c = itr->value["cell"];
			CellType type = CellType(c["type"].GetInt());
			if (type == CellType::BasePair) {

				SBPointer<ADNBasePair> bp_cell = SBPointer<ADNBasePair>(new ADNBasePair());

				const rapidjson::Value& left = c["left"];
				const rapidjson::Value& right = c["right"];
				int ss_id_left = left["strand_id"].GetInt();
				int nt_id_left = left["nt_id"].GetInt();
				std::pair<int, int> leftKey = std::make_pair(ss_id_left, nt_id_left);

				int ss_id_right = right["strand_id"].GetInt();
				int nt_id_right = right["nt_id"].GetInt();
				std::pair<int, int> rightKey = std::make_pair(ss_id_right, nt_id_right);

				if (origNucleotideId.find(leftKey) != origNucleotideId.end()) {

					const auto& nt = origNucleotideId.at(leftKey);
					bp_cell->SetLeftNucleotide(nt);
					nt->SetBaseSegment(bs);

				}
				if (origNucleotideId.find(rightKey) != origNucleotideId.end()) {

					const auto& nt = origNucleotideId.at(rightKey);
					bp_cell->SetRightNucleotide(nt);
					nt->SetBaseSegment(bs);

				}

				bs->SetCell(bp_cell());

			}
			else if (type == CellType::LoopPair) {

				SBPointer<ADNLoopPair> lp_cell = SBPointer<ADNLoopPair>(new ADNLoopPair());

				const rapidjson::Value& left = c["left_loop"];
				const rapidjson::Value& right = c["right_loop"];

				if (left.HasMember("id")) {

					auto leftLoop = SBPointer<ADNLoop>(new ADNLoop());

					const rapidjson::Value& startNt = left["start_nt"];
					const rapidjson::Value& endNt = left["end_nt"];
					int id = left["id"].GetInt();
					int strandId = left["strand_id"].GetInt();
					int startNtId = startNt["nt_id"].GetInt();
					int startNtSsId = startNt["strand_id"].GetInt();
					std::pair<int, int> startKey = std::make_pair(startNtSsId, startNtId);
					int endNtId = endNt["nt_id"].GetInt();
					int endNtSsId = endNt["strand_id"].GetInt();
					std::pair<int, int> endKey = std::make_pair(endNtSsId, endNtId);

					SBPointer<ADNNucleotide> sNt;
					SBPointer<ADNNucleotide> eNt;
					if (origNucleotideId.find(startKey) != origNucleotideId.end()) {
						sNt = origNucleotideId.at(startKey);
					}
					if (origNucleotideId.find(endKey) != origNucleotideId.end()) {
						eNt = origNucleotideId.at(endKey);
					}

					auto order = ADNBasicOperations::OrderNucleotides(sNt, eNt);
					leftLoop->SetStart(order.first);
					leftLoop->SetEnd(order.second);

					const rapidjson::Value& nucleotides = left["nucleotides_list"];
					for (rapidjson::Value::ConstMemberIterator itr = nucleotides.MemberBegin(); itr != nucleotides.MemberEnd(); ++itr) {

						int nt_id = itr->value["nt_id"].GetInt();
						int ss_id = itr->value["strand_id"].GetInt();
						std::pair<int, int> ntKey = std::make_pair(ss_id, nt_id);
						if (origNucleotideId.find(ntKey) != origNucleotideId.end()) {

							const auto& nt = origNucleotideId.at(ntKey);
							leftLoop->AddNucleotide(nt);
							nt->SetBaseSegment(bs);

						}

					}

					lp_cell->SetLeftLoop(leftLoop);

				}

				if (right.HasMember("id")) {

					auto rightLoop = SBPointer<ADNLoop>(new ADNLoop());

					const rapidjson::Value& startNt = right["start_nt"];
					const rapidjson::Value& endNt = right["end_nt"];
					int id = right["id"].GetInt();
					int strandId = right["strand_id"].GetInt();
					int startNtId = startNt["nt_id"].GetInt();
					int startNtSsId = startNt["strand_id"].GetInt();
					std::pair<int, int> startKey = std::make_pair(startNtSsId, startNtId);
					int endNtId = endNt["nt_id"].GetInt();
					int endNtSsId = endNt["strand_id"].GetInt();
					std::pair<int, int> endKey = std::make_pair(endNtSsId, endNtId);

					SBPointer<ADNNucleotide> sNt;
					SBPointer<ADNNucleotide> eNt;
					if (origNucleotideId.find(startKey) != origNucleotideId.end()) {
						sNt = origNucleotideId.at(startKey);
					}
					if (origNucleotideId.find(endKey) != origNucleotideId.end()) {
						eNt = origNucleotideId.at(endKey);
					}

					auto order = ADNBasicOperations::OrderNucleotides(sNt, eNt);
					rightLoop->SetStart(order.first);
					rightLoop->SetEnd(order.second);

					const rapidjson::Value& nucleotides = right["nucleotides_list"];
					for (rapidjson::Value::ConstMemberIterator itr = nucleotides.MemberBegin(); itr != nucleotides.MemberEnd(); ++itr) {

						int nt_id = itr->value["nt_id"].GetInt();
						int ss_id = itr->value["strand_id"].GetInt();
						std::pair<int, int> ntKey = std::make_pair(ss_id, nt_id);
						if (origNucleotideId.find(ntKey) != origNucleotideId.end()) {

							const auto& nt = origNucleotideId.at(ntKey);
							rightLoop->AddNucleotide(nt);
							nt->SetBaseSegment(bs);

						}

					}

					lp_cell->SetRightLoop(rightLoop);

				}

				bs->SetCell(lp_cell());

			}

		}
		else {

			// base segment stores the nt as double strand
			SBPointer<ADNBasePair> bp_cell = SBPointer<ADNBasePair>(new ADNBasePair());
			const rapidjson::Value& val_nt = itr->value["nt"];
			int ss_id = val_nt["strandId"].GetInt();
			int nt_id = val_nt["ntId"].GetInt();
			std::pair<int, int> ntKey = std::make_pair(ss_id, nt_id);

			if (origNucleotideId.find(ntKey) != origNucleotideId.end()) {

				const auto& nt = origNucleotideId.at(ntKey);
				auto ntPair = nt->GetPair();
				bp_cell->SetLeftNucleotide(nt);
				bp_cell->SetRightNucleotide(ntPair);
				nt->SetBaseSegment(bs);
				if (ntPair != nullptr) ntPair->SetBaseSegment(bs);

			}

			bs->SetCell(bp_cell());

		}

	}

	fclose(fp);

	ADNGeometrySynchronization::syncPartFramesFromGeometry(*part,
		ADNGeometrySynchronization::SyncReason::AfterUnserialization);

	return part;

}

void ADNLoader::SavePartToJson(SBPointer<ADNPart> p, rapidjson::Writer<rapidjson::StringBuffer>& writer) {

	if (p != nullptr)
		ADNGeometrySynchronization::syncPartFramesFromGeometry(*p,
			ADNGeometrySynchronization::SyncReason::BeforeSerialization);

	writer.Key("name");
	writer.String(p->getName().c_str());

	auto doubleStrands = p->GetDoubleStrands();
	writer.Key("doubleStrands");
	writer.StartObject();

	ElementMap<ADNNucleotide> ntsMap;
	ElementMap<ADNBaseSegment> bssMap;

	SB_FOR(SBStructuralNode * p, doubleStrands) {

		SBPointer<ADNDoubleStrand> ds = static_cast<ADNDoubleStrand*>(p);

		std::string key = std::to_string(ds->getNodeIndex());
		writer.Key(key.c_str());
		writer.StartObject();

		writer.Key("firstBaseSegment");
		writer.Int(bssMap.GetIndex(ds->GetFirstBaseSegment()));

		writer.Key("lastBaseSegment");
		writer.Int(bssMap.GetIndex(ds->GetLastBaseSegment()));

		writer.Key("initialTwistAngle");
		writer.Double(ds->GetInitialTwistAngle());

		auto bases = ds->GetBaseSegments();
		writer.Key("bases");
		writer.StartObject();
		SB_FOR(SBStructuralNode * pair, bases) {

			SBPointer<ADNBaseSegment> bs = static_cast<ADNBaseSegment*>(pair);

			std::string key = std::to_string(bssMap.GetIndex(bs));
			writer.Key(key.c_str());
			writer.StartObject();

			writer.Key("number");
			writer.Int(bs->GetNumber());

			writer.Key("position");
			writer.String(ADNAuxiliary::SBPositionToString(bs->GetPosition()).c_str());

			int nextId = -1;
			if (bs->GetNext() != nullptr) nextId = bssMap.GetIndex(bs->GetNext());
			writer.Key("next");
			writer.Int(nextId);

			int prevId = -1;
			if (bs->GetPrev() != nullptr) prevId = bssMap.GetIndex(bs->GetPrev());
			writer.Key("previous");
			writer.Int(prevId);

			const ADNFrameUtils::Frame frame = ADNFrameAdapters::sanitizedFrame(*bs);

			writer.Key("e1");
			writer.String(ADNAuxiliary::UblasVectorToString(frameVectorToUblas(frame.e1)).c_str());

			writer.Key("e2");
			writer.String(ADNAuxiliary::UblasVectorToString(frameVectorToUblas(frame.e2)).c_str());

			writer.Key("e3");
			writer.String(ADNAuxiliary::UblasVectorToString(frameVectorToUblas(frame.e3)).c_str());

			writer.Key("cell");
			writer.StartObject();

			auto type = bs->GetCellType();
			writer.Key("type");
			writer.Int(static_cast<int>(type));

			SBPointer<ADNCell> cell = bs->GetCell();
			if (type == CellType::BasePair) {

				SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(cell());

				int idLeft = -1;
				if (bp->GetLeftNucleotide() != nullptr) idLeft = ntsMap.GetIndex(bp->GetLeftNucleotide());
				writer.Key("left");
				writer.Int(idLeft);

				int idRight = -1;
				if (bp->GetRightNucleotide() != nullptr) idRight = ntsMap.GetIndex(bp->GetRightNucleotide());
				writer.Key("right");
				writer.Int(idRight);

			}
			else if (type == CellType::LoopPair) {

				SBPointer<ADNLoopPair> lp = static_cast<ADNLoopPair*>(cell());

				SBPointer<ADNLoop> left = lp->GetLeftLoop();
				writer.Key("leftLoop");
				writer.StartObject();

				if (left != nullptr) {

					int startNtId = -1;
					if (left->GetStart() != nullptr) startNtId = ntsMap.GetIndex(left->GetStart());
					writer.Key("startNt");
					writer.Int(startNtId);

					int endNtId = -1;
					if (left->GetEnd() != nullptr) endNtId = ntsMap.GetIndex(left->GetEnd());
					writer.Key("endNt");
					writer.Int(endNtId);

					auto nts = left->GetNucleotides();
					std::vector<int> ntList;
					writer.Key("nucleotides");
					SB_FOR(SBStructuralNode * n, nts) {

						SBPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(n);
						ntList.push_back(ntsMap.GetIndex(nt));

					}

					std::string str = ADNAuxiliary::VectorToString(ntList);
					writer.String(str.c_str());

				}

				writer.EndObject();  // end left loop

				SBPointer<ADNLoop> right = lp->GetRightLoop();
				writer.Key("rightLoop");
				writer.StartObject();

				if (right != nullptr) {

					int startNtId = -1;
					if (right->GetStart() != nullptr) startNtId = ntsMap.GetIndex(right->GetStart());
					writer.Key("startNt");
					writer.Int(startNtId);

					int endNtId = -1;
					if (right->GetEnd() != nullptr) endNtId = ntsMap.GetIndex(right->GetEnd());
					writer.Key("endNt");
					writer.Int(endNtId);

					auto nts = right->GetNucleotides();
					std::vector<int> ntList;
					writer.Key("nucleotides");
					SB_FOR(SBStructuralNode * n, nts) {

						SBPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(n);
						ntList.push_back(ntsMap.GetIndex(nt));

					}

					std::string strRight = ADNAuxiliary::VectorToString(ntList);
					writer.String(strRight.c_str());

				}

				writer.EndObject();  // end right loop

			}

			writer.EndObject();  // end cell

			writer.EndObject();  // end base segment

		}

		writer.EndObject();  // end bases

		writer.EndObject();  // end of ds

	}
	writer.EndObject();  // end of double strands

	auto singleStrands = p->GetSingleStrands();
	writer.Key("singleStrands");
	writer.StartObject();
	SB_FOR(SBStructuralNode * p, singleStrands) {

		SBPointer<ADNSingleStrand> ss = static_cast<ADNSingleStrand*>(p);

		std::string key = std::to_string(ss->getNodeIndex());
		writer.Key(key.c_str());
		writer.StartObject();

		writer.Key("chainName");
		writer.String(ss->getName().c_str());

		writer.Key("isScaffold");
		writer.Bool(ss->IsScaffold());

		writer.Key("isCircular");
		writer.Bool(ss->IsCircular());

		writer.Key("fivePrimeId");
		writer.Int(ntsMap.GetIndex(ss->GetFivePrime()));

		writer.Key("threePrimeId");
		writer.Int(ntsMap.GetIndex(ss->GetThreePrime()));

		auto nucleotides = ss->GetNucleotides();
		writer.Key("nucleotides");
		writer.StartObject();
		SB_FOR(SBStructuralNode * pair, nucleotides) {

			SBPointer<ADNNucleotide> nt = static_cast<ADNNucleotide*>(pair);

			std::string key = std::to_string(ntsMap.GetIndex(nt));
			writer.Key(key.c_str());
			writer.StartObject();

			writer.Key("type");
			//char t = ADNModel::ResidueNameToType(nt->getNucleotideType());
			//std::string typ = std::string(&t, 0, 1);
			std::string typ = nt->getOneLetterNucleotideTypeString();
			writer.String(typ.c_str());

			int pairId = -1;
			if (nt->GetPair() != nullptr) pairId = ntsMap.GetIndex(nt->GetPair());
			writer.Key("pair");
			writer.Int(pairId);

			int prevId = -1;
			if (nt->GetPrev() != nullptr) prevId = ntsMap.GetIndex(nt->GetPrev());
			writer.Key("prev");
			writer.Int(prevId);

			int nextId = -1;
			if (nt->GetNext() != nullptr) nextId = ntsMap.GetIndex(nt->GetNext());
			writer.Key("next");
			writer.Int(nextId);

			const ADNFrameUtils::Frame frame = ADNFrameAdapters::sanitizedFrame(*nt);

			writer.Key("e1");
			writer.String(ADNAuxiliary::UblasVectorToString(frameVectorToUblas(frame.e1)).c_str());

			writer.Key("e2");
			writer.String(ADNAuxiliary::UblasVectorToString(frameVectorToUblas(frame.e2)).c_str());

			writer.Key("e3");
			writer.String(ADNAuxiliary::UblasVectorToString(frameVectorToUblas(frame.e3)).c_str());

			writer.Key("position");
			writer.String(ADNAuxiliary::SBPositionToString(nt->GetPosition()).c_str());

			writer.Key("backboneCenter");
			writer.String(ADNAuxiliary::SBPositionToString(nt->GetBackbonePosition()).c_str());

			writer.Key("sidechainCenter");
			writer.String(ADNAuxiliary::SBPositionToString(nt->GetSidechainPosition()).c_str());

			writer.Key("tag");
			writer.String(nt->getTag().c_str());

			writer.EndObject();  // end nt

		}

		writer.EndObject();  // end nucleotides

		writer.EndObject();  // end ss

	}

	writer.EndObject();

}

void ADNLoader::SaveNanorobotToJson(ADNNanorobot* nr, const std::string& filename) {

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("version");
	writer.Double(ADNConstants::JSON_FORMAT_VERSION);

	auto parts = nr->GetParts();
	writer.Key("parts");
	writer.StartObject();
	SB_FOR(SBPointer<ADNPart> p, parts) {

		writer.Key(p->getName().c_str());
		writer.StartObject();
		SavePartToJson(p, writer);
		writer.EndObject();  // end part

	}

	writer.EndObject();  // end parts

	writer.EndObject();  // end json document

	QIODevice::OpenModeFlag mode = QIODevice::WriteOnly;

	QFile file(QString::fromStdString(filename));
	if (file.open(mode)) {

		QTextStream out(&file);

		out << s.GetString();

		file.close();

	}

}

SBPointer<ADNPart> ADNLoader::GenerateModelFromDataGraph(SBNode* n) {

	SBPointer<ADNPart> part = new ADNPart();

	SBNodeIndexer nodes;
	n->getNodes(nodes, SBNode::Chain);

	SB_FOR(SBNode * node, nodes) {

		SBPointer<SBChain> chain = static_cast<SBChain*>(node);

		SBPointer<ADNSingleStrand> ss = new ADNSingleStrand();
		ss->SetDefaultName();
		part->RegisterSingleStrand(ss);

		SBPointerList<SBStructuralNode> children = *chain->getChildren();
		SBPosition3 prevPos;

		SB_FOR(SBStructuralNode * n, children) {

			SBPointer<SBResidue> res = static_cast<SBResidue*>(n);
			if (!res->isNucleicAcid()) continue;

			SBPosition3 pos = SBPosition3();
			SBPosition3 bbPos = SBPosition3();
			SBPosition3 scPos = SBPosition3();
			int count = 0;
			SBPointer<SBBackbone> bb = res->getBackbone();
			SBPointerList<SBStructuralNode> bbAtoms = *bb->getChildren();
			SB_FOR(SBStructuralNode * at, bbAtoms) {

				if (at->getType() == SBNode::Atom) {

					SBPointer<SBAtom> atom = static_cast<SBAtom*>(at);
					pos += atom->getPosition();
					bbPos += atom->getPosition();
					++count;

				}

			}
			bbPos /= count;

			int scCount = 0;
			SBPointer<SBSideChain> sc = res->getSideChain();
			SBPointerList<SBStructuralNode> scAtoms = *sc->getChildren();
			ublas::matrix<double> scPoints(0, 3);
			SB_FOR(SBStructuralNode * at, scAtoms) {

				if (at->getType() == SBNode::Atom) {

					SBPointer<SBAtom> atom = static_cast<SBAtom*>(at);
					pos += atom->getPosition();
					scPos += atom->getPosition();
					++count;
					++scCount;
					// fill matrix
					ublas::vector<double> r = ADNAuxiliary::SBPositionToUblas(atom->getPosition());
					ADNVectorMath::AddRowToMatrix(scPoints, r);

				}

			}
			pos /= count;
			scPos /= scCount;

			// Calculate local axis

			// to calculate e3 we fit the sidechain atoms to a plain, and set e3 as the normal to that plain in the 5' -> 3' direction
			ublas::vector<double> e3 = ADNVectorMath::CalculatePlane(scPoints);

			// we only need C1 position and e3 to find pair:
			// - check neighbors
			// -(e3, e3n) needs to be negative
			// - |(e3, e3n)| amounts to just a few grades:
			//   --> Build planes with C1 and C1n as center, check (C1n-C1) vector is almost contained in the plain
			//   --> Check (C1n-C1) doesn't intersect backbone
			//SBVector3 e3SB = (pos - prevPos).normalizedVersion();
			//SBVector3 e2SB = (scPos - bbPos).normalizedVersion();

			//ublas::vector<double> e3 = ADNAuxiliary::SBVectorToUblasVector(e3SB);
			//ublas::vector<double> e2 = ADNAuxiliary::SBVectorToUblasVector(e2SB);
			//ublas::vector<double> e1 = ADNVectorMath::CrossProduct(e2, e3);

			SBPointer<ADNNucleotide> nt = new ADNNucleotide();
			nt->Init();
			nt->SetPosition(pos);
			nt->SetBackbonePosition(bbPos);
			nt->SetSidechainPosition(scPos);
			nt->SetE3(e3);
			//nt->SetE2(e2);
			//nt->SetE1(e1);
			nt->setNucleotideType(res->getResidueType());

			part->RegisterNucleotideThreePrime(ss, nt);
			prevPos = pos;

		}

		// fix directionality of first nucleotide
		/*if (ss->getNumberOfNucleotides() > 1) {
		  auto fPrime = ss->GetFivePrime();
		  SBVector3 newE3 = (fPrime->GetNext()->GetPosition() - fPrime->GetPosition()).normalizedVersion();
		  ublas::vector<double> e3 = ADNAuxiliary::SBVectorToUblasVector(newE3);
		  fPrime->SetE3(e3);
		  ublas::vector<double> e1 = ADNVectorMath::CrossProduct(fPrime->GetE2(), e3);
		  fPrime->SetE1(e1);
		}*/

		if (ss->getNumberOfNucleotides() == 0) {

			// delete single strands since it's empty
			part->DeregisterSingleStrand(ss);
			ss->erase();
			ss.deleteReferenceTarget();

		}

	}

	BuildTopScales(part);

	return part;
}

SBPointer<ADNPart> ADNLoader::GenerateModelFromDataGraphParametrized(SBNode* sn, const SBQuantity::length& maxCutOff, const SBQuantity::length& minCutOff, double maxAngle) {

	SBPointer<ADNPart> part = new ADNPart();

	SBNodeIndexer nodes;
	sn->getNodes(nodes, SBNode::Chain);

	SB_FOR(SBNode * node, nodes) {

		SBPointer<SBChain> chain = static_cast<SBChain*>(node);

		SBPointer<ADNSingleStrand> ss = new ADNSingleStrand();
		ss->SetDefaultName();
		part->RegisterSingleStrand(ss);

		SBPointerList<SBStructuralNode> children = *chain->getChildren();
		SBPosition3 prevPos;

		ublas::vector<double> e3avg(3, 0.0);

		SB_FOR(SBStructuralNode * n, children) {

			SBPointer<SBResidue> res = static_cast<SBResidue*>(n);
			if (!res->isNucleicAcid()) continue;

			SBPosition3 pos = SBPosition3();
			SBPosition3 bbPos = SBPosition3();
			SBPosition3 scPos = SBPosition3();
			int count = 0;
			SBPointer<SBBackbone> bb = res->getBackbone();
			SBPointerList<SBStructuralNode> bbAtoms = *bb->getChildren();
			SB_FOR(SBStructuralNode * at, bbAtoms) {

				if (at->getType() == SBNode::Atom) {

					SBPointer<SBAtom> atom = static_cast<SBAtom*>(at);
					pos += atom->getPosition();
					bbPos += atom->getPosition();
					++count;

				}

			}
			bbPos /= count;
			int scCount = 0;

			SBPointer<SBSideChain> sc = res->getSideChain();
			SBPointerList<SBStructuralNode> scAtoms = *sc->getChildren();
			SB_FOR(SBStructuralNode * at, scAtoms) {

				if (at->getType() == SBNode::Atom) {

					SBPointer<SBAtom> atom = static_cast<SBAtom*>(at);
					pos += atom->getPosition();
					scPos += atom->getPosition();
					++count;
					++scCount;

				}

			}
			pos /= count;
			scPos /= scCount;

			// Calculate local axis
			const SBVector3 e3SB = (pos - prevPos).normalizedVersion();
			const SBVector3 e2SB = -(scPos - bbPos).normalizedVersion();

			ublas::vector<double> e3 = ADNAuxiliary::SBVectorToUblasVector(e3SB);
			e3avg += e3;
			ublas::vector<double> e2 = ADNAuxiliary::SBVectorToUblasVector(e2SB);
			//ublas::vector<double> e1 = ADNVectorMath::CrossProduct(e2, e3);

			SBPointer<ADNNucleotide> nt = new ADNNucleotide();
			nt->Init();
			nt->SetPosition(pos);
			//nt->SetE3(e3);
			nt->SetE2(e2);
			//nt->SetE1(e1);
			nt->setNucleotideType(res->getResidueType());

			part->RegisterNucleotideThreePrime(ss, nt);
			prevPos = pos;

		}

		// fix directionality of nucleotides
		if (ss->getNumberOfNucleotides() > 1) {

			ublas::vector<double> e3 = e3avg / ss->getNumberOfNucleotides();
			e3 /= ublas::norm_2(e3);
			auto nucleotides = ss->GetNucleotides();
			SB_FOR(SBPointer<ADNNucleotide> nt, nucleotides) {

				const auto& e2 = nt->GetE2();
				ublas::vector<double> e1 = ADNVectorMath::CrossProduct(e2, e3);
				nt->SetE3(e3);
				nt->SetE1(e1);

			}

		}
		else if (ss->getNumberOfNucleotides() == 0) {

			// delete single strands since it's empty
			part->DeregisterSingleStrand(ss);
			ss->erase();
			ss.deleteReferenceTarget();

		}

		//if (ss->getNumberOfNucleotides() > 1) {
		//  auto fPrime = ss->GetFivePrime();
		//  SBVector3 newE3 = (fPrime->GetNext()->GetPosition() - fPrime->GetPosition()).normalizedVersion();
		//  ublas::vector<double> e3 = ADNAuxiliary::SBVectorToUblasVector(newE3);
		//  fPrime->SetE3(e3);
		//  ublas::vector<double> e1 = ADNVectorMath::CrossProduct(fPrime->GetE2(), e3);
		//  fPrime->SetE1(e1);
		//}
		//else if (ss->getNumberOfNucleotides() == 0) {
		//  // delete single strands since it's empty
		//  part->DeregisterSingleStrand(ss);
		//}

	}

	BuildTopScalesParametrized(part, maxCutOff, minCutOff, maxAngle);

	return part;

}

void ADNLoader::OutputToOxDNA(SBPointer<ADNPart> part, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options) {

	std::ofstream outConf(std::filesystem::u8path(folder + "/" + "config.conf"));
	std::ofstream outTopo(std::filesystem::u8path(folder + "/" + "topo.top"));

	auto singleStrands = part->GetSingleStrands();
	SingleStrandsToOxDNA(singleStrands, outConf, outTopo, options);

	outConf.close();
	outTopo.close();

}

void ADNLoader::OutputToOxDNA(SBPointerIndexer<ADNPart> parts, const std::string& folder, const ADNAuxiliary::OxDNAOptions& options) {

	SBPointerIndexer<ADNSingleStrand> singleStrands;
	SB_FOR(SBPointer<ADNPart> p, parts) {

		auto sss = p->GetSingleStrands();
		SB_FOR(SBPointer<ADNSingleStrand> ss, sss)
			singleStrands.addReferenceTarget(ss());

	}

	std::ofstream outConf(std::filesystem::u8path(folder + "/" + "config.conf"));
	std::ofstream outTopo(std::filesystem::u8path(folder + "/" + "topo.top"));

	SingleStrandsToOxDNA(singleStrands, outConf, outTopo, options);

	outConf.close();
	outTopo.close();

}

void ADNLoader::SingleStrandsToOxDNA(SBPointerIndexer<ADNSingleStrand> singleStrands, std::ofstream& outConf, std::ofstream& outTopo, const ADNAuxiliary::OxDNAOptions& options) {

	// config file header
	const std::string timeStep = "0";
	const std::string boxSizeX = std::to_string(options.boxSizeX_);
	const std::string boxSizeY = std::to_string(options.boxSizeY_);
	const std::string boxSizeZ = std::to_string(options.boxSizeZ_);
	const auto energies = std::tuple<std::string, std::string, std::string>("0.0", "0.0", "0.0");

	outConf << "t = " + timeStep << std::endl;
	outConf << "b = " + boxSizeX + " " + boxSizeY + " " + boxSizeZ << std::endl;
	outConf << "E = " + std::get<0>(energies) + " " + std::get<1>(energies) + " " + std::get<2>(energies) << std::endl;

	// topology file header
	size_t numNt = 0;
	SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) numNt += ss->getNumberOfNucleotides();
	const std::string numberNucleotides = std::to_string(numNt);
	const std::string numberStrands = std::to_string(singleStrands.size());

	outTopo << numberNucleotides << " " << numberStrands << std::endl;

	// config file: velocity and angular velocity are zero for all
	const std::string L = "0 0 0";
	const std::string v = "0 0 0";

	// we assign new ids
	unsigned int strandId = 1;
	unsigned int ntId = 0;
	SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

		SBPointer<ADNNucleotide> nt = ss->GetFivePrime();
		// calculate five prime and three prime ids
		const int numNt = ss->getNumberOfNucleotides();
		const int fivePrimeId = ntId;
		const int threePrimeId = fivePrimeId + numNt;

		do {

			// config file info
			SBPosition3 pos = nt->GetPosition();
			ublas::vector<double> bbVector = nt->GetE2() * (-1.0);
			ublas::vector<double> normal = nt->GetE1() * (-1.0);

			// box size is in nm, so position of nt has to be too
			const std::string positionVector = std::to_string(pos[0].getValue() / 1000.0) + " " + std::to_string(pos[1].getValue() / 1000.0) + " " + std::to_string(pos[2].getValue() / 1000.0);
			const std::string backboneBaseVector = std::to_string(bbVector[0]) + " " + std::to_string(bbVector[1]) + " " + std::to_string(bbVector[2]);
			const std::string normalVector = std::to_string(normal[0]) + " " + std::to_string(normal[1]) + " " + std::to_string(normal[2]);

			outConf << positionVector + " " + backboneBaseVector + " " + normalVector + " " + v + " " + L << std::endl;

			// topology file info
			std::string base = nt->getOneLetterNucleotideTypeString();
			if (base == "N") base = "R";  // oxDNA uses R for random

			std::string threePrime = "-1";
			auto ntPrev = nt->GetPrev(true);
			if (ntPrev != nullptr) {

				if (ntPrev->getEndType() == ADNNucleotide::EndType::ThreePrime) {

					// ? fix for circular DNA
					if (threePrimeId > 0) threePrime = std::to_string(threePrimeId - 1);
					else threePrime = std::to_string(threePrimeId);

				}
				else threePrime = std::to_string(ntId - 1);

			}

			std::string fivePrime = "-1";
			auto ntNext = nt->GetNext(true);
			if (ntNext != nullptr) {

				if (ntNext->getEndType() == ADNNucleotide::EndType::FivePrime) fivePrime = std::to_string(fivePrimeId);
				else fivePrime = std::to_string(ntId + 1);

			}

			outTopo << std::to_string(strandId) + " " + base + " " + threePrime + " " + fivePrime << std::endl;

			nt = nt->GetNext();
			ntId++;

		} while (nt != nullptr);

		++strandId;

	}

}

void ADNLoader::SignOutputFile(std::ofstream& output) {

	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %I:%M:%S", timeinfo);
	std::string str(buffer);

	output << "## File created with Adenita on " + str + "\n";

}

ADNLoader::OxDNAImportResult ADNLoader::InputFromOxDNA(const std::string& topoFile, const std::string& configFile) {

	SBPointer<ADNPart> part = new ADNPart();
	bool error = false;
	std::vector<NucleotideWrap> oxDNAIndices;

	// parse topology file
	std::ifstream topo(std::filesystem::u8path(topoFile));

	if (topo.is_open()) {

		std::string line;
		SBPointer<ADNSingleStrand> ss;
		int currChain = -1;
		int currNt = 0;
		bool fstLine = true;
		error = false;

		while (std::getline(topo, line)) {

			if (fstLine) {

				fstLine = false;
				continue;  // first line of topology contains number of chains and nucleotides

			}

			std::vector<std::string> cont;
			boost::split(cont, line, boost::is_any_of(" "));
			if (cont.size() != 4) {

				error = true;
				break;

			}

			const int numChain = std::stoi(cont[0]);
			const char base = cont[1][0];
			//int numPrevNt = std::stoi(cont[2]);
			//int numNextNt = std::stoi(cont[3]);

			if (numChain != currChain) {

				ss = new ADNSingleStrand();
				ss->SetDefaultName();
				part->RegisterSingleStrand(ss);
				currChain = numChain;

			}

			SBPointer<ADNNucleotide> nt = new ADNNucleotide();
			nt->Init();
			nt->setNucleotideType(ADNModel::ResidueNameToType(base));
			part->RegisterNucleotideThreePrime(ss, nt);

			// if last nucleotide next is same as first close

			// insert wrapper
			NucleotideWrap w = NucleotideWrap();
			w.elem_ = nt;
			w.id_ = currNt;
			w.strandId_ = currChain;
			oxDNAIndices.push_back(w);

			currNt++;

		}

	}
	else {

		error = true;

	}

	// parse config file and set positions if topology file was parsed correctly
	if (!error) {

		std::ifstream config(std::filesystem::u8path(configFile));

		if (config.is_open()) {

			std::string line;
			int lineCount = 0;
			error = false;

			while (std::getline(config, line)) {

				if (lineCount > 2) {

					std::vector<std::string> cont;
					boost::split(cont, line, boost::is_any_of(" "));
					if (cont.size() != 15) {

						error = true;
						break;

					}

					const double x = std::stod(cont[0]);
					const double y = std::stod(cont[1]);
					const double z = std::stod(cont[2]);
					const double e2x = std::stod(cont[3]);
					const double e2y = std::stod(cont[4]);
					const double e2z = std::stod(cont[5]);
					const double e1x = std::stod(cont[6]);
					const double e1y = std::stod(cont[7]);
					const double e1z = std::stod(cont[8]);

					if (oxDNAIndices.size() > (lineCount - 3)) {

						SBPosition3 pos = SBPosition3(SBQuantity::nanometer(x), SBQuantity::nanometer(y), SBQuantity::nanometer(z));
						ublas::vector<double> e2 = ublas::vector<double>(3, 0.0);
						e2[0] = e2x;
						e2[1] = e2y;
						e2[2] = e2z;
						ublas::vector<double> e1 = ublas::vector<double>(3, 0.0);
						e1[0] = e1x;
						e1[1] = e1y;
						e1[2] = e1z;
						ublas::vector<double> e3 = ADNVectorMath::CrossProduct(e1, e2);

						const auto& w = oxDNAIndices.at(lineCount - 3);
						const auto& nt = w.elem_;
						nt->SetPosition(pos);
						nt->SetE2(e2);
						nt->SetE1(e1);
						nt->SetE3(e3);

					}
					else {

						error = true;
						break;

					}

				}

				lineCount++;

			}

		}
		else {

			error = true;

		}

	}

	if (!error) {

		// create base pairs and double strands
		BuildTopScales(part);

	}

	return { error, part };

}

void ADNLoader::OutputToCanDo(ADNNanorobot* nanorobot, const std::string& filename) {

	auto singleStrands = nanorobot->GetSingleStrands();

	std::vector < SBPointerIndexer<ADNBaseSegment>> baseSegmentsVector;
	auto parts = nanorobot->GetParts();
	SB_FOR(SBPointer<ADNPart> part, parts) {

		auto baseSegments = part->GetBaseSegments();
		baseSegmentsVector.push_back(baseSegments);

	}

	ADNLoader::OutputToCanDo(singleStrands, baseSegmentsVector, filename);

}

void ADNLoader::OutputToCanDo(SBPointer<ADNPart> part, const std::string& filename) {

	auto singleStrands = part->GetSingleStrands();

	std::vector < SBPointerIndexer<ADNBaseSegment>> baseSegmentsVector;
	auto baseSegments = part->GetBaseSegments();
	baseSegmentsVector.push_back(baseSegments);

	ADNLoader::OutputToCanDo(singleStrands, baseSegmentsVector, filename);

}

/// CanDo file format description: https://cando-dna-origami.org/cndo-file-converter/
void ADNLoader::OutputToCanDo(const SBPointerIndexer<ADNSingleStrand>& singleStrands, const std::vector < SBPointerIndexer<ADNBaseSegment>>& baseSegmentsVector, const std::string& filename) {

	std::ofstream file(std::filesystem::u8path(filename));

	// A string describing the .cndo file format
	file << "\"CanDo (.cndo) file format version 1.0, Keyao Pan, Laboratory for Computational Biology and Biophysics, Massachusetts Institute of Technology, November 2015\"" << '\n' << std::endl;

	// set nucleotide indices
	std::map<ADNNucleotide*, int> nucleotidesId;
	int ntId = 1;
	SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

		auto nt = ss->GetFivePrime();
		while (nt != nullptr) {

			nucleotidesId.insert(std::make_pair(nt(), ntId));
			++ntId;
			nt = nt->GetNext();

		}

	}

	file << "dnaTop,id,up,down,across,seq" << std::endl;
	const auto getNucleotideId = [&nucleotidesId](ADNNucleotide* nucleotide, const std::string& relation) {

		if (nucleotide == nullptr) return -1;

		const auto it = nucleotidesId.find(nucleotide);
		if (it == nucleotidesId.end()) {

			ADNLogger::LogError("CanDo export omitted a non-exported " + relation + " nucleotide reference.");
			return -1;

		}

		return it->second;

	};

	for (const auto& p : nucleotidesId) {

		const int idx = p.second;
		ADNNucleotide* nt = p.first;
		ADNNucleotide* prevNt = nt->GetPrev(true)();
		ADNNucleotide* nextNt = nt->GetNext(true)();
		ADNNucleotide* pairNt = nt->GetPair()();
		const int prevIdx = getNucleotideId(prevNt, "previous");
		const int nextIdx = getNucleotideId(nextNt, "next");
		const int pairIdx = getNucleotideId(pairNt, "paired");

		// six subfields separated by commas, which are the serial number (1, 2, ..., n_nt), id, up, down, across, and seq
		std::string line = std::to_string(idx) + "," + std::to_string(idx) + "," + std::to_string(prevIdx) + "," + std::to_string(nextIdx) + "," + std::to_string(pairIdx) + "," + nt->getOneLetterNucleotideTypeString();
		file << line << std::endl;

	}
	file << std::endl;

	std::vector<std::string> dNodes;
	std::vector<std::string> triads;
	std::vector<std::string> basePairs;
	int basePairId = 1;
	for (const SBPointerIndexer<ADNBaseSegment>& baseSegments : baseSegmentsVector) {

		SB_FOR(SBPointer<ADNBaseSegment> bs, baseSegments) {

			if (bs->GetCellType() != CellType::BasePair) continue;

			auto cell = bs->GetCell();
			if (cell == nullptr) {

				ADNLogger::LogError(std::string("Skipping CanDo base pair with no cell."));
				continue;

			}

			SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(cell());
			const int id1 = getNucleotideId(bp->GetLeftNucleotide()(), "base-pair left");
			const int id2 = getNucleotideId(bp->GetRightNucleotide()(), "base-pair right");
			if (id1 == -1 || id2 == -1) {

				ADNLogger::LogError(std::string("Skipping CanDo base-pair geometry for an incomplete or non-exported base pair."));
				continue;

			}

			// four subfields separated by commas, which are the serial number (1, 2, ..., n_bp) of the basepair and the Cartesian coordinates e0 of the center of the reference frame
			const auto& pos = bs->GetPosition();
			const std::string line = std::to_string(basePairId) + "," + std::to_string(pos[0].getValue() / 1000.0) + "," + std::to_string(pos[1].getValue() / 1000.0) + "," + std::to_string(pos[2].getValue() / 1000.0);
			dNodes.push_back(line);

			const auto& e3 = bs->GetE3();
			const auto e2 = -1.0 * bs->GetE2();
			const auto e1 = ADNVectorMath::CrossProduct(e2, e3);
			std::string t = std::to_string(basePairId) + ","
				+ std::to_string(e1[0]) + "," + std::to_string(e1[1]) + "," + std::to_string(e1[2]) + ","
				+ std::to_string(e2[0]) + "," + std::to_string(e2[1]) + "," + std::to_string(e2[2]) + ","
				+ std::to_string(e3[0]) + "," + std::to_string(e3[1]) + "," + std::to_string(e3[2]);
			triads.push_back(t);

			std::string s = std::to_string(basePairId) + "," + std::to_string(id1) + "," + std::to_string(id2);
			basePairs.push_back(s);
			++basePairId;

		}

	}

	file << "dNode,\"e0(1)\",\"e0(2)\",\"e0(3)\"" << std::endl;
	for (const auto& s : dNodes) {
		file << s << std::endl;
	}
	file << std::endl;

	file << "triad,\"e1(1)\",\"e1(2)\",\"e1(3)\",\"e2(1)\",\"e2(2)\",\"e2(3)\",\"e3(1)\",\"e3(2)\",\"e3(3)\"" << std::endl;
	// ten subfields separated by commas, which are the serial number(1, 2, ..., n_bp) of the basepair and three axes e1, e2, and e3 of the reference frame
	for (const auto& s : triads) {
		file << s << std::endl;
	}
	file << std::endl;

	file << "id_nt,id1,id2" << std::endl;
	// three subfields separated by commas, which are the serial number (1, 2, ..., n_bp) of the basepair, id1, and id2
	for (const auto& bp : basePairs) {
		file << bp << std::endl;
	}

}

void ADNLoader::BuildTopScalesParametrized(SBPointer<ADNPart> part, const SBQuantity::length& maxCutOff, const SBQuantity::length& minCutOff, double maxAngle) {

	auto neighbors = ADNNeighbors();
	neighbors.SetMaxCutOff(maxCutOff);
	neighbors.SetMinCutOff(minCutOff);
	neighbors.SetIncludePairs(true);
	neighbors.InitializeNeighbors(part);

	auto singleStrands = part->GetSingleStrands();
	SBPointer<ADNDoubleStrand> ds = nullptr;

	SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

		SBPointer<ADNNucleotide> nt = ss->GetFivePrime();
		int number = 0;

		while (nt != nullptr) {

			if (nt->GetBaseSegment() != nullptr) {

				nt = nt->GetNext();
				continue;

			}

			const SBPosition3 posNt = nt->GetPosition();
			const auto& e2Nt = nt->GetE2();
			const auto& e3Nt = nt->GetE3();

			auto ntBors = neighbors.GetNeighbors(nt);
			SBQuantity::length minDist = SBQuantity::nanometer(ADNConstants::DH_DIAMETER);

			SBPointer<ADNNucleotide> pair = nullptr;
			// check possible base pairing against the neighbors
			SB_FOR(SBPointer<ADNNucleotide> bor, ntBors) {

				const SBPosition3 posBor = bor->GetPosition();
				const SBPosition3 dif = posBor - posNt;
				const auto& e2Bor = bor->GetE2();
				const auto& e3Bor = bor->GetE3();
				// check right directionality and co-planarity
				//double s = ublas::inner_prod(e3Nt, e3Bor);
				//if (!ADNVectorMath::IsNearlyZero(s + 1)) continue;

				double t = ublas::inner_prod(e2Nt, e2Bor);
				// check that they are "in front" of each other
				ublas::vector<double> df = ADNAuxiliary::SBPositionToUblas(dif);
				double n = ublas::inner_prod(-e2Nt, df);
				double angle_threshold = maxAngle;
				// check they are complementary
				bool comp = ADNModel::GetComplementaryBase(nt->getNucleotideType()) == bor->getNucleotideType();
				if (n > 0 && t < 0.0 && abs(t) > cos(ADNVectorMath::DegToRad(angle_threshold)) && comp) {

					// possible paired, take closest
					if (dif.norm() < minDist) {

						pair = bor;
						minDist = dif.norm();

					}

				}

			}

			SBPointer<ADNBaseSegment> bs = new ADNBaseSegment(CellType::BasePair);
			SBPosition3 bsPos = posNt - SBQuantity::nanometer(ADNConstants::DH_DIAMETER * 0.5) * ADNAuxiliary::UblasVectorToSBVector(e2Nt);
			const auto& e3 = nt->GetE3();
			/*ADNLogger& logger = ADNLogger::GetLogger();
			std::string msg = ss->GetName() + ": " + std::to_string(e3[0]) + " " + std::to_string(e3[1]) + " " + std::to_string(e3[2]);
			logger.LogDebug(msg);*/
			const auto& e1 = nt->GetE1();
			SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(bs->GetCell()());
			bp->SetLeftNucleotide(nt);
			nt->SetBaseSegment(bs);
			if (pair != nullptr) {

				bp->SetRightNucleotide(pair);
				pair->SetBaseSegment(bs);
				bsPos = (nt->GetPosition() + pair->GetPosition()) * 0.5;
				bp->PairNucleotides();

			}

			bs->SetPosition(bsPos);
			bs->SetE3(e3);
			bs->SetE2(e2Nt);
			bs->SetE1(e1);
			bs->SetNumber(number);

			// add to corresponding double strand
			if (ds == nullptr) {
				ds = new ADNDoubleStrand();
				part->RegisterDoubleStrand(ds);
			}
			part->RegisterBaseSegmentEnd(ds, bs);

			const double turningThreshold = 0.0;
			const bool breakDs = shouldBreakTopScaleDoubleStrand(nt, pair, e3, turningThreshold);

			if (breakDs) ds = nullptr;

			++number;
			nt = nt->GetNext();

		}

	}

}

void ADNLoader::BuildTopScales(SBPointer<ADNPart> part) {

	const auto dh_radius = SBQuantity::nanometer(ADNConstants::DH_DIAMETER) * 0.5;
	const SBQuantity::length maxCutOff = dh_radius + SBQuantity::nanometer(0.2);
	const SBQuantity::length minCutOff = dh_radius - SBQuantity::nanometer(0.1);
	auto neighbors = ADNNeighbors();
	neighbors.SetMaxCutOff(maxCutOff);
	neighbors.SetMinCutOff(minCutOff);
	neighbors.SetIncludePairs(true);
	neighbors.InitializeNeighbors(part);

	auto singleStrands = part->GetSingleStrands();
	SBPointer<ADNDoubleStrand> ds = nullptr;

	SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

		SBPointer<ADNNucleotide> nt = ss->GetFivePrime();
		int number = 0;

		while (nt != nullptr) {

			if (nt->GetBaseSegment() != nullptr) {

				nt = nt->GetNext();
				continue;

			}

			SBPosition3 posNt = nt->GetPosition();
			const auto& e2Nt = nt->GetE2();

			auto ntBors = neighbors.GetNeighbors(nt);
			double maxCos = 0.0;
			SBQuantity::length minDist = SBQuantity::nanometer(ADNConstants::DH_DIAMETER);

			SBPointer<ADNNucleotide> pair = nullptr;
			// check possible base pairing against the neighbors
			SB_FOR(SBPointer<ADNNucleotide> bor, ntBors) {
				const SBPosition3 posBor = bor->GetPosition();
				const SBPosition3 dif = posBor - posNt;
				const auto& e2Bor = bor->GetE2();
				// check right directionality and co-planarity
				const double t = ublas::inner_prod(e2Nt, e2Bor);
				// check that they are "in front" of each other
				const ublas::vector<double> df = ADNAuxiliary::SBPositionToUblas(dif);
				const double n = ublas::inner_prod(e2Nt, df);
				const double angle_threshold = 49.0;
				// check they are complementary
				const bool comp = ADNModel::GetComplementaryBase(nt->getNucleotideType()) == bor->getNucleotideType();
				if (n > 0 && t < 0.0 && abs(t) > cos(ADNVectorMath::DegToRad(angle_threshold)) && comp) {

					// possible paired, take closest
					if (dif.norm() < minDist) {

						pair = bor;
						minDist = dif.norm();

					}

				}

			}

			SBPointer<ADNBaseSegment> bs = new ADNBaseSegment(CellType::BasePair);
			SBPosition3 bsPos = posNt + SBQuantity::nanometer(ADNConstants::DH_DIAMETER * 0.5) * ADNAuxiliary::UblasVectorToSBVector(e2Nt);
			const auto& e3 = nt->GetE3();
			std::string msg = ss->getName() + ": " + std::to_string(e3[0]) + " " + std::to_string(e3[1]) + " " + std::to_string(e3[2]);
			ADNLogger::LogDebug(msg);
			const auto& e1 = nt->GetE1();
			SBPointer<ADNBasePair> bp = static_cast<ADNBasePair*>(bs->GetCell()());
			bp->SetLeftNucleotide(nt);
			nt->SetBaseSegment(bs);
			if (pair != nullptr) {

				bp->SetRightNucleotide(pair);
				pair->SetBaseSegment(bs);
				bsPos = (posNt + pair->GetPosition()) * 0.5;
				bp->PairNucleotides();

			}

			bs->SetPosition(bsPos);
			bs->SetE3(e3);
			bs->SetE2(e2Nt);
			bs->SetE1(e1);
			bs->SetNumber(number);

			// add to corresponding double strand
			if (ds == nullptr) {

				ds = new ADNDoubleStrand();
				part->RegisterDoubleStrand(ds);

			}
			part->RegisterBaseSegmentEnd(ds, bs);

			const double turningThreshold = 0.0;
			const bool breakDs = shouldBreakTopScaleDoubleStrand(nt, pair, e3, turningThreshold);

			if (breakDs) ds = nullptr;

			++number;
			nt = nt->GetNext();

		}

	}

}

void ADNLoader::OutputToCSV(SBPointerIndexer<ADNPart> parts, const std::string& fname, const std::string& folder) {

	int num = 0;

	std::ofstream out(std::filesystem::u8path(folder + "/" + fname));

	SignOutputFile(out);
	SB_FOR(SBPointer<ADNPart> part, parts) {

		auto singleStrands = part->GetSingleStrands();
		SB_FOR(SBPointer<ADNSingleStrand> ss, singleStrands) {

			auto seq = ss->GetSequenceWithTags();
			out << std::to_string(num) + " " + ss->getName() + " | length: " + std::to_string(seq.size());
			out << ",";
			out << seq;
			out << "\n";

		}

	}

	out.close();

}

void ADNLoader::SavePartToJson(SBPointer<ADNPart> p, const std::string& filename) {

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("version");
	writer.Double(ADNConstants::JSON_FORMAT_VERSION);

	SavePartToJson(p, writer);

	writer.EndObject();  // end json document

	QIODevice::OpenModeFlag mode = QIODevice::WriteOnly;

	QFile file(QString::fromStdString(filename));
	if (file.open(mode)) {

		QTextStream out(&file);

		out << s.GetString();

		file.close();

	}

}

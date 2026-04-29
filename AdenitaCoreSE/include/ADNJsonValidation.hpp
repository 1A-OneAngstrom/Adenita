#pragma once

#include <cstdlib>
#include <string>

#include "rapidjson/document.h"

namespace ADNLoader::JsonValidation {

inline bool isIntegerString(const char* text) {

	if (!text || *text == '\0') return false;

	char* end = nullptr;
	(void)std::strtol(text, &end, 10);
	return end && *end == '\0';

}

inline bool isNumberToken(const std::string& token) {

	if (token.empty()) return false;

	char* end = nullptr;
	(void)std::strtod(token.c_str(), &end);
	return end && *end == '\0';

}

inline bool isVectorString(const rapidjson::Value& value, int expectedSize) {

	if (!value.IsString()) return false;

	const std::string text = value.GetString();
	std::size_t pos = 0;
	int count = 0;

	while (true) {

		const std::size_t comma = text.find(',', pos);
		const std::string token = text.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
		if (!isNumberToken(token)) return false;
		++count;

		if (comma == std::string::npos) break;
		pos = comma + 1;

	}

	return count == expectedSize;

}

inline bool isIntegerListString(const rapidjson::Value& value) {

	if (!value.IsString()) return false;

	const std::string text = value.GetString();
	if (text.empty()) return false;

	std::size_t pos = 0;

	while (true) {

		const std::size_t comma = text.find(',', pos);
		const std::string token = text.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
		if (!isIntegerString(token.c_str())) return false;

		if (comma == std::string::npos) break;
		pos = comma + 1;

	}

	return true;

}

inline bool hasObject(const rapidjson::Value& value, const char* name) {
	return value.IsObject() && value.HasMember(name) && value[name].IsObject();
}

inline bool hasString(const rapidjson::Value& value, const char* name) {
	return value.IsObject() && value.HasMember(name) && value[name].IsString();
}

inline bool hasBool(const rapidjson::Value& value, const char* name) {
	return value.IsObject() && value.HasMember(name) && value[name].IsBool();
}

inline bool hasInt(const rapidjson::Value& value, const char* name) {
	return value.IsObject() && value.HasMember(name) && value[name].IsInt();
}

inline bool hasNumber(const rapidjson::Value& value, const char* name) {
	return value.IsObject() && value.HasMember(name) && value[name].IsNumber();
}

inline bool hasVectorString(const rapidjson::Value& value, const char* name, int expectedSize = 3) {
	return value.IsObject() && value.HasMember(name) && isVectorString(value[name], expectedSize);
}

inline bool validateModernLoop(const rapidjson::Value& loop) {

	if (!loop.IsObject()) return false;
	if (!loop.HasMember("startNt")) return true;

	return hasInt(loop, "startNt") &&
		hasInt(loop, "endNt") &&
		loop.HasMember("nucleotides") &&
		isIntegerListString(loop["nucleotides"]);

}

inline bool validateModernCell(const rapidjson::Value& cell) {

	if (!cell.IsObject() || !hasInt(cell, "type")) return false;

	const int type = cell["type"].GetInt();
	if (type == 0) {
		return hasInt(cell, "left") && hasInt(cell, "right");
	}
	if (type == 1) {
		return true;
	}
	if (type == 2) {
		return hasObject(cell, "leftLoop") &&
			hasObject(cell, "rightLoop") &&
			validateModernLoop(cell["leftLoop"]) &&
			validateModernLoop(cell["rightLoop"]);
	}

	return false;

}

inline bool validateModernNucleotide(const rapidjson::Value& nucleotide, double versionValue) {

	if (!nucleotide.IsObject()) return false;

	if (!hasString(nucleotide, "type") ||
		!hasVectorString(nucleotide, "position") ||
		!hasVectorString(nucleotide, "backboneCenter") ||
		!hasVectorString(nucleotide, "sidechainCenter") ||
		!hasVectorString(nucleotide, "e1") ||
		!hasVectorString(nucleotide, "e2") ||
		!hasVectorString(nucleotide, "e3") ||
		!hasInt(nucleotide, "next") ||
		!hasInt(nucleotide, "prev") ||
		!hasInt(nucleotide, "pair")) return false;

	if (versionValue >= 0.5 && !hasString(nucleotide, "tag")) return false;

	return true;

}

inline bool validateModernSingleStrands(const rapidjson::Value& strands, double versionValue) {

	if (!strands.IsObject()) return false;

	for (auto strand = strands.MemberBegin(); strand != strands.MemberEnd(); ++strand) {

		if (!isIntegerString(strand->name.GetString())) return false;
		const rapidjson::Value& value = strand->value;
		if (!value.IsObject() ||
			!hasString(value, "chainName") ||
			!hasBool(value, "isScaffold") ||
			!hasInt(value, "fivePrimeId") ||
			!hasObject(value, "nucleotides")) return false;

		const rapidjson::Value& nucleotides = value["nucleotides"];
		for (auto nucleotide = nucleotides.MemberBegin(); nucleotide != nucleotides.MemberEnd(); ++nucleotide) {
			if (!isIntegerString(nucleotide->name.GetString())) return false;
			if (!validateModernNucleotide(nucleotide->value, versionValue)) return false;
		}

	}

	return true;

}

inline bool validateModernDoubleStrands(const rapidjson::Value& doubleStrands) {

	if (!doubleStrands.IsObject()) return false;

	for (auto strand = doubleStrands.MemberBegin(); strand != doubleStrands.MemberEnd(); ++strand) {

		if (!isIntegerString(strand->name.GetString())) return false;
		const rapidjson::Value& value = strand->value;
		if (!value.IsObject() ||
			!hasNumber(value, "initialTwistAngle") ||
			!hasInt(value, "firstBaseSegment") ||
			!hasInt(value, "lastBaseSegment") ||
			!hasObject(value, "bases")) return false;

		const rapidjson::Value& bases = value["bases"];
		for (auto base = bases.MemberBegin(); base != bases.MemberEnd(); ++base) {

			if (!isIntegerString(base->name.GetString())) return false;
			const rapidjson::Value& baseValue = base->value;
			if (!baseValue.IsObject() ||
				!hasVectorString(baseValue, "position") ||
				!hasVectorString(baseValue, "e1") ||
				!hasVectorString(baseValue, "e2") ||
				!hasVectorString(baseValue, "e3") ||
				!hasInt(baseValue, "number") ||
				!hasObject(baseValue, "cell") ||
				!hasInt(baseValue, "next") ||
				!hasInt(baseValue, "previous") ||
				!validateModernCell(baseValue["cell"])) return false;

		}

	}

	return true;

}

inline bool isValidModernPart(const rapidjson::Value& value, double versionValue) {

	return value.IsObject() &&
		hasString(value, "name") &&
		value.HasMember("singleStrands") &&
		value.HasMember("doubleStrands") &&
		validateModernSingleStrands(value["singleStrands"], versionValue) &&
		validateModernDoubleStrands(value["doubleStrands"]);

}

inline bool validateLegacyPairInfo(const rapidjson::Value& pair) {
	return pair.IsObject() && hasInt(pair, "strandId") && hasInt(pair, "pairId");
}

inline bool validateLegacyNucleotideRef(const rapidjson::Value& ref, const char* strandField, const char* nucleotideField) {
	return ref.IsObject() && hasInt(ref, strandField) && hasInt(ref, nucleotideField);
}

inline bool validateLegacyLoop(const rapidjson::Value& loop) {

	if (!loop.IsObject()) return false;
	if (!loop.HasMember("id")) return true;

	if (!hasInt(loop, "id") ||
		!hasInt(loop, "strand_id") ||
		!hasObject(loop, "start_nt") ||
		!hasObject(loop, "end_nt") ||
		!hasObject(loop, "nucleotides_list")) return false;

	if (!validateLegacyNucleotideRef(loop["start_nt"], "strand_id", "nt_id") ||
		!validateLegacyNucleotideRef(loop["end_nt"], "strand_id", "nt_id")) return false;

	const rapidjson::Value& nucleotides = loop["nucleotides_list"];
	for (auto nucleotide = nucleotides.MemberBegin(); nucleotide != nucleotides.MemberEnd(); ++nucleotide) {
		if (!nucleotide->value.IsObject() ||
			!hasInt(nucleotide->value, "nt_id") ||
			!hasInt(nucleotide->value, "strand_id")) return false;
	}

	return true;

}

inline bool validateLegacyCell(const rapidjson::Value& cell, double versionValue) {

	if (versionValue <= 0.1) return true;
	if (!cell.IsObject() || !hasInt(cell, "type")) return false;

	const int type = cell["type"].GetInt();
	if (type == 0) {
		return hasObject(cell, "left") &&
			hasObject(cell, "right") &&
			validateLegacyNucleotideRef(cell["left"], "strand_id", "nt_id") &&
			validateLegacyNucleotideRef(cell["right"], "strand_id", "nt_id");
	}
	if (type == 1) {
		return true;
	}
	if (type == 2) {
		return hasObject(cell, "left_loop") &&
			hasObject(cell, "right_loop") &&
			validateLegacyLoop(cell["left_loop"]) &&
			validateLegacyLoop(cell["right_loop"]);
	}

	return false;

}

inline bool validateLegacyStrands(const rapidjson::Value& strands) {

	if (!strands.IsObject()) return false;

	for (auto strand = strands.MemberBegin(); strand != strands.MemberEnd(); ++strand) {

		const rapidjson::Value& value = strand->value;
		if (!value.IsObject() ||
			!hasInt(value, "id") ||
			!hasString(value, "chainName") ||
			!hasBool(value, "isScaffold") ||
			!hasInt(value, "fivePrimeId") ||
			!hasObject(value, "nucleotides")) return false;

		const rapidjson::Value& nucleotides = value["nucleotides"];
		for (auto nucleotide = nucleotides.MemberBegin(); nucleotide != nucleotides.MemberEnd(); ++nucleotide) {

			const rapidjson::Value& nt = nucleotide->value;
			if (!nt.IsObject() ||
				!hasString(nt, "type") ||
				!hasVectorString(nt, "e1") ||
				!hasVectorString(nt, "e2") ||
				!hasVectorString(nt, "e3") ||
				!hasVectorString(nt, "position") ||
				!hasVectorString(nt, "backboneCenter") ||
				!hasVectorString(nt, "sidechainCenter") ||
				!hasInt(nt, "id") ||
				!hasInt(nt, "next") ||
				!hasInt(nt, "prev") ||
				!nt.HasMember("pair") ||
				!validateLegacyPairInfo(nt["pair"])) return false;

		}

	}

	return true;

}

inline bool validateLegacyDoubleStrands(const rapidjson::Value& doubleStrands) {

	if (!doubleStrands.IsObject()) return false;

	for (auto strand = doubleStrands.MemberBegin(); strand != doubleStrands.MemberEnd(); ++strand) {

		const rapidjson::Value& value = strand->value;
		if (!value.IsObject() ||
			!hasInt(value, "id") ||
			!hasNumber(value, "initialTwistAngle") ||
			!hasInt(value, "size") ||
			!hasInt(value, "bsStartId")) return false;

	}

	return true;

}

inline bool validateLegacyJoints(const rapidjson::Value& joints) {

	if (!joints.IsObject()) return false;

	for (auto joint = joints.MemberBegin(); joint != joints.MemberEnd(); ++joint) {
		if (!joint->value.IsObject() ||
			!hasInt(joint->value, "id") ||
			!hasVectorString(joint->value, "position")) return false;
	}

	return true;

}

inline bool validateLegacyBases(const rapidjson::Value& bases, double versionValue) {

	if (!bases.IsObject()) return false;

	for (auto base = bases.MemberBegin(); base != bases.MemberEnd(); ++base) {

		const rapidjson::Value& value = base->value;
		if (!value.IsObject() ||
			!hasInt(value, "double_strand") ||
			!hasInt(value, "source") ||
			!hasInt(value, "target") ||
			!hasInt(value, "number") ||
			!hasVectorString(value, "normal") ||
			!hasVectorString(value, "direction") ||
			!hasVectorString(value, "u")) return false;

		if (versionValue > 0.1) {
			if (!value.HasMember("cell") || !validateLegacyCell(value["cell"], versionValue)) return false;
		}
		else if (!hasObject(value, "nt") || !validateLegacyNucleotideRef(value["nt"], "strandId", "ntId")) {
			return false;
		}

	}

	return true;

}

inline bool isValidLegacyPart(const rapidjson::Value& value, double versionValue) {

	return value.IsObject() &&
		hasNumber(value, "version") &&
		hasString(value, "name") &&
		value.HasMember("strands") &&
		value.HasMember("doubleStrands") &&
		value.HasMember("joints") &&
		value.HasMember("bases") &&
		validateLegacyStrands(value["strands"]) &&
		validateLegacyDoubleStrands(value["doubleStrands"]) &&
		validateLegacyJoints(value["joints"]) &&
		validateLegacyBases(value["bases"], versionValue);

}

inline bool isValidPartsDocument(const rapidjson::Value& value, double versionValue) {

	if (!value.IsObject() || !hasObject(value, "parts")) return false;

	const rapidjson::Value& parts = value["parts"];
	for (auto part = parts.MemberBegin(); part != parts.MemberEnd(); ++part) {
		if (!isValidModernPart(part->value, versionValue)) return false;
	}

	return true;

}

} // namespace ADNLoader::JsonValidation

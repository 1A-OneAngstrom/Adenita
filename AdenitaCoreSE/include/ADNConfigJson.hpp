#pragma once

#include <string>

#include "rapidjson/document.h"

namespace ADNConfigJson {

	template <typename Document>
	void setStringMember(Document& document, const char* key, const std::string& value) {

		auto& allocator = document.GetAllocator();
		auto member = document.FindMember(key);

		if (member != document.MemberEnd()) {

			member->value.SetString(value.c_str(), static_cast<rapidjson::SizeType>(value.size()), allocator);
			return;

		}

		typename Document::ValueType memberName;
		memberName.SetString(key, static_cast<rapidjson::SizeType>(std::char_traits<char>::length(key)), allocator);

		typename Document::ValueType memberValue;
		memberValue.SetString(value.c_str(), static_cast<rapidjson::SizeType>(value.size()), allocator);

		document.AddMember(memberName, memberValue, allocator);

	}

} // namespace ADNConfigJson

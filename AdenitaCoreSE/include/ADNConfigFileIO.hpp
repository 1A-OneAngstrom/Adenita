#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

namespace ADNConfigFileIO {

	namespace detail {

		struct FileCloser {

			void operator()(FILE* file) const {

				if (file != nullptr)
					std::fclose(file);

			}

		};

		using FilePtr = std::unique_ptr<FILE, FileCloser>;

		inline FilePtr openFile(const std::string& path, const char* mode) {

#ifdef _MSC_VER
			FILE* file = nullptr;
			if (fopen_s(&file, path.c_str(), mode) != 0)
				return FilePtr(nullptr);

			return FilePtr(file);
#else
			return FilePtr(std::fopen(path.c_str(), mode));
#endif

		}

	} // namespace detail

	template <typename Document>
	bool readDocumentFromFile(const std::string& path, Document& document) {

		auto file = detail::openFile(path, "rb");
		if (file == nullptr)
			return false;

		char readBuffer[65536];
		rapidjson::FileReadStream stream(file.get(), readBuffer, sizeof(readBuffer));
		document.ParseStream(stream);

		return !document.HasParseError() && document.IsObject();

	}

	template <typename Document>
	bool writeDocumentToFile(const std::string& path, const Document& document) {

		auto file = detail::openFile(path, "wb");
		if (file == nullptr)
			return false;

		char writeBuffer[65536];
		rapidjson::FileWriteStream stream(file.get(), writeBuffer, sizeof(writeBuffer));
		rapidjson::Writer<rapidjson::FileWriteStream> writer(stream);

		return document.Accept(writer);

	}

} // namespace ADNConfigFileIO

#pragma once

#include <string>

#include "ADNLogger.hpp"
#include "SAMSON.hpp"

namespace ADNSamsonContext {

inline void LogMissingActiveDocument(const char* context) {

	std::string message = "Missing active SAMSON document";
	if (context != nullptr && context[0] != '\0') message += " in " + std::string(context);
	message += ".";
	ADNLogger::LogError(message);

}

inline SBDocument* GetActiveDocument(const char* context) {

	SBDocument* document = SAMSON::getActiveDocument();
	if (document == nullptr) LogMissingActiveDocument(context);
	return document;

}

inline decltype(SAMSON::getActiveDocument()->getSelectedNodes()) GetSelectedNodes(const char* context) {

	SBDocument* document = GetActiveDocument(context);
	if (document == nullptr) return nullptr;

	auto* selectedNodes = document->getSelectedNodes();
	if (selectedNodes == nullptr) {
		std::string message = "Missing SAMSON selection";
		if (context != nullptr && context[0] != '\0') message += " in " + std::string(context);
		message += ".";
		ADNLogger::LogError(message);
	}
	return selectedNodes;

}

}

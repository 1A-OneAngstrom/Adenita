#pragma once

#include <string>
#include <QString>

#include "SBCHeapExport.hpp"

namespace ADNLogger {

	/// \name Error logging
	//@{
	
	SB_EXPORT void												LogError(const std::string& value);
	SB_EXPORT void												LogError(const QString& value);

	//@}

	/// \name Debug logging
	//@{

	SB_EXPORT void												LogDebug(const std::string& value);
	SB_EXPORT void												LogDebug(const QString& value);

	//@}

	/// \name Normal logging
	//@{

	SB_EXPORT void												Log(const std::string& value);
	SB_EXPORT void												Log(const QString& value);

	//@}

};


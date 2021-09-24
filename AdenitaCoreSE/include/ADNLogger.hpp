#pragma once

#include <string>
#include <QString>


namespace ADNLogger {

	/// \name Error logging
	//@{
	
	void														LogError(const std::string& value);
	void														LogError(const QString& value);

	//@}

	/// \name Debug logging
	//@{

	void														LogDebug(const std::string& value);
	void														LogDebug(const QString& value);

	//@}

	/// \name Normal logging
	//@{

	void														Log(const std::string& value);
	void														Log(const QString& value);

	//@}

};


#include "ADNLogger.hpp"

#include "SBCLog.hpp"
#include "ADNConfig.hpp"


void ADNLogger::LogError(const std::string& value) {

    SB_ERROR("[Adenita] Error: " + value);

}

void ADNLogger::LogError(const QString& value) {

    LogError(value.toStdString());

}

void ADNLogger::LogDebug(const std::string& value) {

    SEConfig& config = SEConfig::GetInstance();
    if (config.mode == SEConfigMode::DEBUG_LOG)
        SB_WARNING("[Adenita] Debug: " + value);

}

void ADNLogger::LogDebug(const QString& value) {

    LogDebug(value.toStdString());

}

void ADNLogger::Log(const std::string& value) {

    SB_INFORMATION("[Adenita] Info: " + value);

}

void ADNLogger::Log(const QString& value) {

    Log(value.toStdString());

}

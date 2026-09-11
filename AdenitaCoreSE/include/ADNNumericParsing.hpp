#pragma once

#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

/// \brief Checked conversions shared by input validation and model construction.
namespace ADNNumericParsing {

/// \brief Removes surrounding whitespace without discarding embedded null characters.
inline std::string_view trim(std::string_view text) {
	while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
	while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
	return text;
}

/// \brief Parses a complete integer token representable by the model's integer identifiers.
/// \param text The token, with optional surrounding whitespace.
/// \param value Receives the value only on success.
/// \return False for invalid, truncated, or out-of-range input.
inline bool tryInteger(std::string_view text, int& value) {
	text = trim(text);
	if (text.empty() || text.find('\0') != std::string_view::npos) return false;
	const std::string token(text);
	char* end = nullptr;
	errno = 0;
	const long parsed = std::strtol(token.c_str(), &end, 10);
	if (end == token.c_str() || end != token.c_str() + token.size() || errno == ERANGE ||
		parsed < (std::numeric_limits<int>::min)() || parsed > (std::numeric_limits<int>::max)()) return false;
	value = static_cast<int>(parsed);
	return true;
}

/// \brief Parses a complete finite double token, rejecting overflow and underflow.
/// \param text The token, with optional surrounding whitespace.
/// \param value Receives the value only on success.
/// \return False if conversion cannot preserve a finite value in range.
inline bool tryDouble(std::string_view text, double& value) {
	text = trim(text);
	if (text.empty() || text.find('\0') != std::string_view::npos) return false;
	const std::string token(text);
	char* end = nullptr;
	errno = 0;
	const double parsed = std::strtod(token.c_str(), &end);
	if (end == token.c_str() || end != token.c_str() + token.size() || errno == ERANGE || !std::isfinite(parsed)) return false;
	value = parsed;
	return true;
}

/// \brief Converts an integer token with the same checks used by validators.
/// \throws std::invalid_argument if the complete token is not a representable integer.
inline int integer(std::string_view text) {
	int value = 0;
	if (!tryInteger(text, value)) throw std::invalid_argument("Invalid integer token");
	return value;
}

/// \brief Converts a finite number with the same checks used by validators.
/// \throws std::invalid_argument if the complete token is not a finite number in range.
inline double number(std::string_view text) {
	double value = 0;
	if (!tryDouble(text, value)) throw std::invalid_argument("Invalid numeric token");
	return value;
}

} // namespace ADNNumericParsing

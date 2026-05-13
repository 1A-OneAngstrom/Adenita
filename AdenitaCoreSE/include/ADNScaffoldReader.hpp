#pragma once

#include <string>
#include <vector>

namespace ADNScaffoldReader {

	namespace detail {

		[[nodiscard]] inline bool isWhitespace(char c) {

			return c == ' ' || c == '\t' || c == '\r' || c == '\n';

		}

		[[nodiscard]] inline char toUpperBase(char c) {

			if (c >= 'a' && c <= 'z')
				return static_cast<char>(c - 'a' + 'A');

			return c;

		}

		[[nodiscard]] inline std::string trim(const std::string& line) {

			std::size_t begin = 0;
			while (begin < line.size() && isWhitespace(line[begin]))
				++begin;

			std::size_t end = line.size();
			while (end > begin && isWhitespace(line[end - 1]))
				--end;

			return line.substr(begin, end - begin);

		}

		[[nodiscard]] inline bool normalizeSequenceLine(const std::string& line, std::string& normalized) {

			normalized.clear();
			normalized.reserve(line.size());

			for (const char c : line) {

				const char base = toUpperBase(c);
				if (base != 'A' && base != 'C' && base != 'G' && base != 'T' && base != 'N')
					return false;

				normalized.push_back(base);

			}

			return true;

		}

	} // namespace detail

	[[nodiscard]] inline std::string readScaffoldLines(const std::vector<std::string>& lines) {

		std::string sequence;

		for (const std::string& rawLine : lines) {

			const std::string line = detail::trim(rawLine);
			if (line.empty())
				continue;

			if (line[0] == '>')
				continue;

			std::string normalized;
			if (detail::normalizeSequenceLine(line, normalized))
				sequence.append(normalized);

		}

		return sequence;

	}

} // namespace ADNScaffoldReader

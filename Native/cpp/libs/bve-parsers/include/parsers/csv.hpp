#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace bve::parsers::csv {
	struct CSVToken {
		std::string text;
		std::size_t line_begin = 0;
		std::size_t line_end = 0;
		std::size_t char_begin = 0;
		std::size_t char_end = 0;
	};

	using ParsedCSV = std::vector<std::vector<CSVToken>>;

	enum class SplitFirstColumn : bool { yes = true, no = false };

	ParsedCSV parse(const std::string& file, SplitFirstColumn sfc = SplitFirstColumn::no, char delim = ',', char split_char = ' ');

	std::ostream& operator<<(std::ostream& os, const CSVToken& rhs);
	std::ostream& operator<<(std::ostream& os, const ParsedCSV& rhs);
} // namespace bve::parsers::csv

#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace parsers {
namespace csv {
	struct csv_token {
		std::string text;
		std::size_t line_begin = 0;
		std::size_t line_end = 0;
		std::size_t char_begin = 0;
		std::size_t char_end = 0;
	};

	using parsed_csv = std::vector<std::vector<csv_token>>;

	parsed_csv parse(const std::string& file);

	std::ostream& operator<<(std::ostream& os, const csv_token& rhs);
	std::ostream& operator<<(std::ostream& os, const parsed_csv& rhs);
} // namespace csv
} // namespace parsers
#pragma once

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

// Log regex
using namespace std::string_literals;

static boost::regex log_regex(
    "[0-9]{4}.[01][0-9].[0-3][0-9] [0-2][0-9]:[0-5][0-9]:[0-6][0-9].[0-9]{3}: "
    "(debug|info|note|warning|error|fatal_error): (?:([^:\\s]+):(\\d+): )?(.*)"s);

inline std::string create_temp_file() {
	return boost::filesystem::unique_path().native();
}
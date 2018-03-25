#include "ini.hpp"
#include "variant_compare.hpp"
#include <doctest.h>

using namespace std::string_literals;

TEST_SUITE_BEGIN("libparsers - core_ini");

TEST_CASE("libparsers - core_ini - empty file") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse("");

	REQUIRE_EQ(parsed.size(), 1);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);
}

TEST_CASE("libparsers - core_ini - single value") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse("val");

	REQUIRE_EQ(parsed.size(), 1);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	REQUIRE_EQ(parsed[0].values.size(), 1);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[0].values[0].line, 1);
	CHECK_EQ(parsed[0].values[0].value, "val");
}

TEST_CASE("libparsers - core_ini - single key value pair") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse("key = value");

	REQUIRE_EQ(parsed.size(), 1);
	REQUIRE_EQ(parsed[0].key_value_pairs.size(), 1);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[0].key_value_pairs[0].line, 1);
	CHECK_EQ(parsed[0].key_value_pairs[0].key, "key");
	CHECK_EQ(parsed[0].key_value_pairs[0].value, "value");
}

TEST_CASE("libparsers - core_ini - empty section") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse("[sec1]");

	REQUIRE_EQ(parsed.size(), 2);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[1].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[1].values.size(), 0);
	CHECK_EQ(parsed[1].name, "sec1");
	CHECK_EQ(parsed[1].line, 1);
}

TEST_CASE("libparsers - core_ini - single value inside section") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse(
	    "[sec1]\n"
	    "val");

	REQUIRE_EQ(parsed.size(), 2);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[1].key_value_pairs.size(), 0);
	REQUIRE_EQ(parsed[1].values.size(), 1);
	CHECK_EQ(parsed[1].name, "sec1");
	CHECK_EQ(parsed[1].line, 1);

	CHECK_EQ(parsed[1].values[0].line, 2);
	CHECK_EQ(parsed[1].values[0].value, "val");
}

TEST_CASE("libparsers - core_ini - single key value pair inside section") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse(
	    "[sec1]\n"
	    "key = value");

	REQUIRE_EQ(parsed.size(), 2);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	REQUIRE_EQ(parsed[1].key_value_pairs.size(), 1);
	CHECK_EQ(parsed[1].values.size(), 0);
	CHECK_EQ(parsed[1].name, "sec1");
	CHECK_EQ(parsed[1].line, 1);

	CHECK_EQ(parsed[1].key_value_pairs[0].line, 2);
	CHECK_EQ(parsed[1].key_value_pairs[0].key, "key");
	CHECK_EQ(parsed[1].key_value_pairs[0].value, "value");
}

TEST_CASE("libparsers - core_ini - single value before section") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse(
	    "val\n"
	    "[sec1]");

	REQUIRE_EQ(parsed.size(), 2);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	REQUIRE_EQ(parsed[0].values.size(), 1);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[0].values[0].line, 1);
	CHECK_EQ(parsed[0].values[0].value, "val");

	CHECK_EQ(parsed[1].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[1].values.size(), 0);
	CHECK_EQ(parsed[1].name, "sec1");
	CHECK_EQ(parsed[1].line, 2);
}

TEST_CASE("libparsers - core_ini - single key value pair before section") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse(
	    "key = value\n"
	    "[sec1]");

	REQUIRE_EQ(parsed.size(), 2);
	REQUIRE_EQ(parsed[0].key_value_pairs.size(), 1);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[0].key_value_pairs[0].line, 1);
	CHECK_EQ(parsed[0].key_value_pairs[0].key, "key");
	CHECK_EQ(parsed[0].key_value_pairs[0].value, "value");

	CHECK_EQ(parsed[1].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[1].values.size(), 0);
	CHECK_EQ(parsed[1].name, "sec1");
	CHECK_EQ(parsed[1].line, 2);
}

TEST_CASE("libparsers - core_ini - single key value pair with second equals") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse("key = value = 2\n");

	REQUIRE_EQ(parsed.size(), 1);
	REQUIRE_EQ(parsed[0].key_value_pairs.size(), 1);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[0].key_value_pairs[0].line, 1);
	CHECK_EQ(parsed[0].key_value_pairs[0].key, "key");
	CHECK_EQ(parsed[0].key_value_pairs[0].value, "value = 2");
}

TEST_CASE("libparsers - core_ini - missing right bracket") {
	parsers::ini::parsed_ini_object parsed = parsers::ini::parse("[sec1\n");

	REQUIRE_EQ(parsed.size(), 2);
	CHECK_EQ(parsed[0].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[0].values.size(), 0);
	CHECK_EQ(parsed[0].name, "");
	CHECK_EQ(parsed[0].line, 0);

	CHECK_EQ(parsed[1].key_value_pairs.size(), 0);
	CHECK_EQ(parsed[1].values.size(), 0);
	CHECK_EQ(parsed[1].name, "sec1");
	CHECK_EQ(parsed[1].line, 1);
}

TEST_SUITE_END();

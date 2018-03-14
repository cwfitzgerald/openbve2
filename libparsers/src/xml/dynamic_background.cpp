#include "parsers/xml/dynamic_background.hpp"
#include "utils.hpp"
#include <rapidxml/rapidxml.hpp>
#include <sstream>
#include <string>

using namespace std::string_literals;

namespace parsers {
namespace xml {
	namespace dynamic_background {
		parsed_dynamic_background parse(const std::string& filename,
		                                std::string input_string,
		                                errors::multi_error& errors,
		                                const find_relative_file_func& get_relative_file) {
			// This is always an vector of texture backgrounds, the object code
			// shortcircuts this variable and returns a newly constructed object
			parsed_dynamic_background db = std::vector<texture_background_info>{};

			rapidxml::xml_document<> doc;
			doc.parse<rapidxml::parse_default>(&input_string[0]);

			// OpenBVE Node is Optional
			auto* openbve_node = doc.first_node("openbve", 0, false);
			rapidxml::xml_node<char>* current_section;
			if (openbve_node != nullptr) {
				current_section = openbve_node->first_node("background", 0, false);
			}
			else {
				current_section = doc.first_node("background", 0, false);
			}

			for (; current_section != nullptr;
			     current_section->next_sibling("background", 0, false)) {
				auto* object = current_section->first_node("object", 0, false);

				// Only one object background allowed, and it takes priority
				if (object != nullptr) {
					std::string object_filename(object->value(), object->value_size());

					auto absolute = get_relative_file(filename, object_filename);

					return object_background_info{absolute};
				}

				texture_background_info tbi;

				auto* texture = current_section->first_node("texture", 0, false);
				auto* repetitions = current_section->first_node("repetitions", 0, false);
				auto* mode = current_section->first_node("mode", 0, false);
				auto* transition_time = current_section->first_node("transitiontime", 0, false);
				auto* time = current_section->first_node("time", 0, false);

				if (texture != nullptr) {
					std::string texture_filename(texture->value(), texture->value_size());

					tbi.filename = get_relative_file(filename, texture_filename);
				}

				if (repetitions != nullptr) {
					try {
						tbi.repetitions = std::size_t(util::parse_loose_integer(
						    std::string(repetitions->value(), repetitions->value_size())));
					}
					catch (const std::invalid_argument& e) {
						errors::add_error(errors, filename, 0, e.what());
					}
				}

				if (mode != nullptr) {
					std::string mode_text =
					    util::lower_copy(std::string(mode->value(), mode->value_size()));

					if (mode_text == "fadein"s) {
						tbi.transition_mode = texture_background_info::FadeIn;
					}
					else if (mode_text == "fadeout"s) {
						tbi.transition_mode = texture_background_info::FadeOut;
					}
					else {
						if (mode_text != "none"s) {
							std::ostringstream err;
							err << "Unrecognized texture mode: \"" << mode->value()
							    << R"(" assuming "None")";
							errors::add_error(errors, filename, 0, err.str());
						}
						tbi.transition_mode = texture_background_info::None;
					}
				}

				if (transition_time != nullptr) {
					try {
						tbi.transition_time = std::size_t(util::parse_loose_integer(
						    std::string(transition_time->value(), transition_time->value_size())));
					}
					catch (const std::invalid_argument& e) {
						errors::add_error(errors, filename, 0, e.what());
					}
				}

				if (time != nullptr) {
					try {
						tbi.time = std::size_t(
						    util::parse_time(std::string(time->value(), time->value_size())));
					}
					catch (const std::invalid_argument& e) {
						errors::add_error(errors, filename, 0, e.what());
					}
				}

				// No need to check as we know the type for sure within this
				// function (see above)
				db.get_unchecked<std::vector<texture_background_info>>().emplace_back(
				    std::move(tbi));
			}

			return db;
		}
	} // namespace dynamic_background
} // namespace xml
} // namespace parsers
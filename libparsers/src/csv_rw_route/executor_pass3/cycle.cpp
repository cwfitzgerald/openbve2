#include "../executor_pass3.hpp"
#include <ostream>
#include <sstream>

namespace parsers {
namespace csv_rw_route {
	boost::optional<filename_set_iterator> get_cycle_filename_index(
	    const std::unordered_map<std::size_t, std::vector<std::size_t>>& cycle_mapping,
	    const std::unordered_map<std::size_t, filename_set_iterator>& object_mapping,
	    std::size_t index,
	    std::size_t position) {
		auto cycle_iterator = cycle_mapping.find(index);
		if (cycle_iterator != cycle_mapping.end()) {
			auto index_to_use = (position / 25) % cycle_iterator->second.size();

			auto to_use_iter = object_mapping.find(cycle_iterator->second[index_to_use]);

			if (to_use_iter == object_mapping.end()) {
				return boost::none;
			}
			else {
				return to_use_iter->second;
			}
		}
		else {
			auto to_use_iter = object_mapping.find(index);

			if (to_use_iter == object_mapping.end()) {
				return boost::none;
			}
			else {
				return to_use_iter->second;
			}
		}
	}

	void print_cycle_type(std::ostream& o, const cycle_type& c) {
		o << "Cycle of: (";
		std::size_t i = 0;
		for (auto& item : c) {
			o << "#" << item;
			if (++i != c.size()) {
				o << ", ";
			}
			o << ")";
		}
	}

	void pass3_executor::operator()(const instructions::cycle::Ground& inst) {
		std::vector<std::size_t> cycle;

		for (auto& ground_index : inst.input_indices) {
			cycle.emplace_back(ground_index);
		}

		auto insert_ret = cycle_ground_mapping.insert({inst.cycle_structure_index, cycle});

		auto& iterator = insert_ret.first;
		auto& inserted = insert_ret.second;

		if (!inserted) {
			auto old_value = iterator->second;
			iterator->second = cycle;

			std::ostringstream err;

			err << "Cycle.Ground overwriting #" << inst.cycle_structure_index << ". Old Filename: \"";
			print_cycle_type(err, old_value);
			err << "\". Current Filename: \"";
			print_cycle_type(err, cycle);
			err << "\".";

			_errors[get_filename(inst.file_index)].emplace_back<errors::error_t>({inst.line, err.str()});
		}
	}

	void pass3_executor::operator()(const instructions::cycle::Rail& inst) {
		std::vector<std::size_t> cycle;

		for (auto& ground_index : inst.input_indices) {
			cycle.emplace_back(ground_index);
		}

		auto insert_ret = cycle_rail_mapping.insert({inst.cycle_structure_index, cycle});

		auto& iterator = insert_ret.first;
		auto& inserted = insert_ret.second;

		if (!inserted) {
			auto old_value = iterator->second;
			iterator->second = cycle;

			std::ostringstream err;

			err << "Cycle.Rail overwriting index #" << inst.cycle_structure_index << ". Old Filename: \"";
			print_cycle_type(err, old_value);
			err << "\". Current Filename: \"";
			print_cycle_type(err, cycle);
			err << "\".";

			_errors[get_filename(inst.file_index)].emplace_back<errors::error_t>({inst.line, err.str()});
		}
	}
} // namespace csv_rw_route
} // namespace parsers

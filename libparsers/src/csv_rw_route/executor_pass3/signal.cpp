#include "../executor_pass3.hpp"
#include <sstream>

namespace parsers {
namespace csv_rw_route {
	void pass3_executor::operator()(const instructions::naked::SignalAnimated& inst) {
		auto this_signal = animated_signal{inst.filename};
		auto insert_pair = std::make_pair(inst.signal_index, this_signal);

		auto insert_ret = signal_mapping.insert(insert_pair);

		auto iterator = insert_ret.first;
		bool inserted = insert_ret.second;

		if (!inserted) {
			auto old_value = iterator->second;
			iterator->second = this_signal;

			std::ostringstream oss;
			oss << "Signal(Animated) is overwriting signal at index (" << inst.signal_index << "). Old Value: ";
			old_value.match([&oss](const animated_signal& as) -> void { oss << "\"" << as.filename << "\"\n"; },
			                [&oss](const traditional_signal& ts) -> void {
				                oss << "(\"" << ts.glow_filename << ".{x,csv,b3d}\", \"" << ts.glow_filename
				                    << ".{bmp,png,jpg}\")";
			                });
			oss << ". New Value: \"" << inst.filename << "\".";
			_errors[get_filename(inst.file_index)].emplace_back<errors::error_t>({inst.line, oss.str()});
		}
	}

	void pass3_executor::operator()(const instructions::naked::Signal& inst) {
		auto this_signal = traditional_signal{inst.signal_filename, inst.glow_filename};

		auto insert_pair = std::make_pair(inst.signal_index, this_signal);

		auto insert_ret = signal_mapping.insert(insert_pair);

		auto iterator = insert_ret.first;
		bool inserted = insert_ret.second;

		if (!inserted) {
			auto old_value = iterator->second;
			iterator->second = this_signal;

			std::ostringstream oss;

			auto print_animated_file = [&oss](const animated_signal& as) -> void {
				oss << "\"" << as.filename << "\"\n";
			};
			auto print_traditional_tuple = [&oss](const traditional_signal& ts) -> void {
				oss << "(\"" << ts.glow_filename << ".{x,csv,b3d}\", \"" << ts.glow_filename << ".{bmp,png,jpg}\")";
			};

			oss << "Signal(Traditional) is overwriting signal at index (" << inst.signal_index << "). Old Value: ";
			old_value.match(print_animated_file, print_traditional_tuple);
			oss << "New Value: ";
			print_traditional_tuple(this_signal);
			oss << ".";
			_errors[get_filename(inst.file_index)].emplace_back<errors::error_t>({inst.line, oss.str()});
		}
	}
} // namespace csv_rw_route
} // namespace parsers

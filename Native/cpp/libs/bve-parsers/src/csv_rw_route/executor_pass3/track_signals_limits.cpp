#include "executor_pass3.hpp"
#include <algorithm>
#include <sstream>

namespace bve::parsers::csv_rw_route {
	void Pass3Executor::operator()(const instructions::track::Limit& inst) const {
		TrackLimit tli;
		tli.position = inst.absolute_position;
		tli.value = inst.speed;
		route_data_.limits.emplace_back(std::move(tli));

		if (inst.post != instructions::track::Limit::Post::none) {
			std::ostringstream obj_name;
			obj_name << "\034compat\034/limit/";

			if (inst.speed != 0) {
				if (inst.post == instructions::track::Limit::Post::left) {
					obj_name << "left-side/";
				}
				else {
					obj_name << "right-side/";
				}

				switch (inst.course) {
					case instructions::track::Limit::Course::left:
						obj_name << "left_bound";
						break;
					default:
					case instructions::track::Limit::Course::none:
						obj_name << "no_bound";
						break;
					case instructions::track::Limit::Course::right:
						obj_name << "right_bound";
						break;
				}
			}
			else {
				obj_name << "no_restriction";
			}

			RailObjectInfo roi;
			roi.filename = addObjectFilename(obj_name.str());
			roi.position = trackPositionAt(inst.absolute_position).position;
			roi.rotation = glm::vec3(0);
			route_data_.objects.emplace_back(std::move(roi));
		}
	}

	void Pass3Executor::operator()(const instructions::track::Section& inst) const {
		Section si;
		if (section_behavior_ == instructions::options::SectionBehavior::Mode::normal) {
			si.position = inst.absolute_position;
			si.value = inst.a_term;
		}
		else {
			si.position = inst.absolute_position;

			auto tmp = inst.a_term;

			std::sort(tmp.begin(), tmp.end());
			tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());

			for (std::size_t i = 0, num = 0; i - 1 < tmp.size(); ++num) {
				if (tmp[i + 1] == num) {
					++i;
				}
				si.value.emplace_back(tmp[i]);
			}
			si.value.emplace_back(tmp.back());
		}
		route_data_.sections.emplace_back(std::move(si));
	}

	void Pass3Executor::operator()(const instructions::track::SigF& inst) {
		auto signal_info_iter = signal_mapping_.find(inst.signal_index);

		std::ostringstream name;
		if (signal_info_iter == signal_mapping_.end()) {
			switch (inst.signal_index) {
				default:
				case 3:
					name << "\034compat\034/signal/3";
					break;
				case 4:
					name << "\034compat\034/signal/4a";
					break;
				case 5:
					name << "\034compat\034/signal/5a";
					break;
				case 6:
					name << "\034compat\034/signal/relay";
					break;
			}
		}
		else if (signal_info_iter->second.is<AnimatedSignal>()) {
			name << signal_info_iter->second.get_unchecked<AnimatedSignal>().filename;
		}
		else {
			auto& sig = signal_info_iter->second.get_unchecked<TraditionalSignal>();
			name << "\034compat\034/user_signal/" << sig.signal_filename << "/\034/" << sig.glow_filename;
		}

		RailObjectInfo roi;
		roi.filename = addObjectFilename(name.str());
		if (inst.y_offset >= 0) {
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, inst.y_offset);
		}
		else {
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, 4.8F);
		}
		roi.rotation = glm::vec3(0);

		route_data_.objects.emplace_back(std::move(roi));
	}

	void Pass3Executor::operator()(const instructions::track::Signal& inst) {
		Section si;
		si.position = inst.absolute_position;

		std::ostringstream name;
		switch (inst.type) {
			case instructions::track::Signal::Aspect::r_y:
				name << "\034compat\034/signal/2a";
				si.value = {0, 1};
				break;
			default:
			case instructions::track::Signal::Aspect::r_g:
				name << "\034compat\034/signal/2b";
				si.value = {0, 1};
				break;
			case instructions::track::Signal::Aspect::r_y_g:
				name << "\034compat\034/signal/3";
				si.value = {0, 1, 2};
				break;
			case instructions::track::Signal::Aspect::r_yy_y_g:
				name << "\034compat\034/signal/4a";
				si.value = {0, 1, 2, 3};
				break;
			case instructions::track::Signal::Aspect::r_y_yg_g:
				name << "\034compat\034/signal/4b";
				si.value = {0, 1, 2, 3};
				break;
			case instructions::track::Signal::Aspect::r_yy_y_yg_g:
				name << "\034compat\034/signal/5a";
				si.value = {0, 1, 2, 3, 4};
				break;
			case instructions::track::Signal::Aspect::r_y_yg_g_gg:
				name << "\034compat\034/signal/5b";
				si.value = {0, 1, 2, 3, 4};
				break;
			case instructions::track::Signal::Aspect::r_yy_y_yg_g_gg:
				name << "\034compat\034/signal/6";
				si.value = {0, 1, 2, 3, 4, 5};
				break;
		}

		route_data_.sections.emplace_back(std::move(si));

		if (inst.x_offset == 0) {
			return;
		}

		RailObjectInfo roi;

		roi.filename = addObjectFilename(name.str());
		if (inst.x_offset != 0 && inst.y_offset < 0) {
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, 4.8F);
		}
		else {
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, inst.y_offset);
		}
		roi.rotation = glm::vec3(0);
	}

	void Pass3Executor::operator()(const instructions::track::Relay& inst) {
		if (inst.x_offset == 0) {
			return;
		}

		RailObjectInfo roi;
		roi.filename = addObjectFilename("\034compat\034/signal/relay");
		if (inst.x_offset != 0 && inst.y_offset < 0) {
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, 4.8F);
		}
		else {
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, inst.y_offset);
		}
		roi.rotation = glm::vec3(0);
	}
} // namespace bve::parsers::csv_rw_route

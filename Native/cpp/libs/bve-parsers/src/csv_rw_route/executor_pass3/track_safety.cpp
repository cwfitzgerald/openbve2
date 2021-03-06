#include "executor_pass3.hpp"
#include <limits>
#include <sstream>

using namespace std::string_literals;

namespace bve::parsers::csv_rw_route {
	void Pass3Executor::operator()(const instructions::track::Beacon& inst) {
		Beacon bi;

		bi.position = inst.absolute_position;
		bi.beacon_type = inst.type;
		bi.beacon_data = inst.data;
		bi.section_offset = inst.section;

		route_data_.beacons.emplace_back(std::move(bi));

		if (inst.beacon_structure_index != std::numeric_limits<std::size_t>::max()) {
			return;
		}

		RailObjectInfo roi;

		auto const file_iter = object_beacon_mapping_.find(inst.beacon_structure_index);

		if (file_iter == object_beacon_mapping_.end()) {
			std::ostringstream oss;

			oss << "Beacon Structure #" << inst.beacon_structure_index << " isn't mapped. Use Structure.Beacon to declare it.";

			add_error(errors_, getFilename(inst.file_index), inst.line, oss);
		}
		else {
			roi.filename = file_iter->second;
			roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, inst.y_offset);
			// TODO(cwfitzgerald): convert PYR to angle vector
			/* roi.rotation = */

			route_data_.objects.emplace_back(std::move(roi));
		}
	}

	void Pass3Executor::operator()(const instructions::track::Transponder& inst) {
		Beacon bi;

		bi.position = inst.absolute_position;
		bi.beacon_type = static_cast<std::underlying_type<decltype(inst.type)>::type>(inst.type);
		if (bi.beacon_type != 2) {
			bi.beacon_data = inst.switch_system ? 0 : -1;
		}
		else {
			bi.beacon_data = 0;
		}
		bi.section_offset = inst.signal;

		route_data_.beacons.emplace_back(std::move(bi));

		RailObjectInfo roi;

		switch (inst.type) {
			case instructions::track::Transponder::Type::s_type:
				roi.filename = addObjectFilename("\034compat\034/transponder/S"s);
				break;
			case instructions::track::Transponder::Type::sn_type:
				roi.filename = addObjectFilename("\034compat\034/transponder/SN"s);
				break;
			case instructions::track::Transponder::Type::departure:
				roi.filename = addObjectFilename("\034compat\034/transponder/AccidentalDep"s);
				break;
			case instructions::track::Transponder::Type::ats_p_renewal:
				roi.filename = addObjectFilename("\034compat\034/transponder/ATSP-Pattern"s);
				break;
			case instructions::track::Transponder::Type::ats_p_stop:
				roi.filename = addObjectFilename("\034compat\034/transponder/ATSP-Immediate"s);
				break;
			default:
				assert(false);
				break;
		}

		roi.position = positionRelativeToRail(0, inst.absolute_position, inst.x_offset, inst.y_offset);
		// TODO(cwfitzgerald): convert PYR to angle vector
		/* roi.rotation = */

		route_data_.objects.emplace_back(std::move(roi));
	}

	void Pass3Executor::operator()(const instructions::track::Pattern& inst) const {
		ATSPSection asi;

		asi.position = inst.absolute_position;
		asi.permanent = bool(inst.type) ? bool(decltype(inst.type)::permanent) : bool(decltype(inst.type)::temporary);
		asi.speed = inst.speed;

		route_data_.patterns.emplace_back(std::move(asi));
	}
} // namespace bve::parsers::csv_rw_route

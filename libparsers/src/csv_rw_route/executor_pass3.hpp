#pragma once

#include "core/math.hpp"
#include "csv_rw_route.hpp"
#include "parsers/find_relative_file.hpp"
#include "utils.hpp"
#include <boost/functional/hash.hpp>
#include <functional>
#include <iosfwd>

namespace parsers {
namespace csv_rw_route {
	using cycle_type = std::vector<std::size_t>;

	// defined in executor_pass3/cycle.cpp
	boost::optional<filename_set_iterator> get_cycle_filename_index(
	    const std::unordered_map<std::size_t, std::vector<std::size_t>>& cycle_mapping,
	    const std::unordered_map<std::size_t, filename_set_iterator>& object_mapping,
	    std::size_t index,
	    std::size_t position);
	void print_cycle_type(std::ostream& o, const cycle_type& c);

	struct rail_state {
		float x_offset = 0;
		float y_offset = 0;
		std::size_t rail_structure_index = 0;
		float position_last_updated = 0;
		bool active = false;

		std::size_t ground_index = 0;
		float position_ground_updated = 0;

		std::size_t wallL_index = 0;
		std::size_t wallR_index = 0;
		std::size_t dikeL_index = 0;
		std::size_t dikeR_index = 0;

		std::size_t pole_additional_rails = 0;
		std::size_t pole_location = 0;
		std::intmax_t pole_interval = 0;
		std::size_t pole_structure_index = 0;

		float position_wallL_updated = 0;
		float position_wallR_updated = 0;
		float position_dikeL_updated = 0;
		float position_dikeR_updated = 0;
		float position_pole_updated = 0;

		bool wallL_active = false;
		bool wallR_active = false;
		bool dikeL_active = false;
		bool dikeR_active = false;
		bool pole_active = false;
	};

	struct pass3_executor {
	  private:
		errors::multi_error& _errors;
		const std::vector<std::string>& _filenames;
		parsed_route_data& _route_data;
		const find_relative_file_func& _get_relative_file;

		// state variables
		std::vector<float> units_of_length = {1, 1};
		float unit_of_speed = 1;
		decltype(instructions::options::SectionBehavior::mode) section_behavior;

		// rall state
		std::unordered_map<std::size_t, rail_state> current_rail_state = {{0, rail_state{0, 0, 0, 0, true}}};

		// structures and poles
		std::unordered_map<std::size_t, filename_set_iterator> object_ground_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_rail_mapping;
		std::unordered_map<std::size_t, std::vector<std::size_t>> cycle_ground_mapping;
		std::unordered_map<std::size_t, std::vector<std::size_t>> cycle_rail_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_wallL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_wallR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_dikeL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_dikeR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_formL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_formR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_formCL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_formCR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_roofL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_roofR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_roofCL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_roofCR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_crackL_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_crackR_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_freeobj_mapping;
		std::unordered_map<std::size_t, filename_set_iterator> object_beacon_mapping;
		// Poles are unique based on the number of rails as well as the pole
		// structure index
		std::unordered_map<std::pair<std::size_t, std::size_t>,
		                   filename_set_iterator,
		                   boost::hash<std::pair<std::size_t, std::size_t>>>
		    object_pole_mapping;

		// Background indices
		std::unordered_map<std::size_t, xml::dynamic_backgrounds::parsed_dynamic_background> background_mapping;

		// Signal indices
		std::unordered_map<std::size_t, signal_info> signal_mapping;

		// error checking values
		bool used_dynamic_light = false;

		struct file_index_line_pair {
			std::size_t file_index;
			std::size_t line;
		};

		// blame indices to get the line number that committed an error
		// these are verified in the verificiation pass
		std::unordered_map<std::size_t, file_index_line_pair> rail_runsound_blame;
		std::unordered_map<std::size_t, file_index_line_pair> rail_flangesound_blame;

		// helper functions
		const std::string& get_filename(std::size_t index) {
			return _filenames[index];
		}

		filename_set_iterator add_object_filename(const std::string& val) {
			return _route_data.object_filenames.insert(util::lower_copy(val)).first;
		}

		filename_set_iterator add_texture_filename(const std::string& val) {
			return _route_data.texture_filenames.insert(util::lower_copy(val)).first;
		}

		filename_set_iterator add_sound_filename(const std::string& val) {
			return _route_data.sound_filenames.insert(util::lower_copy(val)).first;
		}

		// defined in executor_pass3/util.cpp
		float ground_height_at(float position);
		openbve2::math::evaulate_curve_t track_position_at(float position);
		glm::vec3 position_relative_to_rail(std::size_t rail_num, float position, float x_offset, float y_offset);

	  public:
		pass3_executor(parsed_route_data& rd,
		               errors::multi_error& e,
		               const std::vector<std::string>& fn,
		               const find_relative_file_func& grf) :
		    _errors(e),
		    _filenames(fn),
		    _route_data(rd),
		    _get_relative_file(grf){};

		// defined in executor_pass3/finalize.cpp
		// ensure all state is dumped to the structure
		void finalize(float max_position);

		// unused instructions
		template <class T>
		void operator()(const T& /*unused*/) {}

		// defined in executor_pass3/options.cpp
		void operator()(const instructions::options::UnitOfLength& /*inst*/);
		void operator()(const instructions::options::UnitOfSpeed& /*inst*/);
		void operator()(const instructions::options::SectionBehavior& /*inst*/);
		/*void operator()(const instructions::options::FogBehavior&);*/
		void operator()(const instructions::options::CompatibleTransparencyMode& /*inst*/);
		void operator()(const instructions::options::EnableBveTsHacks& /*inst*/);

		// defined in executor_pass3/route.cpp
		void operator()(const instructions::route::Comment& /*inst*/);
		void operator()(const instructions::route::Image& /*inst*/);
		void operator()(const instructions::route::Timetable& /*inst*/);
		void operator()(const instructions::route::Change& /*inst*/);
		void operator()(const instructions::route::Guage& /*inst*/);
		void operator()(const instructions::route::Signal& /*inst*/);
		void operator()(const instructions::route::RunInterval& /*inst*/);
		void operator()(const instructions::route::AccelerationDueToGravity& /*inst*/);
		void operator()(const instructions::route::Elevation& /*inst*/);
		void operator()(const instructions::route::Temperature& /*inst*/);
		void operator()(const instructions::route::Pressure& /*inst*/);
		void operator()(const instructions::route::DisplaySpeed& /*inst*/);
		void operator()(const instructions::route::LoadingScreen& /*inst*/);
		void operator()(const instructions::route::StartTime& /*inst*/);
		void operator()(const instructions::route::DynamicLight& /*inst*/);
		void operator()(const instructions::route::AmbiantLight& /*inst*/);
		void operator()(const instructions::route::DirectionalLight& /*inst*/);
		void operator()(const instructions::route::LightDirection& /*inst*/);

		// defined in executor_pass3/train.cpp
		void operator()(const instructions::train::Folder& /*inst*/);
		void operator()(const instructions::train::Rail& /*inst*/);
		void operator()(const instructions::train::Flange& /*inst*/);
		void operator()(const instructions::train::Timetable& /*inst*/);
		void operator()(const instructions::train::Velocity& /*inst*/);

		// defined in executor_pass3/structure.cpp
		void operator()(const instructions::structure::Command& /*inst*/);
		void operator()(const instructions::structure::Pole& /*inst*/);

		// defined in executor_pass3/texture.cpp
		// helper functions for background_load
	  private:
		void background_load_xml(const instructions::texture::Background_Load& /*inst*/);
		void background_load_image(const instructions::texture::Background_Load& /*inst*/);

	  public:
		void operator()(const instructions::texture::Background_Load& /*inst*/);
		void operator()(const instructions::texture::Background_X& /*inst*/);
		void operator()(const instructions::texture::Background_Aspect& /*inst*/);

		// defined in executor_pass3/cycle.cpp
		void operator()(const instructions::cycle::Ground& /*inst*/);
		void operator()(const instructions::cycle::Rail& /*inst*/);

		// defined in executor_pass3/signal.cpp
		void operator()(const instructions::naked::SignalAnimated& /*inst*/);
		void operator()(const instructions::naked::Signal& /*inst*/);

	  private:
		void add_rail_objects_up_to_position(rail_state& state, float position);

	  public:
		// defined in executor_pass3/rails.cpp
		void operator()(const instructions::track::RailStart& /*inst*/);
		void operator()(const instructions::track::Rail& /*inst*/);
		void operator()(const instructions::track::RailType& /*inst*/);
		void operator()(const instructions::track::RailEnd& /*inst*/);
		void operator()(const instructions::track::Adhesion& /*inst*/);

	  private:
		void add_wall_objects_up_to_position(rail_state& state, float position, uint8_t type);
		void add_poll_objects_up_to_position(std::size_t rail_number, rail_state& state, float position);
		void add_ground_objects_up_to_position(rail_state& state, float position);

	  public:
		// defined in executor_pass3/objects.cpp
		void operator()(const instructions::track::FreeObj& /*inst*/);
		void operator()(const instructions::track::Wall& /*inst*/);
		void operator()(const instructions::track::WallEnd& /*inst*/);
		void operator()(const instructions::track::Dike& /*inst*/);
		void operator()(const instructions::track::DikeEnd& /*inst*/);
		void operator()(const instructions::track::Pole& /*inst*/);
		void operator()(const instructions::track::PoleEnd& /*inst*/);
		void operator()(const instructions::track::Crack& /*inst*/);
		void operator()(const instructions::track::Ground& /*inst*/);

		// defined in executor_pass3/stations.cpp
		void operator()(const instructions::track::Sta& /*inst*/);
		void operator()(const instructions::track::Stop& /*inst*/);
		void operator()(const instructions::track::Form& /*inst*/);

		// defined in executor_pass3/signalling.cpp
		void operator()(const instructions::track::Limit& /*inst*/);
		void operator()(const instructions::track::Section& /*inst*/);
		void operator()(const instructions::track::SigF& /*inst*/);
		void operator()(const instructions::track::Signal& /*inst*/);
		void operator()(const instructions::track::Relay& /*inst*/);

		// defined in executor_pass3/safety.cpp
		void operator()(const instructions::track::Beacon& /*inst*/);
		void operator()(const instructions::track::Transponder& /*unused*/);
		void operator()(const instructions::track::Pattern& /*unused*/);

		// defined in executor_pass3/misc.cpp
		void operator()(const instructions::track::Back& /*unused*/);
		void operator()(const instructions::track::Fog& /*unused*/);
		void operator()(const instructions::track::Brightness& /*unused*/);
		void operator()(const instructions::track::Marker& /*unused*/);
		void operator()(const instructions::track::MarkerXML& /*unused*/);
		void operator()(const instructions::track::TextMarker& /*unused*/);
		void operator()(const instructions::track::PointOfInterest& /*unused*/);
		void operator()(const instructions::track::PreTrain& /*unused*/);
		void operator()(const instructions::track::Announce& /*unused*/);
		void operator()(const instructions::track::Doppler& /*unused*/);
		void operator()(const instructions::track::Buffer& /*unused*/);
	};
} // namespace csv_rw_route
} // namespace parsers

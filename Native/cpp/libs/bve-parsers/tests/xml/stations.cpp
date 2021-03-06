#include "sample_relative_file_func.hpp"
#include "util/parsing.hpp"
#include <doctest/doctest.h>
#include <ostream>
#include <parsers/xml/stations.hpp>

namespace st = bve::parsers::xml::stations;
using namespace std::string_literals;

TEST_SUITE_BEGIN("libparsers - xml - stations and request stops");

TEST_CASE("libparsers - xml - stations and request stops - parse station marker") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<Name>Dockyard</Name>"
	    "	 	<ArrivalTime>12.3130</ArrivalTime>"
	    "		<ArrivalSound>ding.wav</ArrivalSound>"
	    "		<DepartureTime>12.3150</DepartureTime>"
	    "		<DepartureSound>dong.wav</DepartureSound>"
	    "		<Doors>Left</Doors>"
	    "		<ForcedRedSignal>False</ForcedRedSignal>"
	    "		<PassengerRatio>10</PassengerRatio>"
	    "		<StopDuration>10</StopDuration>"
	    "		<TimeTableIndex>2</TimeTableIndex>"
	    "	</Station>"
	    "</openBVE>"s;
	bve::parsers::errors::MultiError output_errors;

	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(output_errors.empty());
	CHECK_EQ(output_errors.size(), 0);

	CHECK_EQ(station_marker.arrival_time, 45090);
	CHECK(station_marker.using_arrival);
	CHECK_EQ(station_marker.arrival_sound_file, "some_file.xml/ding.wav"s);
	CHECK_EQ(station_marker.departure_time, 45110);
	CHECK(station_marker.using_departure);
	CHECK_EQ(station_marker.departure_sound_file, "some_file.xml/dong.wav"s);
	CHECK_EQ(station_marker.door, st::ParsedStationMarker::Doors::left);
	CHECK_EQ(station_marker.force_red_signal, false);
	CHECK_EQ(station_marker.passenger_ratio, 10);
	CHECK_EQ(station_marker.station_name, "Dockyard"s);
	CHECK_EQ(station_marker.stop_duration, 10);
	CHECK_EQ(station_marker.time_table_index, 2);
}

TEST_CASE("libparsers -xml - stations and request stops - stations should add errors") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<Name>Dockyard</Name>"
	    "		<ArrivalTime>ERROR</ArrivalTime>"
	    "		<ArrivalSound>ding.wav</ArrivalSound>"
	    "		<DepartureTime>ERROR</DepartureTime>"
	    "		<DepartureSound>dong.wav</DepartureSound>"
	    "		<Doors>DOOOORS</Doors>"
	    "		<ForcedRedSignal>BOTH</ForcedRedSignal>"
	    "		<PassengerRatio>-1</PassengerRatio>"
	    "		<StopDuration>WECANSTOP</StopDuration>"
	    "		<TimeTableIndex>-1</TimeTableIndex>"
	    "	</Station>"
	    "</openBVE>"s;
	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(!output_errors.empty());
	CHECK_EQ(output_errors["some_file.xml"].size(), 5);

	CHECK_EQ(station_marker.arrival_time, 0);
	CHECK(!station_marker.using_arrival);
	CHECK_EQ(station_marker.departure_time, 0);
	CHECK(!station_marker.using_departure);
	CHECK_EQ(station_marker.door, st::ParsedStationMarker::Doors::none);
	CHECK_EQ(station_marker.stop_duration, 15);
}

TEST_CASE("libparsers - xml - stations and request stops - parse request stop marker") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<RequestStop>"
	    "			<Probability>50</Probability>"
	    "			<Distance>700</Distance>"
	    "			<StopMessage>You will need to stop at Dockyard today.</StopMessage>"
	    "			<PassMessage>No passengers for Dockyard.</PassMessage>"
	    "			<MaxCars>10</MaxCars>"
	    "			<AIBehaviour>FullSpeed</AIBehaviour>"
	    "		</RequestStop>"
	    "	</Station>"
	    "</openBVE>"s;

	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(output_errors.empty());
	CHECK_EQ(output_errors.size(), 0);

	CHECK_EQ(station_marker.request_stop.probability.ontime, 50);
	CHECK_EQ(station_marker.request_stop.distance, 700);
	CHECK_EQ(station_marker.request_stop.stop_message.ontime, "You will need to stop at Dockyard today."s);
	CHECK_EQ(station_marker.request_stop.pass_message.ontime, "No passengers for Dockyard."s);
	CHECK_EQ(station_marker.request_stop.max_cars, 10);
	CHECK_EQ(station_marker.request_stop.ai_behaviour, st::RequestStopMarker::Behaviour::full_speed);
}

TEST_CASE("libparser - xml - stations and request stops - request stops should add errors") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<RequestStop>"
	    "			<Probability>-1</Probability>"
	    "			<Distance>A</Distance>"
	    "			<StopMessage>You will need to stop at Dockyard today.</StopMessage>"
	    "			<PassMessage>No passengers for Dockyard.</PassMessage>"
	    "			<MaxCars>-1</MaxCars>"
	    "			<AIBehaviour>ERROR</AIBehaviour>"
	    "		</RequestStop>"
	    "	</Station>"
	    "</openBVE>"s;

	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(!output_errors.empty());
	CHECK_EQ(output_errors["some_file.xml"].size(), 4);

	CHECK_EQ(station_marker.request_stop.probability.ontime, 0);
	CHECK_EQ(station_marker.request_stop.distance, 0);
	CHECK_EQ(station_marker.request_stop.max_cars, 0);
	CHECK_EQ(station_marker.request_stop.ai_behaviour, st::RequestStopMarker::Behaviour::normal_brake);
}

TEST_CASE(
    "libparser - xml - stations and request stops - probabilities and messages can have node, "
    "early, ontime, late") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<RequestStop>"
	    "			<Probability>"
	    "			<Early>10</Early>"
	    "			<OnTime>20</OnTime>"
	    "			<Late>50</Late>"
	    "			</Probability>"
	    "			<Distance>700</Distance>"
	    "			<StopMessage>"
	    "				<Early>You will need to stop at Dockyard today.</Early>"
	    "				<OnTime>I am on time.</OnTime>"
	    "				<Late>I am late.</Late>"
	    "			</StopMessage>"
	    "			<PassMessage>"
	    "				<Early>I am a early pass message.</Early>"
	    "				<OnTime>I am a OnTime pass message.</OnTime>"
	    "				<Late>I am a late pass message.</Late>"
	    "			</PassMessage>"
	    "			<MaxCars>10</MaxCars>"
	    "			<AIBehaviour>FullSpeed</AIBehaviour>"
	    "		</RequestStop>"
	    "	</Station>"
	    "</openBVE>"s;

	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(output_errors.empty());
	CHECK_EQ(output_errors.size(), 0);
	CHECK_EQ(station_marker.request_stop.probability.early, 10);
	CHECK_EQ(station_marker.request_stop.probability.ontime, 20);
	CHECK_EQ(station_marker.request_stop.probability.late, 50);

	CHECK_EQ(station_marker.request_stop.stop_message.early, "You will need to stop at Dockyard today."s);
	CHECK_EQ(station_marker.request_stop.stop_message.ontime, "I am on time."s);
	CHECK_EQ(station_marker.request_stop.stop_message.late, "I am late."s);

	CHECK_EQ(station_marker.request_stop.pass_message.early, "I am a early pass message."s);
	CHECK_EQ(station_marker.request_stop.pass_message.ontime, "I am a OnTime pass message."s);
	CHECK_EQ(station_marker.request_stop.pass_message.late, "I am a late pass message."s);
}

TEST_CASE(
    "libparser - xml - stations and request stops - early, ontime and late probabilites should add "
    "errors") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<RequestStop>"
	    "			<Probability>"
	    "				<Early>-1</Early>"
	    "				<OnTime>-1</OnTime>"
	    "				<Late>-1</Late>"
	    "			</Probability>"
	    "			<Distance>700</Distance>"
	    "		</RequestStop>"
	    "	</Station>"
	    "</openBVE>"s;

	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(!output_errors.empty());
	CHECK_EQ(output_errors["some_file.xml"].size(), 3);
	CHECK_EQ(station_marker.request_stop.probability.early, 0);
	CHECK_EQ(station_marker.request_stop.probability.ontime, 0);
	CHECK_EQ(station_marker.request_stop.probability.late, 0);
}

TEST_CASE("libparser - xml - stations and request stops - stations can contain request stops") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<Name>Dockyard</Name>"
	    "		<ArrivalTime>12.3130</ArrivalTime>"
	    "		<ArrivalSound>ding.wav</ArrivalSound>"
	    "		<DepartureTime>12.3150</DepartureTime>"
	    "		<DepartureSound>dong.wav</DepartureSound>"
	    "		<Doors>Left</Doors>"
	    "		<ForcedRedSignal>False</ForcedRedSignal>"
	    "		<PassengerRatio>10</PassengerRatio>"
	    "		<StopDuration>10</StopDuration>"
	    "		<TimeTableIndex>2</TimeTableIndex>"
	    "		<RequestStop>"
	    "			<Probability>50</Probability>"
	    "			<Distance>700</Distance>"
	    "			<StopMessage>You will need to stop at Dockyard today.</StopMessage>"
	    "			<PassMessage>No passengers for Dockyard.</PassMessage>"
	    "			<MaxCars>10</MaxCars>"
	    "			<AIBehaviour>FullSpeed</AIBehaviour>"
	    "		</RequestStop>"
	    "	</Station>"
	    "</openBVE>"s;
	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);
	CHECK(output_errors.empty());
	CHECK_EQ(output_errors["some_file.xml"].size(), 0);
	CHECK_EQ(station_marker.arrival_time, 45090);
	CHECK(station_marker.using_arrival);
	CHECK_EQ(station_marker.arrival_sound_file, "some_file.xml/ding.wav"s);
	CHECK_EQ(station_marker.departure_time, 45110);
	CHECK(station_marker.using_departure);
	CHECK_EQ(station_marker.departure_sound_file, "some_file.xml/dong.wav"s);
	CHECK_EQ(station_marker.door, st::ParsedStationMarker::Doors::left);
	CHECK_EQ(station_marker.force_red_signal, false);
	CHECK_EQ(station_marker.passenger_ratio, 10);
	CHECK_EQ(station_marker.station_name, "Dockyard"s);
	CHECK_EQ(station_marker.stop_duration, 10);
	CHECK_EQ(station_marker.time_table_index, 2);
	CHECK_EQ(station_marker.request_stop.probability.ontime, 50);
	CHECK_EQ(station_marker.request_stop.distance, 700);
	CHECK_EQ(station_marker.request_stop.stop_message.ontime, "You will need to stop at Dockyard today."s);
	CHECK_EQ(station_marker.request_stop.pass_message.ontime, "No passengers for Dockyard."s);
	CHECK_EQ(station_marker.request_stop.max_cars, 10);
	CHECK_EQ(station_marker.request_stop.ai_behaviour, st::RequestStopMarker::Behaviour::full_speed);
}

TEST_CASE(
    "libparsers - xml - stations and request stops - stations with request stops should add "
    "errors") {
	std::string const test_value =
	    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<openBVE "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"xmlns:xsd=\"http://www.w3.org/2001/"
	    "XMLSchema\">"
	    "	<Station>"
	    "		<Name>Dockyard</Name>"
	    "		<ArrivalTime>ERROR</ArrivalTime>"
	    "		<ArrivalSound>ding.wav</ArrivalSound>"
	    "		<DepartureTime>ERROR</DepartureTime>"
	    "		<DepartureSound>dong.wav</DepartureSound>"
	    "		<Doors>DOOOORS</Doors>"
	    "		<ForcedRedSignal>BOTH</ForcedRedSignal>"
	    "		<PassengerRatio>-1</PassengerRatio>"
	    "		<StopDuration>WECANSTOP</StopDuration>"
	    "		<TimeTableIndex>-1</TimeTableIndex>"
	    "		<RequestStop>"
	    "			<Probability>-1</Probability>"
	    "			<Distance>A</Distance>"
	    "			<StopMessage>You will need to stop at Dockyard today.</StopMessage>"
	    "			<PassMessage>No passengers for Dockyard.</PassMessage>"
	    "			<MaxCars>-1</MaxCars>"
	    "			<AIBehaviour>ERROR</AIBehaviour>"
	    "		</RequestStop>"
	    "	</Station>"
	    "</openBVE>"s;

	bve::parsers::errors::MultiError output_errors;
	auto const station_marker = st::parse("some_file.xml"s, test_value, output_errors, rel_file_func);

	CHECK(!output_errors.empty());
	CHECK_EQ(output_errors["some_file.xml"].size(), 9);

	CHECK_EQ(station_marker.arrival_time, 0);
	CHECK(!station_marker.using_arrival);
	CHECK_EQ(station_marker.departure_time, 0);
	CHECK(!station_marker.using_departure);
	CHECK_EQ(station_marker.door, st::ParsedStationMarker::Doors::none);
	CHECK_EQ(station_marker.stop_duration, 15);
	CHECK_EQ(station_marker.request_stop.probability.ontime, 0);
	CHECK_EQ(station_marker.request_stop.distance, 0);
	CHECK_EQ(station_marker.request_stop.max_cars, 0);
	CHECK_EQ(station_marker.request_stop.ai_behaviour, st::RequestStopMarker::Behaviour::normal_brake);
}

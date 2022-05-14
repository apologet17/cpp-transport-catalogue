#pragma once

#include <string>
#include <vector>
#include <deque>

#include "geo.h"
#include "transport_catalogue.h"



namespace catalogue_core {

	using namespace transportcatalogue;

	namespace inputreader {

		using namespace std::string_literals;

		const auto ADD_BUS_NAME = "Bus"s;
		const auto ADD_STOP_NAME = "Stop"s;

		enum QueryType {
			ADD_BUS,
			ADD_STOP
		};

		struct BusRouteRaw {
			bool circular;
			std::string name;
			std::vector<std::string> stops;
		};

		struct BusStopRaw {
			transportcatalogue::BusStop bus_stop;
			std::string lengths;
		};

		struct Query {
			QueryType query_type;
			BusStopRaw bus_stop;
			BusRouteRaw bus_route;
		};

		class InputReader {

		public:
			InputReader(transportcatalogue::TransportCatalogue* catalogue)
				: catalogue_(catalogue) {
			}
			void AdditionToCatalogue(std::istream& is);

		private:
			void LoadBufferToCatalogue();

			std::deque<BusRouteRaw> bus_route_buffer_;
			std::deque<BusStopRaw> bus_stop_buffer_;
			transportcatalogue::TransportCatalogue* catalogue_;
		};

		std::istream& operator>>(std::istream& is, Query& q);
	}
}
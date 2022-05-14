#pragma once

#include<list>
#include<string>
#include<set>
#include<unordered_map>
#include <optional> 

#include "geo.h"

namespace catalogue_core {

	namespace transportcatalogue {

		struct RouteStatistic {
			size_t num_of_stops_;
			size_t num_of_unique_stops_;
			int route_length_;
			double curvature;
		};

		struct BusStop {
			std::string name;
			Coordinates coordinates;
		};

		struct BusRoute {
			bool circular;
			std::string name;
			std::vector<std::list<BusStop>::iterator> stops;
		};

		class StopLengthsHasher {
		public:
			size_t operator()(const std::pair<std::list<BusStop>::iterator, std::list<BusStop>::iterator>& stop_len) const {

				return std::hash<void*>{}(&(*stop_len.first)) + std::hash<void*>{}(&(*stop_len.second)) * 17;
			}
		};

		class TransportCatalogue {

		public:
			TransportCatalogue() = default;

			void AddBusRoute(BusRoute& bus_route);
			void AddBusStop(BusStop& bus_stop);
			void AddLength(std::string& bus_stop_name, std::vector<std::pair<std::string, int>>& lengths);

			std::list<BusRoute>::iterator FindBusRoute(std::string& busroute_name) const;
			std::list<BusStop>::iterator FindBusStop(std::string& busstop_name) const;

			std::optional<RouteStatistic> GetInformationAboutBusRoute(std::string& busroute_name) const;
			std::optional<std::set<std::string_view, std::less<>>> GetInformationAboutStop(std::string& busroute_name) const;
			std::optional<int> GetLength(std::string& stop_name_first, std::string& stop_name_second) const;

		private:

			std::list<BusStop> bus_stops_;
			std::unordered_map<std::string_view, std::list<BusStop>::iterator> stopname_to_stops_;

			std::list<BusRoute> bus_routes_;
			std::unordered_map<std::string_view, std::list<BusRoute>::iterator> busname_to_routes_;

			std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_for_stop_;
			std::unordered_map<std::pair<std::list<BusStop>::iterator, std::list<BusStop>::iterator>, int, StopLengthsHasher> stop_lengths_;
		};
	}
}
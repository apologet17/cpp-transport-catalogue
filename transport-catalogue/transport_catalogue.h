#pragma once

#include<list>
#include<string>
#include<vector>
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
			std::vector<BusStop*> stops;
		};

		class StopLengthsHasher {
		public:
			size_t operator()(const std::pair<BusStop*, BusStop*>& stop_len) const {

				return std::hash<void*>{}(&(*stop_len.first)) + std::hash<void*>{}(&(*stop_len.second)) * 17;
			}
		};

		class TransportCatalogue {

		public:
			TransportCatalogue() = default;

			void AddBusRoute(const BusRoute& bus_route);
			void AddBusStop(const BusStop& bus_stop);
			void AddLength(const std::string& bus_stop_name, const std::vector<std::pair<std::string, int>>& lengths);

			BusRoute* FindBusRoute(const std::string& busroute_name) const;
			BusStop* FindBusStop(const std::string& busstop_name) const;

			std::optional<RouteStatistic> GetInformationAboutBusRoute(const std::string& busroute_name) const;
			std::optional<std::set<std::string_view, std::less<>>> GetInformationAboutStop(const std::string& busroute_name) const;
			std::optional<int> GetLength(const std::string& stop_name_first, const std::string& stop_name_second) const;

		private:

			std::list<BusStop> bus_stops_;
			std::unordered_map<std::string_view, BusStop*> stopname_to_stops_;

			std::list<BusRoute> bus_routes_;
			std::unordered_map<std::string_view, BusRoute*> busname_to_routes_;

			std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_for_stop_;
			std::unordered_map<std::pair<BusStop*, BusStop*>, int, StopLengthsHasher> stop_lengths_;
		};
	}
}
#pragma once

#include<list>
#include<string>
#include<vector>
#include<set>
#include<unordered_map>
#include <optional> 

#include "domain.h"

namespace catalogue_core {

namespace transport_catalogue {

class TransportCatalogue {

public:
	TransportCatalogue() = default;

	void AddBusRoute(const domain::BusRoute& bus_route);
	void AddBusStop(const domain::BusStop& bus_stop);
	//void AddLength(const std::string& bus_stop_name, const std::vector<std::pair<std::string, int>>& lengths);
	void AddLength(const std::string& bus_stop_name, const std::vector<std::string>names, const std::vector<int>& lengths);

	domain::BusRoute* FindBusRoute(const std::string_view busroute_name) const;
	domain::BusStop* FindBusStop(const std::string_view busstop_name) const;
	const std::set<std::string_view, std::less<>>& AllRoutesNames() const;

	std::optional<int> GetLength(const std::string& stop_name_first, const std::string& stop_name_second) const;
	std::optional<domain::RouteStatistic> GetInformationAboutBusRoute(const std::string& busroute_name) const;
	std::optional<std::set<std::string_view, std::less<>>> GetInformationAboutStop(const std::string& busroute_name) const;

	void SetAllRoutes();
	const std::vector<domain::BusRoute*>& GetAllRoutes() const;
	 std::vector<const domain::BusStop*> GetAllStops() const;
	 size_t GetNumStops() const;
	 size_t GetNumStopsByBus() const;

private:
	std::list<domain::BusStop> bus_stops_;
	std::unordered_map<std::string_view, domain::BusStop*> stopname_to_stops_;

	std::list<domain::BusRoute> bus_routes_;
	std::unordered_map<std::string_view, domain::BusRoute*> busname_to_routes_;
	std::set<std::string_view, std::less<>> routes_names_;

	std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_for_stop_;
	std::unordered_map<std::pair<domain::BusStop*, domain::BusStop*>, int, domain::StopLengthsHasher> stop_lengths_;

	std::vector<domain::BusRoute*> all_routes_;
	size_t num_stops_by_bus_;
		};
	}
}
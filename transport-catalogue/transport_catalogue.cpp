#include <algorithm>
#include <unordered_set>
#include "transport_catalogue.h"

namespace catalogue_core {

	namespace transportcatalogue {

		static std::set<std::string_view, std::less<>> zeroset;

		std::list<BusRoute>::iterator TransportCatalogue::FindBusRoute(std::string& busroute_name) const {
			return busname_to_routes_.at(busroute_name);
		}

		std::list<BusStop>::iterator TransportCatalogue::FindBusStop(std::string& busstop_name) const {
			return stopname_to_stops_.at(busstop_name);
		}

		void TransportCatalogue::AddBusStop(BusStop& bus_stop) {
			auto it = bus_stops_.insert(bus_stops_.begin(), bus_stop);
			stopname_to_stops_[it->name] = it;
		}

		void TransportCatalogue::AddBusRoute(BusRoute& bus_route) {
			auto it = bus_routes_.insert(bus_routes_.begin(), bus_route);
			busname_to_routes_[it->name] = it;
			for (auto i : bus_route.stops) {
				buses_for_stop_[i->name].insert(it->name);
			}
		}

		void TransportCatalogue::AddLength(std::string& bus_stop_name, std::vector<std::pair<std::string, int>>& lengths) {
			auto first_it = FindBusStop(bus_stop_name);

			for (auto& len : lengths) {
				auto second_it = FindBusStop(len.first);
				stop_lengths_[std::make_pair(first_it, second_it)] = len.second;
			}
		}

		std::optional<RouteStatistic> TransportCatalogue::GetInformationAboutBusRoute(std::string& busroute_name) const {

			RouteStatistic output;

			if (busname_to_routes_.count(busroute_name) == 0) {
				return std::nullopt;
			}

			auto it = busname_to_routes_.at(busroute_name);

			output.route_length_ = 0;
			output.curvature = 0.0;
			std::unordered_set<std::string_view> route;

			if (!it->circular) {
				for (auto first = it->stops.begin(); first != it->stops.end() - 1; ++first) {
					route.insert((*first)->name);

					auto len_forward = GetLength((*first)->name, (*(first + 1))->name);
					auto len_backward = GetLength((*(first + 1))->name, (*first)->name);

					if (len_forward.has_value() && len_backward.has_value()) {
						output.route_length_ += len_forward.value() + len_backward.value();
					}
					else if (len_forward.has_value()) {
						output.route_length_ += 2 * len_forward.value();
					}
					else {
						output.route_length_ += 2 * len_backward.value();
					}

					output.curvature += ComputeDistance(stopname_to_stops_.at((*first)->name)->coordinates,
						stopname_to_stops_.at((*(first + 1))->name)->coordinates);
				}
				output.curvature *= 2.0;
				output.num_of_stops_ = 2 * it->stops.size() - 1;
			}
			else {
				for (auto first = it->stops.begin(); first != it->stops.end() - 1; ++first) {
					route.insert((*first)->name);

					if (auto len = GetLength((*first)->name, (*(first + 1))->name); len.has_value()) {
						output.route_length_ += len.value();
					}
					else if (auto len = GetLength((*(first + 1))->name, (*first)->name); len.has_value()) {
						output.route_length_ += len.value();
					}

					output.curvature += ComputeDistance(stopname_to_stops_.at((*first)->name)->coordinates,
						stopname_to_stops_.at((*(first + 1))->name)->coordinates);
				}
				output.num_of_stops_ = it->stops.size();
			}

			output.curvature = static_cast<double>(output.route_length_) / output.curvature;
			route.insert((*(it->stops.end() - 1))->name);
			output.num_of_unique_stops_ = route.size();

			return output;
		}

		std::optional<std::set<std::string_view, std::less<>>> TransportCatalogue::GetInformationAboutStop(std::string& stop_name) const {
			if (stopname_to_stops_.count(stop_name) == 0) {
				return zeroset;
			}
			if (buses_for_stop_.count(stop_name)) {
				return buses_for_stop_.at(stop_name);
			}
			else {
				return std::nullopt;
			}
		}

		std::optional<int> TransportCatalogue::GetLength(std::string& stop_name_first, std::string& stop_name_second) const {
			auto pair_stops = std::make_pair(stopname_to_stops_.at(stop_name_first), stopname_to_stops_.at(stop_name_second));

			if (stop_lengths_.count(pair_stops)) {
				return  stop_lengths_.at(pair_stops);
			}
			else {
				return std::nullopt;
			}

		}
	}
}
#include <algorithm>
#include <unordered_set>
#include <stdexcept>
#include "transport_catalogue.h"

namespace catalogue_core {

	namespace transportcatalogue {

		static const std::set<std::string_view, std::less<>> zeroset;

		BusRoute* TransportCatalogue::FindBusRoute(const std::string& busroute_name) const {
			if (busname_to_routes_.count(busroute_name)) {
				return busname_to_routes_.at(busroute_name);
			}
			else {
				return nullptr;
			}
		}

		BusStop* TransportCatalogue::FindBusStop(const std::string& busstop_name) const {
			if (stopname_to_stops_.count(busstop_name)) {
				return stopname_to_stops_.at(busstop_name);
			}
			else {
				return nullptr;
			}
		}

		void TransportCatalogue::AddBusStop(const BusStop& bus_stop) {
			auto it = bus_stops_.insert(bus_stops_.begin(), bus_stop);
			stopname_to_stops_[it->name] = &(*it);
		}

		void TransportCatalogue::AddBusRoute(const BusRoute& bus_route) {
			auto ptr = bus_routes_.insert(bus_routes_.begin(), bus_route);
			busname_to_routes_[ptr->name] = &(*ptr);
			for (auto& stop : bus_route.stops) {
				buses_for_stop_[stop->name].insert(ptr->name);
			}
		}

		void TransportCatalogue::AddLength(const std::string& bus_stop_name, const std::vector<std::pair<std::string, int>>& lengths) {

			using namespace std::string_literals;

			if (auto first_ptr = FindBusStop(bus_stop_name); first_ptr != nullptr) {

				for (auto& len : lengths) {
					if (auto second_ptr = FindBusStop(len.first); second_ptr != nullptr) {				
						stop_lengths_[std::make_pair(first_ptr, second_ptr)] = len.second;
					}
					else {
						throw std::invalid_argument("AddLength: Unknown second bus stop name"s);
					}					
				}
			}
			else {
				throw std::invalid_argument("AddLength: Unknown first bus stop name"s);
			}
		}

		std::optional<RouteStatistic> TransportCatalogue::GetInformationAboutBusRoute(const std::string& busroute_name) const {

			RouteStatistic output;

			if (busname_to_routes_.count(busroute_name) == 0) {
				return std::nullopt;
			}

			auto route_ptr = busname_to_routes_.at(busroute_name);

			output.route_length_ = 0;
			output.curvature = 0.0;
			std::unordered_set<std::string_view> route;

			if (!route_ptr->circular) {
				for (auto first = route_ptr->stops.begin(); first != route_ptr->stops.end() - 1; ++first) {
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
				output.num_of_stops_ = 2 * route_ptr->stops.size() - 1;
			}
			else {
				for (auto first = route_ptr->stops.begin(); first != route_ptr->stops.end() - 1; ++first) {
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
				output.num_of_stops_ = route_ptr->stops.size();
			}

			output.curvature = static_cast<double>(output.route_length_) / output.curvature;
			route.insert((*(route_ptr->stops.end() - 1))->name);
			output.num_of_unique_stops_ = route.size();

			return output;
		}

		std::optional<std::set<std::string_view, std::less<>>> TransportCatalogue::GetInformationAboutStop(const std::string& stop_name) const {
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

		std::optional<int> TransportCatalogue::GetLength(const std::string& stop_name_first, const std::string& stop_name_second) const {
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
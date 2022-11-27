#include <unordered_map>
#include "transport_router.h"
#include <iostream>

namespace catalogue_core {
	namespace router {
		constexpr double DIMENSION = 60.0 / 1000.0;

		TransportRouter::TransportRouter(const RouterSettings& settings, catalogue_core::transport_catalogue::TransportCatalogue& cat)
			: cat_(cat)
			, router_settings_(settings)
			, graph_(graph::DirectedWeightedGraph<double>(2 * cat_.GetNumStops())){

			edges_content_.reserve(40 * cat_.GetNumStopsByBus());
			graph::VertexId in = 0;

			for (const auto& stop : cat_.GetAllStops()) {

				bus_stop_to_vertex_[stop].interface_vertex = in;
				bus_stop_to_vertex_[stop].bus_vertex = in+1;

				graph_.AddEdge({ in,	//создать пересадочное ребро к остановке
								in+1,
								static_cast<double>(router_settings_.bus_wait_time) });
				edges_content_.push_back( { WaitOrBus::WAIT,
												"",
											stop->name,
											router_settings_.bus_wait_time,
											0 ,
											0 });
				in += 2;
			}
			
			for (auto& route:cat_.GetAllRoutes()) {
				size_t end_stop =route->stops.size();

				for (size_t first_stop = 0; first_stop < end_stop - 1; first_stop++) {
					int time_forward = 0;
					int time_reverse = 0;
					int span_count = 0;

					auto first_vertex = bus_stop_to_vertex_[route->stops[first_stop]];

					for (size_t second_stop = first_stop+1; second_stop < end_stop; second_stop++) {

						auto second_vertex = bus_stop_to_vertex_[route->stops[second_stop]];

						time_forward += cat_.GetLength(route->stops[second_stop-1]->name, route->stops[second_stop]->name).value() ;

						graph_.AddEdge({ first_vertex.bus_vertex,	//создать прямое ребро между остановками
													second_vertex.interface_vertex,
													static_cast<double>(time_forward) / router_settings_.bus_velocity * DIMENSION });
						RoutePart part = { WaitOrBus::BUS,
										  route->name,
										  "",
										  0,
										  static_cast<double>(time_forward) / router_settings_.bus_velocity * DIMENSION ,
										  ++span_count };
						edges_content_.push_back(part);

						if (!route->circular) {
							time_reverse += cat_.GetLength(route->stops[second_stop]->name, route->stops[second_stop - 1]->name).value();
							graph_.AddEdge({ second_vertex.bus_vertex,	//создать обратное ребро между остановками
													    first_vertex.interface_vertex,
														static_cast<double>(time_reverse) / router_settings_.bus_velocity * DIMENSION });
							part.bus_time = static_cast<double>(time_reverse) / router_settings_.bus_velocity * DIMENSION;
							edges_content_.push_back(part);
						}
					}
				}
			}
			router_ = std::make_unique<graph::Router<double>>(graph_);

		}

		void TransportRouter::LoadRouterSettings(const RouterSettings& settings) {
			router_settings_ = settings;
		}

		std::optional<std::pair<std::vector<router::RoutePart>, double>> TransportRouter::BuildFastestRoute(domain::BusStop* start, domain::BusStop* finish) {		
			std::vector<router::RoutePart> output;

			if ((bus_stop_to_vertex_.count(finish) == 0) || (bus_stop_to_vertex_.count(start) == 0)) {
				return {};
			}

			if (start == finish) {
				return std::make_pair(output, 0);
			}

			auto search_result = router_->BuildRoute(bus_stop_to_vertex_.at(start).interface_vertex, bus_stop_to_vertex_.at(finish).interface_vertex);

			if (!search_result.has_value()) {
				return {};
			}
			output.reserve(search_result.value().edges.size());

			for (auto it = search_result.value().edges.begin(); it != search_result.value().edges.end(); ++it) {
				output.push_back(edges_content_[*it]);
			}

			return std::make_pair(output, search_result->weight);
		}
	
		const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
			return graph_;
		}
	
		const RouterSettings& TransportRouter::GetRouterSettings() const {
			return router_settings_;
		}

		const std::unordered_map<const domain::BusStop*, Exchange>& TransportRouter::GetBusstopToVertex() const {
			return bus_stop_to_vertex_;
		}
	
		const graph::Router<double>::RoutesInternalData& TransportRouter::GetRouterData() const {
			return router_->GetRouteInternalData();
		}

		const std::vector<RoutePart>& TransportRouter::GetEdgesContent() const {
			return edges_content_;
		}

		const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraphLink() const {
			return graph_;
		}

		void TransportRouter::SetRouterLink(std::unique_ptr<graph::Router<double>>&& link) {
			router_ = std::move(link);
		}
	}
}
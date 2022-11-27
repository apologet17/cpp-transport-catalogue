#pragma once
#include <string_view>
#include <optional>
//#include <utility>
#include <memory>
#include <vector>
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace catalogue_core{
	namespace router {

		enum WaitOrBus {
			WAIT = 0,
			BUS  = 1
		};

		struct RouterSettings {

			RouterSettings() = default;

			int bus_wait_time = 0;
			double bus_velocity = 0.0;

		};

		struct RoutePart {

			WaitOrBus wait_or_bus;
			std::string_view bus_name;
			std::string_view stop_name;
			int wait_time = 0;
			double bus_time = 0.0;
			int span_count = 0;
		};

		struct Exchange {
			graph::VertexId interface_vertex;
			graph::VertexId bus_vertex;
		};

		class TransportRouter {
		public:
			explicit TransportRouter(const RouterSettings& settings, catalogue_core::transport_catalogue::TransportCatalogue& cat);
			
			explicit TransportRouter(catalogue_core::transport_catalogue::TransportCatalogue& cat,
										RouterSettings&& router_settings_,
										graph::DirectedWeightedGraph<double>&& graph,
										std::unordered_map<const domain::BusStop*, Exchange>&& bus_stop_to_vertex,
										std::vector<RoutePart>&& edges_content)
				: cat_(cat)
				, router_settings_(std::move(router_settings_))
				, graph_(std::move(graph))
				, bus_stop_to_vertex_(std::move(bus_stop_to_vertex))
				, edges_content_(std::move(edges_content)) {

			}

			void LoadRouterSettings(const RouterSettings& settings);
			std::optional<std::pair<std::vector<router::RoutePart>, double>> BuildFastestRoute(domain::BusStop* start, domain::BusStop* finish);

			void TransferCreate(domain::BusStop* stop, graph::VertexId from);

			const graph::DirectedWeightedGraph<double>& GetGraph() const;

			const RouterSettings& GetRouterSettings() const;

			const std::unordered_map<const domain::BusStop*, Exchange>& GetBusstopToVertex() const;

			const graph::Router<double>::RoutesInternalData& GetRouterData() const;

			const std::vector<RoutePart>& GetEdgesContent() const;

			const graph::DirectedWeightedGraph<double>& GetGraphLink() const;

			void SetRouterLink(std::unique_ptr<graph::Router<double>>&& link);

		private:
			catalogue_core::transport_catalogue::TransportCatalogue& cat_;
			RouterSettings router_settings_;
			graph::DirectedWeightedGraph<double> graph_;
			std::unique_ptr<graph::Router<double>> router_ = nullptr;

			std::unordered_map<const domain::BusStop*, Exchange> bus_stop_to_vertex_;
			std::vector<RoutePart> edges_content_;
		};

	}
}

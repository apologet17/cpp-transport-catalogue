#include "serialization.h"
#include <algorithm>

namespace serializator {

	using NameToId = std::unordered_map<std::string_view, uint32_t>;

	void SerializeColor(const svg::Color& color, transport_serialize::Color& proto) {
		if (std::holds_alternative<std::monostate>(color)) {
			proto.set_is_content(false);
		}
		else if (std::holds_alternative<std::string>(color)) {
			const std::string& name = std::get<std::string>(color);
			proto.set_name(name);
		}
		else {
			if (std::holds_alternative<svg::Rgba>(color)) {
				const svg::Rgba& rgb = std::get<svg::Rgba>(color);
				proto.mutable_rgba()->set_red(rgb.red);
				proto.mutable_rgba()->set_green(rgb.green);
				proto.mutable_rgba()->set_blue(rgb.blue);
				proto.mutable_rgba()->set_opacity(std::get<svg::Rgba>(color).opacity);
				proto.mutable_rgba()->set_has_opacity(true);
			}
			else {
				const svg::Rgb& rgb = std::get<svg::Rgb>(color);
				proto.mutable_rgba()->set_red(rgb.red);
				proto.mutable_rgba()->set_green(rgb.green);
				proto.mutable_rgba()->set_blue(rgb.blue);
				proto.mutable_rgba()->set_has_opacity(false);
			}
		}
	}

	svg::Color DeserializeColor(const transport_serialize::Color& proto) {
		if (proto.is_content()) {
			return std::monostate{};
		}

		if (!proto.has_rgba()) {
			return proto.name();
		}

		const auto& rgba_proto = proto.rgba();
		const auto red = static_cast<uint8_t>(rgba_proto.red());
		const auto green = static_cast<uint8_t>(rgba_proto.green());
		const auto blue = static_cast<uint8_t>(rgba_proto.blue());

		if (rgba_proto.has_opacity()) {
			return svg::Rgba(red, green, blue, rgba_proto.opacity());
		}
		else {
			return  svg::Rgb(red, green, blue);
		}
	}

	std::tuple<transport_serialize::Catalogue, NameToId, NameToId> Serialization::SerializeCatalogue(const catalogue_core::transport_catalogue::TransportCatalogue* catalogue_) {
		
		NameToId stopname_to_ids;
		NameToId busname_to_ids;

		transport_serialize::Catalogue serialize_catalogue;
		unsigned int i = 0;
		for (const auto& catalogue_stop : catalogue_->GetAllStops()) {

			transport_serialize::BusStop  serialize_stop;

			serialize_stop.set_name(catalogue_stop->name);
			serialize_stop.mutable_coordinates()->set_lat(catalogue_stop->coordinates.lat);
			serialize_stop.mutable_coordinates()->set_lng(catalogue_stop->coordinates.lng);

			serialize_catalogue.mutable_stops()->Add(std::move(serialize_stop));

			stopname_to_ids[catalogue_stop->name] = i++;
		}

		i = 0;
		for (const auto& catalogue_bus : catalogue_->GetAllRoutes()) {

			transport_serialize::BusRoute  serialize_route;

			serialize_route.set_circular(catalogue_bus->circular);

			serialize_route.set_name(catalogue_bus->name);
			busname_to_ids[catalogue_bus->name] = i++;

			for (const auto& catalogue_stop : catalogue_bus->stops) {
				serialize_route.mutable_num_stops()->Add(stopname_to_ids.at(catalogue_stop->name));
			}

			serialize_catalogue.mutable_buses()->Add(std::move(serialize_route));
		}

		for (const auto& [pair_stops, len] : catalogue_->GetAllLengths()) {

			transport_serialize::Length serialize_length;

			serialize_length.set_first_stop(stopname_to_ids.at(pair_stops.first->name));
			serialize_length.set_second_stop(stopname_to_ids.at(pair_stops.second->name));
			serialize_length.set_length(len);

			serialize_catalogue.mutable_lengths()->Add(std::move(serialize_length));
		}
		return { serialize_catalogue, stopname_to_ids, busname_to_ids };
	}

	std::pair<std::vector<domain::BusStop*>, std::vector<domain::BusRoute*>> Serialization::DeserializeCatalogue(catalogue_core::transport_catalogue::TransportCatalogue* catalogue_, transport_serialize::Catalogue& serialize_catalogue) {

		std::vector<domain::BusStop*> stopnum_to_ptr(serialize_catalogue.stops().size());
		std::vector<domain::BusRoute*> busnum_to_ptr(serialize_catalogue.buses().size());

		size_t i = 0;
		for (const auto& stop : serialize_catalogue.stops()) {
			domain::BusStop bus_stop;
			bus_stop.name = stop.name();
			bus_stop.coordinates.lat = stop.coordinates().lat();
			bus_stop.coordinates.lng = stop.coordinates().lng();
			stopnum_to_ptr[i++] = catalogue_->AddBusStop(std::move(bus_stop));
		}
		i = 0;
		for (const auto& bus : serialize_catalogue.buses()) {
			domain::BusRoute bus_route;
			bus_route.name = bus.name();
			bus_route.circular = bus.circular();

			for (const auto& stop : bus.num_stops()) {
				bus_route.stops.push_back(stopnum_to_ptr[stop]);
			}

			busnum_to_ptr[i++] = catalogue_->AddBusRoute(std::move(bus_route));
		}

		for (const auto length : serialize_catalogue.lengths()) {
			catalogue_->AddLengthByPtr(stopnum_to_ptr[length.first_stop()], stopnum_to_ptr[length.second_stop()], length.length());
		}
		catalogue_->SetAllRoutes();
		return { stopnum_to_ptr, busnum_to_ptr };
	}

	void Serialization::SetSettings(const std::string& filename) {
		settings_.filename = filename;
	}

	transport_serialize::Settings Serialization::SerializeMap(catalogue_core::renderer::MapRenderer* map_renderer_) {

		transport_serialize::Settings serialize_map_settings;
		const catalogue_core::renderer::RendererSettings& catalog_map_settings = map_renderer_->GetLoadRendererSettings();

		serialize_map_settings.set_width(catalog_map_settings.width);
		serialize_map_settings.set_height(catalog_map_settings.height);

		serialize_map_settings.set_padding(catalog_map_settings.padding);

		serialize_map_settings.set_line_width(catalog_map_settings.line_width);
		serialize_map_settings.set_stop_radius(catalog_map_settings.stop_radius);

		serialize_map_settings.set_bus_label_font_size(catalog_map_settings.bus_label_font_size);
		serialize_map_settings.mutable_bus_label_offset()->Add(catalog_map_settings.bus_label_offset[0]);
		serialize_map_settings.mutable_bus_label_offset()->Add(catalog_map_settings.bus_label_offset[1]);

		serialize_map_settings.set_stop_label_font_size(catalog_map_settings.stop_label_font_size);
		serialize_map_settings.mutable_stop_label_offset()->Add(catalog_map_settings.stop_label_offset[0]);
		serialize_map_settings.mutable_stop_label_offset()->Add(catalog_map_settings.stop_label_offset[1]);

		SerializeColor(catalog_map_settings.underlayer_color, *serialize_map_settings.mutable_underlayer_color());
		serialize_map_settings.set_underlayer_width(catalog_map_settings.underlayer_width);

		for (size_t i = 0; i < catalog_map_settings.color_palette.size(); ++i) {
			transport_serialize::Color proto;
			SerializeColor(catalog_map_settings.color_palette[i], proto);
			serialize_map_settings.mutable_color_palette()->Add(std::move(proto));
		}

		return serialize_map_settings;
	}

	void Serialization::DeserializeMap(catalogue_core::renderer::MapRenderer* map_renderer_, transport_serialize::Settings& serialize_map) {
		
		catalogue_core::renderer::RendererSettings catalog_map_settings;

			catalog_map_settings.width = serialize_map.width();
			catalog_map_settings.height = serialize_map.height();

			catalog_map_settings.padding = serialize_map.padding();

			catalog_map_settings.line_width = serialize_map.line_width();
			catalog_map_settings.stop_radius = serialize_map.stop_radius();

			catalog_map_settings.bus_label_font_size = serialize_map.bus_label_font_size();
			catalog_map_settings.bus_label_offset[0] = (serialize_map.mutable_bus_label_offset()->Get(0));
			catalog_map_settings.bus_label_offset[1] = (serialize_map.mutable_bus_label_offset()->Get(1));

			catalog_map_settings.stop_label_font_size = serialize_map.stop_label_font_size();
			catalog_map_settings.stop_label_offset[0] = (serialize_map.mutable_stop_label_offset()->Get(0));;
			catalog_map_settings.stop_label_offset[1]= (serialize_map.mutable_stop_label_offset()->Get(1));

			catalog_map_settings.underlayer_color = DeserializeColor(*serialize_map.mutable_underlayer_color() );
			catalog_map_settings.underlayer_width = serialize_map.underlayer_width();

			catalog_map_settings.color_palette.resize(serialize_map.mutable_color_palette()->size());

			for (int i = 0; i < serialize_map.mutable_color_palette()->size(); ++i) {
				catalog_map_settings.color_palette[i] = std::move(DeserializeColor(serialize_map.mutable_color_palette()->Get(i)));
			}
			map_renderer_->LoadRendererSettings(std::move(catalog_map_settings));
	}

	void SerializeGraphEdges(const graph::DirectedWeightedGraph<double>& graph, transport_serialize::Graph* ser_graph) {

		ser_graph->mutable_edges()->Reserve(static_cast<int>(graph.GetEdgeCount()));
		for (size_t i = 0; i < graph.GetEdgeCount(); ++i) {

			auto cat_edge = graph.GetEdge(i);
			transport_serialize::Edge ser_edge;

			ser_edge.set_from(static_cast<uint32_t>(cat_edge.from));
			ser_edge.set_to(static_cast<uint32_t>(cat_edge.to));
			ser_edge.set_weight(cat_edge.weight);

			ser_graph->mutable_edges()->Add(std::move(ser_edge));
		}
	}
	
	void SerializeGraphIncidenceLists(const graph::DirectedWeightedGraph<double>& graph, transport_serialize::Graph* ser_graph) {

		ser_graph->mutable_incidence_lists()->Reserve(static_cast<int>(graph.GetVertexCount()));
		for (size_t i = 0; i < graph.GetVertexCount(); ++i) {

			transport_serialize::IncidenceList ser_incident_list;
			const auto range = graph.GetIncidentEdges(i);

			ser_incident_list.mutable_edge_ids()->Reserve(static_cast<int>(distance(range.begin(), range.end())));
			for (auto it = range.begin(); it != range.end(); it++) {
				ser_incident_list.mutable_edge_ids()->Add(*it);
			}

			ser_graph->mutable_incidence_lists()->Add(std::move(ser_incident_list));
		}
	}
	
	void SerializeRouterSettings(const catalogue_core::router::RouterSettings& cat_route_settings, transport_serialize::RouterSettings* ser_settings) {
		ser_settings->set_bus_velocity(cat_route_settings.bus_velocity);
		ser_settings->set_wait_time(cat_route_settings.bus_wait_time);
	}

	void SerializeBusStopToVertex(const std::unordered_map<const domain::BusStop*, catalogue_core::router::Exchange>& cat_bus_stop_to_vertex,
								        google::protobuf::RepeatedPtrField<transport_serialize::Exchange>* stop_to_vertex,
								  const NameToId& stopname_to_ids) {

		stop_to_vertex->Reserve(static_cast<int>(cat_bus_stop_to_vertex.size()));
		for (size_t i = 0; i < cat_bus_stop_to_vertex.size(); ++i) {
			stop_to_vertex->Add();
		}

		for (const auto& [stop, id] : cat_bus_stop_to_vertex) {
			auto exchange_ptr = stop_to_vertex->Mutable(stopname_to_ids.at(stop->name));
			exchange_ptr->set_bus_vertex(id.bus_vertex);
			exchange_ptr->set_interface_vertex(id.interface_vertex);
		}
	}

	void SerializeRouterData(const graph::Router<double>::RoutesInternalData& cat_router_data, 
								  google::protobuf::RepeatedPtrField<transport_serialize::InternalDataIn>* ser_router_internal_data) {
		
		ser_router_internal_data->Reserve(cat_router_data.size());

		for (const auto& cat_internal_vector : cat_router_data) {

			transport_serialize::InternalDataIn ser_internal_data;
			ser_internal_data.mutable_route_internal_data()->Reserve(cat_internal_vector.size());

			for (const auto& data : cat_internal_vector) {
				transport_serialize::RouteInternalData ser_route_internal_data;
				if (data.has_value()) {
					ser_route_internal_data.set_has_content(true);
					ser_route_internal_data.set_weight(data.value().weight);

					if (data.value().prev_edge.has_value()) {
						ser_route_internal_data.mutable_edge_id()->set_has_content(true);
						ser_route_internal_data.mutable_edge_id()->set_edge(data.value().prev_edge.value());
					}
					else {
						ser_route_internal_data.mutable_edge_id()->set_has_content(false);
					}
				}
				else {
					ser_route_internal_data.set_has_content(false);
				}
				ser_internal_data.mutable_route_internal_data()->Add(std::move(ser_route_internal_data));
			}
			ser_router_internal_data->Add(std::move(ser_internal_data));
		}
	}
	
	void SetEdgesContent(const std::vector<catalogue_core::router::RoutePart> cat_edges_content, 
								google::protobuf::RepeatedPtrField<transport_serialize::RouterPart>* ser_edges_content,
						 const NameToId& busname_to_ids_,
						 const NameToId& stopname_to_ids_) {

		ser_edges_content->Reserve(cat_edges_content.size());

		for (const auto& cat_edge : cat_edges_content) {
			transport_serialize::RouterPart route_part;

			route_part.set_wait_or_bus(cat_edge.wait_or_bus);
			if (cat_edge.wait_or_bus == catalogue_core::router::WaitOrBus::WAIT) {
				route_part.set_wait_time(cat_edge.wait_time);
				route_part.set_stop_base_number(stopname_to_ids_.at(cat_edge.stop_name));
				route_part.set_span_count(0);
				route_part.set_bus_time(0);
			}
			else {
				route_part.set_bus_time(cat_edge.bus_time);
				route_part.set_bus_base_number(busname_to_ids_.at(cat_edge.bus_name));
				route_part.set_span_count(cat_edge.span_count);
				route_part.set_wait_time(0);
			}
			ser_edges_content->Add(std::move(route_part));
		}
	}

	transport_serialize::Router Serialization::SerializeRouter(std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_,
													const NameToId& stopname_to_ids_,
													const NameToId& busname_to_ids_) {
		transport_serialize::Router ser_router;
	
		SerializeGraphEdges(transport_router_->GetGraph(), ser_router.mutable_graph());

		SerializeGraphIncidenceLists(transport_router_->GetGraph(), ser_router.mutable_graph());
	
		SerializeRouterSettings(transport_router_->GetRouterSettings(), ser_router.mutable_router_settings());

		SerializeBusStopToVertex(transport_router_->GetBusstopToVertex(), ser_router.mutable_bus_stop_to_vertex(), stopname_to_ids_);

		SerializeRouterData(transport_router_->GetRouterData(), ser_router.mutable_internal_data());
		
		SetEdgesContent(transport_router_->GetEdgesContent(), ser_router.mutable_edges_content(), busname_to_ids_, stopname_to_ids_);
		
		return ser_router;
	}

	catalogue_core::router::RouterSettings DeserializeRouterSettings(const transport_serialize::RouterSettings& router_settings) {
		catalogue_core::router::RouterSettings output;

		output.bus_velocity = router_settings.bus_velocity();
		output.bus_wait_time = router_settings.wait_time();

		return output;
	}

	std::vector<graph::Edge<double>> DeserializeGraphEdges(const transport_serialize::Graph& graph) {

		std::vector<graph::Edge<double>> output;
		output.reserve(graph.edges().size());
		for (auto it = graph.edges().begin(); it != graph.edges().end(); ++it) {
			graph::Edge<double> edge;
			edge.from = (*it).from();
			edge.to = (*it).to();
			edge.weight = (*it).weight();
			output.push_back(std::move(edge));
		}
		return output;
	}

	std::vector<std::vector<graph::EdgeId>> DeserializeGraphIncidentList(const transport_serialize::Graph& graph) {
		std::vector<std::vector<graph::EdgeId>> output  = { static_cast<unsigned int>(graph.incidence_lists_size()),
																	std::vector<graph::EdgeId>() };
		for (size_t i = 0; i < graph.incidence_lists_size(); ++i) {
			output[i].resize(graph.incidence_lists().Get(i).edge_ids_size());
			std::copy(graph.incidence_lists().Get(i).edge_ids().begin(),
					  graph.incidence_lists().Get(i).edge_ids().end(),
				output[i].begin());
		}
		return output;
	}

	graph::Router<double>::RoutesInternalData DeserializeRouteInternalData(const google::protobuf::RepeatedPtrField<transport_serialize::InternalDataIn>& internal_data) {
		graph::Router<double>::RoutesInternalData  output = { static_cast<unsigned int>(internal_data.size()),
																			std::vector<std::optional<graph::Router<double>::RouteInternalData>>() };

		for (size_t i = 0; i < internal_data.size(); ++i) {
			output[i].reserve(internal_data.Get(i).route_internal_data_size());

			auto temp = internal_data.Get(i).route_internal_data();
			for (auto it = temp.begin(); it != temp.end(); ++it) {

				std::optional<graph::Router<double>::RouteInternalData> route_internal_data;

				if ((*it).has_content()) {
					std::optional<graph::EdgeId> prev_edge;
					if ((*it).edge_id().has_content()) {
						prev_edge = (*it).edge_id().edge();
					}
					graph::Router<double>::RouteInternalData data;
					data.weight = (*it).weight();
					data.prev_edge = prev_edge;
					route_internal_data = std::move(data);
				}
				output[i].push_back(std::move(route_internal_data));
			}
		}
		return output;
	}

	std::unordered_map<const domain::BusStop*, catalogue_core::router::Exchange> DeserializeStopToVertex(const google::protobuf::RepeatedPtrField<transport_serialize::Exchange>& stop_to_vertex,
																										 const std::vector<domain::BusStop*>& bus_stops ){
		std::unordered_map<const domain::BusStop*, catalogue_core::router::Exchange> output;

		for (size_t i = 0; i < stop_to_vertex.size(); ++i) {

			auto in = stop_to_vertex.Get(i);
			auto out = &output[bus_stops.at(i)];

			out->bus_vertex = in.bus_vertex();
			out->interface_vertex = in.interface_vertex();
		}
		return output;
	}

	std::vector<catalogue_core::router::RoutePart> DeserializeEdgesContent(const google::protobuf::RepeatedPtrField<transport_serialize::RouterPart>& edges_content,
																		   const std::vector<domain::BusStop*>& bus_stops,
																		   const std::vector<domain::BusRoute*>& bus_routes) {

		std::vector<catalogue_core::router::RoutePart> output(edges_content.size());

		size_t i = 0;
		for (const auto& edge : edges_content) {
			catalogue_core::router::RoutePart route_part;
			route_part.wait_or_bus = (edge.wait_or_bus() == 0) ? catalogue_core::router::WaitOrBus::WAIT :
				catalogue_core::router::WaitOrBus::BUS;
			if (route_part.wait_or_bus == catalogue_core::router::WaitOrBus::WAIT) {
				route_part.wait_time = edge.wait_time();
				route_part.stop_name = bus_stops[edge.stop_base_number()]->name;
			}
			else {
				route_part.bus_name = bus_routes[edge.bus_base_number()]->name;
				route_part.bus_time = edge.bus_time();
				route_part.span_count = edge.span_count();
			}
			output[i++] = std::move(route_part);
		}
		return output;
	}
	
	void Serialization::DeserializeRouter(catalogue_core::transport_catalogue::TransportCatalogue& catalogue_,
										  std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_,
										  const transport_serialize::Router& serialize_router,
										  const std::vector<domain::BusStop*>& bus_stops,
										  const std::vector<domain::BusRoute*>& bus_routes) {

		catalogue_core::router::RouterSettings cat_router_settings(std::move(DeserializeRouterSettings(serialize_router.router_settings())));
		
		std::vector<graph::Edge<double>> graph_edges(std::move(DeserializeGraphEdges(serialize_router.graph())));

		std::vector<std::vector<graph::EdgeId>> incidence_lists_(std::move(DeserializeGraphIncidentList(serialize_router.graph())));

		graph::Router<double>::RoutesInternalData cat_routes_inter_data(std::move(DeserializeRouteInternalData(serialize_router.internal_data())));
			
		std::unordered_map<const domain::BusStop*, catalogue_core::router::Exchange> cat_bus_stop_to_vertex_(std::move(DeserializeStopToVertex(serialize_router.bus_stop_to_vertex(), bus_stops)));

		std::vector<catalogue_core::router::RoutePart> edges_content(std::move(DeserializeEdgesContent(serialize_router.edges_content(), bus_stops, bus_routes)));

		graph::DirectedWeightedGraph<double> graph(std::move(graph_edges), std::move(incidence_lists_));

		transport_router_ = std::make_unique<catalogue_core::router::TransportRouter>(catalogue_,
											std::move(cat_router_settings),
											std::move(graph),
											std::move(cat_bus_stop_to_vertex_),
											std::move(edges_content));

		transport_router_->SetRouterLink(std::move(std::make_unique<graph::Router<double>>(transport_router_->GetGraphLink(), std::move(cat_routes_inter_data))));
	}

	bool Serialization::SerializeFull(const catalogue_core::transport_catalogue::TransportCatalogue* catalogue_,
									  catalogue_core::renderer::MapRenderer* map_renderer_,
									  std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_) {

		if (settings_.filename.empty())
			return false;

		std::ofstream out(settings_.filename, std::ios::binary);

		auto [catalogue, stopname_to_ids, busname_to_ids] = std::move(SerializeCatalogue(catalogue_));
		transport_serialize::Settings settings(std::move(SerializeMap(map_renderer_)));

		transport_serialize::FullBase base;

		*base.mutable_catalogue() = std::move(catalogue) ;
		*base.mutable_settings() = std::move(settings);

		if (transport_router_ != nullptr) {
			transport_serialize::Router router(std::move(SerializeRouter(transport_router_, stopname_to_ids, busname_to_ids)));
			*base.mutable_router() = std::move(router);
		}

		return base.SerializeToOstream(&out);

	}
	
	bool Serialization::DeserializeFull(catalogue_core::transport_catalogue::TransportCatalogue* catalogue_,
									    catalogue_core::renderer::MapRenderer* map_renderer_,
									    std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_) {

		if (settings_.filename.empty())
			return false;

		std::ifstream in(settings_.filename, std::ios::binary);
		transport_serialize::FullBase base;

		if (base.ParseFromIstream(&in)) {
				const auto settings_ptr = base.mutable_settings();
				if (settings_ptr != nullptr)
					DeserializeMap(map_renderer_, *settings_ptr);

				const auto catalogue_ptr = base.mutable_catalogue();
				if (catalogue_ptr != nullptr) {
					auto [stop_num_to_ptr, bus_num_to_ptr] = DeserializeCatalogue(catalogue_, *catalogue_ptr);

					const auto router_ptr = base.mutable_router();
					if (router_ptr != nullptr)
						DeserializeRouter(*catalogue_, transport_router_, *router_ptr, stop_num_to_ptr, bus_num_to_ptr);
				}
			return true;
		}
		return false;
	}
}
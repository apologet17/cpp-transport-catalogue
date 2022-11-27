#include<sstream>

#include "json_reader.h"
#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace catalogue_core {
	using namespace std::string_literals;
	using namespace json;

	 JSONReader::JSONReader(RequestHandler& request_handler, catalogue_core::renderer::MapRenderer& map_renderer)
		: request_handler_(&request_handler){
	}

	svg::Color ConvertJSONToColor(const json::Node& node){

		if (node.IsArray()) {
			if (const_cast<json::Node&>(node).AsArray().size() == 4) {
				return svg::Rgba(static_cast<uint8_t>(const_cast<json::Node&>(node).AsArray()[0].AsInt()),
								static_cast<uint8_t>(const_cast<json::Node&>(node).AsArray()[1].AsInt()),
								static_cast<uint8_t>(const_cast<json::Node&>(node).AsArray()[2].AsInt()),
													const_cast<json::Node&>(node).AsArray()[3].AsDouble());
			}
			else {
				return svg::Rgb(static_cast<uint8_t>(const_cast<json::Node&>(node).AsArray()[0].AsInt()),
								static_cast<uint8_t>(const_cast<json::Node&>(node).AsArray()[1].AsInt()),
								static_cast<uint8_t>(const_cast<json::Node&>(node).AsArray()[2].AsInt()));
			}			
		}
		else {
			return const_cast<json::Node&>(node).AsString();
		}
	}

	void JSONReader::AddStopsAndRoutes(const json::Dict& doc) {

		if (doc.count("base_requests"s) == 0) {
			return;
		}

		for (auto& base_request : const_cast<json::Dict&>(doc).at("base_requests"s).AsArray()) {
			Query q;
			if (base_request.AsDict().at("type"s) == "Stop"s) {
				q.query_type = QueryType::ADD_STOP;
				q.bus_stop.bus_stop.name = base_request.AsDict().at("name"s).AsString();
				q.bus_stop.bus_stop.coordinates.lat = base_request.AsDict().at("latitude"s).AsDouble();
				q.bus_stop.bus_stop.coordinates.lng = base_request.AsDict().at("longitude"s).AsDouble();
				if (base_request.AsDict().count("road_distances"s)) {
					for (auto it = base_request.AsDict().at("road_distances"s).AsDict().begin();
						it != base_request.AsDict().at("road_distances"s).AsDict().end(); ++it) {
						q.bus_stop.names.push_back(it->first);
						q.bus_stop.lengths.push_back(it->second.AsInt());
					}
				}
				request_handler_->AddStopsToBuffer(q.bus_stop);
			}
			else if (base_request.AsDict().at("type"s) == "Bus"s) {
				q.query_type = QueryType::ADD_BUS;
				q.bus_route.circular = base_request.AsDict().at("is_roundtrip"s).AsBool();
				q.bus_route.name = base_request.AsDict().at("name"s).AsString();
				for (auto& stop : base_request.AsDict().at("stops"s).AsArray()) {
					q.bus_route.stops.push_back(stop.AsString());
				}
				request_handler_->AddRoutesToBuffer(q.bus_route);
			}
		}
		request_handler_->LoadBufferToCatalogue();

	}

	void JSONReader::ToProcessTheRequests(const json::Dict& doc, std::ostream& os) {

		if (doc.count("serialization_settings") == 0) {
			return;
		}
		auto settings = doc.at("serialization_settings"s);
		if (settings.AsDict().count("file"s)) {
			request_handler_->FillSerializeSettings(settings.AsDict().at("file").AsString());
			request_handler_->DerializeFull();
		}
		else
			return;

		if (doc.count("stat_requests"s) == 0) {
			return;
		}
	
		os << "["s << std::endl;

		bool first = true;
		for (auto& stat_request : const_cast<json::Dict&>(doc).at("stat_requests"s).AsArray()) {
			if (first) {
				first = false;
			}
			else {
				os << ","s << std::endl;
			}
			QueryToBase q;
			if (stat_request.AsDict().count("name"s)) {
				q.name = stat_request.AsDict().at("name"s).AsString();
			}
			q.id = stat_request.AsDict().at("id"s).AsInt();

			if (stat_request.AsDict().at("type"s) == "Bus"s) {
				q.type = QueryToBaseType::BUS_INFO;
	
				Answer answer = request_handler_->PrepareAnswerFromCatalogue(q);

				if (auto statistic = std::get<std::optional<domain::RouteStatistic>>(answer.buses_and_stops);  statistic.has_value()) {
					PrintInformationAboutBus(statistic.value(), answer.id, os);
				}
				else {
					ErrorMessage(answer.id, os);
				}
			}
			else if (stat_request.AsDict().at("type"s) == "Stop"s) {
				q.type = QueryToBaseType::STOP_INFO;

				Answer answer = request_handler_->PrepareAnswerFromCatalogue(q);

				if (auto buses = std::get<std::optional<std::set<std::string_view, std::less<>>>>(answer.buses_and_stops);  buses.has_value()) {
					PrintInformationAboutStop(buses.value(), answer.id, os);
				}
				else {
					ErrorMessage(answer.id, os);
				}
			}
			else if (stat_request.AsDict().at("type"s) == "Route"s) {
				q.type = QueryToBaseType::ROUTE_BUILD;
				
				auto route = request_handler_->BuildFastestRoute(stat_request.AsDict().at("from"s).AsString(), stat_request.AsDict().at("to"s).AsString());

				if (route.has_value()) {
					PrintFastestRoute(route.value(), q.id, os);
				}
				else {
					ErrorMessage(q.id, os);
				}

			}
			else if (stat_request.AsDict().at("type"s) == "Map"s) {
				std::ostringstream output;
				request_handler_->RenderMap(output);
				PrintSvg(q.id, output, os);
			}
		}
		os << std::endl << "]"s;
	}

	void JSONReader::FillRenderSettings(const json::Dict& doc) {
		
		renderer::RendererSettings settings;
		if (doc.count("width"s))
			settings.width = doc.at("width"s).AsDouble();
		if (doc.count("height"s))
			settings.height = doc.at("height"s).AsDouble();
		if (doc.count("padding"s))
			settings.padding = doc.at("padding"s).AsDouble();
		if (doc.count("line_width"s))
			settings.line_width = doc.at("line_width"s).AsDouble();
		if (doc.count("stop_radius"s))
			settings.stop_radius = doc.at("stop_radius"s).AsDouble();
		if (doc.count("bus_label_font_size"s))
			settings.bus_label_font_size = doc.at("bus_label_font_size"s).AsInt();
		if (doc.count("bus_label_offset"s)) {
			settings.bus_label_offset.clear();
			for (const auto& offset : const_cast<json::Dict&>(doc).at("bus_label_offset"s).AsArray()) {
				settings.bus_label_offset.push_back(offset.AsDouble());
			}
		}
		if (doc.count("stop_label_font_size"s))
			settings.stop_label_font_size = doc.at("stop_label_font_size"s).AsInt();
		if (doc.count("stop_label_offset"s)) {
			settings.stop_label_offset.clear();
			for (const auto& offset : const_cast<json::Dict&>(doc).at("stop_label_offset"s).AsArray()) {
				settings.stop_label_offset.push_back(offset.AsDouble());
			}
		}
		if (doc.count("underlayer_color"s))
			settings.underlayer_color = ConvertJSONToColor(doc.at("underlayer_color"s));
		if (doc.count("underlayer_width"s))
		settings.underlayer_width = doc.at("underlayer_width"s).AsDouble();
		if (doc.count("color_palette"s)) {
			settings.color_palette.clear();
			for (const auto& color : const_cast<json::Dict&>(doc).at("color_palette"s).AsArray()) {
				settings.color_palette.push_back(ConvertJSONToColor(color));
			}
		}

		request_handler_->LoadRendererSettings(std::move(settings));		
	}

	void JSONReader::CreateRouter(const json::Dict& doc) {
		router::RouterSettings settings;
		if (doc.count("bus_wait_time"s))
			settings.bus_wait_time = doc.at("bus_wait_time"s).AsInt();
		if (doc.count("bus_velocity"s))
			settings.bus_velocity = doc.at("bus_velocity"s).AsDouble();
		request_handler_->CreateRouter(settings);
	}

	void JSONReader::ProcessRequests(std::istream& is, [[maybe_unused]]std::ostream& os) {
		ToProcessTheRequests(const_cast<json::Node&>(json::Load(is).GetRoot()).AsDict(), os);
	}

	void JSONReader::MakeBaseProcessing(std::istream& is, [[maybe_unused]] std::ostream& os) {

		std::map<std::string, json::Node> doc = const_cast<json::Node&>(json::Load(is).GetRoot()).AsDict();

		AddStopsAndRoutes(doc);
		if (doc.count("render_settings"s))
			FillRenderSettings(doc.at("render_settings"s).AsDict());
		if (doc.count("routing_settings"s)) {
			CreateRouter(doc.at("routing_settings"s).AsDict());
		}
		if (doc.count("serialization_settings"s)) {
			auto settings = doc.at("serialization_settings"s).AsDict();
			if (settings.count("file"s)) {
				request_handler_->FillSerializeSettings(settings.at("file").AsString());
			}
		}
		request_handler_->SerializeFull();
	}

	void JSONReader::PrintInformationAboutBus(const domain::RouteStatistic& output, int id, std::ostream& os) const {
	
		json::Print(
			json::Document{
				json::Builder{}
				.StartDict()
					.Key("curvature"s).Value(output.curvature)
					.Key("route_length"s).Value(static_cast<double>(output.route_length_))
					.Key("stop_count"s).Value(output.num_of_stops_)
					.Key("unique_stop_count"s).Value(output.num_of_unique_stops_)
					.Key("request_id"s).Value(id)
				.EndDict()
				.Build()
			},
			os
		); 
	}

	void JSONReader::PrintSvg(int id, std::ostringstream& input, std::ostream& os) const {
	
		json::Print(
			json::Document{
				json::Builder{}
				.StartDict()
					.Key("map"s).Value(input.str())
					.Key("request_id"s).Value(id)
				.EndDict()
				.Build()
			},
			os
		);
	}

	void JSONReader::PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, int id, std::ostream& os) const {

		auto first_part = json::Builder{}.StartDict()
			.Key("buses"s).StartArray();

		for (const auto& stop : output) {
			first_part.Value(std::string(stop));
		}

		json::Print(
			json::Document{ first_part.EndArray()
			.Key("request_id"s).Value(id)
			.EndDict().Build()
			},
			os
		);
	}

	void JSONReader::PrintFastestRoute(const std::pair<std::vector<router::RoutePart>, double>& input, int id, std::ostream& os) const {

		auto first_part = json::Builder{}.StartDict()
			.Key("request_id"s).Value(id)
			.Key("total_time"s).Value(input.second)
			.Key("items"s).StartArray();

		for (const auto& part : input.first) {
			if (part.wait_or_bus == router::WaitOrBus::WAIT) {
				first_part.StartDict()
					.Key("type"s).Value("Wait"s)
					.Key("stop_name"s).Value(std::string(part.stop_name))
					.Key("time"s).Value(part.wait_time)
					.EndDict();
			}
			else if (part.wait_or_bus == router::WaitOrBus::BUS) {
				first_part.StartDict()
					.Key("type"s).Value("Bus"s)
					.Key("bus"s).Value(std::string(part.bus_name))
					.Key("span_count"s).Value(part.span_count)
					.Key("time"s).Value(part.bus_time)
					.EndDict();
			}
		}

		json::Print(json::Document{ first_part.EndArray().EndDict().Build()}, os	);
	}

	void JSONReader::ErrorMessage(int id, std::ostream& os) const {
		
		json::Print(
			json::Document{
				json::Builder{}
				.StartDict()
					.Key("error_message"s).Value("not found"s)
					.Key("request_id"s).Value(id)
				.EndDict()
				.Build()
			},
			os
		);
	}
}
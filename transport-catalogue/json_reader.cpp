#include<sstream>

#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace catalogue_core {
	using namespace std::string_literals;

	svg::Color ConvertJSONToColor(const json::Node& node){

		if (node.IsArray()) {
			if (node.AsArray().size() == 4) {
				return svg::Rgba(static_cast<uint8_t>(node.AsArray()[0].AsInt()),
								static_cast<uint8_t>(node.AsArray()[1].AsInt()),
								static_cast<uint8_t>(node.AsArray()[2].AsInt()),
													 node.AsArray()[3].AsDouble());
			}
			else {
				return svg::Rgb(static_cast<uint8_t>(node.AsArray()[0].AsInt()),
								static_cast<uint8_t>(node.AsArray()[1].AsInt()),
								static_cast<uint8_t>(node.AsArray()[2].AsInt()));
			}			
		}
		else {
			return node.AsString();
		}
	}

	void JSONReader::AddStopsAndRoutes(const json::Dict& doc) {

		if (doc.count("base_requests"s) == 0) {
			return;
		}

		for (auto& base_request : doc.at("base_requests"s).AsArray()) {
			Query q;
			if (base_request.AsMap().at("type"s) == "Stop"s) {
				q.query_type = QueryType::ADD_STOP;
				q.bus_stop.bus_stop.name = base_request.AsMap().at("name"s).AsString();
				q.bus_stop.bus_stop.coordinates.lat = base_request.AsMap().at("latitude"s).AsDouble();
				q.bus_stop.bus_stop.coordinates.lng = base_request.AsMap().at("longitude"s).AsDouble();
				if (base_request.AsMap().count("road_distances"s)) {
					for (auto it = base_request.AsMap().at("road_distances"s).AsMap().begin();
						it != base_request.AsMap().at("road_distances"s).AsMap().end(); ++it) {
						q.bus_stop.names.push_back(it->first);
						q.bus_stop.lengths.push_back(it->second.AsInt());
					}
				}
				request_handler_->AddStopsToBuffer(q.bus_stop);
			}
			else if (base_request.AsMap().at("type"s) == "Bus"s) {
				q.query_type = QueryType::ADD_BUS;
				q.bus_route.circular = base_request.AsMap().at("is_roundtrip"s).AsBool();
				q.bus_route.name = base_request.AsMap().at("name"s).AsString();
				for (auto& stop : base_request.AsMap().at("stops"s).AsArray()) {
					q.bus_route.stops.push_back(stop.AsString());
				}
				request_handler_->AddRoutesToBuffer(q.bus_route);
			}
		}
		request_handler_->LoadBufferToCatalogue();
	}

	void JSONReader::ToProcessTheRequests(const json::Dict& doc, std::ostream& os) {

		if (doc.count("stat_requests"s) == 0) {
			return;
		}

		os << "["s << std::endl;

		bool first = true;
		for (auto& stat_request : doc.at("stat_requests"s).AsArray()) {
			if (first) {
				first = false;
			}
			else {
				os << ","s << std::endl;
			}
			QueryToBase q;
			if (stat_request.AsMap().count("name"s)) {
				q.name = stat_request.AsMap().at("name"s).AsString();
			}
			q.id = stat_request.AsMap().at("id"s).AsInt();

			if (stat_request.AsMap().at("type"s) == "Bus"s) {
				q.type = QueryToBaseType::BUS_INFO;

				Answer answer = request_handler_->PrepareAnswerFromCatalogue(q);

				if (auto statistic = std::get<std::optional<domain::RouteStatistic>>(answer.buses_and_stops);  statistic.has_value()) {
					PrintInformationAboutBus(statistic.value(), answer.id, os);
				}
				else {
					ErrorMessage(answer.id, os);
				}
			}
			else if (stat_request.AsMap().at("type"s) == "Stop"s) {
				q.type = QueryToBaseType::STOP_INFO;

				Answer answer = request_handler_->PrepareAnswerFromCatalogue(q);

				if (auto buses = std::get<std::optional<std::set<std::string_view, std::less<>>>>(answer.buses_and_stops);  buses.has_value()) {
					PrintInformationAboutStop(buses.value(), answer.id, os);
				}
				else {
					ErrorMessage(answer.id, os);
				}
			}
			else if (stat_request.AsMap().at("type"s) == "Map"s) {
				std::ostringstream output;
				map_renderer_->RenderMap(request_handler_->GetAllRoutes(), output);
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
			for (const auto& offset : doc.at("bus_label_offset"s).AsArray()) {
				settings.bus_label_offset.push_back(offset.AsDouble());
			}
		}
		if (doc.count("stop_label_font_size"s))
			settings.stop_label_font_size = doc.at("stop_label_font_size"s).AsInt();
		if (doc.count("stop_label_offset"s)) {
			settings.stop_label_offset.clear();
			for (const auto& offset : doc.at("stop_label_offset"s).AsArray()) {
				settings.stop_label_offset.push_back(offset.AsDouble());
			}
		}
		if (doc.count("underlayer_color"s))
			settings.underlayer_color = ConvertJSONToColor(doc.at("underlayer_color"s));
		if (doc.count("underlayer_width"s))
		settings.underlayer_width = doc.at("underlayer_width"s).AsDouble();
		if (doc.count("color_palette"s)) {
			settings.color_palette.clear();
			for (const auto& color : doc.at("color_palette"s).AsArray()) {
				settings.color_palette.push_back(ConvertJSONToColor(color));
			}
		}

		map_renderer_->LoadRendererSettings(std::move(settings));		
	}

	void JSONReader::QueryProcessing(std::istream& is, [[maybe_unused]]std::ostream& os) {

		std::map<std::string, json::Node> doc = json::Load(is).GetRoot().AsMap();
		
		AddStopsAndRoutes(doc);
		if (doc.count("render_settings"s))
			FillRenderSettings(doc.at("render_settings"s).AsMap());
		ToProcessTheRequests(doc, os);
	}

	void JSONReader::PrintInformationAboutBus(const domain::RouteStatistic& output, int id, std::ostream& os) const {
		json::Dict formatted_answer;

		formatted_answer["curvature"s] = json::Node(output.curvature);
		formatted_answer["route_length"s] = json::Node(static_cast<double>(output.route_length_));
		formatted_answer["stop_count"s] = json::Node(output.num_of_stops_);
		formatted_answer["unique_stop_count"s] = json::Node(output.num_of_unique_stops_);
		formatted_answer["request_id"s] = json::Node(id);

		json::PrintNode(json::Node(formatted_answer), os);
	}

	void JSONReader::PrintSvg(int id, std::ostringstream& input, std::ostream& os) const {
		json::Dict formatted_answer;

		formatted_answer["map"s] = json::Node(input.str());
		formatted_answer["request_id"s] = json::Node(id);

		json::PrintNode(json::Node(formatted_answer), os);
	}

	void JSONReader::PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, int id, std::ostream& os) const {
		json::Dict formatted_answer;
		json::Array stops;

		for (const auto& stop : output) {
			stops.push_back(json::Node(std::string(stop)));
		}

		formatted_answer["buses"s] = json::Node(stops);
		formatted_answer["request_id"s] = json::Node(id);

		json::PrintNode(json::Node(formatted_answer), os);
	}

	void JSONReader::ErrorMessage(int id, std::ostream& os) const {
		json::Dict formatted_answer;

		formatted_answer["error_message"s] = json::Node("not found"s);
		formatted_answer["request_id"s] = json::Node(id);

		json::PrintNode(json::Node(formatted_answer), os);
	}
}
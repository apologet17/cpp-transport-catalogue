#include <set>

#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace catalogue_core {
namespace renderer {

void MapRenderer::RenderMap(const std::vector<domain::BusRoute*>& routes, std::ostream& os) const {

	svg::Document doc;
	std::vector<geo::Coordinates> for_sp;

	for (const auto& route : routes) {
		for (const auto& stop : route->stops) {
			for_sp.push_back(stop->coordinates);
		}
	}

	SphereProjector sphere_projector(for_sp.begin(), for_sp.end(), renderer_settings_.width, renderer_settings_.height, renderer_settings_.padding);

	std::set<domain::BusStop*, decltype(cmp)> stops_for_output(cmp);
	for (const auto route : routes) {
		for (const auto& stop : route->stops) {
			stops_for_output.insert(stop);
		}
	}

	DrawLines(routes, doc, sphere_projector);
	DrawRouteNames(routes, doc, sphere_projector);
	DrawStopSymbols(stops_for_output, doc, sphere_projector);
	DrawStopNames(stops_for_output, doc, sphere_projector);
		doc.Render(os);

}

void MapRenderer::CreateStopName(svg::Text& stop_name, svg::Document& doc) const {
	stop_name.SetFillColor(renderer_settings_.underlayer_color);
	stop_name.SetStrokeColor(renderer_settings_.underlayer_color);
	stop_name.SetStrokeWidth(renderer_settings_.underlayer_width);
	stop_name.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	stop_name.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	doc.AddPtr(std::make_unique<svg::Text>(stop_name));

	stop_name.SetFillColor("black"s);
	stop_name.ResetStrokeColor();
	stop_name.ResetStrokeWidth();
	stop_name.ResetStrokeLineCap();
	stop_name.ResetStrokeLineJoin();

	doc.AddPtr(std::make_unique<svg::Text>(stop_name));
}

void MapRenderer::DrawStopNames(const std::set<domain::BusStop*, decltype(cmp)>& stops_for_output, svg::Document& doc, const SphereProjector& sphere_projector) const {

	svg::Text stop_name;

	stop_name.SetOffset(svg::Point(renderer_settings_.stop_label_offset[0], renderer_settings_.stop_label_offset[1]));
	stop_name.SetFontSize(renderer_settings_.stop_label_font_size);
	stop_name.SetFontFamily("Verdana"s);

	for (const auto stop : stops_for_output) {
		stop_name.SetPosition(sphere_projector(stop->coordinates));
		stop_name.SetData(stop->name);
		CreateStopName(stop_name, doc);
	}
}

void MapRenderer::CreateRouteName(svg::Text& route_name, svg::Document& doc, svg::Color color) const {

	route_name.SetFillColor(renderer_settings_.underlayer_color);
	route_name.SetStrokeColor(renderer_settings_.underlayer_color);
	route_name.SetStrokeWidth(renderer_settings_.underlayer_width);
	route_name.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	route_name.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	doc.AddPtr(std::make_unique<svg::Text>(route_name));

	route_name.SetFillColor(color);
	route_name.ResetStrokeColor();
	route_name.ResetStrokeWidth();
	route_name.ResetStrokeLineCap();
	route_name.ResetStrokeLineJoin();

	doc.AddPtr(std::make_unique<svg::Text>(route_name));
}

void MapRenderer::DrawStopSymbols(const std::set<domain::BusStop*, decltype(cmp)>& stops_for_output, svg::Document& doc, const SphereProjector& sphere_projector) const {

	svg::Circle stop_symbol;

	stop_symbol.SetRadius(renderer_settings_.stop_radius);
	stop_symbol.SetFillColor("white"s);

	for (const auto stop : stops_for_output) {
		stop_symbol.SetCenter(sphere_projector(stop->coordinates));
		doc.AddPtr(std::make_unique<svg::Circle>(stop_symbol));
	}
}

void MapRenderer::DrawRouteNames(const std::vector<domain::BusRoute*>& routes, svg::Document& doc, const SphereProjector& sphere_projector) const {

	svg::Text route_name;

	route_name.SetOffset(svg::Point(renderer_settings_.bus_label_offset[0], renderer_settings_.bus_label_offset[1]));
	route_name.SetFontSize(renderer_settings_.bus_label_font_size);
	route_name.SetFontFamily("Verdana"s);
	route_name.SetFontWeight("bold"s);

	size_t color_number = 0;
	for (const auto route : routes) {

		if (route->stops.empty()) {
			continue;
		}

		route_name.SetData(route->name);

		route_name.SetPosition(sphere_projector((*route->stops.begin())->coordinates));
		CreateRouteName(route_name, doc, renderer_settings_.color_palette[color_number]);

		if ((route->circular == false) && ((*(route->stops.end() - 1))->coordinates != (*route->stops.begin())->coordinates)) {
			route_name.SetPosition(sphere_projector((*(route->stops.end()-1))->coordinates));
			CreateRouteName(route_name, doc, renderer_settings_.color_palette[color_number]);
		}

		color_number = (color_number == renderer_settings_.color_palette.size() - 1) ? 0 : color_number + 1;
	}
}

void MapRenderer::DrawLines(const std::vector<domain::BusRoute*>& routes, svg::Document& doc, const SphereProjector& sphere_projector) const {

	size_t color_number = 0;
	for (const auto route : routes) {
		if (route->stops.empty()) {
			continue;
		}
		svg::Polyline route_curve;
		for (const auto& stop : route->stops) {
			route_curve.AddPoint(sphere_projector(stop->coordinates));
		}
		if ((route->circular == false) && (route->stops.size() > 1)) {
			for (auto it = route->stops.end() - 2; it != route->stops.begin(); --it) {
				route_curve.AddPoint(sphere_projector((*it)->coordinates));
			}
			route_curve.AddPoint(sphere_projector((*route->stops.begin())->coordinates));
		}
		route_curve.SetStrokeColor(renderer_settings_.color_palette[color_number]);
		route_curve.SetStrokeWidth(renderer_settings_.line_width);
		route_curve.SetFillColor(svg::NoneColor);
		route_curve.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		route_curve.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc.AddPtr(std::move(std::make_unique<svg::Polyline>(route_curve)));
		color_number = (color_number == renderer_settings_.color_palette.size() - 1) ? 0 : color_number + 1;
	}
}

void MapRenderer::LoadRendererSettings(RendererSettings&& renderer_settings) {
	renderer_settings_ = renderer_settings;
}

}
}
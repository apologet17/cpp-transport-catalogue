#pragma once

#include <vector>
#include <variant>
#include <string>
#include <algorithm>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace catalogue_core {
namespace renderer {
    using namespace std::string_literals;
    inline const double EPSILON = 1e-6;

    inline bool IsZero(double value) {
         return std::abs(value) < EPSILON;
    }

    inline constexpr auto cmp = [](const auto& lhs, const auto& rhs) {return lhs->name < rhs->name; };

	struct RendererSettings {

        RendererSettings() = default;

		double width = 1200.0;
		double height = 1200.0;

		double padding = 50.0;

		double line_width = 14.0;
		double stop_radius = 5.0;

		int bus_label_font_size = 20;
		std::vector<double> bus_label_offset{ 7.0, 15.0 };

		int stop_label_font_size = 20;
		std::vector<double> stop_label_offset{ 7.0, -3.0 };

        svg::Color underlayer_color = svg::Rgba{ 255, 255, 255, 0.85 };
		double underlayer_width = 3.0;

        std::vector<svg::Color> color_palette = { "green"s, svg::Rgb{255, 160, 0},  "red"s };
	};

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	class MapRenderer {

	public:
		MapRenderer() = default;
        void RenderMap(const std::vector<domain::BusRoute*>& routes, std::ostream& os) const;
		void LoadRendererSettings(RendererSettings&& renderer_settings);
        const RendererSettings& GetLoadRendererSettings() const;

	private:
        void DrawLines(const std::vector<domain::BusRoute*>& routes, svg::Document& doc, const SphereProjector& sphere_projector) const;
        void DrawRouteNames(const std::vector<domain::BusRoute*>& routes, svg::Document& doc, const SphereProjector& sphere_projector) const;
        void CreateRouteName(svg::Text& route_name, svg::Document& doc, svg::Color color) const;
        void DrawStopSymbols(const std::set<domain::BusStop*, decltype(cmp)>& stops_for_output, svg::Document& doc, const SphereProjector& sphere_projector) const;
        void DrawStopNames (const std::set<domain::BusStop*, decltype(cmp)>& stops_for_output, svg::Document& doc, const SphereProjector& sphere_projector) const;
        void CreateStopName(svg::Text& stop_name, svg::Document& doc) const;
		RendererSettings renderer_settings_;

	};

   
}
}


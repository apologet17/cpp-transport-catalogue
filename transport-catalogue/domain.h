#pragma once

#include<string>
#include<vector>

#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
namespace domain {
	struct RouteStatistic {
		int num_of_stops_;
		int num_of_unique_stops_;
		int route_length_;
		double curvature;
	};

	struct BusStop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct BusRoute {
		bool circular;
		std::string name;
		std::vector<BusStop*> stops;
	};

	class StopLengthsHasher {
	public:
		size_t operator()(const std::pair<BusStop*, BusStop*>& stop_len) const;
	};
}

#pragma once

#include <string>
#include <variant>

#include "transport_catalogue.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace catalogue_core {

    enum QueryToBaseType {
        BUS_INFO,
        STOP_INFO
    };

    struct QueryToBase {
        QueryToBaseType type;
        std::string name;
        int id;
    };

    enum QueryType {
        ADD_BUS,
        ADD_STOP
    };

    struct BusRouteRaw {
        bool circular;
        std::string name;
        std::vector<std::string> stops;
    };

    struct BusStopRaw {
        domain::BusStop bus_stop;
        std::vector<std::string> names;
        std::vector<int> lengths;
    };

    struct Query {
        QueryType query_type;
        BusStopRaw bus_stop;
        BusRouteRaw bus_route;
    };

    struct Answer {
        int id;
        std::string name;
        std::variant<std::optional<domain::RouteStatistic>, std::optional<std::set<std::string_view, std::less<>>>> buses_and_stops;
    };

    class RequestHandler {
    public:
        explicit RequestHandler(transport_catalogue::TransportCatalogue& catalogue);

        void LoadBufferToCatalogue();
        Answer PrepareAnswerFromCatalogue(const QueryToBase& query) const;
        void AddStopsToBuffer(const BusStopRaw& bus_stop);
        void AddRoutesToBuffer(const BusRouteRaw& bus_route);
        void AddQueryToBuffer(const QueryToBase& query);

        const std::vector<domain::BusRoute*> GetAllRoutes() const;

    private:     
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
         transport_catalogue::TransportCatalogue* catalogue_;

         std::vector<BusRouteRaw> bus_route_buffer_;
         std::vector<BusStopRaw> bus_stop_buffer_; 
    };
}

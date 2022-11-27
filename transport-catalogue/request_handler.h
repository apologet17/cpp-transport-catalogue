#pragma once

#include <string>
#include <variant>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "serialization.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace catalogue_core {

    enum QueryToBaseType {
        BUS_INFO,
        STOP_INFO,
        ROUTE_BUILD
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
        std::optional < std::vector<router::RoutePart>> route;
    };

    class RequestHandler {
    public:
        explicit RequestHandler(transport_catalogue::TransportCatalogue& catalogue,
                                           renderer::MapRenderer& map_renderer_);

        void LoadBufferToCatalogue();
        Answer PrepareAnswerFromCatalogue(const QueryToBase& query) const;
        void AddStopsToBuffer(const BusStopRaw& bus_stop);
        void AddRoutesToBuffer(const BusRouteRaw& bus_route);
        void AddQueryToBuffer(const QueryToBase& query);
        void RenderMap(std::ostream& out);
        void LoadRendererSettings(renderer::RendererSettings&& settings);
        bool SerializeFull();
        bool DerializeFull();
        void FillSerializeSettings(const std::string& filename);

        const std::vector<domain::BusRoute*> GetAllRoutes() const;

        void CreateRouter(router::RouterSettings settings);
        std::optional<std::pair<std::vector<router::RoutePart>, double>> BuildFastestRoute(const std::string& from, const std::string& to);
    private:     
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
         transport_catalogue::TransportCatalogue* catalogue_;
         renderer::MapRenderer* map_renderer_;
         std::unique_ptr<router::TransportRouter> transport_router_ = nullptr;
         serializator::Serialization serializator_;

         std::vector<BusRouteRaw> bus_route_buffer_;
         std::vector<BusStopRaw> bus_stop_buffer_; 
    };
}

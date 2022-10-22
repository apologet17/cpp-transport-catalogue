
#include <iomanip>
#include <iostream>
#include <unordered_set>

#include "request_handler.h"
//#include "input_reader.h"
#include "domain.h"
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace catalogue_core {

    using namespace std::string_literals;

    RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalogue)
        : catalogue_(&catalogue){
    }

    const std::vector<domain::BusRoute*> RequestHandler::GetAllRoutes() const {

        return catalogue_->GetAllRoutes();
    }

    Answer RequestHandler::PrepareAnswerFromCatalogue(const QueryToBase& query) const {
        Answer answer;

        answer.name = query.name;
        answer.id = query.id;

        switch (query.type) {

        case QueryToBaseType::BUS_INFO:
            answer.buses_and_stops = catalogue_->GetInformationAboutBusRoute(query.name);
            break;

        case QueryToBaseType::STOP_INFO:
            answer.buses_and_stops = catalogue_->GetInformationAboutStop(query.name);
            break;
        }
        return answer;
    }

    void RequestHandler::LoadBufferToCatalogue() {

        for (auto it = bus_stop_buffer_.begin(); it != bus_stop_buffer_.end(); it++) {
            catalogue_->AddBusStop(it->bus_stop);
        }

        for (auto it = bus_stop_buffer_.begin(); it != bus_stop_buffer_.end(); it++) {
            catalogue_->AddLength(it->bus_stop.name, it->names, it->lengths);
        }

        bus_stop_buffer_.clear();

        for (auto it = bus_route_buffer_.begin(); it != bus_route_buffer_.end(); it++) {
            domain::BusRoute output;
            output.circular = it->circular;
            output.name = it->name;

            output.stops.reserve(it->stops.size());
            for (const auto& stop : it->stops) {
                output.stops.push_back(catalogue_->FindBusStop(stop));
            }

            catalogue_->AddBusRoute(output);
        }
        bus_route_buffer_.clear();

        catalogue_->SetAllRoutes();
    }

    void RequestHandler::AddStopsToBuffer(const BusStopRaw& bus_stop) {
        bus_stop_buffer_.push_back(bus_stop);
    }

    void RequestHandler::AddRoutesToBuffer(const BusRouteRaw& bus_route) {
        bus_route_buffer_.push_back(bus_route);
    }

    void RequestHandler::CreateRouter(router::RouterSettings settings) {
        transport_router_ = std::make_unique<router::TransportRouter>(settings, *catalogue_);
    }

    std::optional<std::pair<std::vector<router::RoutePart>, double>> RequestHandler::BuildFastestRoute(const std::string& from, const std::string& to) {     
        return transport_router_->BuildFastestRoute(catalogue_->FindBusStop(from), catalogue_->FindBusStop(to));
    }
}

#pragma once

#include "domain.h"
#include "json.h"
#include "request_handler.h"

#include "transport_router.h"
#include "serialization.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace catalogue_core {
	class JSONReader {

	public:

		explicit JSONReader(RequestHandler& request_handler, 
							renderer::MapRenderer& map_renderer);

		void ProcessRequests(std::istream& is, [[maybe_unused]] std::ostream& os);
		void MakeBaseProcessing(std::istream& is, [[maybe_unused]] std::ostream& os);

	private:

		void AddStopsAndRoutes(const json::Dict& doc);
		void CreateRouter(const json::Dict& doc);
		void SerializeCatalogue();

		void ToProcessTheRequests(const json::Dict& doc, std::ostream& os);
		void PrintSvg(int id, std::ostringstream& input, std::ostream& os) const;
		void FillRenderSettings(const json::Dict& doc);
		void FillRoutingSettings(const json::Dict& doc);
		void FillSerializeSettings( json::Dict& doc);
		void GetAndPrintInformation(std::ostream& os) const;

		void PrintFastestRoute(const std::pair<std::vector<router::RoutePart>, double>& input, int id, std::ostream& os) const;
		void PrintInformationAboutBus(const domain::RouteStatistic& output, int id, std::ostream& os) const;
		void PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, int id, std::ostream& os) const;
		void ErrorMessage(int id, std::ostream& os) const;

		RequestHandler* request_handler_;
	};
}
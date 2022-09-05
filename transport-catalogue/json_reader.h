#pragma once

#include "domain.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace catalogue_core {
	class JSONReader {

	public:

		explicit JSONReader(RequestHandler& request_handler, catalogue_core::renderer::MapRenderer& map_renderer);

		void QueryProcessing(std::istream& is, [[maybe_unused]] std::ostream& os);

	private:
		void AddStopsAndRoutes(const json::Dict& doc);
		void ToProcessTheRequests(const json::Dict& doc, std::ostream& os);
		void PrintSvg(int id, std::ostringstream& input, std::ostream& os) const;
		void FillRenderSettings(const json::Dict& doc);
		void GetAndPrintInformation(std::ostream& os) const;

		void PrintInformationAboutBus(const domain::RouteStatistic& output, int id, std::ostream& os) const;
		void PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, int id, std::ostream& os) const;
		void ErrorMessage(int id, std::ostream& os) const;

		catalogue_core::RequestHandler* request_handler_;
		catalogue_core::renderer::MapRenderer* map_renderer_;
	};
}
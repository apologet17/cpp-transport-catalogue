#pragma once

#include <string>
#include <set>
#include <deque>
#include "transport_catalogue.h"

namespace catalogue_core {

	namespace statreader {

		enum QueryToBaseType {
			BUS_INFO,
			STOP_INFO
		};

		struct QueryToBase {
			QueryToBaseType type;
			std::string name;
		};

		class StatReader {
		public:
			explicit StatReader(transportcatalogue::TransportCatalogue& catalogue)
				: catalogue_(&catalogue) {

			}
			void QueryProcessing(std::istream& is, std::ostream& os);

		private:
			void PrintInformationAboutBus(const transportcatalogue::RouteStatistic& output, const std::string& name, std::ostream& os) const;
			void PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, const std::string& name, std::ostream& os) const;

			transportcatalogue::TransportCatalogue* catalogue_;
		};

		std::istream& operator>>(std::istream& is, QueryToBase& query);
	}
}
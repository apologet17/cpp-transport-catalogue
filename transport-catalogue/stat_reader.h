#pragma once

#include <string>
#include <set>
#include <deque>
#include "transport_catalogue.h"

namespace catalogue_core {

	using namespace transportcatalogue;

	namespace statreader {

		using namespace std::string_literals;

		const auto BUS_INFO_NAME = "Bus"s;
		const auto STOP_INFO_NAME = "Stop"s;

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
			StatReader(TransportCatalogue* catalogue)
				: catalogue_(catalogue) {

			}
			void QueryProcessing(std::istream& is, std::ostream& os);

		private:
			void PrintInformationAboutBus(RouteStatistic& output, std::string& name, std::ostream& os) const;
			void PrintInformationAboutStop(std::set<std::string_view, std::less<>>& output, std::string& name, std::ostream& os) const;

			std::deque<QueryToBase> querys_to_base_;
			TransportCatalogue* catalogue_;
		};

		std::istream& operator>>(std::istream& is, QueryToBase& query);
	}
}
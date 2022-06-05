#pragma once

#include <string>
#include <vector>
#include <deque>

#include "domain.h"
#include "request_handler.h"


namespace catalogue_core {
namespace inputreader {

	class InputReader {

	public:

		explicit InputReader(RequestHandler& request_handler)
			: request_handler_(&request_handler) {
		}

	void AdditionToCatalogue(std::istream& is);
	void QueryProcessing(std::istream& is, std::ostream& os);

	private:

		void PrintInformationAboutBus(const domain::RouteStatistic& output, const std::string& name, std::ostream& os) const;
		void PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, const std::string& name, std::ostream& os) const;
		RequestHandler* request_handler_;
	};

	std::istream& operator>>(std::istream& is, Query& q);
	std::istream& operator>>(std::istream& is, QueryToBase& query);
}
}
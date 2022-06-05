#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include "request_handler.h"
#include "input_reader.h"

namespace catalogue_core{

    using namespace transportcatalogue;
    using namespace std::string_literals;

namespace inputreader {

    static const auto ADD_BUS_NAME = "Bus"s;
    static const auto ADD_STOP_NAME = "Stop"s;

    static const auto BUS_INFO_NAME = "Bus"s;
    static const auto STOP_INFO_NAME = "Stop"s;

void InputReader::PrintInformationAboutBus(const domain::RouteStatistic& output, const std::string& name, std::ostream& os) const {

        os << "Bus "s << name << ": "s << output.num_of_stops_ << " stops on route, "s
            << output.num_of_unique_stops_ << " unique stops, "s << std::setprecision(6) << static_cast<double>(output.route_length_) << " route length, "s
            << std::setprecision(6) << output.curvature << " curvature"s << std::endl;
    }

void InputReader::PrintInformationAboutStop(const std::set<std::string_view, std::less<>>& output, const std::string& name, std::ostream& os) const {

        os << "Stop "s << name << ": buses"s;

        for (auto it = output.begin(); it != output.end(); ++it) {
            os << " "s << std::string(*it);
        }

        os << std::endl;
    }

void InputReader::QueryProcessing(std::istream& is, std::ostream& os) {

        size_t number;
        std::string str;

        getline(is, str);
        number = std::stoi(str);
        
        for (size_t i = 0; i < number; ++i) {
            QueryToBase q;
            is >> q;
            request_handler_->AddQueryToBuffer(q);
        }

        for (const auto& pos : request_handler_->PrepareInformationForOutput()) {
            if (pos.buses_and_stops.index() == 0) {
                
                if (auto statistic = std::get<std::optional<domain::RouteStatistic>>(pos.buses_and_stops);  statistic.has_value()) {
                    PrintInformationAboutBus(statistic.value(), pos.name, os);
                }
                else {
                    os << "Bus "s << pos.name << ": "s << "not found"s << std::endl;
                }
            }
            else {
                if (auto buses = std::get<std::optional<std::set<std::string_view, std::less<>>>>(pos.buses_and_stops);  buses.has_value()) {
                    if (buses.value().empty()) {
                        os << "Stop "s << pos.name << ": "s << "no buses"s << std::endl;
                    }
                    else {
                        PrintInformationAboutStop(buses.value(), pos.name, os);
                    }
                }
                else {                  
                    os << "Stop "s << pos.name << ": "s << "not found"s << std::endl;
                }
            }
        }   
}

void InputReader::AdditionToCatalogue(std::istream& is) {

        size_t number;
        std::string str;

        getline(is, str);
        number = std::stoi(str);

        Query q;
        for (size_t i = 0; i < number; ++i) {
            is >> q;

            if (q.query_type == QueryType::ADD_STOP) {
                request_handler_->AddStopsToBuffer(q.bus_stop);
            }
            else if (q.query_type == QueryType::ADD_BUS) {
                request_handler_->AddRoutesToBuffer(q.bus_route);
            }
        }

        request_handler_->LoadBufferToCatalogue();
    }

std::vector<std::string> StringParser(std::string_view str, std::string delimiter, size_t num_str_for_search) {
    std::vector<std::string> output;

    auto delimiter_length = delimiter.size();

    while (!str.empty()) {
        auto prefix_space = str.find_first_not_of(" ");
        if (prefix_space != std::string_view::npos) {
            str.remove_prefix(prefix_space);
        }

        if (output.size() == num_str_for_search - 1) {
            output.push_back(std::string(str.substr(0, str.find_last_not_of(" ") + delimiter_length)));
           return output;
        }

         auto pos_end = str.find(delimiter);
         prefix_space = (pos_end == std::string_view::npos) ? str.find_last_not_of(" ") : str.find_last_not_of(" ", pos_end - delimiter_length);

         if (prefix_space != std::string_view::npos) {
             output.push_back(std::string(str.substr(0, prefix_space + delimiter_length)));
             if (pos_end == std::string_view::npos) {
                 return output;
             }
             else {
                 str.remove_prefix(pos_end + delimiter_length);
             }
         }
         else {
              output.push_back(std::string(str.substr(0, str.size() - delimiter_length)));
              return output;
         }
    }
      return output;
}
  
std::istream& operator>>(std::istream& is, Query& q) {

    std::string input_data;
    getline(is, input_data);

    auto type_name_content = StringParser(input_data, ":"s, 2);
    auto type_name = StringParser(type_name_content[0], " "s, 2);
    if (type_name[0] == ADD_BUS_NAME) {
        q.query_type = QueryType::ADD_BUS;
        q.bus_route.name = move(type_name[1]);

        if (type_name_content[1].find(">") != std::string::npos) {
            q.bus_route.circular = true;
            q.bus_route.stops = move(StringParser(type_name_content[1], ">"s, UINT32_MAX));
        }
        else if (type_name_content[1].find("-") != std::string::npos) {
            q.bus_route.circular = false;
            q.bus_route.stops = move(StringParser(type_name_content[1], "-"s, UINT32_MAX));
        }
        else {
            throw std::invalid_argument("Unknown query"s);
        }
    }
    else if (type_name[0] == ADD_STOP_NAME) {
        q.query_type = QueryType::ADD_STOP;
        q.bus_stop.bus_stop.name = move(type_name[1]);

        auto coordinats_and_lengths = StringParser(type_name_content[1], ","s, 3);

        q.bus_stop.bus_stop.coordinates.lat = std::stod(coordinats_and_lengths[0]);
        q.bus_stop.bus_stop.coordinates.lng = std::stod(coordinats_and_lengths[1]);
        if (coordinats_and_lengths.size() > 2) {
            auto lengths = StringParser(coordinats_and_lengths[2], ","s, UINT32_MAX);

            for (const auto& len : lengths) {
                if (auto length_and_name = StringParser(len, "to"s, 2); length_and_name.size() > 1) {
                    q.bus_stop.lengths.push_back( std::stoi(length_and_name[0].substr(0, length_and_name[0].size() - 1)));
                    q.bus_stop.names.push_back(std::move(length_and_name[1]));
                }
            }
        }
    }
    else {
        throw std::invalid_argument("Unknown query"s);
    }
    return is;
}

std::istream& operator>>(std::istream& is, QueryToBase& query) {

    std::string input_data;
    getline(is, input_data);

    auto type_name = StringParser(input_data, " "s, 2);
    if (type_name[0] == BUS_INFO_NAME) {
        query.type = QueryToBaseType::BUS_INFO;
    }
    else if (type_name[0] == STOP_INFO_NAME) {
        query.type = QueryToBaseType::STOP_INFO;
    }
    else {
        throw std::invalid_argument("Unknown query"s);
    }

    query.name = type_name[1];

    return is;
}

}
}
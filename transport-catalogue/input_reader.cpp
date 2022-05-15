#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "input_reader.h"

namespace catalogue_core{

    using namespace transportcatalogue;
    using namespace std::string_literals;

    namespace inputreader {

        inline const auto ADD_BUS_NAME = "Bus"s;
        inline const auto ADD_STOP_NAME = "Stop"s;

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
    
    void InputReader::LoadBufferToCatalogue() {

        for (auto it = bus_stop_buffer_.begin(); it != bus_stop_buffer_.end(); it++) {
            catalogue_->AddBusStop(it->bus_stop);
        }

        for (auto it = bus_stop_buffer_.begin(); it != bus_stop_buffer_.end(); it++) {
            std::vector<std::pair<std::string, int>> output_lengths;

            auto lengths = inputreader::StringParser(it->lengths, ","s, UINT32_MAX);

            for (auto& len : lengths) {             
                if (auto length_and_name = inputreader::StringParser(len, "to"s, 2); length_and_name.size() > 1) {
                    output_lengths.push_back(std::move(std::pair{ length_and_name[1], std::stoi(length_and_name[0].substr(0, length_and_name[0].size() - 1)) }));
                }
            }

            catalogue_->AddLength(it->bus_stop.name, output_lengths);
        }

        bus_stop_buffer_.clear();

        for (auto it = bus_route_buffer_.begin(); it != bus_route_buffer_.end(); it++) {
            BusRoute output;
            output.circular = it->circular;
            output.name = it->name;

            output.stops.reserve(it->stops.size());
            for (auto& stop : it->stops) {
                output.stops.push_back(catalogue_->FindBusStop(stop));
            }

            catalogue_->AddBusRoute(output);
        }
        bus_route_buffer_.clear();
    }

    std::istream& operator>>(std::istream& is, Query& q) {

        std::string input_data;
        getline(is, input_data);

        auto type_name_content = inputreader::StringParser(input_data, ":"s, 2);
        auto type_name = inputreader::StringParser(type_name_content[0], " "s, 2);
        if (type_name[0] == ADD_BUS_NAME) {
            q.query_type = QueryType::ADD_BUS;
            q.bus_route.name = move(type_name[1]);

            if (type_name_content[1].find(">") != std::string::npos) {
                q.bus_route.circular = true;
                q.bus_route.stops = move(inputreader::StringParser(type_name_content[1], ">"s, UINT32_MAX));
            }
            else if (type_name_content[1].find("-") != std::string::npos) {
                q.bus_route.circular = false;
                q.bus_route.stops = move(inputreader::StringParser(type_name_content[1], "-"s, UINT32_MAX));
            }
            else {
                throw std::invalid_argument("Unknown query"s);
            }
        }
        else if (type_name[0] == ADD_STOP_NAME) {
            q.query_type = QueryType::ADD_STOP;
            q.bus_stop.bus_stop.name = move(type_name[1]);

            auto coordinats_and_lengths = inputreader::StringParser(type_name_content[1], ","s, 3);

            q.bus_stop.bus_stop.coordinates.lat = std::stod(coordinats_and_lengths[0]);
            q.bus_stop.bus_stop.coordinates.lng = std::stod(coordinats_and_lengths[1]);
            if (coordinats_and_lengths.size()>2)
                q.bus_stop.lengths = coordinats_and_lengths[2];
        }
        else {
            throw std::invalid_argument("Unknown query"s);
        }
        return is;
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
                bus_stop_buffer_.push_back(q.bus_stop);
            }
            else if (q.query_type == QueryType::ADD_BUS) {
                bus_route_buffer_.push_back(q.bus_route);
            }
        }

        LoadBufferToCatalogue();
    }
    
    }
}

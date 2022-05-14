#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "stat_reader.h"

namespace catalogue_core {

    namespace statreader {

        std::vector<std::string> StringParser(std::string_view str, std::string delimiter) {
            std::vector<std::string> output;

            while (!str.empty()) {
                auto prefix_space = str.find_first_not_of(" ");
                if (prefix_space != std::string_view::npos) {
                    str.remove_prefix(prefix_space);
                }

                if (output.size() && delimiter == " ") {
                    output.push_back(std::string(str.substr(0, str.find_last_not_of(" ") + 1)));
                    return output;
                }
                auto pos_end = str.find(delimiter);
                prefix_space = (pos_end == std::string_view::npos) ? str.find_last_not_of(" ") : str.find_last_not_of(" ", pos_end - 1);

                if (prefix_space != std::string_view::npos) {
                    output.push_back(std::string(str.substr(0, prefix_space + 1)));
                    if (pos_end == std::string_view::npos) {
                        return output;
                    }
                    else {
                        str.remove_prefix(pos_end + 1);
                    }
                }
                else {
                    output.push_back(std::string(str.substr(0, str.size() - 1)));
                    return output;
                }
            }
            return output;
        }

        std::istream& operator>>(std::istream& is, QueryToBase& query) {

            std::string input_data;
            getline(is, input_data);

            auto type_name = statreader::StringParser(input_data, " "s);
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

        void StatReader::PrintInformationAboutBus(RouteStatistic& output, std::string& name, std::ostream& os) const {

            os << "Bus "s << name << ": "s << output.num_of_stops_ << " stops on route, "s
                << output.num_of_unique_stops_ << " unique stops, "s << std::setprecision(6) << static_cast<double>(output.route_length_) << " route length, "s
                << std::setprecision(6) << output.curvature << " curvature"s << std::endl;
        }

        void StatReader::PrintInformationAboutStop(std::set<std::string_view, std::less<>>& output, std::string& name, std::ostream& os) const {

            os << "Stop "s << name << ": buses"s;

            for (auto it = output.begin(); it != output.end(); ++it) {
                os << " "s << std::string(*it);
            }

            os << std::endl;
        }

        void StatReader::QueryProcessing(std::istream& is, std::ostream& os) {

            size_t number;
            std::string str;

            getline(is, str);
            number = std::stoi(str);

            QueryToBase q;

            for (size_t i = 0; i < number; ++i) {
                is >> q;
                querys_to_base_.push_back(q);
            }

            while (!querys_to_base_.empty()) {

                auto name = querys_to_base_.front().name;

                switch (querys_to_base_.front().type) {

                case QueryToBaseType::BUS_INFO:
                    if (auto result = catalogue_->GetInformationAboutBusRoute(name); result.has_value()) {
                        PrintInformationAboutBus(result.value(), name, os);
                    }
                    else {
                        os << "Bus "s << name << ": "s << "not found"s << std::endl;
                    }
                    break;

                case QueryToBaseType::STOP_INFO:
                    if (auto result = catalogue_->GetInformationAboutStop(name); result.has_value()) {
                        if (result.value().size() == 0) {
                            os << "Stop "s << name << ": "s << "not found"s << std::endl;
                        }
                        else {
                            PrintInformationAboutStop(result.value(), name, os);
                        }

                    }
                    else {
                        os << "Stop "s << name << ": "s << "no buses"s << std::endl;
                    }
                    break;
                }
                querys_to_base_.pop_front();
            }
        }
    }
}
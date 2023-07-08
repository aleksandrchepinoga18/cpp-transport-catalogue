#include "stat_reader.h"
#include <iostream>
#include <cassert>
#include <iomanip>

namespace catalog::output_utils {

using namespace std::literals;
    
    std::string_view ReqBusStat(std::string_view text) {   // ввод номера автобуса
    size_t bus_begin = text.find(' ') + 1;
    return text.substr(bus_begin);
}

std::string_view BusSearchToReq(std::string_view text) {   // ввод остановки имени
    size_t stop_begin = text.find(' ') + 1;
    return text.substr(stop_begin);
}

std::ostream& operator<<(std::ostream& os, const BusStatistics& bus_info) {
    os << "Bus " << bus_info.number << ": " << bus_info.stops_count << " stops on route, "
       << bus_info.unique_stops_count << " unique stops, ";
    os << bus_info.rout_length << " route length, ";
    os << std::setprecision(6) << bus_info.curvature << " curvature";
    return os;
}    
       
void PrintBusStop(std::ostream& os, std::string_view stop_name,
                                  const std::set<std::string_view>* buses) {
    if (!buses) {
        os << "Stop " << stop_name << ": not found" << std::endl;
    } else if (buses->empty()) {
        os << "Stop " << stop_name << ": no buses" << std::endl;
    } else {
        os << "Stop " << stop_name << ": buses ";
        size_t index{0u};
        for (std::string_view bus : *buses) {
            if (index++ != 0)
                os << " ";
            os << bus;
        }
        os << std::endl;
    }
}
    // отвечает за обработку запросов к TransportCatalogue на основе данных, полученных из входного потока input_stream
    void ParseTransportRequest(std::istream& input_stream, const TransportCatalogue& catalogue) {
    int queries_count{0};
    input_stream >> queries_count;
    input_stream.get();

    for (int id = 0; id < queries_count; ++id) {
        std::string query;
        std::getline(input_stream, query);
        if (query.substr(0, 3) == "Bus") {
            std::string_view bus_number = ReqBusStat(query);

            if (auto bus_statistics = catalogue.GetBusStatistics(bus_number)) {
                std::cout << *bus_statistics << '\n';
            } else {
                std::cout << "Bus " << bus_number << ": not found\n";
            }
        } else if (query.substr(0, 4) == "Stop") {
            std::string_view stop_name = BusSearchToReq(query);
            auto* buses = catalogue.GetBusStop(stop_name);

            PrintBusStop(std::cout, stop_name, buses);
        }
    }
}

}  // namespace catalog::output_utils

#include "input_reader.h"
#include "stat_reader.h"

#include <cassert>
#include <regex>

namespace catalog::input_utils {

using namespace std::literals;
using namespace catalog;
using namespace output_utils;

//ввод остановки х - ширина и долгота, расст до ост 1, расст. до ост 2
ToStopsRoutes DistBetStops(std::string_view text) {
        ToStopsRoutes result;
    size_t start = text.find(',');
    start = text.find(',', start + 1) + 2;       //  + 2  =>>  (" "sv).size();
    size_t end = start;

    while (start != std::string_view::npos) {
        end = text.find("m", start);
        int distance = std::stoi(std::string(text.substr(start, end - start)));

        start = end + 5;                 //("m to "sv).size();
        end = text.find(",", start);

        std::string_view stop_to = text.substr(start, end - start);
        result.emplace_back(stop_to, distance);

        start = (end == std::string::npos) ? end : end + 2;
    }
    return result;
}

// ==============ввод широты и долготы / ввод с инфой по остановке ширина и долгота до остановки 
std::pair<Stop, bool> InputBusStop(const std::string& text) {
        Stop stop;
    size_t stop_begin = 5; 
    size_t stop_end = text.find(": ", stop_begin);
    stop.name = text.substr(stop_begin, stop_end - stop_begin);

    size_t latitude_begin = stop_end + 2;  
    size_t latitude_end = text.find(",", latitude_begin);
    stop.point.lat = std::stod(text.substr(latitude_begin, latitude_end - latitude_begin));

    size_t longitude_begin = latitude_end + 2;  
    size_t longitude_end = text.find(",", longitude_begin);
    stop.point.lng = std::stod(text.substr(longitude_begin, longitude_end - longitude_begin));

    bool has_stops_info = (longitude_end != std::string::npos);
    return {std::move(stop), has_stops_info};
}
    
//=============ввод кругового и двустороннего маршрута (1-2-3 остановки )
Bus InputBusRoute(std::string_view text) {
        Bus result;
    size_t bus_start = text.find(' ') + 1;  
    size_t bus_end = text.find(": "sv, bus_start);
    result.number = text.substr(bus_start, bus_end - bus_start);

    result.type = (text[text.find_first_of("->")] == '>') ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;
    std::string_view razdelitel_stops = (result.type == RouteType::CIRCLE) ? " > " : " - ";

    size_t stop_begin = bus_end + 2;   
    while (stop_begin <= text.length()) {
        size_t stop_end = text.find(razdelitel_stops, stop_begin);

        result.stop_names.push_back(text.substr(stop_begin, stop_end - stop_begin));
        stop_begin = (stop_end == std::string_view::npos) ? stop_end : stop_end + razdelitel_stops.size();
    }
           // result.unique_stops = {result.stop_names.begin(), result.stop_names.end()};
        return result;
}
//отвечает за заполнение TransportCatalogue данными из входного потока input_stream
void TransportRequest(std::istream& input_stream, TransportCatalogue& catalogue) {
    int queries_count{0};
    input_stream >> queries_count;
     input_stream.get();

    std::vector<std::string> bus_queries;
    bus_queries.reserve(queries_count);
    std::vector<std::pair<std::string, std::string>> stop_distances;
    stop_distances.reserve(queries_count);

    std::string query;
    for (int id = 0; id < queries_count; ++id) {
        std::getline(input_stream, query);
        if (query.substr(0, 4) == "Stop") {
            auto [stop, is_store_query] = InputBusStop(query);
            if (is_store_query)
                stop_distances.emplace_back(stop.name, std::move(query));
            catalogue.AddStop(std::move(stop));
        } else if (query.substr(0, 3) == "Bus") {
            bus_queries.emplace_back(std::move(query));
        }
    }
    for (const auto& [stop_from, query] : stop_distances) {
        for (auto [stop_to, distance] : DistBetStops(query))
            catalogue.AddDistance(stop_from, stop_to, distance);
    }

    for (const auto& bus_query : bus_queries)
        catalogue.AddBus(InputBusRoute(bus_query));
}
    
}  // namespace catalog::input_utils

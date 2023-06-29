#pragma once
#include "transport_catalogue.h"
#include <algorithm>
#include <vector>
 
namespace transport_catalogue {
namespace detail {
namespace bus {
    
struct BusQueryResult{
    std::string_view name;
    bool not_found;
    size_t stops_on_route;
    size_t unique_stops;
    size_t route_length;
    double curvature;
};
    
void show_terminal(BusQueryResult bus_info, std::ostream& output);
BusQueryResult query(TransportCatalogue& catalogue, std::string_view str);
    
}//end namespace bus
    
namespace stop {
struct StopQueryResult{
    std::string_view name;
    bool not_found;
    std::vector <std::string_view> buses_name;
};
    
void show_terminal(const StopQueryResult& stop_info, std::ostream& output);
StopQueryResult query(TransportCatalogue& catalogue, std::string_view stop_name);
    
}//end namespace stop
    
void execute_query(TransportCatalogue& catalogue, std::string_view str);
void parse_and_execute_queries(std::istream& input, TransportCatalogue& catalogue);
    
}//end namespace detail
}//end namespace transport_catalogue

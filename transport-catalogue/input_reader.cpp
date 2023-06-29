#include "input_reader.h"
#include "stat_reader.h"
#include <algorithm>
 
namespace transport_catalogue {
namespace detail {
namespace stop {
    
Stop parse_stop(const std::string& str) {
    auto twopoint_pos = str.find(':');
    auto comma_pos = str.find(',');
    auto entry_length = 5;
    auto distance = 2;
    Stop stop;
 
    stop.name = str.substr(entry_length, 
                             twopoint_pos - entry_length);
    stop.latitude = stod(str.substr(twopoint_pos + distance, 
                                      comma_pos - twopoint_pos - distance));
    stop.longitude = stod(str.substr(comma_pos + distance));
    
    return stop;
}
    
}//end namespace stop
    
namespace distance {
    
std::vector<Distance> parse_distances(const std::string& str, TransportCatalogue& catalogue) {
    std::vector<Distance> distances;
    auto entry_length = 5;
    auto twopoint_pos = str.find(':');
    auto space = 2;
    
    std::string str_copy = str;
    std::string name = str_copy.substr(entry_length, 
                                   twopoint_pos - entry_length);
    str_copy = str_copy.substr(str_copy.find(',') + 1);
    str_copy = str_copy.substr(str_copy.find(',') + space);
 
    while(str_copy.find(',') != std::string::npos){                    
        int distance = stoi(str_copy.substr(0, str_copy.find('m')));
        std::string stop_dist_name = str_copy.substr(str_copy.find('m') + entry_length);
        stop_dist_name = stop_dist_name.substr(0, stop_dist_name.find(','));
        distances.push_back({catalogue.get_stop(name), 
                              catalogue.get_stop(stop_dist_name), distance});
        str_copy = str_copy.substr(str_copy.find(',') + space);
    }
    std::string last_name = str_copy.substr(str_copy.find('m') + entry_length);
    int distance = stoi(str_copy.substr(0, str_copy.find('m')));
    distances.push_back({catalogue.get_stop(name), catalogue.get_stop(last_name), distance});
    
    return distances;
}
    
}//end namespace distance
    
namespace bus {
    
Bus parse_bus(std::string_view str, TransportCatalogue& catalogue) {
    auto entry_length = 4;
    auto distance = 2;
    auto twopoint_pos = str.find(':');
    Bus bus;
    bus.name = str.substr(entry_length, 
                           twopoint_pos - entry_length);
 
    str = str.substr(twopoint_pos + distance);
 
    auto more_pos = str.find('>');
    if (more_pos == std::string_view::npos) {
        auto tire_pos = str.find('-');
 
        while (tire_pos != std::string_view::npos) {
            bus.stops.push_back(catalogue.get_stop(str.substr(0, tire_pos - 1)));
 
            str = str.substr(tire_pos + distance);
            tire_pos = str.find('-');
        }
 
        bus.stops.push_back(catalogue.get_stop(str.substr(0, tire_pos - 1)));
        size_t size = bus.stops.size() - 1;
 
        for (size_t i = size; i > 0; i--) {
            bus.stops.push_back(bus.stops[i-1]);
        }
 
    } else {
        while (more_pos != std::string_view::npos) {
            bus.stops.push_back(catalogue.get_stop(str.substr(0, more_pos - 1)));
 
            str = str.substr(more_pos + distance);
            more_pos = str.find('>');
        }
 
        bus.stops.push_back(catalogue.get_stop(str.substr(0, more_pos - 1)));
    }
    
    return bus;
}
    
}//end namespace bus
    
void fill_catalogue(std::istream& input, TransportCatalogue& catalogue) {
 
    std::string count;
    std::getline(input, count);
 
    if (count != "") {
        std::string str;
        std::vector<std::string> buses;
        std::vector<std::string> stops;
        int amount = stoi(count);
        auto bus_distance = 3;
 
        for (int i = 0; i < amount; ++i) {
            std::getline(input, str);
 
            if (str != "") {
                auto space_pos = str.find_first_not_of(' ');
                str = str.substr(space_pos);
 
                if (str.substr(0, bus_distance) != "Bus") {
                    stops.push_back(str);
                } else {
                    buses.push_back(str);
                }
            }
        }
 
        for (auto stop : stops) {
            catalogue.add_stop(stop::parse_stop(stop));
        }
 
        for (auto stop : stops) {
            catalogue.add_distance(distance::parse_distances(stop, catalogue));
        }
 
        for (auto bus : buses) {
            catalogue.add_bus(bus::parse_bus(bus, catalogue));
        }
    }
}
    
}//end namespace detail
}//end namespace transport_catalogue

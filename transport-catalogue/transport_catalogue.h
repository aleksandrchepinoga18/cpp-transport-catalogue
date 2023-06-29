#pragma once
#include <deque>
#include <string>
#include <vector>
 
#include <iomanip>
#include <iostream>
 
#include <unordered_set>
#include <unordered_map>
 
#include "geo.h"
 
namespace transport_catalogue {   
    
struct Bus;
    
struct Stop {    
    std::string name;
    double latitude;
    double longitude;
    
    std::vector<Bus*> buses;
};
 
struct Bus { 
    std::string name;
    std::vector<Stop*> stops;
    size_t route_length;
};
 
struct Distance {    
    const Stop* A;
    const Stop* B;
    int distance;
};
 
struct DistanceHasher {
    std::hash<const void*> hasher;
    
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept {
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);
        return hasher(hash_1) * 17 + hasher(hash_2);
    }  
};
    
typedef  std::unordered_map<std::string_view, Stop*> StopMap;
typedef  std::unordered_map<std::string_view, Bus*> BusMap;
typedef  std::unordered_map<std::pair<const Stop*, const  Stop*>, int, DistanceHasher> DistanceMap;
 
class TransportCatalogue {
public:  
    
    void add_bus(Bus&& bus);
    void add_stop(Stop&& stop);
    void add_distance(const std::vector<Distance>& distances);
    
    Bus* get_bus(std::string_view bus_name);
    Stop* get_stop(std::string_view stop_name);
 
    std::unordered_set<const Bus*> stop_get_uniq_buses(Stop* stop);    
    std::unordered_set<const Stop*> get_uniq_stops(Bus* bus);
    double get_length(Bus* bus);
    
    size_t get_distance_stop(const Stop* start, const Stop* finish);
    size_t get_distance_to_bus(Bus* bus);
    
private:
    
    std::deque<Stop> stops;
    StopMap stopname_to_stop;
    
    std::deque<Bus> buses;
    BusMap busname_to_bus;
    
    DistanceMap distance_to_stop;
    
};
    
}//end namespace transport_catalogue

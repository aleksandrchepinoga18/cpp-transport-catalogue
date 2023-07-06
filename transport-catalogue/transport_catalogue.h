#pragma once

#include <deque>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

using ToStopsRoutes = std::vector<std::pair<std::string_view, int>>;

namespace catalog {

enum class RouteType { CIRCLE, TWO_DIRECTIONAL };

struct Bus {
    std::string number;
    RouteType type;
    std::vector<std::string_view> stop_names;
    std::set<std::string_view> unique_stops;

 size_t GetStopsCount() const;
};

struct Stop {
    std::string name;
    geo::Coordinates point;

    size_t Hash() const {
    size_t hash_value = std::hash<std::string>{}(name); //значение хеша имени остановки 
    hash_value += even_value * std::hash<double>{}(point.lng); // добавляем хеш долготы 
    hash_value += even_value * even_value * std::hash<double>{}(point.lat); // хеш широты 
    return hash_value;
}

private:
    static const size_t even_value{37};
};

using PointStops = std::pair<const Stop*, const Stop*>;

struct BusStatistics {
    std::string_view number;
    size_t stops_count{0u};
    size_t unique_stops_count{0u};
    int rout_length{0};
    double curvature{0.};
};

std::ostream& operator<<(std::ostream& os, const BusStatistics& statistics);

class TransportCatalogue {
public:  
    TransportCatalogue() = default;

public:   
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);

     std::optional<BusStatistics> BusStat(std::string_view bus_number) const;
       const std::set<std::string_view>* GetBusStop(std::string_view stop_name) const;

private:   
    struct PointStopsHash {
        size_t operator()(const PointStops& pair) const {
            return pair.first->Hash() + prime_number * pair.second->Hash();
        }

    private:
        static const size_t prime_number{31};
    };

private:  
    int AllRouteLen(const Bus* bus_info) const;
    double GeoLenCal(const Bus* bus_info) const;

private:   
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, const Stop*> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string_view, const Bus*> buses_;

    std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
    std::unordered_map<PointStops, int, PointStopsHash> distances_between_stops_;
};

}  // namespace catalog

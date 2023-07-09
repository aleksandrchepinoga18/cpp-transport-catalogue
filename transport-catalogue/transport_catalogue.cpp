#include "transport_catalogue.h"

#include <execution>
#include <iomanip>
#include <numeric>

namespace catalog {
    
size_t Bus::GetUniqueStopsCount() const {
    std::set<std::string_view> unique_stops(stop_names.begin(), stop_names.end());
    return unique_stops.size();
}
    
size_t Bus::GetStopsCount() const {
    return (type == RouteType::CIRCLE) ? stop_names.size() : 2 * stop_names.size() - 1;
}
/*
Функция принимает объект Stop по значению и перемещает его в вектор stops_storage_, используя std::move.
Функция insert используется для вставки остановки в начало вектора stops_storage_. 
Указатель на вставленную позицию - &(*position).
Словарь stops_ обновляется путем вставки пары ключ-значение с именем остановки в качестве ключа и указателем на вставленную остановку в качестве значения.
Словарь buses_through_stop_ обновляется путем установки пары ключ-значение с именем остановки в качестве ключа и пустым std::set<std::string_view> в качестве значения
*/
 void TransportCatalogue::AddStop(const Stop&& stop) {
    auto position = stops_storage_.insert(stops_storage_.begin(), std::move(stop));
       const Stop* inserted_stop = &(*position);
             stops_.insert({inserted_stop->name, inserted_stop});
    buses_through_stop_.emplace(inserted_stop->name, std::set<std::string_view>());
} 
       
//тут остановки уже все пропарсены (в теле функции) - получим указатели на остановки 
void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
    distances_between_stops_.insert({{stops_.at(stop_from), stops_.at(stop_to)}, distance});
}
    
 void TransportCatalogue::AddBus(Bus bus) {
    Bus modified_bus = std::move(bus); // Перемещаем автобус в модифицированный экземпляр
    for (auto& stop : modified_bus.stop_names) {   // Анализируем и модифицируем остановки
            stop = stops_.find(stop)->first;
    }
    //modified_bus.unique_stops = {modified_bus.stop_names.begin(), modified_bus.stop_names.end()};
    buses_storage_.insert(buses_storage_.begin(), std::move(modified_bus));  // Вставляем модифицированный автобус в хранилище
        const Bus* inserted_bus = &(*buses_storage_.begin());
    buses_.insert({inserted_bus->number, inserted_bus});
        for (std::string_view stop : inserted_bus->stop_names) {    // Добавляем автобус к остановкам
                buses_through_stop_[stop].insert(inserted_bus->number);
    }
}
    
std::optional<BusStatistics> TransportCatalogue::GetBusStatistics(std::string_view bus_number) const {
    if (buses_.count(bus_number) == 0)
        return std::nullopt;
    const Bus* bus_info = buses_.at(bus_number);
    BusStatistics result;
    result.number = bus_info->number;
    result.stops_count = bus_info->GetStopsCount();
    result.unique_stops_count = bus_info->GetUniqueStopsCount();
    result.rout_length = AllRouteLen(bus_info);
    result.curvature = static_cast<double>(result.rout_length) / GeoLenCal(bus_info);
    return result;
}

/*если не нашли куда, то ищем откуда 
Если расстояние найдено, оно возвращается. В противном случае функция извлекает расстояние, используя обратную пару остановок.*/
int TransportCatalogue::AllRouteLen(const Bus* bus_info) const {
    auto get_route_length = [this](std::string_view from, std::string_view to) {
        auto key = std::make_pair(stops_.at(from), stops_.at(to));
        return (distances_between_stops_.count(key) > 0)
                   ? distances_between_stops_.at(key)
                   : distances_between_stops_.at({stops_.at(to), stops_.at(from)});
    };
    int forward_route =
        std::transform_reduce(bus_info->stop_names.begin(), std::prev(bus_info->stop_names.end()),
                              std::next(bus_info->stop_names.begin()), 0, std::plus<>(), get_route_length);
    if (bus_info->type == RouteType::CIRCLE)
        return forward_route;  // возврат прямого маршрута , если так, то ниже считаем двусторонний
    int backward_route =
        std::transform_reduce(bus_info->stop_names.rbegin(), std::prev(bus_info->stop_names.rend()),
                              std::next(bus_info->stop_names.rbegin()), 0, std::plus<>(), get_route_length);
//иначе двусторонний маршрут - поэтому считаем расстояние по обратному пути и возвращаем его в ретарн 
    return forward_route + backward_route; 
}

/*функция перебирает названия остановок в bus_info и вычисляет географическую длину путем суммирования расстояний между последовательными остановками */
double TransportCatalogue::GeoLenCal(const Bus* bus_info) const {
  double geographic_length = 0.0;
    for (size_t i = 1; i < bus_info->stop_names.size(); ++i) {
        std::string_view from = bus_info->stop_names[i - 1];
        std::string_view to = bus_info->stop_names[i];
        geographic_length += ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
    }
    //расчетная географическая протяженность умножается на 2, если тип автобуса не является кольцевым маршрутом
 return (bus_info->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.0;
}

//извлекает набор имен автобусов, проходящих через заданную остановку
const std::set<std::string_view>* TransportCatalogue::GetBusStop(std::string_view stop_name) const {
    auto it = buses_through_stop_.find(stop_name);
    if (it != buses_through_stop_.end()) {
        return &it->second;
    }
    return nullptr;
}

}  // namespace catalog

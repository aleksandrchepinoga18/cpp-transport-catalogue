#include "json_reader.h"
#include "json_builder.h"

#include <string>

namespace request {

using namespace catalogue;
using namespace request;

using namespace std::literals;

namespace {

std::pair<catalogue::Stop, bool> InputBusStop(const json::Dict& info) {
    Stop stop;

    stop.name = info.at("name"s).AsString();
    stop.point.lat = info.at("latitude"s).AsDouble();
    stop.point.lng = info.at("longitude"s).AsDouble();

    bool has_road_distances = !info.at("road_distances"s).AsDict().empty();

    return {std::move(stop), has_road_distances};
}

Bus InputBusRoute(const json::Dict& info) {
    Bus bus;

    bus.number = info.at("name"s).AsString();
    bus.type = info.at("is_roundtrip"s).AsBool() ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;

    const auto& stops = info.at("stops"s).AsArray();
    bus.stop_names.reserve(stops.size());

    for (const auto& stop : stops)
        bus.stop_names.emplace_back(stop.AsString());

    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

    return bus;
}

void MakeBusResponse(int request_id, const BusStatistics& statistics, json::Builder& response) {
  //   нет необходимости использовать std::move(), потому что все типы справа тривиальны

    response.StartDict();
    response.Key("curvature"s).Value(statistics.curvature);
    response.Key("request_id"s).Value(request_id);
    response.Key("route_length"s).Value(statistics.rout_length);
    response.Key("stop_count"s).Value(static_cast<int>(statistics.stops_count));
    response.Key("unique_stop_count"s).Value(static_cast<int>(statistics.unique_stops_count));
    response.EndDict();
}

void MakeStopResponse(int request_id, const std::set<std::string_view>& buses, json::Builder& response) {
    response.StartDict();
    response.Key("request_id"s).Value(request_id);

    response.Key("buses"s).StartArray();
    for (std::string_view bus : buses)
        response.Value(std::string(bus));
    response.EndArray();

    response.EndDict();
}

void MakeErrorResponse(int request_id, json::Builder& response) {
    response.StartDict();
    response.Key("request_id"s).Value(request_id);
    response.Key("error_message"s).Value("not found"s);
    response.EndDict();
}

void MakeMapImageResponse(int request_id, const std::string& image, json::Builder& response) {
    response.StartDict();
    response.Key("request_id"s).Value(request_id);
    response.Key("map"s).Value(image);
    response.EndDict();
}

/* METHODS FOR MAP IMAGE RENDERING */

render::Screen ParseScreenSettings(const json::Dict& settings) {
    render::Screen screen;

    screen.width_ = settings.at("width"s).AsDouble();
    screen.height_ = settings.at("height"s).AsDouble();
    screen.padding_ = settings.at("padding"s).AsDouble();

    return screen;
}

render::Label ParseLabelSettings(const json::Dict& settings, const std::string& key_type) {
    int font_size = settings.at(key_type + "_label_font_size"s).AsInt();
    const json::Array offset = settings.at(key_type + "_label_offset"s).AsArray();

    double offset_x = offset.at(0).AsDouble();
    double offset_y = offset.at(1).AsDouble();

    return {font_size, {offset_x, offset_y}};
}

    
    
svg::Color ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }
    const auto& array = node.AsArray();
    if (array.size() >= 3) {
        uint8_t red = array.at(0).AsInt();
        uint8_t green = array.at(1).AsInt();
        uint8_t blue = array.at(2).AsInt();

        // Если в массиве всего 3 цвета - это rgb
        if (array.size() == 3) {
            return svg::Rgb(red, green, blue);
        }
        // Otherwise - this is rgba
        if (array.size() >= 4) {
            double alpha = array.at(3).AsDouble();
            return svg::Rgba(red, green, blue, alpha);
        }
    }
    // Возвращаем значение по умолчанию или выбрасываем ошибку
    // в зависимости от требуемого поведения
    throw std::runtime_error("Invalid color format");
}


render::UnderLayer ParseLayer(const json::Dict& settings) {
    render::UnderLayer layer;

    layer.color_ = ParseColor(settings.at("underlayer_color"s));
    layer.width_ = settings.at("underlayer_width"s).AsDouble();

    return layer;
}

}  // namespace


//====================================================================================================
    
TransportCatalogue ProcessBaseRequest(const json::Array& requests) {
    TransportCatalogue catalogue;

    // Мы могли добавлять расстояния между остановками, ТОЛЬКО если они СУЩЕСТВУЮТ в каталоге.
    std::vector<int> requests_ids_with_road_distances;
    requests_ids_with_road_distances.reserve(requests.size());

    std::vector<int> requests_ids_with_buses;
    requests_ids_with_buses.reserve(requests.size());

    // Шаг 1. Сохраните все остановки в каталоге и отметьте заявки, которые необходимо обработать позже
    for (int id = 0; id != static_cast<int>(requests.size()); ++id) {
        const auto& request_dict_view = requests.at(id).AsDict();

        if (request_dict_view.at("type"s) == "Stop"s) {
            auto [stop, has_road_distances] = InputBusStop(request_dict_view);
            if (has_road_distances)
                requests_ids_with_road_distances.emplace_back(id);

            catalogue.AddStop(std::move(stop));
        } else if (request_dict_view.at("type"s) == "Bus"s) {
            requests_ids_with_buses.emplace_back(id);
        }
    }

    // второй шаг - рассточние между остановками 
    for (int id : requests_ids_with_road_distances) {
        const auto& request_dict_view = requests.at(id).AsDict();
        
        std::string_view stop_from = request_dict_view.at("name"s).AsString();
       for (const auto& [stop_to, distance] : request_dict_view.at("road_distances"s).AsDict())
            catalogue.AddDistance(stop_from, stop_to, distance.AsInt());
    }

    // Шаг 3. Добавьте информацию о маршрутах автобусов через остановки
    for (int id : requests_ids_with_buses) {
        const auto& request_dict_view = requests.at(id).AsDict();
        catalogue.AddBus(InputBusRoute(request_dict_view));
    }

    return catalogue;
}

render::Visualization ParseVisualizationSettings(const json::Dict& settings) {
    render::Visualization final_settings;

    double line_width = settings.at("line_width"s).AsDouble();
    double stop_radius = settings.at("stop_radius"s).AsDouble();

    // Разобрать список цветов
    const auto& colors = settings.at("color_palette"s).AsArray();
    std::vector<svg::Color> svg_colors;
    svg_colors.reserve(colors.size());
    for (const auto& color : colors)
        svg_colors.emplace_back(ParseColor(color));

    final_settings.SetScreen(ParseScreenSettings(settings))
        .SetLineWidth(line_width)
        .SetStopRadius(stop_radius)
        .SetLabels(render::LabelType::Stop, ParseLabelSettings(settings, "stop"s))
        .SetLabels(render::LabelType::Bus, ParseLabelSettings(settings, "bus"s))
        .SetUnderLayer(ParseLayer(settings))
        .SetColors(std::move(svg_colors));

    return final_settings;
}

json::Node MakeStatResponse(const TransportCatalogue& catalogue, const json::Array& requests,
                            const render::Visualization& settings) {
    auto response = json::Builder();
    response.StartArray();

    for (const auto& request : requests) {
        const auto& request_dict_view = request.AsDict();

        int request_id = request_dict_view.at("id"s).AsInt();
        std::string type = request_dict_view.at("type"s).AsString();
        std::string name;  //> Could be a name of bus or a stop

        if (type == "Bus"s) {
            name = request_dict_view.at("name"s).AsString();

            if (auto bus_statistics = catalogue.GetBusStatistics(name)) {
                MakeBusResponse(request_id, *bus_statistics, response);
            } else {
                MakeErrorResponse(request_id, response);
            }
        } else if (type == "Stop"s) {
            name = request_dict_view.at("name"s).AsString();
            if (auto buses = catalogue.GetBusStop(name)) {
                MakeStopResponse(request_id, *buses, response);
            } else {
                MakeErrorResponse(request_id, response);
            }
        } else if (type == "Map"s) {
            std::string image = RenderTransportMap(catalogue, settings);
            MakeMapImageResponse(request_id, image, response);
        }
    }

    response.EndArray();
    return std::move(response.Build());
}

}  // namespace request

#include "request_handler.h"

#include <string>

namespace request {

using namespace std::literals;
using namespace catalogue;

void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output) {
    const auto input_json = json::Load(input).GetRoot();

    // Step 1. Формировать каталог на основе входных данных
    TransportCatalogue catalogue;
    const auto& base_requests = input_json.AsMap().at("base_requests"s).AsArray();
    auto transport_catalogue = ProcessBaseRequest(base_requests);

    // Step 2. Разобрать настройки rendering 
    const auto& render_settings = input_json.AsMap().at("render_settings"s).AsMap();
    const auto& visualization_settings = ParseVisualizationSettings(render_settings);

    // Step 3. Форма ответа
    const auto& stat_requests = input_json.AsMap().at("stat_requests"s).AsArray();
    auto response = MakeStatResponse(transport_catalogue, stat_requests, visualization_settings);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace request

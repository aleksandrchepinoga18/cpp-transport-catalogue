#include "request_handler.h"
#include <string>

namespace request {

using namespace std::literals;
using namespace catalogue;


    // вариант упрощенный более понятный 
void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output) {
    // Загружаем JSON-документ и получаем корневой узел
    json::Document doc = json::Load(input);
    const auto& input_json = doc.GetRoot().AsDict();

    // Формируем каталог на основе входных данных с помощью метода из json_reader.cpp
    auto transport_catalogue = request::ProcessBaseRequest(input_json.at("base_requests").AsArray());

    // Разбираем настройки отображения с помощью метода из json_reader.cpp
    auto visualization_settings = request::ParseVisualizationSettings(input_json.at("render_settings").AsDict());

    // Формируем ответ на основе каталога и запросов с помощью метода из json_reader.cpp
    auto response = request::MakeStatResponse(transport_catalogue, input_json.at("stat_requests").AsArray(), visualization_settings);

    // Выводим ответ в формате JSON
    json::Print(json::Document(std::move(response)), output);
}
    
}  // namespace request

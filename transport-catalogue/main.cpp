#include "json_builder.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "domain.h"
#include "map_renderer.h"

#include "request_handler.h"

using namespace std;
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace catalogue;

int main() {
        // Раскомментируйте эту строку, чтобы вызвать функцию ProcessTransportCatalogueQuery
   // request::ProcessTransportCatalogueQuery(std::cin, std::cout);

  // Чтение входного JSON-документа
    json::Document doc = json::Load(std::cin);
    const auto& input_json = doc.GetRoot().AsDict();

    // Шаг 1. Формирование каталога на основе входных данных с помощью метода из json_reader.cpp
    const auto& base_requests = input_json.at("base_requests").AsArray();
    TransportCatalogue transport_catalogue = request::ProcessBaseRequest(base_requests);

    // Шаг 2. Разбор настроек отображения с помощью метода из json_reader.cpp
    const auto& render_settings = input_json.at("render_settings").AsDict();
    render::Visualization visualization_settings = request::ParseVisualizationSettings(render_settings);

    // Шаг 3. Формирование ответа на основе каталога и запросов с помощью метода из json_reader.cpp
   const auto& stat_requests = input_json.at("stat_requests").AsArray();
    json::Node response = request::MakeStatResponse(transport_catalogue, stat_requests, visualization_settings);


    // Вывод ответа в формате JSON
    json::Print(json::Document(std::move(response)), std::cout);
    
    return 0;
}

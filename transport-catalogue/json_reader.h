#pragma once

/*
* Описание: парсит данные JSON, созданные во время парсинга, и формирует массив
 * ответов JSON
 */

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include "domain.h"

namespace request {

catalogue::TransportCatalogue ProcessBaseRequest(const json::Array& requests);
    
render::Visualization ParseVisualizationSettings(const json::Dict& settings);
    
json::Node MakeStatResponse(const catalogue::TransportCatalogue& catalogue, const json::Array& requests,
                            const render::Visualization& settings);
       

}  // namespace request

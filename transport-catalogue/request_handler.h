#pragma once

/*
* Описание: модуль обработки запросов.
 * Действует как Фасад, упрощающий взаимодействие с транспортным каталогом
 */

#include "json_reader.h"

namespace request {

void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output);

}  // namespace request 

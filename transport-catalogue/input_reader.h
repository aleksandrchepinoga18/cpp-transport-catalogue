#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"

namespace catalog::input_utils {

ToStopsRoutes DistBetStops(std::string_view text);

std::pair<catalog::Stop, bool> InputBusStop(const std::string& text);
catalog::Bus InputBusRoute(std::string_view text);
 
void TransportRequest(std::istream& input_stream, TransportCatalogue& catalogue);

}  // namespace catalog::input_utils

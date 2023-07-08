#pragma once
#include <string>

#include "transport_catalogue.h"

namespace catalog::output_utils {
    
std::string_view ReqBusStat(std::string_view text);
std::string_view BusSearchToReq(std::string_view text);
void PrintBusStop(std::ostream& os, std::string_view stop_name,const std::set<std::string_view>* buses);
    
std::ostream& operator<<(std::ostream& os, const BusStatistics& bus_info);
void ParseTransportRequest(std::istream& input_stream, const TransportCatalogue& catalogue);
 
}  // namespace catalog::output_utils

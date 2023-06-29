#pragma once
#include "transport_catalogue.h"
 
namespace transport_catalogue {
namespace detail {  
namespace stop {
    Stop parse_stop(const std::string& str);
    
}//end namespace stop
    
namespace distance {
    std::vector<Distance> parse_distances(const std::string& str, TransportCatalogue& catalogue);
    
}//end namespace distance
    
namespace bus {
    Bus parse_bus(std::string_view str, TransportCatalogue& catalogue);
    
}//end namespace bus
 
void fill_catalogue(std::istream& input, TransportCatalogue& catalogue);
 
}//end namespace detail 
}//end namespace transport_catalogue

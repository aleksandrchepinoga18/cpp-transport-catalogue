#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

using namespace std;

int main() {     
catalog::TransportCatalogue catalogue;
catalog::input_utils::TransportRequest(std::cin, catalogue);
catalog::output_utils::ParseTransportRequest(std::cin, catalogue);
} 

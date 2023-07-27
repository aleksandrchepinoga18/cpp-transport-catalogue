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

using namespace catalogue;

int main() {

        request::ProcessTransportCatalogueQuery(std::cin, std::cout);


    return 0;
}

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
 
using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::detail;
 
int main() {
    TransportCatalogue catalogue;
    fill_catalogue(std::cin, catalogue);
    parse_and_execute_queries(std::cin, catalogue);
} 

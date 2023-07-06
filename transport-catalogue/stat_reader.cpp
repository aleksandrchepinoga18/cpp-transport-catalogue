#include "stat_reader.h"

#include <cassert>
#include <iomanip>

namespace catalog::output_utils {

using namespace std::literals;
    
    std::string_view ReqBusStat(std::string_view text) {   // ввод номера автобуса
    size_t bus_begin = text.find(' ') + 1;
    return text.substr(bus_begin);
}

std::string_view BusSearchToReq(std::string_view text) {   // ввод остановки имени
    size_t stop_begin = text.find(' ') + 1;
    return text.substr(stop_begin);
}

void PrintBusStop(std::ostream& os, std::string_view stop_name,
                                  const std::set<std::string_view>* buses) {
    if (!buses) {
        os << "Stop " << stop_name << ": not found" << std::endl;
    } else if (buses->empty()) {
        os << "Stop " << stop_name << ": no buses" << std::endl;
    } else {
        os << "Stop " << stop_name << ": buses ";
        size_t index{0u};
        for (std::string_view bus : *buses) {
            if (index++ != 0)
                os << " ";
            os << bus;
        }
        os << std::endl;
    }
}

}  // namespace catalog::output_utils

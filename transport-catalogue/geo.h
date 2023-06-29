#pragma once
#include <cmath>
 
namespace transport_catalogue {
namespace detail {
namespace geo {
    
const int EARTH_RADIUS = 6371000;
const double PI = 3.1415926535;
    
struct Coordinates {
    double latitude;
    double longitude;
    bool operator==(const Coordinates& other) const {
        return latitude == other.latitude 
            && longitude == other.longitude;
    }
};
    
inline double compute_distance(Coordinates start, Coordinates end) {
    using namespace std;
    if (!(start == end)) {
        const double dr = PI / 180.;
        return acos(sin(start.latitude * dr) * sin(end.latitude * dr)
                    + cos(start.latitude * dr) * cos(end.latitude * dr) 
                    * cos(abs(start.longitude - end.longitude) * dr)) * EARTH_RADIUS;
    } else {
        return 0.0;
    }
}
    
}//end namespace geo
}//end namespace detail
}//end namespace transport_catalogue

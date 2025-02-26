#include <numbers>
#include <cmath>
#include "convert.hpp"

namespace darauble {

int32_t fromDouble(double value) {
    return (int32_t) (value * degreesToSemiCircle);
}

double fromInt32(int32_t value) {
    return (double)value * semiCircleToDegrees;
}

void calculate_square(const double lat, const double lon, const double distance_m, double &top_lat, double &left_lon, double &bottom_lat, double &right_lon) {
    double lat_rad = lat * (std::numbers::pi / 180.0);

    // Calculate degree offsets
    double lat_offset = distance_m / METERS_PER_DEGREE_LAT;
    double lon_offset = distance_m / (METERS_PER_DEGREE_LAT * std::cos(lat_rad));

    // Calculate corners
    top_lat = lat + lat_offset;
    left_lon = lon - lon_offset;
    bottom_lat = lat - lat_offset;
    right_lon = lon + lon_offset;
}

} // namespace darauble

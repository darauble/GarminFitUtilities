#include "formulae.hpp"
#include "convert.hpp"

namespace darauble::coordinates {

double haversine_degrees(double lat1, double lon1, double lat2, double lon2) {
    return haversine_radians(radiansFromDegrees(lat1), radiansFromDegrees(lon1),
                             radiansFromDegrees(lat2), radiansFromDegrees(lon2));
}

double haversine_radians(double rlat1, double rlon1, double rlat2, double rlon2) {
    double dlat = rlat2 - rlat1;
    double dlon = rlon2 - rlon1;

    double a = std::pow(std::sin(dlat / 2), 2) + std::cos(rlat1) * std::cos(rlat2) * std::pow(std::sin(dlon / 2), 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EARTH_RADIUS * c;
}

double bearing_radians(double lat1, double lon1, double lat2, double lon2) {
    double dlon = lon2 - lon1;

    double x = std::cos(lat2) * std::sin(dlon);
    double y = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(dlon);

    return std::atan2(x, y);
}

void next_point_radians(double lat1, double lon1, double bearing, double distance, double &lat2, double &lon2) {
    lat2 = std::asin(std::sin(lat1) * std::cos(distance / EARTH_RADIUS) +
                    std::cos(lat1) * std::sin(distance / EARTH_RADIUS) * std::cos(bearing));

    lon2 = lon1 + std::atan2(std::sin(bearing) * std::sin(distance / EARTH_RADIUS) * std::cos(lat1),
                              std::cos(distance / EARTH_RADIUS) - std::sin(lat1) * std::sin(lat2));
}

} // namespace darauble::coordinates
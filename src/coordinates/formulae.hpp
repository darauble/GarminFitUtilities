#include <cmath>

namespace darauble::coordinates {

inline constexpr double EARTH_RADIUS = 6371000.0;

double haversine_degrees(double lat1, double lon1, double lat2, double lon2);

double haversine_radians(double rlat1, double rlon1, double rlat2, double rlon2);

double bearing_radians(double lat1, double lon1, double lat2, double lon2);

void next_point_radians(double lat1, double lon1, double bearing, double distance, double &lat2, double &lon2);

}
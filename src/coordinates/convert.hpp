/*
  Convert coordinates in the floating point to Garmin's integer (semicircles) representation and back.
 */
#pragma once
#include <cstdint>

namespace darauble {

constexpr double semiCircleToDegrees = 180.0 / ((uint32_t)1 << 31);
constexpr double degreesToSemiCircle = ((uint32_t)1 << 31) / 180.0;
constexpr double METERS_PER_DEGREE_LAT = 111320.0;

int32_t fromDouble(double);

double fromInt32(int32_t);

void calculate_square(const double lat, const double lon, const double distance_m, double &top_lat, double &left_lon, double &bottom_lat, double &right_lon);

} // namespace darauble

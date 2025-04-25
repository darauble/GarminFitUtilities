/*
  Convert coordinates in the floating point to Garmin's integer (semicircles) representation and back.
 */
#pragma once
#include <cstdint>
#include <numbers>

namespace darauble {

constexpr double semiCircleToDegrees = 180.0 / ((uint32_t)1U << 31);
constexpr double semiCircleToRadians = std::numbers::pi / static_cast<double>(1U << 31);
constexpr double degreesToSemiCircle = ((uint32_t)1U << 31) / 180.0;
constexpr double degreesToRadians = std::numbers::pi / 180.0;
constexpr double radiansToSemicircles = static_cast<double>(1U << 31) / std::numbers::pi;
constexpr double METERS_PER_DEGREE_LAT = 111320.0;

int32_t fromDouble(double);

double fromInt32(int32_t);

double radiansFromInt32(int32_t);

double radiansFromDegrees(double);

uint32_t int32FromRadians(double);

void calculate_square(const double lat, const double lon, const double distance_m, double &top_lat, double &left_lon, double &bottom_lat, double &right_lon);

} // namespace darauble

#pragma once

#include <iostream>
#include <filesystem>
#include <vector>


namespace fs = std::filesystem;
namespace darauble::parsers {

struct TrackPoint {
    double lat;
    double lon;
    double distance;
    double cumulative_distance;
};

std::vector<TrackPoint> ReadTrackpoints(const fs::path& filename, bool calculate_distance = false);

} // namespace darauble
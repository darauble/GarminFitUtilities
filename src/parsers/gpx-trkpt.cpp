#include "gpx-trkpt.hpp"
#include "formulae.hpp"
#include "convert.hpp"
#include <pugixml.hpp>

namespace fs = std::filesystem;
namespace darauble::parsers {

using namespace darauble;

std::vector<TrackPoint> ReadTrackpoints(const fs::path& filename, bool calculate_distance) {
    std::vector<TrackPoint> points;
    pugi::xml_document doc;

    if (!doc.load_file(filename.string().c_str())) {
        std::cerr << "Failed to load GPX\n";
        return points;
    }

    for (auto trkpt : doc.select_nodes("//trkpt")) {
        auto node = trkpt.node();
        TrackPoint point;

        point.lat = node.attribute("lat").as_double();
        point.lon = node.attribute("lon").as_double();
        point.distance = -1.0;
        point.cumulative_distance = 0;

        if (calculate_distance && (!points.empty())) {
            TrackPoint& previousPoint = points.at(points.size() - 1);

            point.distance = coordinates::haversine_degrees(
                previousPoint.lat, previousPoint.lon,
                point.lat, point.lon
            );

            point.cumulative_distance = previousPoint.cumulative_distance + point.distance;
        }

        points.push_back(point);

        // std::cout << "Track point: lat " << point.lat << ", lon " << point.lon << ", distance " << point.distance << ", cumulative " << point.cumulative_distance << std::endl;
    }

    // std::cout << "Total track points: " << points.size() << std::endl;

    return points;
}

} // namespace darauble
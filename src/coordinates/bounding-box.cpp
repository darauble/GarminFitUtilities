#include <algorithm>
#include "bounding-box.hpp"
#include "convert.hpp"

namespace darauble {

BoundingBox::BoundingBox(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2) :
    top_lat {std::max(lat1, lat2)},
    left_lon {std::min(lon1, lon2)},
    bottom_lat {std::min(lat1, lat2)},
    right_lon {std::max(lon1, lon2)}
{}

BoundingBox::BoundingBox(double lat1, double lon1, double lat2, double lon2) :
    top_lat {fromDouble(std::max(lat1, lat2))},
    left_lon {fromDouble(std::min(lon1, lon2))},
    bottom_lat {fromDouble(std::min(lat1, lat2))},
    right_lon {fromDouble(std::max(lon1, lon2))}
{}

bool BoundingBox::intersect(const BoundingBox &other) {
    return ! (
        bottom_lat >= other.top_lat || top_lat <= other.bottom_lat
        || left_lon >= other.right_lon || right_lon <= other.left_lon
    );
}

std::ostream& operator<< (std::ostream& out, const BoundingBox &b) {
    out << "{ " << b.top_lat << ", " << b.left_lon << ", "  << b.bottom_lat << ", " << b.right_lon << " }";
    out << "{ " << fromInt32(b.top_lat) << ", " << fromInt32(b.left_lon) << ", "  << fromInt32(b.bottom_lat) << ", " << fromInt32(b.right_lon) << " }";
    return out;
}

} // namespace darauble

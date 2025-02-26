#pragma once
#include <iostream>
#include <cstdint>

namespace darauble {

class BoundingBox {
    friend std::ostream& operator<< (std::ostream& out, const BoundingBox &b);
private:
    int32_t top_lat;
    int32_t left_lon;
    int32_t bottom_lat;
    int32_t right_lon;

public:
    BoundingBox(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2);
    
    BoundingBox(double lat1, double lon1, double lat2, double lon2);

    bool intersect(const BoundingBox &other);
};

} // namespace darauble

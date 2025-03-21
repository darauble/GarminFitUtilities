#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "binary-scanner.hpp"
#include "fit_profile.hpp"

namespace darauble {

class CoordinatesScanner : public BinaryScanner {
private:
    FIT_SPORT sport;
    std::vector<int32_t>& longitudes;
    std::vector<int32_t>& latitudes;

public:
    static const uint16_t SPORT {0};
    static const uint16_t POSITION_LAT {0};
    static const uint16_t POSITION_LON {1};

    CoordinatesScanner(BinaryMapper& _mapper, FIT_SPORT _sport, std::vector<int32_t>& _latitudes, std::vector<int32_t>& _longitudes);

    virtual void reset() override;
    virtual void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;
};

} // namespace darauble

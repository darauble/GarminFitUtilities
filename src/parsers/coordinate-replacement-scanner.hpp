#pragma once

#include <cstdint>
#include <vector>

#include "binary-scanner.hpp"

namespace darauble {

struct RecordOffset {
    uint64_t lat;
    uint64_t lon;
    uint64_t distance;
    uint8_t architecture;
};

struct SessionOffset {
    uint64_t startLat;
    uint64_t startLon;
    uint64_t endLat;
    uint64_t endLon;
    uint8_t architecture;
};

class CoordinateReplacementScanner : public BinaryScanner {
private:
    std::vector<RecordOffset> offsets;
    SessionOffset sessionOffset;

public:
    CoordinateReplacementScanner(BinaryMapper& _mapper);

    virtual void reset() override {
        offsets.clear();
    };

    virtual void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;
    // virtual void end() override;

    std::vector<RecordOffset> getOffsets() const {
        return offsets;
    }

    SessionOffset getSessionOffset() const {
        return sessionOffset;
    }
};

} // namespace darauble

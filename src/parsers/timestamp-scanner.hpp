#pragma once

#include "binary-scanner.hpp"

#include <fit_profile.hpp>
#include <vector>

namespace darauble {

struct timestampId {
    const FitDefinitionMessage& definition;
    uint64_t offset;
};

class TimestampScanner : public BinaryScanner {
protected:
    std::vector<timestampId> timestampIdOffsets;
public:
    TimestampScanner(BinaryMapper& _mapper) :
        BinaryScanner(_mapper)
    {}

    virtual void reset() override;
    virtual void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;

    const std::vector<timestampId>& timestampIds() const {
        return timestampIdOffsets;
    }
};

} // namespace darauble
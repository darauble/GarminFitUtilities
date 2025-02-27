#pragma once

#include "binary-scanner.hpp"

#include <fit_profile.hpp>
#include <vector>

namespace darauble {

struct productId {
    uint8_t architecture;
    uint64_t offset;
};

class ProductScanner : public BinaryScanner {
protected:
    uint16_t searchProductId;
    std::vector<productId> productIdOffsets;
public:
    ProductScanner(BinaryMapper& _mapper) :
        BinaryScanner(_mapper), searchProductId {FIT_UINT16_INVALID}
    {}

    ProductScanner(BinaryMapper& _mapper, uint16_t _searchProductId) :
        BinaryScanner(_mapper), searchProductId {_searchProductId}
    {}

    virtual void reset() override;
    virtual void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;

    void setSearchProductId(uint16_t _searchProductId) {
        searchProductId = _searchProductId;
    }

    const std::vector<productId>& productIds() const {
        return productIdOffsets;
    }
};

} // namespace darauble
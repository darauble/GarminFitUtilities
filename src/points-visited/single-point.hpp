#pragma once

#include <cstdint>
#include <string>

#include <fit_profile.hpp>

#include "bounding-box.hpp"
#include "directory-scanner.hpp"
#include "single-summary.hpp"

namespace darauble {

class SinglePointHandler : public IFileHandler {
private:
    FIT_SPORT sport;
    BoundingBox box;
    SingleSummary& summary;

    void parse(const fs::path& filepath, std::vector<int32_t>& la, std::vector<int32_t>& lo);
    uint32_t search(std::vector<int32_t>& la, std::vector<int32_t>& lo);
public:
    SinglePointHandler(SingleSummary& _summary, std::string _sport, BoundingBox _box);
    void handle(const fs::path& filename) override;
};

} // namespace darauble

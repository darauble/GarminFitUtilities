#pragma once

#include <cstdint>
#include <iostream>

namespace darauble {

class SingleSummary {
private:
    uint64_t totalFiles {0};
    uint64_t parsedFiles {0};
    uint64_t filteredFiles {0};
    uint64_t totalVisits {0};

public:
    void resetCounters() {
        totalFiles = 0;
        parsedFiles = 0;
        filteredFiles = 0;
        totalVisits = 0;
    }

    void incrementTotalFiles() {
        totalFiles++;
    }

    void incrementParsedFiles() {
        parsedFiles++;
    }

    void incrementFilteredFiles() {
        filteredFiles++;
    }

    void incrementTotalVisits() {
        totalVisits++;
    }

    uint64_t getTotalFiles() const {
        return totalFiles;
    }

    uint64_t getParsedFiles() const {
        return parsedFiles;
    }

    uint64_t getFilteredFiles() const {
        return filteredFiles;
    }

    uint64_t getTotalVisits() const {
        return totalVisits;
    }
};

std::ostream& operator << (std::ostream& os, const SingleSummary& s);

} // namespace darauble

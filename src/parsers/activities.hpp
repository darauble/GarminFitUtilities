#pragma once

#include <string>
#include <vector>

#include "binary-scanner.hpp"
#include "directory-scanner.hpp"
#include "table.hpp"

namespace darauble {

class ActivityScanner : public BinaryScanner {
private:
    std::string fileName;
protected:
    std::unordered_map<std::string, std::string> activityData;
public:
    static const std::string HEAD_FILE_NAME;
    static const std::string HEAD_ACTIVITY_NAME;
    static const std::string HEAD_SPORT;
    
    static const uint8_t FIELD_NAME;
    static const uint8_t FIELD_SPORT;
    static const uint8_t FIELD_SUB_SPORT;

    ActivityScanner(std::string _fileName, BinaryMapper& _mapper) :
        fileName {_fileName},
        BinaryScanner {_mapper}
    {}

    void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;

    std::unordered_map<std::string, std::string> getData() {
        return activityData;
    }
};

class ActivityHandler : public IFileHandler {
private:
    containers::Table &table;
public:
    ActivityHandler(containers::Table &_table) :
        table {_table}
    {};

    void handle(const fs::path& filename) override;
};

} // namespace darauble

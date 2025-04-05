#include <iostream>
#include "activities.hpp"
#include "sports.hpp"

#include <fit_profile.hpp>

namespace darauble {

const std::string ActivityScanner::HEAD_FILE_NAME {"File Name"};
const std::string ActivityScanner::HEAD_ACTIVITY_NAME {"Activity Name"};
const std::string ActivityScanner::HEAD_SPORT {"Sport"};

const uint8_t ActivityScanner::FIELD_NAME {3};
const uint8_t ActivityScanner::FIELD_SPORT {0};
const uint8_t ActivityScanner::FIELD_SUB_SPORT {1};

void ActivityScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    if (d.globalMessageNumber == FIT_MESG_NUM_SPORT) {
        // std::cout << "ActivityScanner::record() - FIT_MESG_NUM_SPORT" << std::endl;
        activityData[HEAD_FILE_NAME] = fileName;
        activityData[HEAD_ACTIVITY_NAME] = "?";
        activityData[HEAD_SPORT] = "?";

        uint8_t sport = 0;
        uint8_t subSport = 0;

        for (auto fieldDef : d.fields) {
            if (fieldDef.fieldNumber == FIELD_NAME) {
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                std::string activityName = mapper.readString(fieldOffset, fieldDef.size);
                activityData[HEAD_ACTIVITY_NAME] = activityName;
            } else if (fieldDef.fieldNumber == FIELD_SPORT) {
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                sport = mapper.read(fieldOffset);
            } else if (fieldDef.fieldNumber == FIELD_SUB_SPORT) {
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                subSport = mapper.read(fieldOffset);
            }
        }

        if (subSport > 0) {
            if (metadata::Sports::subNames.containsKey(subSport)) {
                activityData[HEAD_SPORT] = metadata::Sports::subNames.atKey(subSport);
            }
            
        } else {
            if (metadata::Sports::names.containsKey(sport)) {
                activityData[HEAD_SPORT] = metadata::Sports::names.atKey(sport);
            }
        }

        stop();
    }
}

void ActivityHandler::handle(const fs::path& filename) {
    BinaryMapper mapper {filename};
    ActivityScanner scanner {filename.filename().string(), mapper};

    scanner.scan();

    if (scanner.getData().empty()) {
        return;
    }

    table.addRow(scanner.getData());
}

} // namespace darauble

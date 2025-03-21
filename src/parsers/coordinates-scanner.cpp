#include "coordinates-scanner.hpp"
#include "exceptions.hpp"

#include <format>

namespace darauble {

CoordinatesScanner::CoordinatesScanner(BinaryMapper& _mapper, FIT_SPORT _sport, std::vector<int32_t>& _latitudes, std::vector<int32_t>& _longitudes) :
    BinaryScanner(_mapper),
    sport {_sport}, latitudes {_latitudes}, longitudes {_longitudes}
{

}

void CoordinatesScanner::reset() {
    latitudes.clear();
    longitudes.clear();
}

void CoordinatesScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    if (sport != FIT_SPORT_ALL && d.globalMessageNumber == FIT_MESG_NUM_SPORT) {
        for (auto fieldDef : d.fields) {
            if (fieldDef.fieldNumber == SPORT) {
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                uint8_t messageSport = mapper.read(fieldOffset);
                
                if (messageSport != sport) {
                    throw WrongSportException(std::format("Sport {} is filtered out.", messageSport));
                }
            }
        }
    }

    if (d.globalMessageNumber == FIT_MESG_NUM_RECORD) {
        for (auto fieldDef : d.fields) {
            if (!fieldDef.developer) {
                uint64_t fieldOffset {0};
                if ((fieldDef.fieldNumber == POSITION_LAT) || (fieldDef.fieldNumber == POSITION_LON)) {
                    fieldOffset = m.offset + fieldDef.offset;

                    if (fieldDef.fieldNumber == POSITION_LAT) {
                        latitudes.push_back(mapper.readS32(fieldOffset, d.architecture));
                    } else {
                        longitudes.push_back(mapper.readS32(fieldOffset, d.architecture));
                    }
                }
            }
        }
    }
}

} // namespace darauble

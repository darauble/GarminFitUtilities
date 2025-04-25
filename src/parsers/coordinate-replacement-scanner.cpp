#include "coordinate-replacement-scanner.hpp"

#include <fit_record_mesg.hpp>
#include <fit_session_mesg.hpp>

namespace darauble {

CoordinateReplacementScanner::CoordinateReplacementScanner(BinaryMapper& _mapper) :
    BinaryScanner(_mapper)
{}

void CoordinateReplacementScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    if (d.globalMessageNumber == FIT_MESG_NUM_RECORD) {
        RecordOffset offset;
        bool foundCoordinates = false;

        offset.architecture = d.architecture;

        for (const auto& field : d.fields) {
            if (!field.developer) {
                if (field.fieldNumber == fit::RecordMesg::FieldDefNum::PositionLat) {   
                    offset.lat = m.offset + field.offset;
                    foundCoordinates = true;
                } else if (field.fieldNumber == fit::RecordMesg::FieldDefNum::PositionLong) {
                    offset.lon = m.offset + field.offset;
                    foundCoordinates = true;
                } else if (field.fieldNumber == fit::RecordMesg::FieldDefNum::Distance) {
                    offset.distance = m.offset + field.offset;
                }
            }
        }

        if (foundCoordinates) {
            offsets.push_back(offset);
        }
        
    } else if (d.globalMessageNumber == FIT_MESG_NUM_SESSION) {
        sessionOffset.architecture = d.architecture;
        
        for (const auto& field : d.fields) {
            if (!field.developer) {
                if (field.fieldNumber == fit::SessionMesg::FieldDefNum::StartPositionLat) {   
                    sessionOffset.startLat = m.offset + field.offset;
                } else if (field.fieldNumber == fit::SessionMesg::FieldDefNum::StartPositionLong) {
                    sessionOffset.startLon = m.offset + field.offset;
                } else if (field.fieldNumber == fit::SessionMesg::FieldDefNum::EndPositionLat) {
                    sessionOffset.endLat = m.offset + field.offset;
                } else if (field.fieldNumber == fit::SessionMesg::FieldDefNum::EndPositionLong) {
                    sessionOffset.endLon = m.offset + field.offset;
                }
            }
        }
    }
}

} // namespace darauble

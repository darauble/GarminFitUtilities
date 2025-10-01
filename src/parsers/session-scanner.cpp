#include "session-scanner.hpp"

#include <fit_profile.hpp>
#include <fit.hpp>
#include <iostream>

namespace darauble {

void SessionScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    if (d.globalMessageNumber == FIT_MESG_NUM_SPORT) {
        // Handle sport message for primary sport determination
        uint8_t sport = 0;
        uint8_t subSport = 0;
        
        for (const auto& fieldDef : d.fields) {
            if (fieldDef.fieldNumber == 0) {  // FIELD_SPORT
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                sport = mapper.read(fieldOffset);
            } else if (fieldDef.fieldNumber == 1) {  // FIELD_SUB_SPORT
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                subSport = mapper.read(fieldOffset);
            } else if (fieldDef.fieldNumber == 3) {  // FIELD_NAME
                uint64_t fieldOffset = m.offset + fieldDef.offset;
                std::string activityName = mapper.readString(fieldOffset, fieldDef.size);
                activityData.activityName = activityName;
            }
        }
        
        activityData.primarySport = sport;
        activityData.primarySubSport = subSport;
        
    } else if (d.globalMessageNumber == FIT_MESG_NUM_SESSION) {
        // Handle session message for activity data
        SessionData session;
        
        for (const auto& fieldDef : d.fields) {
            uint64_t fieldOffset = m.offset + fieldDef.offset;
            
            switch (fieldDef.fieldNumber) {
                case fit::SessionMesg::FieldDefNum::Timestamp:
                    session.timestamp = mapper.readU32(fieldOffset, d.architecture);
                    break;
                    
                case fit::SessionMesg::FieldDefNum::Sport:
                    session.sport = mapper.read(fieldOffset);
                    break;
                    
                case fit::SessionMesg::FieldDefNum::SubSport:
                    session.subSport = mapper.read(fieldOffset);
                    break;
                    
                case fit::SessionMesg::FieldDefNum::TotalElapsedTime:
                    if (fieldDef.size == 4) {
                        uint32_t timeMs = mapper.readU32(fieldOffset, d.architecture);
                        session.totalElapsedTime = timeMs / 1000.0;  // Convert ms to seconds
                    }
                    break;
                    
                case fit::SessionMesg::FieldDefNum::TotalTimerTime:
                    if (fieldDef.size == 4) {
                        uint32_t timeMs = mapper.readU32(fieldOffset, d.architecture);
                        session.totalTimerTime = timeMs / 1000.0;  // Convert ms to seconds
                    }
                    break;
                    
                case fit::SessionMesg::FieldDefNum::TotalDistance:
                    if (fieldDef.size == 4) {
                        uint32_t distanceCm = mapper.readU32(fieldOffset, d.architecture);
                        if (distanceCm != FIT_UINT32_INVALID && distanceCm != 0) {  // Check for invalid values
                            session.totalDistance = distanceCm / 100.0;  // Convert cm to meters
                        }
                    }
                    break;
                    
                case fit::SessionMesg::FieldDefNum::TotalCalories:
                    if (fieldDef.size == 2) {
                        session.totalCalories = mapper.readU16(fieldOffset, d.architecture);
                    }
                    break;
                    
                case fit::SessionMesg::FieldDefNum::AvgSpeed:
                    if (fieldDef.size == 2) {
                        uint16_t speed = mapper.readU16(fieldOffset, d.architecture);
                        if (speed != FIT_UINT16_INVALID) {  // Accept even 0 speed (valid for some activities)
                            session.avgSpeed = speed;
                        }
                    }
                    break;
                    
                case fit::SessionMesg::FieldDefNum::AvgHeartRate:
                    {
                        uint8_t hr = mapper.read(fieldOffset);
                        if (hr != FIT_UINT8_INVALID && hr != 0) {  // Check for invalid values
                            session.avgHeartRate = hr;
                        }
                    }
                    break;
                    
                case fit::SessionMesg::FieldDefNum::MaxHeartRate:
                    {
                        uint8_t hr = mapper.read(fieldOffset);
                        if (hr != FIT_UINT8_INVALID && hr != 0) {  // Check for invalid values
                            session.maxHeartRate = hr;
                        }
                    }
                    break;
                    
                case 151:  // Total sets field (for strength training)
                    if (fieldDef.size == 2) {
                        uint16_t sets = mapper.readU16(fieldOffset, d.architecture);
                        if (sets != FIT_UINT16_INVALID && sets != 0) {
                            session.totalSets = sets;
                        }
                    }
                    break;
            }
        }
        
        
        // Only add sessions that have meaningful data
        if (session.sport > 0 || session.totalElapsedTime > 0) {
            activityData.sessions.push_back(session);
        }
    }
}

} // namespace darauble
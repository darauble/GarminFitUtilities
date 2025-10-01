#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "binary-scanner.hpp"
#include "sports.hpp"

#include <fit_profile.hpp>
#include <fit_session_mesg.hpp>
#include <fit.hpp>
#include <iostream>
#include <cstdio>

namespace darauble {

struct SessionData {
    uint32_t timestamp = 0;
    uint8_t sport = 0;
    uint8_t subSport = 0;
    double totalElapsedTime = 0.0;    // seconds
    double totalTimerTime = 0.0;      // seconds  
    double totalDistance = 0.0;       // meters
    uint16_t totalCalories = 0;
    uint16_t avgSpeed = 0;            // m/s * 1000
    uint16_t avgHeartRate = 0;        // bpm
    uint16_t maxHeartRate = 0;        // bpm
    uint16_t totalSets = 0;           // total sets (for strength training)
    
    std::string getSportName() const {
        if (subSport > 0 && metadata::Sports::subNames.containsKey(subSport)) {
            return metadata::Sports::subNames.atKey(subSport);
        } else if (metadata::Sports::names.containsKey(sport)) {
            return metadata::Sports::names.atKey(sport);
        }
        return "Unknown";
    }
    
    std::string getFormattedDuration() const {
        if (totalElapsedTime <= 0) return "--:--";
        int totalSeconds = static_cast<int>(totalElapsedTime);
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        
        if (hours > 0) {
            return std::to_string(hours) + ":" + 
                   (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
                   (seconds < 10 ? "0" : "") + std::to_string(seconds);
        } else {
            return std::to_string(minutes) + ":" + 
                   (seconds < 10 ? "0" : "") + std::to_string(seconds);
        }
    }
    
    std::string getFormattedDistance() const {
        return getFormattedDistance(sport, subSport);
    }
    
    std::string getFormattedDistance(uint8_t sportId, uint8_t subSportId) const {
        // Strength training-specific formatting (show sets as "Work")
        if (sportId == FIT_SPORT_TRAINING && subSportId == FIT_SUB_SPORT_STRENGTH_TRAINING) {
            if (totalSets > 0) {
                return std::to_string(totalSets) + " sets";
            } else {
                return "-- sets";
            }
        }
        
        // Swimming-specific formatting
        if (sportId == FIT_SPORT_SWIMMING) {
            // Check for invalid values using FIT SDK constants
            if (totalDistance <= 0 || totalDistance >= FIT_UINT32_INVALID) return "-- km";
            
            if (totalDistance < 2000.0) {
                // Show in meters for shorter distances
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.0f", totalDistance);
                return std::string(buffer) + " m";
            }
        }
        
        // Default distance-based formatting
        if (totalDistance <= 0 || totalDistance >= FIT_UINT32_INVALID) return "-- km";
        
        double km = totalDistance / 1000.0;
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f", km);
        return std::string(buffer) + " km";
    }
    
    std::string getFormattedSpeed() const {
        return getFormattedSpeed(sport, subSport);
    }
    
    std::string getFormattedSpeed(uint8_t sportId, uint8_t subSportId) const {
        double speedKmh = 0.0;
        
        // Try to get speed from avgSpeed field first
        if (avgSpeed > 0 && avgSpeed < FIT_UINT16_INVALID) {
            double speedMs = avgSpeed / 1000.0;  // Convert from m/s * 1000 to m/s
            speedKmh = speedMs * 3.6;     // Convert to km/h
        }
        // Fallback: calculate speed from distance and time
        else if (totalDistance > 0 && totalDistance < FIT_UINT32_INVALID && 
                 totalElapsedTime > 0) {
            double distanceKm = totalDistance / 1000.0;  // Convert m to km
            double timeHours = totalElapsedTime / 3600.0;  // Convert seconds to hours
            speedKmh = distanceKm / timeHours;
        }
        
        // If still no valid speed
        if (speedKmh <= 0) {
            return "--";
        }
        
        // Strength training-specific formatting (show duration as "Result")
        if (sportId == FIT_SPORT_TRAINING && subSportId == FIT_SUB_SPORT_STRENGTH_TRAINING) {
            if (totalElapsedTime > 0) {
                int totalSeconds = static_cast<int>(totalElapsedTime);
                int minutes = totalSeconds / 60;
                int seconds = totalSeconds % 60;
                return std::to_string(minutes) + ":" + 
                       (seconds < 10 ? "0" : "") + std::to_string(seconds) + " duration";
            } else {
                return "--:-- duration";
            }
        }
        
        // Swimming-specific formatting
        if (sportId == FIT_SPORT_SWIMMING) {
            // Calculate pace per 100m
            double speedMs = speedKmh / 3.6;  // Convert km/h to m/s
            double pace100m = 100.0 / speedMs;  // seconds per 100m
            int minutes = static_cast<int>(pace100m / 60);
            int seconds = static_cast<int>(pace100m) % 60;
            
            return std::to_string(minutes) + ":" + 
                   (seconds < 10 ? "0" : "") + std::to_string(seconds) + " /100m";
        }
        
        // Default formatting for other sports
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f", speedKmh);
        std::string speedStr(buffer);
        
        // Calculate pace in min/km
        double paceMinPerKm = 60.0 / speedKmh;  // minutes per km
        int minutes = static_cast<int>(paceMinPerKm);
        int seconds = static_cast<int>((paceMinPerKm - minutes) * 60);
        
        return speedStr + " km/h, " + std::to_string(minutes) + ":" + 
               (seconds < 10 ? "0" : "") + std::to_string(seconds) + " /km";
    }
    
    std::string getFormattedHeartRate() const {
        if (avgHeartRate <= 0) return "--";
        return std::to_string(avgHeartRate);
    }
};

struct ActivityData {
    std::string fileName;
    std::string activityName;
    uint8_t primarySport = 0;
    uint8_t primarySubSport = 0;
    std::vector<SessionData> sessions;
    
    // Aggregate data from all sessions
    SessionData getAggregatedData() const {
        if (sessions.empty()) return SessionData{};
        
        SessionData aggregated;
        aggregated.sport = primarySport;
        aggregated.subSport = primarySubSport;
        aggregated.timestamp = sessions[0].timestamp;  // Use first session timestamp
        
        // Sum up values from all sessions
        for (const auto& session : sessions) {
            aggregated.totalElapsedTime += session.totalElapsedTime;
            aggregated.totalTimerTime += session.totalTimerTime;
            aggregated.totalDistance += session.totalDistance;
            aggregated.totalCalories += session.totalCalories;
            aggregated.totalSets += session.totalSets;
        }
        
        // Calculate weighted averages for heart rate and speed
        double totalTimeForAvg = 0;
        double weightedHR = 0;
        double weightedSpeed = 0;
        uint16_t maxHR = 0;
        
        for (const auto& session : sessions) {
            if (session.totalElapsedTime > 0) {
                totalTimeForAvg += session.totalElapsedTime;
                weightedHR += session.avgHeartRate * session.totalElapsedTime;
                weightedSpeed += session.avgSpeed * session.totalElapsedTime;
                maxHR = std::max(maxHR, session.maxHeartRate);
            }
        }
        
        if (totalTimeForAvg > 0) {
            aggregated.avgHeartRate = static_cast<uint16_t>(weightedHR / totalTimeForAvg);
            aggregated.avgSpeed = static_cast<uint16_t>(weightedSpeed / totalTimeForAvg);
        }
        aggregated.maxHeartRate = maxHR;
        
        
        return aggregated;
    }
    
    std::string getPrimarySportName() const {
        if (primarySubSport > 0 && metadata::Sports::subNames.containsKey(primarySubSport)) {
            return metadata::Sports::subNames.atKey(primarySubSport);
        } else if (metadata::Sports::names.containsKey(primarySport)) {
            return metadata::Sports::names.atKey(primarySport);
        }
        return "Unknown";
    }
    
    bool isMultisport() const {
        return primarySport == FIT_SPORT_MULTISPORT;
    }
};

class SessionScanner : public BinaryScanner {
private:
    std::string fileName;
    ActivityData activityData;
    
public:
    SessionScanner(std::string _fileName, BinaryMapper& _mapper) :
        fileName(_fileName),
        BinaryScanner(_mapper) {
        activityData.fileName = fileName;
    }
    
    virtual ~SessionScanner() = default;
    
    void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;
    
    const ActivityData& getData() const {
        return activityData;
    }
    
    bool hasData() const {
        return !activityData.sessions.empty() || activityData.primarySport > 0;
    }
};

} // namespace darauble
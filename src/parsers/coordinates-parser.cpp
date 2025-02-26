#include "coordinates-parser.hpp"

#include <format>

#include <fit_profile.hpp>

namespace darauble {

SportCoordinatesListener::SportCoordinatesListener(FIT_SPORT _sport, std::vector<int32_t>& _latitudes, std::vector<int32_t>& _longitudes) :
    sport {_sport}, latitudes {_latitudes}, longitudes {_longitudes}
{

}

void SportCoordinatesListener::OnMesg(fit::SportMesg& mesg) {
    if (sport != FIT_SPORT_ALL) {
        if (mesg.GetSport() != sport) {
            throw WrongSportException(std::format("Sport {} is filtered out.", mesg.GetSport()));
        }
    }
    // std::cout << "Sport: " << (int)mesg.GetSport() << ", " << (int)mesg.GetSubSport() << std::endl;
    // throw fit::RuntimeException("TEST CUT :-)");
}

// void OnMesg(fit::FileIdMesg& mesg) override {
//     std::cout << "File ID timestamp: " << mesg.GetTimeCreated() << std::endl;
// }

void SportCoordinatesListener::OnMesg(fit::RecordMesg& mesg) {
    latitudes.push_back(mesg.GetPositionLat());
    longitudes.push_back(mesg.GetPositionLong());
}

} // namespace darauble

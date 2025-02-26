#pragma once

#include <string>
#include <vector>

#include <fit_sport_mesg_listener.hpp>
#include <fit_record_mesg_listener.hpp>

#include "exceptions.hpp"
#include "directory-scanner.hpp"

namespace darauble {

class SportCoordinatesListener :
    public fit::SportMesgListener,
    public fit::RecordMesgListener
{
private:
    FIT_SPORT sport;
    std::vector<int32_t>& longitudes;
    std::vector<int32_t>& latitudes;

public:
    SportCoordinatesListener(FIT_SPORT _sport, std::vector<int32_t>& _latitudes, std::vector<int32_t>& _longitudes);
    void OnMesg(fit::SportMesg& mesg) override;
    void OnMesg(fit::RecordMesg& mesg) override;
};

} // namespace darauble

#pragma once
#include <fit_date_time.hpp>
#include <fit_decode.hpp>
#include <fit_mesg_broadcaster.hpp>

#include "directory-scanner.hpp"

namespace darauble {

class Listener :
    public fit::FileIdMesgListener
{
private:
    std::time_t &fileCreated;
public:
    Listener(std::time_t &_fileCreated) :
        fileCreated {_fileCreated}
    {

    }
    
    void OnMesg(fit::FileIdMesg& mesg) override;
};

class FitFileHandler: public IFileHandler {
private:
    fit::Decode decode;
    fit::MesgBroadcaster broadcaster;
    std::time_t fileCreated {0};
    Listener listener {fileCreated};
public:
    FitFileHandler();
    void handle(const fs::path& filename);
};


} // namespace darauble

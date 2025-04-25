#pragma once

#include "IEditCommand.hpp"

namespace darauble {

class GpxCommand : public IEditCommand {

public:
    GpxCommand() :
        IEditCommand("gpx")
    {}

    virtual void show(int argc, char* argv[]) override;
    virtual void replace(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble

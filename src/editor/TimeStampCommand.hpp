#pragma once
#include "IEditCommand.hpp"

namespace darauble {

class TimeStampCommand: public IEditCommand {
public:
    TimeStampCommand() : 
        IEditCommand("timestamp")
    {}

    virtual void show(int argc, char* argv[]) override;
    virtual void set(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble
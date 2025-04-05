#pragma once

#include "IEditCommand.hpp"

namespace darauble {

class ActivitiesCommand : public IEditCommand {

public:
    ActivitiesCommand() :
        IEditCommand("activities")
    {}

    virtual void show(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble

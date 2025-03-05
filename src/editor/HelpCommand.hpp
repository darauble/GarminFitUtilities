#pragma once
#include "IEditCommand.hpp"
#include "CommandMap.hpp"

#include <optional>

namespace darauble {

class HelpCommand final: public IEditCommand {
private:
    std::optional<std::reference_wrapper<CliActionMap>> actionMap;

public:
    HelpCommand() :
        IEditCommand("help")
    {}

    void setActionMap(CliActionMap & map) {
        actionMap = map;
    }

    virtual void show(int argc, char* argv[]) override;
    // virtual void set(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble

#pragma once
#include "IEditCommand.hpp"

namespace darauble {

class MessageCommand : public IEditCommand {
public:
    MessageCommand() : IEditCommand("message") {}

    virtual void show(int argc, char* argv[]) override;
    // virtual void set(int argc, char* argv[]) override;
    // virtual void replace(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble

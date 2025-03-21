#pragma once
#include "IEditCommand.hpp"

namespace darauble {

class RawCommand : public IEditCommand {
public:
    RawCommand() : 
      IEditCommand("raw")
    {}

    virtual void show(int argc, char* argv[]) override;
    virtual void set(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble

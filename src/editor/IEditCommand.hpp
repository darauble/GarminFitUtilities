#pragma once
#include <string>
#include <iostream>

namespace darauble {

class IEditCommand {
protected:
    std::string commandName;
public:
    IEditCommand(std::string _name) : commandName {_name} {}

    virtual const std::string & name() final { return commandName; }

    virtual void show(int argc, char* argv[]) { std::cout << "Show not implemented for [" << commandName << "]" << std::endl; };
    virtual void set(int argc, char* argv[]) { std::cout << "Set not implemented for [" << commandName << "]" << std::endl; };
    virtual void replace(int argc, char* argv[]) { std::cout << "Replace not implemented for [" << commandName << "]" << std::endl; };
    virtual void help(int argc, char* argv[]) { std::cout << "Help not implemented for [" << commandName << "]" << std::endl; };
    virtual const std::string description() = 0;
};

} // namespace darauble
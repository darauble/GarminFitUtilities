#pragma once
#include "IEditCommand.hpp"

#include <optional>

namespace darauble {

class ProductCommand final: public IEditCommand {
private:

public:
    ProductCommand() :
        IEditCommand("product")
    {}

    virtual void show(int argc, char* argv[]) override;
    virtual void replace(int argc, char* argv[]) override;
    virtual void help(int argc, char* argv[]) override;
    virtual const std::string description() override;
};

} // namespace darauble

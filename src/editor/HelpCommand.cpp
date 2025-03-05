#include "HelpCommand.hpp"

namespace darauble {

void HelpCommand::show(int argc, char* argv[]) {
    std::cout << "Usage: " << argv[0] << " show|set|replace <command> help|[options]" << std::endl;
    
    if (actionMap) {
        std::cout << "Actions and commands:" << std::endl;

        for (const auto& [key, value] : actionMap->get()) {
            

            std::cout << "  " << key << std::endl;
    
            for (const auto& [subkey, subvalue] : value) {
                std::cout << "    " << subkey << " - " << subvalue.description() << std::endl;
            }
        }
    }
}

void HelpCommand::help(int argc, char* argv[]) {
    std::cout << "Usage: " << std::endl
        << "  " << argv[0] << " show help" << std::endl;

}

const std::string HelpCommand::description() {
    return "show help for commands";
}

} // namespace darauble

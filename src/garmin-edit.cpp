#include <iostream>
#include <map>
#include <string>

#include "CommandMap.hpp"
#include "HelpCommand.hpp"
#include "TimeStampCommand.hpp"

using namespace darauble;

int main(int argc, char* argv[]) {
    HelpCommand helpCommand;
    TimeStampCommand timeStampCommand;
    
    CliActionMap actionMap = {
        {
            "show", {
                {"help", helpCommand},
                {"timestamp", timeStampCommand},
            }
        },
        {
            "set", {
                {"timestamp", timeStampCommand},
            }
        },
        // {
        //     "replace", {
        //         {"help", helpCommand},
        //         {"timestamp", timeStampCommand},
        //     }
        // }
    };

    helpCommand.setActionMap(actionMap);

    if (argc > 2) {
        const auto it = actionMap.find(argv[1]);

        if (it != actionMap.end()) {
            const CliCommandMap& subMap = it->second;
            const auto& it_command = subMap.find(argv[2]);

            if (it_command != subMap.end()) {
                if (it->first == "show") {
                    it_command->second.show(argc, argv);
                } else if (it->first == "set") {
                    it_command->second.set(argc, argv);
                } else if (it->first == "replace") {
                    it_command->second.replace(argc, argv);
                } else {
                    std::cout << "Error: unknown action " << argv[1] << std::endl;
                }
            } else {
                std::cout << "Error: unknown command " << argv[2] << std::endl;
            }
        } else {
            std::cout << "Error: unknown action " << argv[1] << std::endl;
        }
    } else {
        helpCommand.help(argc, argv);
    }
}

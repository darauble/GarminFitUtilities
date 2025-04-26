#include <iostream>
#include <map>
#include <string>

#include "ActivitiesCommand.hpp"
#include "CommandMap.hpp"
#include "GpxCommand.hpp"
#include "HelpCommand.hpp"
#include "MessageCommand.hpp"
#include "ProductCommand.hpp"
#include "RawCommand.hpp"
#include "TimeStampCommand.hpp"

constexpr auto VERSION = "1.2.0";

using namespace darauble;

int main(int argc, char* argv[]) {
    std::cout << "Garmin FIT file editor/analyzer version " << VERSION << std::endl;

    ActivitiesCommand activitiesCommand;
    GpxCommand gpxCommand;
    HelpCommand helpCommand;
    MessageCommand messageCommand;
    ProductCommand productCommand;
    RawCommand rawCommand;
    TimeStampCommand timeStampCommand;
    
    CliActionMap actionMap = {
        {
            "show", {
                {"help", helpCommand},
                {"activities", activitiesCommand},
                {"gpx", gpxCommand},
                {"message", messageCommand},
                {"product", productCommand},
                {"raw", rawCommand},
                {"timestamp", timeStampCommand},
            }
        },
        {
            "set", {
                {"raw", rawCommand},
                {"timestamp", timeStampCommand},
            }
        },
        {
            "replace", {
                {"gpx", gpxCommand},
                {"product", productCommand},
            }
        }
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

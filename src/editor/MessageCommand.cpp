#include "MessageCommand.hpp"
#include "binary-mapper.hpp"
#include "print-scanner.hpp"

#include <fit_profile.hpp>

#include <sstream>
#include <string>
#include <unordered_set>

namespace darauble {

void MessageCommand::show(int argc, char* argv[]) {
    if (argc < 4) {
        help(argc, argv);
        return;
    }

    int argsStart {3};
    PrintScannerOptions options;
    PrintScanner::defaultOptions(options);

    if (strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if (argc >= 5) {
        std::istringstream stream(argv[argsStart]);
        std::string token;
        bool foundOptions {false};

        while (std::getline(stream, token, '|')) {
            if (token == "offset") {
                foundOptions = true;
                options.offset = true;
            } else if (token == "raw") {
                foundOptions = true;
                options.raw = true;
            }
        }

        if (foundOptions) {
            argsStart = 4;
        }       
    }

    std::unordered_set<uint16_t> messageFilter;
    std::unordered_set<uint16_t> fieldFilter;

    if (argc > argsStart + 1) {
        std::istringstream stream(argv[argsStart++]);
        std::string token;
        

        while (std::getline(stream, token, '|')) {
            if (token == "*") {
                break;
            }

            uint16_t messageNumber {FIT_UINT16_INVALID};
            
            try {
                messageNumber = std::stoi(token);
            } catch (...) {
                // Do nothing
            }

            if (messageNumber != FIT_UINT16_INVALID) {
                messageFilter.insert(messageNumber);
            } else {
                for (auto &m : fit::Profile::mesgs) {
                    if (m.name == token) {
                        messageFilter.insert(m.num);
                        break;
                    }
                }
            }
        }
    }

    if (argc > argsStart + 1) {
        std::istringstream stream(argv[argsStart++]);
        std::string token;
        
        if (messageFilter.size() != 1) {
            std::cerr << "Field filter can be used only with ONE mesage in the filter!" << std::endl;
            return;
        }

        while (std::getline(stream, token, '|')) {
            if (token == "*") {
                break;
            }
            
            uint16_t fieldNumber {FIT_UINT16_INVALID};

            try {
                fieldNumber = std::stoi(token);
            } catch (...) {
                // Do nothing
            }

            if (fieldNumber != FIT_UINT16_INVALID) {
                fieldFilter.insert(fieldNumber);
            } else {
                for (auto &m : fit::Profile::mesgs) {
                    if (messageFilter.find(m.num) != messageFilter.end()) {
                        for (uint16_t i = 0; i < m.numFields; i++) {
                            if (m.fields[i].name == token) {
                                fieldFilter.insert(m.fields[i].num);
                            }
                        }
                    }
                }
            }
        }
    }

    try {
        BinaryMapper mapper(argv[argsStart]);
        PrintScanner scanner(mapper, messageFilter, fieldFilter, options);
        scanner.scan();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void MessageCommand::help(int argc, char* argv[]) {
    std::cout << "Usage: " << argv[0] << " show message [offset] [message filter] <file name>" << std::endl << std::endl;
    std::cout << "      show all the messages in the file with optionally displaying" << std::endl;
    std::cout << "      every field's offset in the file." << std::endl << std::endl;
    std::cout << "      Message filter can be used to show only desired messages." << std::endl;
    std::cout << "      Message names can be used (as per fit::Profile::mesgs) or their numbers." << std::endl;
    std::cout << "      Several message names/numbers should by separated by |:" << std::endl;
    std::cout << "       \"file_id\" or \"file_id|record\" or \"0|20\"." << std::endl;
}

const std::string MessageCommand::description() {
    return "show messages in a table";
}

} // namespace darauble

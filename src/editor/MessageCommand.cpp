#include "MessageCommand.hpp"
#include "binary-mapper.hpp"
#include "print-scanner.hpp"

namespace darauble {

void MessageCommand::show(int argc, char* argv[]) {
    if (argc < 4) {
        help(argc, argv);
        return;
    }

    int argsStart {3};
    bool showOffset {false};

    if (strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if ((argc >= 5) && (strcmp(argv[3], "offset") == 0)) {
        argsStart = 4;
        showOffset = true;
    }

    try {
        BinaryMapper mapper(argv[argsStart]);
        PrintScanner scanner(mapper, {}, {}, showOffset);
        scanner.scan();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void MessageCommand::help(int argc, char* argv[]) {
    std::cout << "Usage: " << argv[0] << " show message [offset] <file name>" << std::endl;
    std::cout << "      show all the messages in the file with optionally displaying" << std::endl;
    std::cout << "      every field's offset in the file." << std::endl;
}

const std::string MessageCommand::description() {
    return "show messages in a table";
}

} // namespace darauble

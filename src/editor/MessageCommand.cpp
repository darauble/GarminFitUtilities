#include "MessageCommand.hpp"
#include "binary-mapper.hpp"
#include "print-scanner.hpp"

namespace darauble {

void MessageCommand::show(int argc, char* argv[]) {
    if (argc < 4) {
        help(argc, argv);
        return;
    }

    try {
        BinaryMapper mapper(argv[3]);
        PrintScanner scanner(mapper, {}, {});
        scanner.scan();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void MessageCommand::help(int argc, char* argv[]) {
    std::cout << "Usage: " << argv[0] << " show message <file name>" << std::endl;
}

const std::string MessageCommand::description() {
    return "show messages in a table";
}

} // namespace darauble

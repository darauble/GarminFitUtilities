#include "ActivitiesCommand.hpp"

#include "activities.hpp"
#include "directory-scanner.hpp"
#include "table.hpp"

#include <cstring>

namespace darauble {
    void ActivitiesCommand::show(int argc, char* argv[]) {
        if (argc != 4) {
            std::cerr << "Invalid number of arguments." << std::endl << std::endl;
            help(argc, argv);
            return;
        }

        if (strcmp(argv[3], "help") == 0) {
            help(argc, argv);
            return;
        }

        containers::Table table {{ActivityScanner::HEAD_FILE_NAME, ActivityScanner::HEAD_ACTIVITY_NAME, ActivityScanner::HEAD_SPORT}};
        ActivityHandler handler {table};
        DirectoryScanner scanner {handler};

        scanner.scan(argv[3]);

        std::cout << "Found " << table.getData().size() << " activities." << std::endl;

        table.sortByColumn(ActivityScanner::HEAD_FILE_NAME);

        std::cout << table << std::endl;

        // for (const auto& column : table.getHeader()) {
        //     // std::cout << "Column: " << column << std::endl;
        //     std::cout << std::setw(table.getTableWidths().at(column)) << std::setfill(' ') << column << " ";
        // }

        // std::cout << std::endl;

        // for (const auto& row : table.getData()) {
        //     for (const auto& column : table.getHeader()) {
        //         std::cout << std::setw(table.getTableWidths().at(column) + row.at(column).length() - containers::Table::utf8Length(row.at(column))) << std::setfill(' ') << row.at(column) << " ";
        //     }

        //     std::cout << std::endl;
        // }
    }

    void ActivitiesCommand::help(int argc, char* argv[]) {
        std::cout << "Usage: " << argv[0] << " show activities <directory|file>" << std::endl;
        std::cout << "Scan given directory or a single file and show short information about found activities." << std::endl;
    }

    const std::string ActivitiesCommand::description() {
        return "display activities in the given directory or a single file";
    }
} // namespace darauble

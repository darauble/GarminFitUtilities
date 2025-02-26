
#include <algorithm>

#include "directory-scanner.hpp"


namespace darauble {

static std::string to_lowercase(const std::string& str) {
    std::string lower;

    std::transform(str.begin(), str.end(), std::back_inserter(lower), [](unsigned char c) {
        return std::tolower(c);
    });

    return lower;
}

void DirectoryScanner::scan(const fs::path& directory) {
    if (fs::is_regular_file(directory)) {
        std::string ext = to_lowercase(directory.extension().string());

        if (filter.empty() || std::find(filter.begin(), filter.end(), ext) != filter.end()) {
            handler.handle(directory);
        }
    } else if (fs::is_directory(directory)) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string ext = to_lowercase(entry.path().extension().string());
                
                if (filter.empty() || std::find(filter.begin(), filter.end(), ext) != filter.end()) {
                    handler.handle(entry.path());
                }
            }
        }
    } else {
        throw std::runtime_error("Not a file or directory: " + directory.string());
    }
}

} // namespace darauble

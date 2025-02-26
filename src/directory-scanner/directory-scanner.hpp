#pragma once
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
namespace darauble {

class IFileHandler {
public:
    virtual ~IFileHandler() = default;
    virtual void handle(const fs::path& filename) = 0;
};
    
class DirectoryScanner {
private:
    IFileHandler &handler;
    std::vector<std::string> filter;
public:
    DirectoryScanner(IFileHandler &_handler, std::vector<std::string> _filter) : handler {_handler}, filter {_filter} {};
    DirectoryScanner(IFileHandler &_handler) : DirectoryScanner {_handler, {}} {};
    ~DirectoryScanner() = default;
    
    void scan(const fs::path& directory);
};

} // namespace darauble
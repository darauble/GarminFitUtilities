#pragma once

#include <stdexcept>
#include <string>

namespace darauble {

class StopParsingException : public std::runtime_error {
public:
    explicit StopParsingException(const std::string& message)
        : std::runtime_error(message) {}
};

class NotActivityException : public std::runtime_error {
public:
    explicit NotActivityException(const std::string& message)
        : std::runtime_error(message) {}
};

class WrongSportException : public std::runtime_error {
public:
    explicit WrongSportException(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace darauble
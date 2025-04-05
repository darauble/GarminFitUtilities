#pragma once

#include <cstdint>
#include <string>
#include <bimap.hpp>

namespace darauble::metadata {

class Sports {
public:
    static const containers::BiMap<uint8_t, std::string> names;
    static const containers::BiMap<uint8_t, std::string> subNames;
};

}
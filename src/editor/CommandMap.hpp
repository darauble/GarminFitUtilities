#pragma once
#include <map>
#include <string>

#include "IEditCommand.hpp"

namespace darauble {

using CliCommandMap = std::map<std::string, IEditCommand&>;
using CliActionMap = std::map<std::string, CliCommandMap>;

} // namespace darauble

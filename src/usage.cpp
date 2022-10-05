#include "usage.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace  {

const char * kUsage =
#include "usage.txt"
;

} // ns a

void misc::printUsage(const char *appPath)
{
    const std::string kPlaceHolder = "%TOOL_NAME%";

    std::string usage = kUsage;

    const auto p = usage.find(kPlaceHolder);
    if (p != std::string::npos) {
        const auto executableName = std::filesystem::path(appPath).filename();
        usage = usage.replace(p, kPlaceHolder.size(), executableName);
    }

    std::cerr << usage << std::endl << std::endl;
}

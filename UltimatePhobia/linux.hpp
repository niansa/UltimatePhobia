#pragma once

#include <string>

namespace linux {
std::string getEnvVar(std::string_view key);
bool isMangoHudEnabled();
bool isMangoHudAvailable();
std::string getLinuxWineLoader();
void restartWithMangoHud(bool enable);
} // namespace linux
#include "linux.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <windows.h>

namespace linux {
std::string getEnvVar(std::string_view key) {
    std::ifstream file("Z:\\proc\\self\\environ", std::ios::binary);
    if (!file.is_open())
        return "";

    std::string envVar;
    while (std::getline(file, envVar, '\0')) {
        const std::string_view envVarView{envVar};
        if (envVarView.size() > key.size() && envVarView.substr(0, key.size()) == key && envVarView[key.size()] == '=') {
            return envVar.substr(key.size() + 1);
        }
    }

    return "";
}

bool isMangoHudEnabled() {
    char buffer[4];
    const DWORD envLen = GetEnvironmentVariableA("MANGOHUD", buffer, sizeof(buffer));
    return envLen == 1 && buffer[0] == '1';
}

bool isMangoHudAvailable() {
    const HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll || !GetProcAddress(ntdll, "wine_get_version"))
        return false;

    std::string linuxPath = getEnvVar("PATH");
    if (linuxPath.empty())
        linuxPath = "/usr/bin:/usr/local/bin";

    std::stringstream ss(linuxPath);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        if (dir.empty())
            continue;

        const std::string mangoHudPath = "Z:" + dir + "/mangohud";
        const DWORD attrs = GetFileAttributesA(mangoHudPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
            return true;
    }

    return false;
}

std::string getLinuxWineLoader() {
    std::string loader = getEnvVar("WINELOADER");

    if (loader.empty())
        return "wine";

    constexpr std::string_view kWinePreloaderSuffix{"-preloader"};
    const std::string::size_type pos = loader.find(kWinePreloaderSuffix);
    if (pos != std::string::npos)
        loader.erase(pos, kWinePreloaderSuffix.size());

    return loader;
}

void restartWithMangoHud(const bool enable) {
    const HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll || !GetProcAddress(ntdll, "wine_get_version"))
        return;

    if (enable == isMangoHudEnabled())
        return;

    if (enable && !isMangoHudAvailable())
        return;

    const std::string wineBin = getLinuxWineLoader();

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string safePath = exePath;
    for (char& c : safePath) {
        if (c == '\\')
            c = '/';
    }

    std::string args = "Z:\\bin\\sh -c \"";

    if (enable)
        args += "export MANGOHUD=1; export VK_INSTANCE_LAYERS=VK_LAYER_MANGOHUD_overlay; ";
    else
        args += "export MANGOHUD=0; export VK_INSTANCE_LAYERS=; ";

    args += "sleep 0.5; ";
    args += "exec \\\"" + wineBin + "\\\" \\\"" + safePath + "\\\"\"";

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if (CreateProcessA("Z:\\bin\\sh", args.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        ExitProcess(0);
    }
}
} // namespace linux

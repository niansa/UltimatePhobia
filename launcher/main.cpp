#include <iostream>
#include <string_view>
#include <filesystem>
#include <windows.h>
#include <detours.h>


std::filesystem::path getexepath() {
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
}

bool startup(const char *lpApplicationName) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char argv[] = "";

    // Set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the program up
    if (DetourCreateProcessWithDllExA(lpApplicationName, argv, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi, (getexepath().parent_path()/"UltimatePhobia.dll").string().c_str(), CreateProcessA) == 0)
        return false;

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

int main(int argc, char **argv) {
    const char *arg1;

    // Process args
    if (argc <= 1) {
        std::filesystem::current_path("Game");
        arg1 = "Phasmophobia.exe";
    } else {
        arg1 = argv[1];
    }

    // Start game
    std::cout << "Launching game..." << std::endl;
    if (!startup(arg1)) {
        std::cerr << "Error: Failed to launch game" << std::endl;
        return -3;
    }
}

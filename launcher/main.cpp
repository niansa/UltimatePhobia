#include <iostream>
#include <windows.h>

int UnityMain(HINSTANCE hInstance, HINSTANCE hPrevInstanc, LPWSTR lpCmdLine, int nShowCmd);


static std::string lastErrorString() {
    auto errorMessageID = GetLastError();
    if (errorMessageID == 0) {
        return "No error";
    }
    LPSTR messageBuffer = nullptr;
    auto size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}


int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
    // Load engine
    std::cout << "Loading Unity Player..." << std::flush;
    HMODULE unityPlayer = LoadLibraryW(L"UnityPlayer.dll");
    if (unityPlayer == nullptr) {
        std::cout << " Error: " << lastErrorString() << std::endl;
        abort();
    }
    auto unityMain = reinterpret_cast<decltype(UnityMain) *>(GetProcAddress(unityPlayer, "UnityMain"));
    if (unityMain == nullptr) {
        std::cout << " Error: " << lastErrorString() << std::endl;
        abort();
    }
    std::cout << " OK" << std::endl;

    // Load UltimatePhobia
    std::cout << "Loading UltimatePhobia..." << std::flush;
    if (LoadLibraryW(L"..\\UltimatePhobia.dll") == nullptr) {
        std::cout << " Error: " << lastErrorString() << std::endl;
        abort();
    }
    std::cout << " OK" << std::endl;

    // Start game
    std::cout << "Starting game...\n\n" << std::endl;
    unityMain(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

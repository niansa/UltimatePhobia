#include "getbaseaddr.hpp"
#include "global_state.hpp"
#include "detours_helpers.hpp"
#include "misc_utils.hpp"

#include <detours.h>


static decltype(&LoadLibraryW) trueLoadLibraryW;
std::function<void ()> finalCallback;


static HMODULE detourLoadLibraryW(LPWSTR lpLibFileName) {
    const std::string lpLibFileNameA = utf8Encode(lpLibFileName);
    const HMODULE fres = trueLoadLibraryW(lpLibFileName);

    if (lpLibFileNameA == "GameAssembly.dll") {
        g.base = fres;
        if (finalCallback)
            std::exchange(finalCallback, nullptr)();
    }

    return fres;
}


void getBaseAddr(const std::function<void ()>& callback) {
    finalCallback = callback;
    DetoursTransaction DT;
    trueLoadLibraryW = LoadLibraryW;
    DetourAttach(&reinterpret_cast<PVOID&>(trueLoadLibraryW), reinterpret_cast<void*>(detourLoadLibraryW));
}

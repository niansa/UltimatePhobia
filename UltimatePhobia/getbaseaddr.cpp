#include "getbaseaddr.hpp"
#include "game_hook.hpp"
#include "global_state.hpp"
#include "misc_utils.hpp"

#include <windows.h>

static std::function<void()> finalCallback;
static std::optional<GameHook> LoadLibraryWHook;

static HMODULE LoadLibraryWFnc(LPWSTR lpLibFileName) {
    GameHookRelease GHR(*LoadLibraryWHook);

    const std::string lpLibFileNameA = utf8Encode(lpLibFileName);
    const HMODULE fres = LoadLibraryW(lpLibFileName);

    g.logger->debug("Loading module {}...", lpLibFileNameA);
    if (lpLibFileNameA == "GameAssembly.dll") {
        g.base = reinterpret_cast<uintptr_t>(fres);
        if (finalCallback)
            std::exchange(finalCallback, nullptr)();
    }

    return fres;
}

void getBaseAddr(const std::function<void()>& callback) {
    finalCallback = callback;
    GameHook::safeCreate(LoadLibraryWHook, reinterpret_cast<void *>(LoadLibraryW), reinterpret_cast<void *>(LoadLibraryWFnc));
}

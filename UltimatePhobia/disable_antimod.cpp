#include "disable_antimod.hpp"
#include "global_state.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"
#include "game_hook.hpp"
#include "detours_helpers.hpp"

#include <algorithm>
#include <string_view>
#include <windows.h>


GameHook *file$$ExistsHook,
         *directory$$ExistsHook,
         *path$$GetFileNameHook;


static void fixPath(std::string& path) {
    for (char& c : path)
        if (c == '/')
            c = '\\';
}

static bool maybeForbiddenFile(std::string path, bool dllSearch = false) {
    // Lowercase string
    std::transform(path.begin(), path.end(), path.begin(),::tolower);

    // Check blacklist
    for (std::string_view term : {".exe", ".dll", "melon", "bepin", "doorstop", "dotnet", "mono", "coreclr", "bootstrap", "up_log.txt", "up_log.txt", "imgui.ini", "ultimatephobia.dll"}) {
        if (dllSearch && term[0] == '.')
            continue;
        if (path.find(term) != path.npos) {
            return true;
        }
    }

    return false;
}


static void *tryCheckFnc(System_String_o *path, const MethodInfo *method) {
    auto cpp_path = GameTypes::toCppString(path);
    fixPath(cpp_path);
    g.logger->debug("Game is trying to check if {} exists!", cpp_path);
    g.logger->flush();

    // Prevent mod detection
    if (maybeForbiddenFile(cpp_path)) {
        g.logger->info("Denying existence of {}.", cpp_path);
        return nullptr;
    }

    // Run actual check (shortcut over std::filesystem)
    GameHook *hook;
    void *caller = GameHook::getTrampolineCaller();
    if (caller == file$$ExistsHook->getAddr())
        hook = file$$ExistsHook;
    if (caller == directory$$ExistsHook->getAddr())
        hook = directory$$ExistsHook;
    if (caller == path$$GetFileNameHook->getAddr())
        hook = path$$GetFileNameHook;
    GameHookRelease GHR(*hook);
    return hook->getFunction<decltype(tryCheckFnc)>()
        (path, method);
}
GAMEHOOK_TRAMPOLINE(tryCheckFnc)

auto *getModuleHandleOrig = GetModuleHandleA;
HMODULE getModuleHandleFnc(LPCSTR lpModuleName) {
    g.logger->debug("Game is trying to check if {} is loaded!", lpModuleName);
    g.logger->flush();

    // Prevent mod detection
    if (maybeForbiddenFile(lpModuleName, true)) {
        g.logger->info("Denying {} being loaded.", lpModuleName);
        return nullptr;
    }

    // Run original function
    return getModuleHandleOrig(lpModuleName);
}


void disableAntiMod() {
    g.logger->info("Disabling mod detection...");

    auto file$$ExistsMethod = GameData::getMethod("System.IO.File$$Exists");
    file$$ExistsHook = new GameHook(file$$ExistsMethod.address, reinterpret_cast<void *>(hookTrampoline_tryCheckFnc), true);

    auto directory$$ExistsMethod = GameData::getMethod("System.IO.Directory$$Exists");
    directory$$ExistsHook = new GameHook(directory$$ExistsMethod.address, reinterpret_cast<void *>(hookTrampoline_tryCheckFnc), true);

    auto path$$GetFileNameMethod = GameData::getMethod("System.IO.Path$$GetFileName");
    path$$GetFileNameHook = new GameHook(path$$GetFileNameMethod.address, reinterpret_cast<void *>(hookTrampoline_tryCheckFnc), true);

    DetoursTransaction DT;
    DetourAttach(&reinterpret_cast<PVOID&>(getModuleHandleOrig), reinterpret_cast<void*>(getModuleHandleFnc));

    return;
}

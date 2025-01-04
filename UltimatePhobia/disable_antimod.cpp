#include "disable_antimod.hpp"
#include "global_state.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"
#include "game_hook.hpp"

#include <algorithm>
#include <string_view>


GameHook *fileExistsHook,
         *directoryExistsHook;


static void fixPath(std::string& path) {
    for (char& c : path)
        if (c == '/')
            c = '\\';
}

static bool maybeForbiddenFile(std::string path) {
    // Lowercase string
    std::transform(path.begin(), path.end(), path.begin(),::tolower);

    // Check blacklist
    for (std::string_view term : {".dll", "melon", "bepin", "doorstep", "dotnet", "mono", "coreclr"}) {
        if (path.find(term) != path.npos)
            return true;
    }

    return false;
}


static bool fileOrDirectoryExistsFnc(System_String_o *path, const MethodInfo *method) {
    auto cpp_path = GameTypes::toCppString(path);
    fixPath(cpp_path);
    g.logger->debug("Game is trying to check if {} exists!", cpp_path);
    g.logger->flush();

    // Prevent mod detection
    if (maybeForbiddenFile(cpp_path)) {
        g.logger->info("Denying existence of {}.", cpp_path);
        return false;
    }

    // Run actual check (shortcut over std::filesystem)
    GameHook *hook;
    if (GameHook::getTrampolineCaller() == fileExistsHook->getAddr())
        hook = fileExistsHook;
    if (GameHook::getTrampolineCaller() == directoryExistsHook->getAddr())
        hook = directoryExistsHook;
    GameHookRelease GHR(*hook);
    return hook->getFunction<decltype(fileOrDirectoryExistsFnc)>()
        (path, method);
}
GAMEHOOK_TRAMPOLINE(fileOrDirectoryExistsFnc)


void disableAntiMod() {
    g.logger->info("Disabling mod detection...");

    auto checkMethod = GameData::getMethod("\u0A72\u0A72\u0A68\u0A70\u0A6E\u0A6D\u0A69\u0A6D\u0A68\u0A6B\u0A6C\u0024\u0024\u0A74\u0A71\u0A6C\u0A69\u0A6F\u0A70\u0A69\u0A71\u0A65\u0A70\u0A71");
    if (!checkMethod.isValid()) {
        g.logger->warn("Couldn't find AntiMod check method. Update " __FILE_NAME__ " in function " __FUNCTION__ " to fix this! The function can easily be found by searching for 'System.IO.File.Exists' function calls.");
        g.logger->info("Falling back to FileExists/DirectoryExists AntiMod disablement method!");

        auto fileExistsMethod = GameData::getMethod("System.IO.File$$Exists");
        fileExistsHook = new GameHook(fileExistsMethod.address, reinterpret_cast<void *>(hookTrampoline_fileOrDirectoryExistsFnc), true);

        auto directoryExistsMethod = GameData::getMethod("System.IO.Directory$$Exists");
        directoryExistsHook = new GameHook(directoryExistsMethod.address, reinterpret_cast<void *>(hookTrampoline_fileOrDirectoryExistsFnc), true);

        return;
    }
    new GameHook(checkMethod.address, reinterpret_cast<void*>(GameHook::noop));
}

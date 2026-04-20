#include "configure_persistentdatapath.hpp"
#include "global_state.hpp"
#include "game_hook.hpp"
#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"

#include <format>
#include <filesystem>
#include <stdexcept>

std::optional<GameHook> unityEngineApplication$$get_persistentDataPathHook, unityEngineApplication$$get_productNameHook;

System_String_o *unityEngineApplication$$get_productNameFnc(const MethodInfo *method) { return Il2Cpp::CppInterop::ToCsString("UltimatePhobia"); }

System_String_o *unityEngineApplication$$get_persistentDataPathFnc(const MethodInfo *method) {
    static auto fres = [method]() -> std::string {
        // Get original path
        GameHookRelease GHR(unityEngineApplication$$get_persistentDataPathHook.value());
        auto originalPath = Il2Cpp::CppInterop::ToCppString(
            unityEngineApplication$$get_persistentDataPathHook->getFunction<decltype(unityEngineApplication$$get_persistentDataPathFnc)>()(method));

        // Find product name
        const char originalProductName[] = "Phasmophobia";
        auto productNameIdx = originalPath.rfind(originalProductName);

        // Make sure it's at end of path
        if (originalPath.size() - productNameIdx == sizeof(originalProductName)) {
            g.logger->critical("Failed to override persistent data path at runtime");
            throw std::runtime_error("Could not override persistent data path at runtime");
        }

        // Strip away product name and insert our name instead
        auto newPath = std::format("{}UltimatePhobia", originalPath.substr(0, productNameIdx));
        g.logger->info("Set persistent data path to {}", newPath);
        std::filesystem::create_directories(newPath);
        return newPath;
    }();
    return Il2Cpp::CppInterop::ToCsString(fres);
}

void configurePersistentDataPath() {
    g.logger->info("Overriding persistent data path...");

    GameHook::safeCreate(unityEngineApplication$$get_productNameHook, Il2Cpp::UnityEngine::Application::get_productName_getPtr(),
                         reinterpret_cast<void *>(unityEngineApplication$$get_productNameFnc));
    GameHook::safeCreate(unityEngineApplication$$get_persistentDataPathHook, Il2Cpp::UnityEngine::Application::get_persistentDataPath_getPtr(),
                         reinterpret_cast<void *>(unityEngineApplication$$get_persistentDataPathFnc));

    if (!unityEngineApplication$$get_productNameHook.has_value() || !unityEngineApplication$$get_persistentDataPathHook.has_value()) {
        g.logger->critical("Failed to override persistent data path");
        throw std::runtime_error("Could not override persistent data path");
    }
}

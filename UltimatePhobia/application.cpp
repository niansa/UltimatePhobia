#include "application.hpp"
#include "global_state.hpp"
#include "il2cpp.h"
#include "game_hook.hpp"
#include "imgui_man.hpp"
#include "safe_path.hpp"
#include "ffi_loader.hpp"
#include "il2cpp_dynamic.hpp"
#include "generated/il2cpp.hpp"

#include "mods/global_instance_manager.hpp"
#include "mods/tracer.hpp"
#include "mods/explorer.hpp"
#include "mods/photon_settings.hpp"
#include "mods/save_file_manager.hpp"
#include "mods/player_manager.hpp"
#include "mods/goldberg_emu_manager.hpp"
#include "mods/fixes.hpp"
#include "mods/improvements.hpp"
#include "mods/cheats.hpp"

#include <optional>
#include <filesystem>
#include <fstream>
#include <exception>
#include <imgui.h>

static Application *currentApplication = nullptr;

struct ApplicationHooks {
    inline static std::optional<GameHook> appUpdateHook, splashScreenCtorHook, il2cppInitHook, il2cppInitUtf8Hook;

    static int il2cppInitFnc(void *v) {
        g.logger->info("Initializing il2cpp...");
        il2cppInitHook.reset();
        il2cppInitUtf8Hook.reset();
        int fres = reinterpret_cast<decltype(&il2cppInitFnc)>(GameHook::getTrampolineCaller())(v);

        g.logger->info("Starting to process il2cpp runtime reflection data...");
        Il2Cpp::Dynamic::init();

        const auto scriptJsonPath = SafePath::get() / "script.json";
        if (std::filesystem::exists(scriptJsonPath))
            std::ofstream(scriptJsonPath) << Il2Cpp::Dynamic::dump();

        return fres;
    }
    GAMEHOOK_TRAMPOLINE(il2cppInitFnc)

    static void appUpdateFnc(Photon_Pun_PhotonHandler_o *__this, const MethodInfo *method) {
        try {
            currentApplication->update();
        } catch (const std::exception& e) {
            g.logger->error("Exception in main loop: {}", e.what());
        } catch (const ModPanic& e) {
            g.logger->error("Mod '{}' has panicked: {}", e.where(), e.what());
        } catch (...) {
            g.logger->error("Unknown exception (maybe C# exception?) in main loop");
            std::rethrow_exception(std::current_exception());
        }
        GameHookRelease GHR(*appUpdateHook);
        appUpdateHook->getFunction<decltype(ApplicationHooks::appUpdateFnc)>()(__this, method);
    }

    static void splashScreenCtorFnc(SplashScreen_o *__this, const MethodInfo *method) {
        g.logger->info("Game has started!");
        auto orig = splashScreenCtorHook->getFunction<decltype(ApplicationHooks::splashScreenCtorFnc)>();
        splashScreenCtorHook.reset();
        currentApplication->init();
        GameHookRelease GHR(*splashScreenCtorHook);
        orig(__this, method);
    }
};

Application::Application() {
    currentApplication = this;
    mods = {
        &photonSettingsInfo, &saveFileManagerInfo, &fixesInfo, &playerManagerInfo, &goldbergEmuManagerInfo, &tracerInfo, &explorerInfo, &improvementsInfo,
#ifdef MOD_ENABLE_CHEATS
        &cheatsInfo,
#endif
    };

    g.logger->info("Waiting for game start...");
    GameHook::safeCreate(ApplicationHooks::il2cppInitHook, reinterpret_cast<void *>(Il2Cpp::API::il2cpp_init),
                         reinterpret_cast<void *>(ApplicationHooks::hookTrampoline_il2cppInitFnc), true);
    GameHook::safeCreate(ApplicationHooks::il2cppInitUtf8Hook, reinterpret_cast<void *>(Il2Cpp::API::il2cpp_init),
                         reinterpret_cast<void *>(ApplicationHooks::hookTrampoline_il2cppInitFnc), true);
    GameHook::safeCreate(ApplicationHooks::splashScreenCtorHook, Il2Cpp::SplashScreen::_ctor_getPtr(),
                         reinterpret_cast<void *>(ApplicationHooks::splashScreenCtorFnc));
}

void Application::init() {
    g.logger->info("Starting to listen for local player updates...");
    GameHook::safeCreate(ApplicationHooks::appUpdateHook, Il2Cpp::PlayerSanity::Update_getPtr(), reinterpret_cast<void *>(ApplicationHooks::appUpdateFnc));

    g.logger->info("Calling onAppStart functions...");
    for (auto& mod : mods) {
        if (mod->onAppStart)
            mod->onAppStart();
    }

    g.logger->info("Loading essential mods...");
    globalInstanceManagerInfo.load();
    fixesInfo.load();

    const auto modsDir = SafePath::get() / "mods";
    bool modsDirExists = false;
    try {
        modsDirExists = std::filesystem::is_directory(modsDir);
    } catch (...) {
        modsDirExists = false;
    }

    if (modsDirExists) {
        if (Il2Cpp::Dynamic::isLoaded()) {
            g.logger->info("Preparing FFI mods...");
            for (const auto& entry : std::filesystem::directory_iterator(modsDir)) {
                if (!entry.is_regular_file())
                    continue;
                if (entry.path().extension() != ".json")
                    continue;
                const auto filename = entry.path().filename().string();
                const auto identifier = filename.substr(0, filename.size() - 5);
                mods.emplace_back(FFILoader::createModInfo(modsDir, identifier));
            }
        } else {
            g.logger->warn("FFI mods found but ignored because script.json is missing.");
        }
    }
}

void Application::update() {
    if (!ImGuiMan::pre_update())
        return;

    {
        using namespace ImGui;
        Begin("Mod manager");

        Text("%f FPS", ImGui::GetIO().Framerate);
        NewLine();

        for (const auto mod : mods) {
            if (mod->hidden)
                continue;
            bool isLoaded = mod->instance != nullptr;
            if (Checkbox(mod->name.c_str(), &isLoaded)) {
                if (isLoaded)
                    mod->load();
                else
                    mod->unload();
            }
        }

        if (Button("Exit game"))
            exit(0);

        End();
    }

    for (auto& mod : mods)
        if (mod->instance)
            mod->instance->uiUpdate();

    g.logger->flush();
    ImGuiMan::post_update();
}

void Application::exit(int code) {
    g.logger->info("Exiting application...");
    Il2Cpp::UnityEngine::Application::Quit();
    ImGuiMan::deinit();
    ApplicationHooks::appUpdateHook.reset();
}

bool Application::isActive() { return ApplicationHooks::appUpdateHook.has_value() && ApplicationHooks::appUpdateHook.value().isActive(); }

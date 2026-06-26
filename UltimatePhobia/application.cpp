#include "application.hpp"
#include "global_state.hpp"
#include "il2cpp.h"
#include "linux.hpp"
#include "game_hook.hpp"
#include "imgui_man.hpp"
#include "safe_path.hpp"
#include "ffi_loader.hpp"
#include "dx11_hook.hpp"
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

namespace Il2Cpp::Profiling {
void InstallHooks();
}

static Application *currentApplication = nullptr;

struct ApplicationHooks {
    inline static std::optional<GameHook> appUpdateHook, splashScreenCtorHook, il2cppInitHook, il2cppInitUtf8Hook;

    static int il2cppInitFnc(void *v) {
        g.logger->info("Initializing il2cpp...");
        il2cppInitHook.reset();
        il2cppInitUtf8Hook.reset();
        int fres = reinterpret_cast<decltype(&il2cppInitFnc)>(GameHook::getTrampolineCaller())(v);

        g.logger->info("Starting to process il2cpp runtime reflection data...");
        Il2Cpp::Dynamic::init([]() {
            g.logger->info("Dumping il2cpp runtime reflection data to script.json file...");
            const auto scriptJsonPath = SafePath::get() / "script.json";
            if (std::filesystem::exists(scriptJsonPath))
                std::ofstream(scriptJsonPath) << Il2Cpp::Dynamic::dump();
        });

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

    DX11Hook::init();
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

        for (unsigned idx = 0; idx != mods.size(); ++idx) {
            auto& mod = mods[idx];
            PushID(idx);
            if (!mod->hidden) {
                bool isLoaded = mod->instance != nullptr;
                if (Checkbox(mod->name.c_str(), &isLoaded)) {
                    if (isLoaded)
                        mod->load();
                    else
                        mod->unload();
                }
            }
            PopID();
        }

        if (Button("Exit game"))
            exit(0);
        if (linux::isMangoHudAvailable()) {
            SameLine();
            bool enabled = linux::isMangoHudEnabled();
            if (Checkbox("Enable MangoHud", &enabled))
                linux::restartWithMangoHud(enabled);
        }

#ifdef TRACY_ENABLE
        BeginDisabled(tracingHooksInstalled);
        if (Button("Install profiling hooks")) {
            g.logger->info("Installing profiling hooks...");
            Il2Cpp::Profiling::InstallHooks();
        }
        EndDisabled();
#endif

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
    ExitProcess(code);
}

bool Application::isActive() { return ApplicationHooks::appUpdateHook.has_value() && ApplicationHooks::appUpdateHook.value().isActive(); }

std::string getLinuxWineLoader() {
    // Wine maps the raw Linux root directory to Z:
    std::ifstream file("Z:\\proc\\self\\environ", std::ios::binary);
    if (!file.is_open()) {
        return "wine"; // Failsafe fallback
    }

    std::string envVar;
    // /proc/self/environ separates variables with a null character (\0)
    while (std::getline(file, envVar, '\0')) {
        if (envVar.find("WINELOADER=") == 0) {
            std::string loader = envVar.substr(11); // Length of "WINELOADER="

            // Strip "-preloader" if it exists to avoid EACCES permission blocks
            size_t pos = loader.find("-preloader");
            if (pos != std::string::npos) {
                loader.erase(pos, 10);
            }
            return loader;
        }
    }
    return "wine"; // Failsafe fallback if WINELOADER is missing
}

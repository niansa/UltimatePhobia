#include "application.hpp"
#include "global_state.hpp"
#include "gamedata.hpp"
#include "il2cpp.h"
#include "game_hook.hpp"
#include "imgui_man.hpp"

#include "mods/global_instance_manager.hpp"
#include "mods/tracer.hpp"
#include "mods/photon_settings.hpp"
#include "mods/save_file_manager.hpp"
#include "mods/goldberg_emu_manager.hpp"
#include "mods/cheats.hpp"

#include <optional>
#include <imgui.h>



static Application *currentApplication = nullptr;


struct ApplicationHooks {
    inline static std::optional<GameHook> appUpdateHook,
                                          splashScreenCtorHook;

    static void appUpdateFnc(Player_o *__this, const MethodInfo *method) {
        //if (__this->fields.photonView->fields._AmOwner_k__BackingField)
            currentApplication->update();
        GameHookRelease GHR(*appUpdateHook);
        appUpdateHook->getFunction<decltype(ApplicationHooks::appUpdateFnc)>()(__this, method);
    }

    static void splashScreenCtorFnc(SplashScreen_o *__this, const MethodInfo *method) {
        g.logger->info("Game has started!");
        auto orig = splashScreenCtorHook->getFunction<decltype(ApplicationHooks::splashScreenCtorFnc)>();
        splashScreenCtorHook.reset();
        currentApplication->init();
        orig(__this, method);
    }
};


Application::Application() {
    currentApplication = this;
    mods = {&tracerInfo, &photonSettingsInfo, &saveFileManagerInfo, &goldbergEmuManagerInfo,
#ifdef MOD_ENABLE_CHEATS
            &cheatsInfo
#endif
    };

    g.logger->info("Waiting for game start...");
    ApplicationHooks::splashScreenCtorHook.emplace(GameData::getMethod("SplashScreen$$.ctor").address, ApplicationHooks::splashScreenCtorFnc);
}

void Application::init() {
    g.logger->info("Starting to listen for local player updates...");
    ApplicationHooks::appUpdateHook.emplace(GameData::getMethod("Player$$Update").address, ApplicationHooks::appUpdateFnc);

    g.logger->info("Calling onAppStart functions...");
    for (auto& mod : mods) {
        if (mod->onAppStart)
            mod->onAppStart();
    }

    g.logger->info("Loading essential mods...");
    globalInstanceManagerInfo.load();
}

void Application::update() {
    if (!ImGuiMan::pre_update())
        return;

    {
        using namespace ImGui;
        Begin("Mod manager");

        static unsigned counter = 0;
        Text("Frame %u", counter++);
        NewLine();

        for (const auto mod : mods) {
            bool isLoaded = mod->instance != nullptr;
            if (Checkbox(mod->name, &isLoaded)) {
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
    GameData::getMethod("void UnityEngine_Application__Quit (int32_t exitCode, const MethodInfo* method);")
    .getFunction<void (int, const MethodInfo*)>()
        (0, nullptr);
    ImGuiMan::deinit();
    ApplicationHooks::appUpdateHook.reset();
}

bool Application::isActive() {
    return ApplicationHooks::appUpdateHook.has_value() && ApplicationHooks::appUpdateHook.value().isActive();
}

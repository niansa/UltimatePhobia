#include "application.hpp"
#include "global_state.hpp"
#include "gamedata.hpp"
#include "il2cpp.h"
#include "game_hook.hpp"
#include "imgui_man.hpp"

#include "mods/global_instance_manager.hpp"
#include "mods/tracer.hpp"
#include "mods/photon_settings.hpp"
#include "mods/hacks.hpp"

#include <optional>
#include <imgui.h>



static Application *currentApplication = nullptr;


struct ApplicationHooks {
    inline static std::optional<GameHook> appUpdateHook;

    static void appUpdateFnc(Player_o* __this, const MethodInfo* method) {
        GameHookRelease GHR(*appUpdateHook);
        appUpdateHook->getFunction<decltype(ApplicationHooks::appUpdateFnc)>()(__this, method);
        //if (__this->fields.photonView->fields._AmOwner_k__BackingField)
            currentApplication->update();
    }
};


Application::Application() {
    currentApplication = this;

    mods = {&tracerInfo, &photonSettingsInfo, &hacksInfo};

    g.logger->info("Starting to listen for local player updates...");
    ApplicationHooks::appUpdateHook.emplace(GameData::getMethod("Player$$Update").address, ApplicationHooks::appUpdateFnc);

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

        End();
    }

    for (auto& mod : mods)
        if (mod->instance)
            mod->instance->uiUpdate();

    ImGuiMan::post_update();
}

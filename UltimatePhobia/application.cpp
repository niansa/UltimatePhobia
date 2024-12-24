#include "application.hpp"
#include "global_state.hpp"
#include "gamedata.hpp"
#include "il2cpp.h"
#include "game_hook.hpp"
#include "imgui_man.hpp"

#include <optional>
#include <imgui.h>



Application *currentApplication = nullptr;


struct ApplicationHooks {
    inline static std::optional<GameHook> appUpdateHook;

    static void appUpdateFnc(Player_o* __this, const MethodInfo* method) {
        GameHookRelease GHR(*appUpdateHook);
        appUpdateHook->getFunction<decltype(ApplicationHooks::appUpdateFnc)>()(__this, method);
        if (__this->fields.photonView->fields._IsMine_k__BackingField)
            currentApplication->update();
    }
};


Application::Application() {
    currentApplication = this;

    g.logger->info("Initializing UltimatePhobia application...");

    // Test...
    ApplicationHooks::appUpdateHook.emplace(getGameMethod("Player$$Update").address, ApplicationHooks::appUpdateFnc);
}

void Application::update() {
    if (!ImGuiMan::pre_update())
        return;

    ImGui::Begin("Mod manager");
    static unsigned counter = 0;
    ImGui::Text("%u", counter++);
    ImGui::End();

    ImGuiMan::post_update();
}

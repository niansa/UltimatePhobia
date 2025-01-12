#include "global_instance_manager.hpp"
#include "gamedata.hpp"
#include "game_hook.hpp"



static void player$$StartFnc(Player_o* __this, const MethodInfo* method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->player$$StartHook.value();
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(player$$StartFnc)>()(__this, method);
    self->player = __this;
}

static void ghostAI$$StartFnc(GhostAI_o* __this, const MethodInfo* method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->ghostAI$$StartHook.value();
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(ghostAI$$StartFnc)>()(__this, method);
    self->ghost = __this;
}


GlobalInstanceManager::GlobalInstanceManager() {
    player$$StartHook.emplace(GameData::getMethod("Player$$Start").address, player$$StartFnc);
    ghostAI$$StartHook.emplace(GameData::getMethod("GhostAI$$Start").address, ghostAI$$StartFnc);
}


ModInfo globalInstanceManagerInfo {
    "Global Instance Manager",
    true,
    [] () {return std::make_unique<GlobalInstanceManager>();}
};

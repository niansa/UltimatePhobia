#include "global_instance_manager.hpp"
#include "gamedata.hpp"
#include "game_hook.hpp"



static void playerStartFnc(Player_o* __this, const MethodInfo* method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->playerStartHook.value();
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(playerStartFnc)>()(__this, method);
    self->player = __this;
}

static void ghostAIStartFnc(GhostAI_o* __this, const MethodInfo* method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->ghostAIStartHook.value();
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(ghostAIStartFnc)>()(__this, method);
    self->ghost = __this;
}


GlobalInstanceManager::GlobalInstanceManager() {
    playerStartHook.emplace(GameData::getMethod("Player$$Start").address, playerStartFnc);
    ghostAIStartHook.emplace(GameData::getMethod("GhostAI$$Start").address, ghostAIStartFnc);
}


ModInfo globalInstanceManagerInfo {
    "Global Instance Manager",
    [] () {return std::make_unique<GlobalInstanceManager>();}
};

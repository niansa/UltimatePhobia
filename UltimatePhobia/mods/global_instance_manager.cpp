#include "global_instance_manager.hpp"
#include "game_hook.hpp"
#include "generated/il2cpp.hpp"



static void player$$StartFnc(Player_o* __this, const MethodInfo* method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->player$$StartHook;
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(player$$StartFnc)>()(__this, method);
    if (self->player->fields.photonView->fields._AmOwner_k__BackingField)
        self->player = __this;
}

static void ghostAI$$StartFnc(GhostAI_o* __this, const MethodInfo* method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->ghostAI$$StartHook;
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(ghostAI$$StartFnc)>()(__this, method);
    self->ghost = __this;
}


GlobalInstanceManager::GlobalInstanceManager()
    : player$$StartHook(GameHook::safeCreateOrPanic(globalInstanceManagerInfo, Il2Cpp::Player::Start_getPtr(), reinterpret_cast<void *>(player$$StartFnc)))
    , ghostAI$$StartHook(GameHook::safeCreateOrPanic(globalInstanceManagerInfo, Il2Cpp::GhostAI::Start_getPtr(), reinterpret_cast<void *>(ghostAI$$StartFnc)))
{}


ModInfo globalInstanceManagerInfo {
    "Global Instance Manager",
    true,
    [] () {return std::make_unique<GlobalInstanceManager>();}
};

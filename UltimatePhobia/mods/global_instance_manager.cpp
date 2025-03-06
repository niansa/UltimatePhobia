#include "global_instance_manager.hpp"
#include "game_hook.hpp"
#include "generated/il2cpp.hpp"

static void player$$AwakeFnc(Player_o *__this, const MethodInfo *method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->player$$AwakeHook;
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(player$$AwakeFnc)>()(__this, method);
    auto photonView = __this->fields.photonView;
    if (!photonView || photonView->fields._AmOwner_k__BackingField)
        self->player = __this;
}

static void ghostAI$$AwakeFnc(GhostAI_o *__this, const MethodInfo *method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->ghostAI$$AwakeHook;
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(ghostAI$$AwakeFnc)>()(__this, method);
    self->ghost = __this;
}

static void gameController$$AwakeFnc(GameController_o *__this,
                                     const MethodInfo *method) {
    const auto self = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto& hook = self->gameController$$AwakeHook;
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(gameController$$AwakeFnc)>()(__this, method);
    self->gameController = __this;
}

GlobalInstanceManager::GlobalInstanceManager()
    : player$$AwakeHook(GameHook::safeCreateOrPanic(
          globalInstanceManagerInfo, Il2Cpp::Player::Awake_getPtr(),
          reinterpret_cast<void *>(player$$AwakeFnc))),
      ghostAI$$AwakeHook(GameHook::safeCreateOrPanic(
          globalInstanceManagerInfo, Il2Cpp::GhostAI::Awake_getPtr(),
          reinterpret_cast<void *>(ghostAI$$AwakeFnc))),
      gameController$$AwakeHook(GameHook::safeCreateOrPanic(
          globalInstanceManagerInfo, Il2Cpp::GameController::Awake_getPtr(),
          reinterpret_cast<void *>(gameController$$AwakeFnc))) {}

ModInfo globalInstanceManagerInfo{
    "Global Instance Manager", true,
    []() { return std::make_unique<GlobalInstanceManager>(); }};

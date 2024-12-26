#include "hacks.hpp"
#include "global_instance_manager.hpp"
#include "gamedata.hpp"
#include "game_hook.hpp"

#include <memory>
#include <imgui.h>



static void ghostAIChangeStateFnc(GhostAI_o* __this, int32_t newState, PhotonObjectInteract_o* objectInteraction, PhotonObjectInteract_array* interactionArray, bool unk0, const MethodInfo* method) {
    const auto self = hacksInfo.get<Hacks>();
    auto& hook = self->ghostAIChangeStateHook.value();
    GameHookRelease GHR(hook);
    if (!self->ghostStateQueue.empty()) {
        newState = static_cast<int>(self->ghostStateQueue.front());
        self->ghostStateQueue.pop();
    }
    hook.getFunction<decltype(ghostAIChangeStateFnc)>()(__this, newState, objectInteraction, interactionArray, unk0, method);
}


void Hacks::setGhostAIChangeStateHook() {
    if (!ghostAIChangeStateHook.has_value())
        ghostAIChangeStateHook.emplace(GameData::getMethod("GhostAI$$ChangeState").address, ghostAIChangeStateFnc);
}

void Hacks::uiUpdate() {
    using namespace ImGui;
    Begin("Hacks");

#define TOGGLE(description, hook, boolean, method, hookFnc) \
    if (Checkbox(description, &boolean)) { \
        if (boolean) \
            hook.emplace(GameData::getMethod(method).address, hookFnc); \
        else \
            hook.reset(); \
    }
    TOGGLE("Infinite stamina", playerStaminaStartDrainingHook, infiniteStamina, "PlayerStamina$$StartDraining", GameHook::noop)
    TOGGLE("Pause ghost", ghostUpdateHook, pauseGhost, "GhostAI$$Update", GameHook::noop)
    TOGGLE("Invincibility", playerStartKillingPlayerHook, invincibility, "Player$$StartKillingPlayer", GameHook::noop)

    SeparatorText("Queue ghost states");
    if (Button("Short event")) {
        setGhostAIChangeStateHook();
        ghostStateQueue.emplace(GhostState::normalEvent);
    }
    if (Button("Hunt")) {
        setGhostAIChangeStateHook();
        ghostStateQueue.emplace(GhostState::hunt);
    }
    if (Button("Summon")) {
        setGhostAIChangeStateHook();
        ghostStateQueue.emplace(GhostState::summon);
    }
    Separator();
    if (Button("Trigger interaction")) {
        auto ghost = globalInstanceManagerInfo.get<GlobalInstanceManager>()->ghost;
        GameData::getMethod("GhostActivity$$Interact").getFunction<void (GhostActivity_o*, const MethodInfo*)>()(ghost->fields.ghostActivity, nullptr);
    }

    End();
}


ModInfo hacksInfo {
    "Hacks",
    [] () {return std::make_unique<Hacks>();}
};

#include "cheats.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_cppinterop.hpp"
#include "game_hook.hpp"
#include "misc_utils.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"

#include <memory>
#include <imgui.h>

static void
ghostAI$$ChangeStateFnc(GhostAI_o *__this, int32_t newState,
                        PhotonObjectInteract_o *objectInteraction,
                        PhotonObjectInteract_array *interactionArray, bool unk0,
                        const MethodInfo *method) {
    const auto self = cheatsInfo.get<Cheats>();
    auto hook = self->hooks.get(Il2Cpp::GhostAI::ChangeState_getPtr());

    // Adjust ghost state
    if (!self->ghostStateQueue.empty()) {
        newState = static_cast<int>(self->ghostStateQueue.front());
        self->ghostStateQueue.pop();
    }

    // Call original function
    GameHookRelease GHR(*hook);
    hook->getFunction<decltype(ghostAI$$ChangeStateFnc)>()(
        __this, newState, objectInteraction, interactionArray, unk0, method);
}

static void door$$UpdateFnc(Door_o *__this, const MethodInfo *method) {
    const auto self = cheatsInfo.get<Cheats>();
    auto hook = self->hooks.get(Il2Cpp::Door::Update_getPtr());

    // Unlock and open door, then permanently disable hunt collider
    using namespace Il2Cpp;
    Door::UnlockDoor(__this);
    Door::DisableOrEnableDoor(__this, true);
    Door::OpenDoor(__this, 1.0f, true);
    using namespace Il2Cpp::UnityEngine;
    if (__this->fields.huntCollider)
        GameObject::SetActive(Component::get_gameObject(
                                  reinterpret_cast<UnityEngine_Component_o *>(
                                      (__this->fields.huntCollider))),
                              false);

    // Call original method first
    GameHookRelease GHR(*hook);
    hook->getFunction<decltype(door$$UpdateFnc)>()(__this, method);
}

void Cheats::setGhostAIChangeStateHook() {
    auto fnc = Il2Cpp::GhostAI::ChangeState_getPtr();
    if (hooks.get(fnc) == nullptr)
        hooks.add(fnc, ghostAI$$ChangeStateFnc);
}

void Cheats::uiUpdate() {
    using namespace ImGui;
    Begin("Cheats");

    using namespace Il2Cpp;
    hookToggle("Infinite stamina", hooks, infiniteStamina,
               PlayerStamina::StartDraining_getPtr(),
               reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Keep items after death", hooks, keepItemsAfterDeath,
               InventoryManager::RemoveItemsFromInventory_getPtr(),
               reinterpret_cast<void *>(GameHook::noop));
#ifdef MOD_ENABLE_CHEATS_EXTRA
    Separator();
    hookToggle("Invincibility", hooks, invincibility,
               Player::StartKillingPlayer_getPtr(),
               reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Pause ghost", hooks, pauseGhost, GhostAI::Update_getPtr(),
               reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Unlock all doors", hooks, autoUnlockDoors,
               Door::Update_getPtr(),
               reinterpret_cast<void *>(door$$UpdateFnc));
    hookToggle("Allow grab when dead", hooks, allowGrabWhenDead,
               PCPropGrab::PlayerDied_getPtr(),
               reinterpret_cast<void *>(GameHook::noop));

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
        GhostActivity::Interact(
            globalInstanceManagerInfo.get<GlobalInstanceManager>()
                ->getGhostAI()
                ->fields.ghostActivity);
    }
#endif

    End();
}

ModInfo cheatsInfo{"Cheats", false,
                   []() { return std::make_unique<Cheats>(); }};

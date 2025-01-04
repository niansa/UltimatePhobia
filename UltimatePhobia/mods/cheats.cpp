#include "cheats.hpp"
#include "global_instance_manager.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"
#include "game_hook.hpp"
#include "misc_utils.hpp"
#include "unityengine.hpp"

#include <memory>
#include <imgui.h>



static void ghostAI$$ChangeStateFnc(GhostAI_o* __this, int32_t newState, PhotonObjectInteract_o* objectInteraction, PhotonObjectInteract_array* interactionArray, bool unk0, const MethodInfo* method) {
    const auto self = cheatsInfo.get<Cheats>();
    auto& hook = self->ghostAI$$ChangeStateHook.value();

    // Adjust ghost state
    if (!self->ghostStateQueue.empty()) {
        newState = static_cast<int>(self->ghostStateQueue.front());
        self->ghostStateQueue.pop();
    }

    // Call original function
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(ghostAI$$ChangeStateFnc)>()(__this, newState, objectInteraction, interactionArray, unk0, method);
}

static void key$$StartFnc(Key_o *__this, const MethodInfo *method) {
    const auto self = cheatsInfo.get<Cheats>();
    auto& hook = self->key$$StartHook.value();

    // Start key first
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(key$$StartFnc)>()
        (__this, method);

    // Call GrabbedKey photon RPC
    Cheats::sendRPC(__this->fields.view, "GrabbedKey", 0);
}

static void door$$UpdateFnc(Door_o *__this, const MethodInfo *method) {
    const auto self = cheatsInfo.get<Cheats>();
    auto& hook = self->door$$UpdateHook.value();

    // Call unlock function
    GameData::getMethod("Door$$UnlockDoor")
        .getFunction<void (Door_o *, const MethodInfo *)>()
        (__this, nullptr);
    GameData::getMethod("Door$$DisableOrEnableDoor")
        .getFunction<void (Door_o *, bool, const MethodInfo *)>()
        (__this, true, nullptr);
    GameData::getMethod("Door$$DisableOrEnableCollider")
        .getFunction<void (Door_o *, bool, const MethodInfo *)>()
        (__this, false, nullptr);

    // Call original method first
    GameHookRelease GHR(hook);
    hook.getFunction<decltype(door$$UpdateFnc)>()
        (__this, method);
}


void Cheats::setGhostAIChangeStateHook() {
    if (!ghostAI$$ChangeStateHook.has_value())
        ghostAI$$ChangeStateHook.emplace(GameData::getMethod("GhostAI$$ChangeState").address, ghostAI$$ChangeStateFnc);
}

void Cheats::uiUpdate() {
    using namespace ImGui;
    Begin("Cheats");

    hookToggle("Infinite stamina", playerStamina$$StartDrainingHook, infiniteStamina, "PlayerStamina$$StartDraining", reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Pause ghost", ghost$$UpdateHook, pauseGhost, "GhostAI$$Update", reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Invincibility", player$$StartKillingPlayerHook, invincibility, "Player$$StartKillingPlayer", reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Auto grab key", key$$StartHook, autoGrabKey, "Key$$Start", reinterpret_cast<void *>(key$$StartFnc));
    hookToggle("Keep items after death", inventoryManager$$RemoveItemsFromInventoryHook, keepItemsAfterDeath, "InventoryManager$$RemoveItemsFromInventory", reinterpret_cast<void *>(GameHook::noop));
    hookToggle("Unlock all doors", door$$UpdateHook, autoUnlockDoors, "Door$$Update", reinterpret_cast<void *>(door$$UpdateFnc));

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


void Cheats::sendRPC(Photon_Pun_PhotonView_o *view, std::string_view methodName, int32_t target) {
    GameData::getMethod("void Photon_Pun_PhotonNetwork__RPC (Photon_Pun_PhotonView_o* view, System_String_o* methodName, int32_t target, bool encrypt, System_Object_array* parameters, const MethodInfo* method);")
        .getFunction<void (Photon_Pun_PhotonView_o *view, System_String_o *methodName, int32_t target, bool encrypt, System_Object_array *parameters, const MethodInfo *method)>()
        (view, GameTypes::createCsString(methodName), target, false, nullptr, nullptr);
}


ModInfo cheatsInfo {
    "Cheats",
    [] () {return std::make_unique<Cheats>();}
};

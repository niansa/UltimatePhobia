#include "improvements.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_api_cpp.hpp"
#include "game_hook.hpp"
#include "misc_utils.hpp"

#include <memory>
#include <chrono>
#include <imgui.h>
#include <commoncpp/timer.hpp>

static void key$$StartFnc(Key_o *__this, const MethodInfo *method) {
    const auto self = improvementsInfo.get<Improvements>();
    auto hook = self->hooks.get(Il2Cpp::API::get_method_cached<"Assembly-CSharp", "", "Key", "Start", 0>().function_pointer());

    // Start key first
    GameHookRelease GHR(*hook);
    hook->getFunction<decltype(key$$StartFnc)>()(__this, method);

    // Call GrabbedKey photon RPC
    using namespace Il2Cpp;
    API::call(API::get_class_cached<"PhotonUnityNetworking", "Photon.Pun", "PhotonNetwork">(), "RPC", reinterpret_cast<Il2CppObject *>(__this->fields.view),
              "GrabbedKey", 0, false, nullptr);
}

static void painKillers$$NetworkedUseFnc(Il2CppObject *__this, Photon_Pun_PhotonMessageInfo_o *messageInfo, const MethodInfo *method) {
    const auto self = improvementsInfo.get<Improvements>();
    const auto gim = globalInstanceManagerInfo.get<GlobalInstanceManager>();
    auto hook = self->hooks.get(Il2Cpp::GhostAI::ChangeState_getPtr());

    if (!gim->getGameController()) {
        g.logger->error("Game controller not initialized on pain killer use");
        return;
    }

    using namespace Il2Cpp;

    // Get audio source
    static auto audioSourceType = []() -> API::Type { return API::get_class_cached<"UnityEngine.AudioModule", "UnityEngine", "AudioSource">().type(); }();
    API::call(API::call(API::call(__this, "get_gameObject"), "GetComponent", audioSourceType.object()), "Play");

    // Create timer
    self->painKillerTimers[__this] = common::Timer();
}

static void painKillers$$UseFnc(Il2CppObject *__this, const MethodInfo *method) {
    const auto self = improvementsInfo.get<Improvements>();

    // Get hook
    auto hook = self->hooks.get(Il2Cpp::PainKillers::Use_getPtr());
    if (!hook) {
        g.logger->error("Refilling pain killer use function called but hook not set???");
        return;
    }

    // Check timer
    if ([self, __this]() -> bool {
            auto res = self->painKillerTimers.find(__this);
            if (res == self->painKillerTimers.end())
                return false;
            if (res->second.get<std::chrono::minutes>() > 5) {
                self->painKillerTimers.erase(__this);
                return false;
            }
            return true;
        }()) {
        return;
    }

    GameHookRelease GHR(*hook);
    Il2Cpp::API::call(__this, "Use");
}

void Improvements::uiUpdate() {
    using namespace ImGui;
    Begin("Improvements");

    using namespace Il2Cpp;
    hookToggle("Auto grab keys", hooks, autoGrabKeys, Key::Start_getPtr(), reinterpret_cast<void *>(key$$StartFnc));
    if (hookToggle("Refill sanity pills", hooks, refillPills, PainKillers::NetworkedUse_getPtr(), reinterpret_cast<void *>(painKillers$$NetworkedUseFnc)) &&
        refillPills)
        hooks.add(PainKillers::Use_getPtr(), reinterpret_cast<void *>(painKillers$$UseFnc));

    End();
}

ModInfo improvementsInfo{"Improvements", false, []() { return std::make_unique<Improvements>(); }};

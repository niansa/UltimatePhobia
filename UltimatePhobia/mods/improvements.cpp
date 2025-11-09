#include "improvements.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_api_cpp.hpp"
#include "il2cpp_cppinterop.hpp"
#include "game_hook.hpp"
#include "misc_utils.hpp"
#include "generated/il2cpp.hpp"

#include <memory>
#include <chrono>
#include <imgui.h>
#include <commoncpp/timer.hpp>

static void key$$StartFnc(Key_o *__this, const MethodInfo *method) {
    const auto self = improvementsInfo.get<Improvements>();
    auto hook = self->hooks.get(Il2Cpp::Key::Start_getPtr());

    // Start key first
    GameHookRelease GHR(*hook);
    hook->getFunction<decltype(key$$StartFnc)>()(__this, method);

    // Call GrabbedKey photon RPC
    Il2Cpp::Photon::Pun::PhotonNetwork::RPC(__this->fields.view, Il2Cpp::CppInterop::ToCsString("GrabbedKey"), 0, false, nullptr);
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
    using namespace CppInterop;

    // Get some types
    static auto audioSourceType = []() -> API::Type {
        return Il2Cpp::API::get_class_cached<"UnityEngine.AudioModule", "UnityEngine", "AudioSource">().type();
    }();

    // Get some context
    auto gameObject = API::call<"UnityEngine.CoreModule", "UnityEngine", "Component", "get_gameObject">(__this);

    // Play audio source
    API::call<"UnityEngine.AudioModule", "UnityEngine", "AudioSource", "Play">(
        API::call<"UnityEngine.CoreModule", "UnityEngine", "GameObject", "GetComponent">(gameObject, audioSourceType.object()));

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
    Il2Cpp::API::call<"Assembly-CSharp", "", "PainKillers", "Use">(__this);
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

#include "player_manager.hpp"
#include "slugger_manager.hpp"
#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"

using namespace Il2Cpp::CppInterop;
using namespace Il2Cpp;

namespace {
constexpr float kHeadWorldYOffset = 0.2f;
constexpr float kTargetFontSize = 32.0f;
} // namespace

void player$$UpdateFnc(Player_o *__this, const MethodInfo *method) {
    auto self = playerManagerInfo.get<PlayerManager>();
    if (!self || !__this)
        return;

    std::string nickName;
    bool shouldProject = false;

    // Register player and retrieve cached nickname
    {
        std::lock_guard<std::mutex> lock(self->m_playerMutex);

        auto it = self->trackedPlayers.find(__this);
        if (it == self->trackedPlayers.end()) {
            if (__this->fields.photonView) {
                if (__this->fields.photonView->fields._AmOwner_k__BackingField) {
                    // Cache local player as empty label
                    self->trackedPlayers[__this] = PlayerTextData{""};
                } else {
                    Photon_Realtime_Player_o *photonPlayer = __this->fields.photonView->fields._Owner_k__BackingField;
                    if (photonPlayer) {
                        System_String_o *nickNameCs = Photon::Realtime::Player::get_NickName(photonPlayer, nullptr);
                        if (nickNameCs) {
                            nickName = ToCppString(nickNameCs);
                            self->trackedPlayers[__this] = PlayerTextData{nickName};
                            shouldProject = true;
                        }
                    }
                }
            }
        } else {
            nickName = it->second.text;
            shouldProject = !nickName.empty();
        }
    }

    // Pass the draw request to SluggerManager if this is a remote player
    if (shouldProject && __this->fields.pcPlayerHead) {
        UnityEngine_Transform_o *headTrans = UnityEngine::GameObject::get_transform(__this->fields.pcPlayerHead);
        if (headTrans) {
            UnityEngine_Vector3_o headPos = UnityEngine::Transform::get_position(headTrans);
            headPos.fields.y += kHeadWorldYOffset;

            if (auto slugger = sluggerManagerInfo.get<SluggerManager>()) {
                Slugger::SlugTextBuildOptions options;
                options.minYColor = {1.0f, 1.0f, 1.0f, 1.0f};
                options.maxYColor = {1.0f, 1.0f, 1.0f, 1.0f};
                options.centerOrigin = false;

                float minScale = kTargetFontSize * 0.60f;
                float maxScale = kTargetFontSize * 1.50f;

                // Draw billboard text
                slugger->drawBillboardText(nickName, headPos, kTargetFontSize, minScale, maxScale, options);
            }
        }
    }

    // Call original
    GameHookRelease GHR(self->player$$UpdateHook);
    self->player$$UpdateHook.getFunction<decltype(player$$UpdateFnc)>()(__this, method);
}

void player$$OnDisableFnc(Player_o *__this, const MethodInfo *method) {
    auto self = playerManagerInfo.get<PlayerManager>();
    if (!self)
        return;

    {
        std::lock_guard<std::mutex> lock(self->m_playerMutex);
        self->trackedPlayers.erase(__this);
    }

    GameHookRelease GHR(self->player$$OnDisableHook);
    self->player$$OnDisableHook.getFunction<decltype(player$$OnDisableFnc)>()(__this, method);
}

PlayerManager::PlayerManager()
    : player$$UpdateHook(GameHook::safeCreateOrPanic(playerManagerInfo, Il2Cpp::Player::Update_getPtr(), reinterpret_cast<void *>(player$$UpdateFnc))),
      player$$OnDisableHook(
          GameHook::safeCreateOrPanic(playerManagerInfo, Il2Cpp::Player::OnDisable_getPtr(), reinterpret_cast<void *>(player$$OnDisableFnc))) {}

PlayerManager::~PlayerManager() = default;

Player_o *PlayerManager::getLocalPlayer() const {
    std::lock_guard<std::mutex> lock(m_playerMutex);

    for (const auto& [player, data] : trackedPlayers) {
        if (!player || !player->fields.photonView)
            continue;

        if (player->fields.photonView->fields._AmOwner_k__BackingField)
            return player;
    }

    return nullptr;
}

ModInfo playerManagerInfo{"Player Manager", false, []() { return std::make_unique<PlayerManager>(); }};

#include "player_manager.hpp"
#include "il2cpp_cppinterop.hpp"
#include "global_instance_manager.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"

using namespace Il2Cpp::CppInterop;

void player$$UpdateFnc(Player_o *__this, const MethodInfo *method) {
    auto self = playerManagerInfo.get<PlayerManager>();

    // Track player if it isn't being tracked already
    if (self->trackedPlayers.find(__this) == self->trackedPlayers.end()) {
        g.logger->info("Registering player...");

        const auto error = [self, __this]() -> std::string_view {
            auto entry = self->trackedPlayers.emplace(__this, nullptr);

            // Stop here if local player
            if (__this->fields.photonView->fields._AmOwner_k__BackingField)
                return {};

            // Get all the info we need
            Photon_Pun_PhotonView_o *photonView = __this->fields.photonView;
            if (!photonView)
                return "Photon view unavailable";
            Photon_Realtime_Player_o *photonPlayer =
                photonView->fields._Owner_k__BackingField;
            if (!photonPlayer)
                return "Photon player unavailable";
            System_String_o *nickNameCs =
                Il2Cpp::Photon::Realtime::Player::get_NickName(photonPlayer,
                                                               nullptr);
            if (!nickNameCs)
                return "Nick name unavailable";
            const std::string nickName = ToCppString(nickNameCs);
            g.logger->debug("Registering player '{}'...", nickName);

            // Get Player object
            using namespace Il2Cpp::UnityEngine;
            UnityEngine_GameObject_o *playerObject =
                GameObject::get_parent(__this->fields.pcPlayerHead);

            // Create NameTag object
            UnityEngine_GameObject_o *nameObject = GameObject::New("NameTag");
            UnityEngine_Transform_o *nameTransform =
                GameObject::get_transform(nameObject);
            Transform::SetParent(
                nameTransform, GameObject::get_transform(playerObject), false);
            Transform::set_localPosition(nameTransform, {{0.0f, 1.0f, 0.0f}});

            // Add TextMesh
            auto textMesh = reinterpret_cast<UnityEngine_TextMesh_o *>(
                GameObject::AddComponent(nameObject, "UnityEngine.TextMesh",
                                         "UnityEngine.TextRenderingModule"));
            if (!textMesh)
                return "TextMesh component unavailable";
            TextMesh::set_text(textMesh, nickNameCs);
            TextMesh::set_characterSize(textMesh, 0.003);
            TextMesh::set_fontSize(textMesh, 1000);
            TextMesh::set_alignment(textMesh, TextAlignment::Center);
            TextMesh::set_anchor(textMesh, TextAnchor::MiddleCenter);

            entry.first->second = nameObject;
            return {};
        }();

        if (!error.empty())
            g.logger->error("Failed to register player: {}", error);
    }

    // Get current name tag of player
    UnityEngine_GameObject_o *nameObject = self->trackedPlayers[__this];

    // Turn NameTag towards player
    using namespace Il2Cpp::UnityEngine;
    if (nameObject) {
        Player_o *localPlayer = self->getLocalPlayer();
        if (localPlayer) {
            UnityEngine_Transform_o *nameTransform =
                GameObject::get_transform(nameObject);
            Transform::LookAt(
                nameTransform,
                GameObject::get_transform(localPlayer->fields.pcPlayerHead));
            Transform::Rotate(nameTransform, {{0, 180, 0}});
        }
    }

    // Invoke actual function
    GameHookRelease GHR(self->player$$UpdateHook);
    self->player$$UpdateHook.getFunction<decltype(player$$UpdateFnc)>()(__this,
                                                                        method);
}

void player$$OnDisableFnc(Player_o *__this, const MethodInfo *method) {
    auto self = playerManagerInfo.get<PlayerManager>();

    // Remove from list of tracked players
    auto res = self->trackedPlayers.find(__this);
    if (res != self->trackedPlayers.end())
        self->trackedPlayers.erase(res);

    // Invoke actual function
    GameHookRelease GHR(self->player$$OnDisableHook);
    self->player$$OnDisableHook.getFunction<decltype(player$$OnDisableFnc)>()(
        __this, method);
}

PlayerManager::PlayerManager()
    : player$$UpdateHook(GameHook::safeCreateOrPanic(
          playerManagerInfo, Il2Cpp::Player::Update_getPtr(),
          reinterpret_cast<void *>(player$$UpdateFnc))),
      player$$OnDisableHook(GameHook::safeCreateOrPanic(
          playerManagerInfo, Il2Cpp::Player::OnDisable_getPtr(),
          reinterpret_cast<void *>(player$$OnDisableFnc))) {}

PlayerManager::~PlayerManager() {
    for (auto [player, gameObject] : trackedPlayers)
        if (gameObject)
            Il2Cpp::UnityEngine::Object::Destroy(
                reinterpret_cast<UnityEngine_Object_o *>(gameObject));
}

Player_o *PlayerManager::getLocalPlayer() const {
    for (auto [player, gameObject] : trackedPlayers)
        if (player->fields.photonView->fields._AmOwner_k__BackingField)
            return player;
    return nullptr;
}

ModInfo playerManagerInfo{"Player Manager", false,
                          []() { return std::make_unique<PlayerManager>(); }};

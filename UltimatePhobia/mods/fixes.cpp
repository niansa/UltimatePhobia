#include "fixes.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"

#include <string_view>

using namespace Il2Cpp::CppInterop;
using namespace Il2Cpp::UnityEngine;



static inline void fixDestroy(std::string_view name) {
    if (auto object = GameObject::Find(ToCsString(name)))
        Object::Destroy(reinterpret_cast<UnityEngine_Object_o *>(object));
}

static inline void fixPlayerController(Player_o *player) {
    if (player->fields.characterController)
        Il2Cpp::UnityEngine::CharacterController::set_detectCollisions(player->fields.characterController, false);
}


static bool photonNetwork$$ConnectToBestCloudServerFnc(const MethodInfo* method) {
    // TODO: Use last region instead
    return Il2Cpp::Photon::Pun::PhotonNetwork::ConnectToRegion("EU"_cs);
}


Fixes::Fixes()
    : photonNetwork$$ConnectToBestCloudServerHook(GameHook::safeCreateOrPanic(fixesInfo,
          Il2Cpp::Photon::Pun::PhotonNetwork::ConnectToBestCloudServer_getPtr(),
          reinterpret_cast<void*>(photonNetwork$$ConnectToBestCloudServerFnc)
          )) {}

bool Fixes::isSceneFixed() {
    return fixMark == GameObject::Find("UP_fixes_fixMark"_cs);
}

void Fixes::markSceneFixed() {
    fixMark = GameObject::New("UP_fixes_fixMark");
}

void Fixes::sceneFix() {
    // Mark as fixed first so exceptions during fixup don't cause infinite havoc
    markSceneFixed();

    // Do fixups
    g.logger->info("Fixing scene...");
    fixDestroy("/_House/_Second Floor/_Girls Bedroom/_Lighting/floor lamp (1)");
}

void Fixes::playerFix() {
    if (auto player = globalInstanceManagerInfo.get<GlobalInstanceManager>()->player)
        fixPlayerController(player);
}

void Fixes::uiUpdate() {
    if (!isSceneFixed())
        sceneFix();
    playerFix();
}


ModInfo fixesInfo {
    "Fixes",
    true,
    [] () {return std::make_unique<Fixes>();}
};

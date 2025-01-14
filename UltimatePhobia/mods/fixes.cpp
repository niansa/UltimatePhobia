#include "fixes.hpp"
#include "game_types.hpp"
#include "global_instance_manager.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"

#include <string_view>

using namespace UnityEngine;



static inline void fixDestroy(std::string_view name) {
    auto object = GameObject::Find(name);
    if (!object)
        return;
    GameObject::Destroy(object);
}

static inline void fixPlayerController(Player_o *player) {
    Il2Cpp::Methods::UnityEngine_CharacterController__set_detectCollisions(player->fields.characterController, player);
}


static bool photonNetwork$$ConnectToBestCloudServerFnc(const MethodInfo* method) {
    // TODO: Use last region instead
    return Il2Cpp::Methods::Photon_Pun_PhotonNetwork__ConnectToRegion(GameTypes::createCsString("EU"));
}


Fixes::Fixes()
    : photonNetwork$$ConnectToBestCloudServerHook(
          Il2Cpp::Methods::Photon_Pun_PhotonNetwork__ConnectToBestCloudServer_getPtr(),
          reinterpret_cast<void*>(photonNetwork$$ConnectToBestCloudServerFnc)
          ) {}

bool Fixes::isSceneFixed() {
    return fixMark == GameObject::Find("UP_fixes_fixMark");
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
    fixPlayerController(globalInstanceManagerInfo.get<GlobalInstanceManager>()->player);
}

void Fixes::uiUpdate() {
    if (!isSceneFixed())
        sceneFix();
}


ModInfo fixesInfo {
    "Fixes",
    true,
    [] () {return std::make_unique<Fixes>();}
};

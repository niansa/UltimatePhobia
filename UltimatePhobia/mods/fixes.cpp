#include "fixes.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"
#include "bindings/unityengine.hpp"

#include <string_view>

using namespace UnityEngine;



static inline void fixDestroy(std::string_view name) {
    auto object = GameObject::Find(name);
    if (!object)
        return;
    GameObject::Destroy(object);
}


static bool photonNetwork$$ConnectToBestCloudServerFnc(const MethodInfo* method) {
    // TODO: Use last region instead
    return GameData::getMethod("Photon.Pun.PhotonNetwork$$ConnectToRegion")
        .getFunction<bool (System_String_o *, const MethodInfo *)>()
        (GameTypes::createCsString("EU"), nullptr);
}


Fixes::Fixes()
    : photonNetwork$$ConnectToBestCloudServerHook(
          GameData::getMethod("Photon.Pun.PhotonNetwork$$ConnectToBestCloudServer").address,
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

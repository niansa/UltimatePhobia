#include "fixes.hpp"
#include "global_instance_manager.hpp"
#include "gamedata.hpp"
#include "bindings/unityengine.hpp"

#include <string_view>

using namespace UnityEngine;



static inline void fixDestroy(std::string_view name) {
    auto object = GameObject::Find(name);
    if (!object)
        return;
    GameObject::Destroy(object);
}


Fixes::Fixes() {}

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
    // /_House/_Exterior/_Props/Porch_Rail_2x (1)
    // /_House/_Exterior/_Props/Porch_Rail_2x
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

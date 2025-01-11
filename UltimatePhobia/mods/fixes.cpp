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


static UnityEngine_Vector3_o characterController$$get_velocityFnc(UnityEngine_CharacterController_o *__this, const MethodInfo *method) {
    auto self = fixesInfo.get<Fixes>();

    if (self->velocity < 0.0065f)
        return Vector3::Zero;

    GameHookRelease GHR(self->characterController$$get_velocityHook);
    return self->characterController$$get_velocityHook
        .getFunction<decltype(characterController$$get_velocityFnc)>()
        (__this, method);
}


Fixes::Fixes()
    : characterController$$get_velocityHook(
          GameData::getMethod("UnityEngine.CharacterController$$get_velocity").address,
          reinterpret_cast<void*>(characterController$$get_velocityFnc)
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

void Fixes::updateVelocity() {
    Player_o *player = globalInstanceManagerInfo.get<GlobalInstanceManager>()->player;
    if (!player)
        return;

    // Get player transform
    auto playerTrans = GameObject::get_transform(Component::get_gameObject(Component::From(player)));

    // Get player position
    const auto playerWorldPos = Transform::get_position(playerTrans).fields;

    // Calculate velocity
    velocity = Vector2::Distance(Vector2::From({playerWorldPos}), Vector2::From({lastPlayerWorldPos}));
    distanceWalked += velocity;

    lastPlayerWorldPos = playerWorldPos;
}

void Fixes::uiUpdate() {
    if (!isSceneFixed())
        sceneFix();

    updateVelocity();
}


ModInfo fixesInfo {
    "Fixes",
    [] () {return std::make_unique<Fixes>();}
};

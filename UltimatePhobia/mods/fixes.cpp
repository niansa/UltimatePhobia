#include "fixes.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_api_cpp.hpp"

#include <string_view>

using Il2Cpp::API::CachedMethodLookup;
using Il2Cpp::API::call;
using Il2Cpp::API::get_class_cached;
using Il2Cpp::API::Object;
using Il2Cpp::API::object_new;
using Il2Cpp::API::object_unbox;

static inline void fixDestroy(std::string_view name) {
    auto gameObject = get_class_cached<"UnityEngine.CoreModule", "UnityEngine", "GameObject">();
    if (auto object = call(gameObject, "Find", name))
        call(gameObject, "Destroy", object);
}

static inline void fixPlayerController(Object player) {
    static auto field = player.klass().get_field(get_class_cached<"UnityEngine.PhysicsModule", "UnityEngine", "CharacterController">());
    static CachedMethodLookup cml("set_detectCollisions");
    if (auto characterController = field.get_value_object(player))
        call(characterController, cml, false);
}

static bool photonNetwork$$ConnectToBestCloudServerFnc(const MethodInfo *method) {
    // TODO: Use last region instead
    auto photonNet = get_class_cached<"PhotonUnityNetworking", "Photon.Pun", "PhotonNetwork">();
    return object_unbox<bool>(call(photonNet, "ConnectToRegion", "EU"));
}

Fixes::Fixes()
    : photonNetwork$$ConnectToBestCloudServerHook(GameHook::safeCreateOrPanic(
          fixesInfo, get_class_cached<"PhotonUnityNetworking", "Photon.Pun", "PhotonNetwork">().get_method("ConnectToBestCloudServer", 0).function_pointer(),
          reinterpret_cast<void *>(photonNetwork$$ConnectToBestCloudServerFnc))) {}

bool Fixes::isSceneFixed() {
    static CachedMethodLookup cml("Find");
    return fixMark == call(get_class_cached<"UnityEngine.CoreModule", "UnityEngine", "GameObject">(), cml, "UP_fixes_fixMark").ptr;
}

void Fixes::markSceneFixed() {
    Object markObject = object_new(get_class_cached<"UnityEngine.CoreModule", "UnityEngine", "GameObject">());
    call(markObject, ".ctor", "UP_fixes_fixMark");
    fixMark = markObject.ptr;
}

void Fixes::sceneFix() {
    // Mark as fixed first so exceptions during fixup don't cause infinite havoc
    markSceneFixed();

    // Do fixups
    g.logger->info("Fixing scene...");
    fixDestroy("/_House/_Second Floor/_Girls Bedroom/_Lighting/floor lamp (1)");
}

void Fixes::playerFix() {
    if (auto player = globalInstanceManagerInfo.get<GlobalInstanceManager>()->getPlayer())
        fixPlayerController(Object{reinterpret_cast<Il2Cpp::API::Il2CppObject *>(player)});
}

void Fixes::uiUpdate() {
    if (!isSceneFixed())
        sceneFix();
    playerFix();
}

ModInfo fixesInfo{"Fixes", true, []() { return std::make_unique<Fixes>(); }};

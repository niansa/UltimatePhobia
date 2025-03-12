#include "simulation_environment.hpp"
#include "SteamAudioImpl.hpp"
#include "phonontools.hpp"
#include "utils.hpp"

#include <vector>
#include <map>
#include <phonon.h>
#include <ffi_interface.hpp>

using namespace FFIInterface;
using namespace Helpers;

namespace PhononSimulation {
std::optional<Environment> env;

struct SubScene {
    IPLScene subScene = nullptr;
    IPLInstancedMesh instancedMesh = nullptr;
    IPLStaticMesh staticMesh = nullptr;

    SubScene() {}
    ~SubScene() {
        if (instancedMesh) {
            iplInstancedMeshRemove(instancedMesh, env->getScene());
            iplInstancedMeshRelease(&instancedMesh);
        }
        if (staticMesh) {
            iplStaticMeshRemove(staticMesh, subScene);
            iplStaticMeshRelease(&staticMesh);
        }
        if (subScene) {
            iplSceneRelease(&subScene);
        }
    }
    SubScene(const SubScene&) = delete;
    SubScene(SubScene&& o) : subScene(o.subScene), instancedMesh(o.instancedMesh), staticMesh(o.staticMesh) {
        o.subScene = nullptr;
        o.instancedMesh = nullptr;
        o.staticMesh = nullptr;
    }
};

struct Environment::Impl {
    IPLScene scene;
    std::map<ObjectHandle, GCHandle> gcHandles;
    std::map<ObjectHandle, SubScene> meshes;

    Impl() {
        IPLSceneSettings sceneSettings{IPL_SCENETYPE_DEFAULT};
        auto status = iplSceneCreate(GlobalState::phononCtx, &sceneSettings, &scene);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage("create scene", status));
    }
    ~Impl() { iplSceneRelease(&scene); }
    Impl(const Impl&) = delete;
    Impl(Impl&&) = delete;
};

Environment::Environment() { i = new Impl; }
Environment::~Environment() { delete i; }

void Environment::updateScene() {
    using namespace Literals;

    // Clean up dead objects
    {
        std::vector<ObjectHandle> deletionQueue;
        for (auto [object, gcHandle] : i->gcHandles) {
            if (call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(object, nullptr) && !call_error)
                continue;

            // Object is dead, clean up remainings
            deletionQueue.push_back(object);
        }
        for (const ObjectHandle object : deletionQueue) {
            auto res = i->gcHandles.find(object);
            FFI gcDeleteHandle(res->second);
            FFI dropObject(object);
            i->meshes.erase(object);
            i->gcHandles.erase(res);
        }
    }

    // Get MeshCollider type
    const auto meshColliderType = call<"System_Type_o* System_Type__GetType (System_String_o* typeName, const MethodInfo* method);", ObjectHandle>(
        "UnityEngine.MeshCollider, UnityEngine.PhysicsModule, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null"_cs, nullptr);

    // Get mesh colliders
    const auto meshColliders = call<"UnityEngine.Object$$FindObjectsOfType", ObjectHandle>(meshColliderType, nullptr);
    const int32_t meshColliderCount = call<"System.Array$$get_Length", int32_t>(meshColliders, nullptr); // This will leak the object when unloading mod

    // Handle each collider
    for (int32_t idx = 0; idx != meshColliderCount; ++idx) {
        // Get individual mesh collider
        const auto meshCollider = call<"Il2CppObject* System_Array__GetValue (System_Array_o* __this, int64_t index, const MethodInfo* method);", ObjectHandle>(
            meshColliders, idx, nullptr);
        if (meshCollider == ObjectHandle::Null) {
            FFI logError("Invalid array entry in mesh collider component list"_cs);
            continue;
        }

        // Get mesh
        const auto mesh = call<"UnityEngine.MeshCollider$$get_sharedMesh", ObjectHandle>(meshCollider, nullptr);
        FFI dropObject(meshCollider);

        // Skip if mesh is already known
        bool alreadyExists = false;
        for (auto& [otherMesh, _] : i->meshes) {
            if (call<"UnityEngine.Object$$Equals", bool>(mesh, otherMesh, nullptr)) {
                alreadyExists = true;
                break;
            }
        }
        if (alreadyExists) {
            FFI dropObject(mesh);
            continue;
        }

        // Create sub scene
        auto subScene = [mesh]() -> SubScene {
            SubScene subScene;

            // Create scene
            IPLSceneSettings sceneSettings{IPL_SCENETYPE_DEFAULT};
            auto status = iplSceneCreate(GlobalState::phononCtx, &sceneSettings, &subScene.subScene);
            if (status != IPL_STATUS_SUCCESS) {
                FFI logError(Utils::createErrorMessage("create subscene", status));
                return {};
            }

            // Create static mesh
            subScene.staticMesh = PhononTools::staticMeshfromMesh(subScene.subScene, mesh);
            if (subScene.staticMesh == nullptr)
                return {};
            iplStaticMeshAdd(subScene.staticMesh, subScene.subScene);

            // Create instanced mesh
            IPLInstancedMeshSettings instancedMeshSettings{.subScene = subScene.subScene, .transform = {}};
            status = iplInstancedMeshCreate(env->getScene(), &instancedMeshSettings, &subScene.instancedMesh);
            if (status != IPL_STATUS_SUCCESS) {
                FFI logError(Utils::createErrorMessage("create instanced mesh", status));
                return {};
            }
            iplInstancedMeshAdd(subScene.instancedMesh, env->getScene());

            return subScene;
        }();

        // Add mesh to lists
        i->meshes.emplace(mesh, std::move(subScene));
        i->gcHandles.emplace(mesh, FFI gcCreateHandle(mesh, true));
    }
    FFI dropObject(meshColliders);
}

IPLScene Environment::getScene() { return i->scene; }
unsigned int Environment::getMeshCount() { return i->meshes.size(); }
} // namespace PhononSimulation

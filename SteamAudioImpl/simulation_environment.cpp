#include "simulation_environment.hpp"
#include "SteamAudioImpl.hpp"
#include "phonontools.hpp"
#include "utils.hpp"

#include <string>
#include <vector>
#include <map>
#include <cstring>
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
    IPLMatrix4x4 matrix;

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
    IPLSimulator simulator;
    std::map<ObjectHandle, GCHandle> gcHandles;
    std::map<ObjectHandle, SubScene> meshes;

    Impl() {
        IPLSceneSettings sceneSettings{IPL_SCENETYPE_DEFAULT};
        auto status = iplSceneCreate(GlobalState::phononCtx, &sceneSettings, &scene);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage("create scene", status));

        IPLSimulationSettings simulationSettings{};
        simulationSettings.flags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT);
        simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;

        IPLSimulator simulator = nullptr;
        iplSimulatorCreate(GlobalState::phononCtx, &simulationSettings, &simulator);
        iplSimulatorSetScene(simulator, scene);
        iplSimulatorCommit(simulator);
    }
    ~Impl() { iplSceneRelease(&scene); }
    Impl(const Impl&) = delete;
    Impl(Impl&&) = delete;
};

Environment::Environment() { i = new Impl; }
Environment::~Environment() { delete i; }

void Environment::updateScene() {
    using namespace Literals;

    {
        std::vector<ObjectHandle> deletionQueue;
        // Update transforms
        for (auto& [object, mesh] : i->meshes) {
            // Make sure object isn't dead
            if (!call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(object, nullptr) || call_error) {
                deletionQueue.push_back(object);
                continue;
            }

            // Get transform
            const auto transform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(object, nullptr);

            // Get matrix
            const auto matrix = PhononTools::TransformUtils::get_localToWorldMatrix(transform);

            // Update internal matrix if updated
            if (std::memcmp(matrix.elements, mesh.matrix.elements, sizeof(matrix.elements)) != 0) {
                iplInstancedMeshUpdateTransform(mesh.instancedMesh, env->getScene(), matrix);
                mesh.matrix = matrix;
            }

            FFI dropObject(transform);
        }
        // Clean up dead objects collected on the way
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
    if (meshColliders != ObjectHandle::Null) {
        const int32_t meshColliderCount = call<"System.Array$$get_Length", int32_t>(meshColliders, nullptr);

        // Handle each collider
        bool sceneDirty = false;
        for (int32_t idx = 0; idx != meshColliderCount; ++idx) {
            // Get individual mesh collider
            const auto meshCollider =
                call<"Il2CppObject* System_Array__GetValue (System_Array_o* __this, int64_t index, const MethodInfo* method);", ObjectHandle>(meshColliders,
                                                                                                                                              idx, nullptr);
            if (meshCollider == ObjectHandle::Null) {
                FFI logError("Invalid array entry in mesh collider component list"_cs);
                continue;
            }

            // Get mesh
            const auto mesh = call<"UnityEngine.MeshCollider$$get_sharedMesh", ObjectHandle>(meshCollider, nullptr);

            // Get transform
            const auto transform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(meshCollider, nullptr);

            // Skip if mesh is already known
            bool alreadyExists = false;
            for (auto& [otherMeshCollider, _] : i->meshes) {
                if (call<"UnityEngine.Object$$Equals", bool>(meshCollider, otherMeshCollider, nullptr)) {
                    alreadyExists = true;
                    break;
                }
            }
            if (alreadyExists) {
                FFI dropObject(mesh);
                continue;
            }

            // Create sub scene
            auto subScene = [mesh, transform, &sceneDirty]() -> SubScene {
                SubScene subScene;

                // Get matrix
                subScene.matrix = PhononTools::TransformUtils::get_localToWorldMatrix(transform);

                // Create scene
                IPLSceneSettings sceneSettings{IPL_SCENETYPE_DEFAULT};
                auto status = iplSceneCreate(GlobalState::phononCtx, &sceneSettings, &subScene.subScene);
                if (status != IPL_STATUS_SUCCESS) {
                    FFI logError(Utils::createErrorMessage("create subscene", status));
                    return {};
                }

                // Create static mesh
                subScene.staticMesh = PhononTools::staticMeshFromMesh(subScene.subScene, mesh);
                if (subScene.staticMesh == nullptr)
                    return {};
                iplStaticMeshAdd(subScene.staticMesh, subScene.subScene);
                iplSceneCommit(subScene.subScene);

                // Create instanced mesh
                IPLInstancedMeshSettings instancedMeshSettings{.subScene = subScene.subScene, .transform = subScene.matrix};
                status = iplInstancedMeshCreate(env->getScene(), &instancedMeshSettings, &subScene.instancedMesh);
                if (status != IPL_STATUS_SUCCESS) {
                    FFI logError(Utils::createErrorMessage("create instanced mesh", status));
                    return {};
                }
                iplInstancedMeshAdd(subScene.instancedMesh, env->getScene());
                sceneDirty = true;

                static unsigned idx = 0;
                iplSceneSaveOBJ(subScene.subScene, ("Z:\\tmp\\phonons\\scene-" + std::to_string(idx++) + ".obj").c_str());
                return subScene;
            }();
            FFI dropObject(mesh);
            FFI dropObject(transform);

            // Add mesh to lists
            i->meshes.emplace(meshCollider, std::move(subScene));
            i->gcHandles.emplace(meshCollider, FFI gcCreateHandle(meshCollider, true));
        }
        FFI dropObject(meshColliders);

        if (sceneDirty)
            iplSceneCommit(env->getScene());
    } else {
        FFI logError("Failed to get mesh colliders!"_cs);
    }
}

IPLScene Environment::getScene() { return i->scene; }
unsigned int Environment::getMeshCount() { return i->meshes.size(); }
} // namespace PhononSimulation

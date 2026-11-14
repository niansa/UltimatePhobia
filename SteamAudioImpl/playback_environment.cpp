#include "playback_environment.hpp"

#include "SteamAudioImpl.hpp"
#include "fixedsettings.hpp"
#include "phonontools.hpp"
#include "playback.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>
#include <ffi_interface.hpp>

#include <shared_mutex>
#include <system_error>
using namespace FFIInterface;
using namespace Helpers;

namespace PhononPlayback {
std::optional<Environment> env;
RayTracer rayTracer;

namespace {
void logIplError(const char *operation, IPLerror status) {
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(Utils::createErrorMessage(operation, status));
}

bool isValidObject(ObjectHandle object) { return object != ObjectHandle::Null && object != ObjectHandle::Invalid; }

double secondsSince(RayTracer::Clock::time_point now, RayTracer::Clock::time_point then) {
    if (then.time_since_epoch().count() == 0)
        return 1.0e9;
    return std::chrono::duration<double>(now - then).count();
}
} // namespace

Environment::Environment(IPLint32 frameSize)
    : bufferPool(FixedSettings::outputChannels, frameSize), monoBufferPool(1, frameSize),
      ambisonicsBufferPool(FixedSettings::ambisonicsChannels, frameSize) {
    audioSettings.frameSize = frameSize;
    audioSettings.samplingRate = static_cast<IPLint32>(GlobalState::maDevice.sampleRate);

    IPLerror status =
        iplAudioBufferAllocate(GlobalState::phononCtx, FixedSettings::outputChannels, frameSize, &frameBuffer);
    logIplError("allocate the output mix buffer", status);

    IPLHRTFSettings hrtfSettings{};
    hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
    hrtfSettings.volume = 1.0f;
    status = iplHRTFCreate(GlobalState::phononCtx, &audioSettings, &hrtfSettings, &hrtf);
    logIplError("create the HRTF", status);

    IPLReflectionEffectSettings reflectionSettings{};
    reflectionSettings.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    reflectionSettings.irSize =
        static_cast<IPLint32>(std::ceil(audioSettings.samplingRate * FixedSettings::reflectionDurationSeconds));
    reflectionSettings.numChannels = FixedSettings::ambisonicsChannels;
    status = iplReflectionMixerCreate(GlobalState::phononCtx, &audioSettings, &reflectionSettings, &reflectionMixer);
    logIplError("create the reflection mixer", status);

    IPLAmbisonicsDecodeEffectSettings decodeSettings{};
    decodeSettings.maxOrder = FixedSettings::ambisonicsOrder;
    decodeSettings.hrtf = hrtf;
    status = iplAmbisonicsDecodeEffectCreate(GlobalState::phononCtx, &audioSettings, &decodeSettings,
                                             &ambisonicsDecodeEffect);
    logIplError("create the ambisonics decoder", status);
}

Environment::~Environment() {
    if (ambisonicsDecodeEffect)
        iplAmbisonicsDecodeEffectRelease(&ambisonicsDecodeEffect);
    if (reflectionMixer)
        iplReflectionMixerRelease(&reflectionMixer);
    if (hrtf)
        iplHRTFRelease(&hrtf);
    if (frameBuffer.data)
        iplAudioBufferFree(GlobalState::phononCtx, &frameBuffer);
}

RayTracer::~RayTracer() { Shutdown(); }

bool RayTracer::Initialize() {
    if (IsInitialized())
        return true;

    IPLSceneSettings sceneSettings{};
    sceneSettings.type = IPL_SCENETYPE_DEFAULT;

    IPLerror status = iplSceneCreate(GlobalState::phononCtx, &sceneSettings, &scene_);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create the Steam Audio ray tracing scene", status));
        scene_ = nullptr;
        return false;
    }

    const IPLint32 samplingRate = static_cast<IPLint32>(GlobalState::maDevice.sampleRate);
    const IPLint32 frameSize = GlobalState::maDevice.playback.internalPeriodSizeInFrames > 0
                                   ? static_cast<IPLint32>(GlobalState::maDevice.playback.internalPeriodSizeInFrames)
                                   : 1024;
    if (samplingRate <= 0) {
        iplSceneRelease(&scene_);
        return false;
    }

    IPLSimulationSettings simulationSettings{};
    simulationSettings.flags =
        static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
    simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
    simulationSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    simulationSettings.maxNumOcclusionSamples = FixedSettings::maxOcclusionSamples;
    simulationSettings.maxNumRays = FixedSettings::maxReflectionRays;
    simulationSettings.numDiffuseSamples = 32;
    simulationSettings.maxDuration = FixedSettings::reflectionDurationSeconds;
    simulationSettings.maxOrder = FixedSettings::ambisonicsOrder;
    simulationSettings.maxNumSources = FixedSettings::maxRaytracedSources;
    simulationSettings.samplingRate = samplingRate;
    simulationSettings.frameSize = frameSize;

    const unsigned hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
    simulationSettings.numThreads = static_cast<IPLint32>(std::min(hardwareThreads, 4u));
    simulationSettings.rayBatchSize = 64;

    status = iplSimulatorCreate(GlobalState::phononCtx, &simulationSettings, &simulator_);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create the Steam Audio simulator", status));
        iplSceneRelease(&scene_);
        simulator_ = nullptr;
        return false;
    }

    iplSceneCommit(scene_);
    iplSimulatorSetScene(simulator_, scene_);
    iplSimulatorCommit(simulator_);

    const auto now = Clock::now();
    lastTransformUpdate_ = now;
    lastColliderScan_ = Clock::time_point{};
    lastReflectionUpdate_ = Clock::time_point{};

    if (!StartReflectionWorker()) {
        Shutdown();
        return false;
    }

    return true;
}

void RayTracer::Shutdown() {
    StopReflectionWorker();
    std::unique_lock simulationLock(simulationMutex_);
    for (auto& [id, collider] : colliders_) {
        if (collider.instance) {
            if (scene_ && collider.active)
                iplInstancedMeshRemove(collider.instance, scene_);
            iplInstancedMeshRelease(&collider.instance);
        }

        if (isValidObject(collider.object)) {
            FFI dropObject(collider.object);
            FFI gcDeleteHandle(collider.gcHandle);
        }
    }
    colliders_.clear();

    for (auto& [id, geometry] : geometries_) {
        if (geometry.staticMesh) {
            if (geometry.subScene)
                iplStaticMeshRemove(geometry.staticMesh, geometry.subScene);
            iplStaticMeshRelease(&geometry.staticMesh);
        }
        if (geometry.subScene)
            iplSceneRelease(&geometry.subScene);
    }
    geometries_.clear();

    if (isValidObject(meshColliderType_))
        FFI dropObject(meshColliderType_);
    meshColliderType_ = ObjectHandle::Invalid;

    if (isValidObject(boxColliderType_))
        FFI dropObject(boxColliderType_);
    boxColliderType_ = ObjectHandle::Invalid;

    if (isValidObject(meshRendererType_))
        FFI dropObject(meshRendererType_);
    meshRendererType_ = ObjectHandle::Invalid;

    unreadableMeshes_.clear();

    if (simulator_) {
        iplSimulatorSetScene(simulator_, nullptr);
        iplSimulatorCommit(simulator_);
        iplSimulatorRelease(&simulator_);
    }

    if (scene_)
        iplSceneRelease(&scene_);

    sceneDirty_ = false;
    reflectionsReady_ = false;
}

bool RayTracer::ResolveColliderTypes() {
    if (!isValidObject(meshRendererType_)) {
        static constexpr const char *rendererNames[] = {"UnityEngine.MeshRenderer, UnityEngine.CoreModule",
                                                        "UnityEngine.MeshRenderer, UnityEngine"};
        for (const char *typeName : rendererNames) {
            ObjectHandle name = FFI toCsString(typeName);
            ObjectHandle type =
                call<"System_Type_o* System_Type__GetType (System_String_o* typeName, const MethodInfo* method);",
                     ObjectHandle>(name, nullptr);
            FFI dropObject(name);
            if (isValidObject(type)) {
                meshRendererType_ = type;
                break;
            }
        }
    }

    if (!isValidObject(boxColliderType_)) {
        static constexpr const char *boxTypeNames[] = {"UnityEngine.BoxCollider, UnityEngine.PhysicsModule",
                                                       "UnityEngine.BoxCollider, UnityEngine"};
        for (const char *typeName : boxTypeNames) {
            ObjectHandle name = FFI toCsString(typeName);
            ObjectHandle type =
                call<"System_Type_o* System_Type__GetType (System_String_o* typeName, const MethodInfo* method);",
                     ObjectHandle>(name, nullptr);
            FFI dropObject(name);

            if (isValidObject(type)) {
                boxColliderType_ = type;
                break;
            }
        }
    }

    if (isValidObject(meshColliderType_))
        return true;

    static constexpr const char *typeNames[] = {"UnityEngine.MeshCollider, UnityEngine.PhysicsModule",
                                                "UnityEngine.MeshCollider, UnityEngine"};

    for (const char *typeName : typeNames) {
        ObjectHandle name = FFI toCsString(typeName);
        ObjectHandle type =
            call<"System_Type_o* System_Type__GetType (System_String_o* typeName, const MethodInfo* method);",
                 ObjectHandle>(name, nullptr);
        FFI dropObject(name);

        if (isValidObject(type)) {
            meshColliderType_ = type;
            return true;
        }
    }

    FFI logWarn(FFI toCsString("Steam Audio could not resolve UnityEngine.MeshCollider. "
                               "Collision-scene ray tracing has been disabled."));
    return false;
}

RayTracer::Geometry *RayTracer::AcquireGeometry(ObjectHandle mesh, int meshId) {
    auto existing = geometries_.find(meshId);
    if (existing != geometries_.end()) {
        ++existing->second.references;
        return &existing->second;
    }

    IPLSceneSettings settings{};
    settings.type = IPL_SCENETYPE_DEFAULT;

    Geometry geometry{};
    IPLerror status = iplSceneCreate(GlobalState::phononCtx, &settings, &geometry.subScene);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create a collision mesh sub-scene", status));
        return nullptr;
    }

    geometry.staticMesh = PhononTools::staticMeshFromMesh(geometry.subScene, mesh);
    if (!geometry.staticMesh) {
        iplSceneRelease(&geometry.subScene);
        return nullptr;
    }

    iplStaticMeshAdd(geometry.staticMesh, geometry.subScene);
    iplSceneCommit(geometry.subScene);
    geometry.references = 1;

    auto [it, inserted] = geometries_.emplace(meshId, geometry);
    if (!inserted) {
        iplStaticMeshRemove(geometry.staticMesh, geometry.subScene);
        iplStaticMeshRelease(&geometry.staticMesh);
        iplSceneRelease(&geometry.subScene);
        ++it->second.references;
    }
    return &it->second;
}

void RayTracer::ReleaseGeometry(int meshId) {
    const auto found = geometries_.find(meshId);
    if (found == geometries_.end())
        return;

    Geometry& geometry = found->second;
    if (geometry.references > 1) {
        --geometry.references;
        return;
    }

    if (geometry.staticMesh) {
        iplStaticMeshRemove(geometry.staticMesh, geometry.subScene);
        iplStaticMeshRelease(&geometry.staticMesh);
    }
    if (geometry.subScene)
        iplSceneRelease(&geometry.subScene);

    geometries_.erase(found);
}

bool RayTracer::AddCollider(ObjectHandle collider, int colliderId, ObjectHandle mesh, int meshId) {
    Geometry *geometry = AcquireGeometry(mesh, meshId);
    if (!geometry)
        return false;

    Collider record{};
    record.object = collider;
    record.gcHandle = FFI gcCreateHandle(collider, true);
    record.meshId = meshId;

    ObjectHandle transform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(collider, nullptr);
    if (!isValidObject(transform)) {
        FFI dropObject(record.object);
        FFI gcDeleteHandle(record.gcHandle);
        ReleaseGeometry(meshId);
        return false;
    }

    record.transform = PhononTools::TransformUtils::get_localToWorldMatrix(transform);
    FFI dropObject(transform);

    IPLInstancedMeshSettings instanceSettings{};
    instanceSettings.subScene = geometry->subScene;
    instanceSettings.transform = record.transform;

    IPLerror status = iplInstancedMeshCreate(scene_, &instanceSettings, &record.instance);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create an instanced collision mesh", status));
        FFI dropObject(record.object);
        FFI gcDeleteHandle(record.gcHandle);
        ReleaseGeometry(meshId);
        return false;
    }

    const bool enabled = call<"UnityEngine.Collider$$get_enabled", bool>(collider, nullptr);
    ObjectHandle gameObject = call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(collider, nullptr);
    const bool active = isValidObject(gameObject)
                            ? call<"UnityEngine.GameObject$$get_activeInHierarchy", bool>(gameObject, nullptr)
                            : false;
    if (isValidObject(gameObject))
        FFI dropObject(gameObject);

    if (enabled && active) {
        iplInstancedMeshAdd(record.instance, scene_);
        record.active = true;
        sceneDirty_ = true;
    }

    colliders_.emplace(colliderId, record);
    return true;
}

RayTracer::Geometry *RayTracer::AcquireBoxGeometry(ObjectHandle boxCollider, int colliderId) {
    auto existing = geometries_.find(colliderId);
    if (existing != geometries_.end()) {
        ++existing->second.references;
        return &existing->second;
    }

    IPLSceneSettings settings{};
    settings.type = IPL_SCENETYPE_DEFAULT;

    Geometry geometry{};
    IPLerror status = iplSceneCreate(GlobalState::phononCtx, &settings, &geometry.subScene);
    if (status != IPL_STATUS_SUCCESS)
        return nullptr;

    geometry.staticMesh = PhononTools::staticMeshFromBoxCollider(geometry.subScene, boxCollider);
    if (!geometry.staticMesh) {
        iplSceneRelease(&geometry.subScene);
        return nullptr;
    }

    iplStaticMeshAdd(geometry.staticMesh, geometry.subScene);
    iplSceneCommit(geometry.subScene);
    geometry.references = 1;

    geometries_.emplace(colliderId, geometry);
    return &geometries_[colliderId];
}

bool RayTracer::AddBoxCollider(ObjectHandle collider, int colliderId) {
    Geometry *geometry = AcquireBoxGeometry(collider, colliderId);
    if (!geometry)
        return false;

    Collider record{};
    record.object = collider;
    record.gcHandle = FFI gcCreateHandle(collider, true);
    record.meshId = colliderId;

    ObjectHandle transform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(collider, nullptr);
    if (!isValidObject(transform)) {
        FFI dropObject(record.object);
        FFI gcDeleteHandle(record.gcHandle);
        ReleaseGeometry(colliderId);
        return false;
    }

    record.transform = PhononTools::TransformUtils::get_localToWorldMatrix(transform);
    FFI dropObject(transform);

    IPLInstancedMeshSettings instanceSettings{};
    instanceSettings.subScene = geometry->subScene;
    instanceSettings.transform = record.transform;

    IPLerror status = iplInstancedMeshCreate(scene_, &instanceSettings, &record.instance);
    if (status != IPL_STATUS_SUCCESS) {
        FFI dropObject(record.object);
        FFI gcDeleteHandle(record.gcHandle);
        ReleaseGeometry(colliderId);
        return false;
    }

    const bool enabled = call<"UnityEngine.Collider$$get_enabled", bool>(collider, nullptr);
    ObjectHandle gameObject = call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(collider, nullptr);
    const bool active = isValidObject(gameObject)
                            ? call<"UnityEngine.GameObject$$get_activeInHierarchy", bool>(gameObject, nullptr)
                            : false;
    if (isValidObject(gameObject))
        FFI dropObject(gameObject);

    if (enabled && active) {
        iplInstancedMeshAdd(record.instance, scene_);
        record.active = true;
        sceneDirty_ = true;
    }

    colliders_.emplace(colliderId, record);
    return true;
}

void RayTracer::RemoveCollider(int colliderId) {
    const auto found = colliders_.find(colliderId);
    if (found == colliders_.end())
        return;

    Collider& collider = found->second;
    if (collider.instance) {
        if (collider.active) {
            iplInstancedMeshRemove(collider.instance, scene_);
            sceneDirty_ = true;
        }
        iplInstancedMeshRelease(&collider.instance);
    }

    if (isValidObject(collider.object)) {
        FFI dropObject(collider.object);
        FFI gcDeleteHandle(collider.gcHandle);
    }

    const int meshId = collider.meshId;
    colliders_.erase(found);
    ReleaseGeometry(meshId);
}

void RayTracer::RescanColliders() {
    if (!ResolveColliderTypes())
        return;

    std::unordered_set<int> seen;
    bool meshScanCompleted = false;
    bool boxScanCompleted = false;

    const auto hasAttachedRigidbody = [&](ObjectHandle collider) {
        ObjectHandle rigidbody = call<"UnityEngine.Collider$$get_attachedRigidbody", ObjectHandle>(collider, nullptr);

        const bool attached = !call_error && isValidObject(rigidbody);

        if (isValidObject(rigidbody))
            FFI dropObject(rigidbody);

        return attached;
    };

    if (isValidObject(meshColliderType_)) {
        ObjectHandle array = call<"UnityEngine.Object$$FindObjectsOfType", ObjectHandle>(meshColliderType_, nullptr);

        if (isValidObject(array)) {
            const int32_t count = call<"System.Array$$get_Length", int32_t>(array, nullptr);

            seen.reserve(seen.size() + static_cast<std::size_t>(std::max(count, int32_t{0})));

            for (int64_t index = 0; index < count; ++index) {
                ObjectHandle collider =
                    call<"Il2CppObject* System_Array__GetValue (System_Array_o* __this, int64_t index, "
                         "const MethodInfo* method);",
                         ObjectHandle>(array, index, nullptr);

                if (!isValidObject(collider))
                    continue;

                if (call<"UnityEngine.Collider$$get_isTrigger", bool>(collider, nullptr) ||
                    hasAttachedRigidbody(collider)) {
                    FFI dropObject(collider);
                    continue;
                }

                const int colliderId = call<"UnityEngine.Object$$GetInstanceID", int32_t>(collider, nullptr);

                ObjectHandle mesh = call<"UnityEngine.MeshCollider$$get_sharedMesh", ObjectHandle>(collider, nullptr);

                if (!isValidObject(mesh) || !call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(mesh, nullptr)) {
                    if (isValidObject(mesh))
                        FFI dropObject(mesh);

                    FFI dropObject(collider);
                    continue;
                }

                const int meshId = call<"UnityEngine.Object$$GetInstanceID", int32_t>(mesh, nullptr);

                if (unreadableMeshes_.contains(meshId)) {
                    FFI dropObject(mesh);
                    FFI dropObject(collider);
                    continue;
                }

                auto existing = colliders_.find(colliderId);
                if (existing != colliders_.end() && existing->second.meshId != meshId) {
                    RemoveCollider(colliderId);
                    existing = colliders_.end();
                }

                bool tracked = false;

                if (existing == colliders_.end()) {
                    if (AddCollider(collider, colliderId, mesh, meshId)) {
                        tracked = true;
                    } else {
                        unreadableMeshes_.insert(meshId);
                        FFI dropObject(collider);
                    }
                } else {
                    tracked = true;
                    FFI dropObject(collider);
                }

                if (tracked)
                    seen.insert(colliderId);

                FFI dropObject(mesh);
            }

            FFI dropObject(array);
            meshScanCompleted = true;
        }
    }

    if (isValidObject(boxColliderType_)) {
        ObjectHandle array = call<"UnityEngine.Object$$FindObjectsOfType", ObjectHandle>(boxColliderType_, nullptr);

        if (isValidObject(array)) {
            const int32_t count = call<"System.Array$$get_Length", int32_t>(array, nullptr);

            seen.reserve(seen.size() + static_cast<std::size_t>(std::max(count, int32_t{0})));

            for (int32_t index = 0; index < count; ++index) {
                ObjectHandle collider =
                    call<"Il2CppObject* System_Array__GetValue (System_Array_o* __this, int64_t index, "
                         "const MethodInfo* method);",
                         ObjectHandle>(array, index, nullptr);

                if (!isValidObject(collider))
                    continue;

                if (call<"UnityEngine.Collider$$get_isTrigger", bool>(collider, nullptr) ||
                    hasAttachedRigidbody(collider)) {
                    FFI dropObject(collider);
                    continue;
                }

                const int colliderId = call<"UnityEngine.Object$$GetInstanceID", int32_t>(collider, nullptr);

                auto existing = colliders_.find(colliderId);
                if (existing != colliders_.end() && existing->second.meshId != colliderId) {
                    RemoveCollider(colliderId);
                    existing = colliders_.end();
                }

                bool tracked = false;

                if (existing == colliders_.end()) {
                    if (AddBoxCollider(collider, colliderId)) {
                        tracked = true;
                    } else {
                        FFI dropObject(collider);
                    }
                } else {
                    tracked = true;
                    FFI dropObject(collider);
                }

                if (tracked)
                    seen.insert(colliderId);
            }

            FFI dropObject(array);
            boxScanCompleted = true;
        }
    }

    std::vector<int> removed;
    removed.reserve(colliders_.size());

    for (const auto& [colliderId, collider] : colliders_) {
        // Box colliders use their collider instance ID as their mesh ID
        const bool isBoxCollider = collider.meshId == colliderId;
        const bool categoryWasScanned = isBoxCollider ? boxScanCompleted : meshScanCompleted;

        if (categoryWasScanned && !seen.contains(colliderId))
            removed.push_back(colliderId);
    }

    for (const int colliderId : removed)
        RemoveCollider(colliderId);
}

bool RayTracer::MatricesDiffer(const IPLMatrix4x4& a, const IPLMatrix4x4& b) {
    const auto *af = reinterpret_cast<const float *>(&a);
    const auto *bf = reinterpret_cast<const float *>(&b);
    constexpr std::size_t count = sizeof(IPLMatrix4x4) / sizeof(float);

    for (std::size_t i = 0; i < count; ++i) {
        if (std::abs(af[i] - bf[i]) > 1.0e-5f)
            return true;
    }
    return false;
}

bool RayTracer::SetColliderActive(Collider& collider, bool active, const IPLMatrix4x4 *transform) {
    if (!collider.instance)
        return false;

    bool changed = false;

    if (transform && MatricesDiffer(collider.transform, *transform)) {
        collider.transform = *transform;
        iplInstancedMeshUpdateTransform(collider.instance, scene_, collider.transform);
        changed = true;
    }

    if (active != collider.active) {
        if (active)
            iplInstancedMeshAdd(collider.instance, scene_);
        else
            iplInstancedMeshRemove(collider.instance, scene_);

        collider.active = active;
        changed = true;
    }

    return changed;
}

void RayTracer::UpdateColliderTransforms() {
    for (auto& [id, collider] : colliders_) {
        if (!call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(collider.object, nullptr)) {
            if (SetColliderActive(collider, false, nullptr))
                sceneDirty_ = true;
            continue;
        }

        const bool enabled = call<"UnityEngine.Collider$$get_enabled", bool>(collider.object, nullptr);

        ObjectHandle gameObject = call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(collider.object, nullptr);
        const bool activeInHierarchy =
            isValidObject(gameObject) ? call<"UnityEngine.GameObject$$get_activeInHierarchy", bool>(gameObject, nullptr)
                                      : false;
        if (isValidObject(gameObject))
            FFI dropObject(gameObject);

        const bool active = enabled && activeInHierarchy;
        if (!active) {
            if (SetColliderActive(collider, false, nullptr))
                sceneDirty_ = true;
            continue;
        }

        ObjectHandle transform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(collider.object, nullptr);
        if (!isValidObject(transform)) {
            if (SetColliderActive(collider, false, nullptr))
                sceneDirty_ = true;
            continue;
        }

        const IPLMatrix4x4 matrix = PhononTools::TransformUtils::get_localToWorldMatrix(transform);
        FFI dropObject(transform);

        if (SetColliderActive(collider, true, &matrix))
            sceneDirty_ = true;
    }
}

void RayTracer::CommitScene() {
    if (!sceneDirty_ || !scene_ || !simulator_)
        return;

    iplSceneCommit(scene_);
    iplSimulatorSetScene(simulator_, scene_);
    iplSimulatorCommit(simulator_);

    sceneDirty_ = false;
}

void RayTracer::UpdateScene() {
    if (!IsInitialized() && !Initialize())
        return;

    std::unique_lock simulationLock(simulationMutex_);
    const auto now = Clock::now();

    if (secondsSince(now, lastColliderScan_) >= FixedSettings::colliderRescanIntervalSeconds) {
        RescanColliders();
        lastColliderScan_ = now;
    }

    if (secondsSince(now, lastTransformUpdate_) >= FixedSettings::transformUpdateIntervalSeconds) {
        UpdateColliderTransforms();
        lastTransformUpdate_ = now;
    }

    CommitScene();
}

IPLSource RayTracer::CreateSource() {
    if (!IsInitialized())
        return nullptr;

    std::unique_lock simulationLock(simulationMutex_);

    IPLSourceSettings sourceSettings{};
    sourceSettings.flags =
        static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);

    IPLSource source = nullptr;
    const IPLerror status = iplSourceCreate(simulator_, &sourceSettings, &source);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create a Steam Audio simulation source", status));
        return nullptr;
    }

    iplSourceAdd(source, simulator_);

    return source;
}

void RayTracer::DestroySource(IPLSource source) {
    if (!source)
        return;

    std::unique_lock simulationLock(simulationMutex_);

    if (simulator_)
        iplSourceRemove(source, simulator_);

    iplSourceRelease(&source);
}

bool RayTracer::StartReflectionWorker() {
    std::scoped_lock lock(reflectionMutex_);
    if (reflectionThread_.joinable())
        return true;

    reflectionStop_ = false;
    reflectionPending_ = false;

    reflectionThread_ = std::thread(&RayTracer::ReflectionWorkerMain, this);

    return true;
}

void RayTracer::StopReflectionWorker() {
    {
        std::scoped_lock lock(reflectionMutex_);
        reflectionStop_ = true;
        reflectionPending_ = false;
    }

    reflectionCv_.notify_all();

    if (reflectionThread_.joinable())
        reflectionThread_.join();

    {
        std::scoped_lock lock(reflectionMutex_);
        reflectionStop_ = false;
        reflectionPending_ = false;
    }
}

void RayTracer::QueueReflectionSimulation() {
    {
        std::scoped_lock lock(reflectionMutex_);
        if (reflectionStop_ || !reflectionThread_.joinable())
            return;

        reflectionPending_ = true;
    }

    reflectionCv_.notify_one();
}

void RayTracer::ReflectionWorkerMain() {
    for (;;) {
        {
            std::unique_lock lock(reflectionMutex_);
            reflectionCv_.wait(lock, [this] { return reflectionStop_ || reflectionPending_; });

            if (reflectionStop_)
                return;

            reflectionPending_ = false;
        }

        {
            std::shared_lock simulationLock(simulationMutex_);
            if (simulator_)
                iplSimulatorRunReflections(simulator_);
        }

        reflectionsReady_.store(true, std::memory_order_release);
    }
}

void RayTracer::Simulate(std::list<Playback>& playbacks) {
    if (!IsInitialized())
        return;

    IPLSimulationSharedInputs sharedInputs{};
    sharedInputs.listener = GlobalState::playerCoord;

    const std::size_t sourceCount = std::max<std::size_t>(1, playbacks.size());
    sharedInputs.numRays =
        static_cast<IPLint32>(std::clamp<std::size_t>(1024 * sourceCount, 1024, FixedSettings::maxReflectionRays));
    sharedInputs.numBounces = FixedSettings::reflectionBounces;
    sharedInputs.duration = FixedSettings::reflectionDurationSeconds;
    sharedInputs.order = FixedSettings::ambisonicsOrder;
    sharedInputs.irradianceMinDistance = 1.0f;

    const auto allFlags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
    {
        std::shared_lock simulationLock(simulationMutex_);
        iplSimulatorSetSharedInputs(simulator_, allFlags, &sharedInputs);

        {
            std::scoped_lock lock(playbackMutex);
            for (Playback& playback : playbacks) {
                if (!playback.raySource)
                    continue;

                IPLSimulationInputs inputs{};
                inputs.flags = allFlags;
                inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
                    IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION | IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
                    IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY | IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
                    IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);

                inputs.source.origin = playback.worldPosition;
                inputs.source.right = {1.0f, 0.0f, 0.0f};
                inputs.source.up = {0.0f, 1.0f, 0.0f};
                inputs.source.ahead = {0.0f, 0.0f, 1.0f};

                inputs.distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
                inputs.airAbsorptionModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;
                inputs.directivity.dipoleWeight = 0.0f;
                inputs.directivity.dipolePower = 1.0f;

                inputs.occlusionType = IPL_OCCLUSIONTYPE_VOLUMETRIC;
                inputs.occlusionRadius = 0.25f;
                inputs.numOcclusionSamples = FixedSettings::maxOcclusionSamples;
                inputs.numTransmissionRays = FixedSettings::maxTransmissionRays;
                inputs.baked = IPL_FALSE;

                inputs.reverbScale[0] = 1.0f;
                inputs.reverbScale[1] = 1.0f;
                inputs.reverbScale[2] = 1.0f;
                inputs.hybridReverbTransitionTime = 1.0f;
                inputs.hybridReverbOverlapPercent = 0.25f;

                iplSourceSetInputs(playback.raySource, allFlags, &inputs);
            }
        }
    }

    {
        std::unique_lock simulationLock(simulationMutex_, std::try_to_lock);
        if (simulationLock.owns_lock()) {
            iplSimulatorCommit(simulator_);
            iplSceneSaveOBJ(scene_, "Z:\\tmp\\steamaudio_scene_dump.obj");
        }
    }

    {
        std::shared_lock simulationLock(simulationMutex_);
        iplSimulatorRunDirect(simulator_);
    }

    const auto now = Clock::now();
    if (secondsSince(now, lastReflectionUpdate_) >= FixedSettings::reflectionUpdateIntervalSeconds) {
        QueueReflectionSimulation();
        lastReflectionUpdate_ = now;
    }
}
} // namespace PhononPlayback

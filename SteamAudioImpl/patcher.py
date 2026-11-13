#!/usr/bin/env python3
"""
Patch the Steam Audio implementation to add:

- Steam Audio scene construction from Unity MeshCollider collision meshes.
- Instanced collision geometry with real-time transform updates.
- Direct sound ray tracing:
  - Distance attenuation
  - Air absorption
  - Occlusion
  - Frequency-dependent transmission
- Real-time reflection simulation and binaural ambisonics decoding.
- Cached audio buffers to avoid per-source allocations in the audio callback.
- Several lifecycle, iterator, loop-setting, formatting, and optional-value fixes.

Run this script from the project directory:

    python3 patch_audio_raytracing.py

The script creates a timestamped backup directory before modifying files.
"""

from __future__ import annotations

import datetime
import os
import shutil
import sys
import tempfile
from pathlib import Path


FIXEDSETTINGS_HPP = r'''#pragma once

#include <phonon.h>

namespace FixedSettings {
constexpr unsigned outputChannels = 2;
constexpr unsigned ambisonicsOrder = 1;
constexpr unsigned ambisonicsChannels = 4;

constexpr float transformUpdateIntervalSeconds = 0.05f;
constexpr float colliderRescanIntervalSeconds = 1.0f;
constexpr float reflectionUpdateIntervalSeconds = 0.20f;
constexpr float reflectionDurationSeconds = 1.0f;

constexpr IPLint32 maxOcclusionSamples = 16;
constexpr IPLint32 maxTransmissionRays = 8;
constexpr IPLint32 maxReflectionRays = 4096;
constexpr IPLint32 reflectionBounces = 4;
constexpr IPLint32 maxRaytracedSources = 256;

constexpr IPLDirectEffectFlags directEffectFlags = static_cast<IPLDirectEffectFlags>(
    IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
    IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION |
    IPL_DIRECTEFFECTFLAGS_APPLYDIRECTIVITY |
    IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
    IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
} // namespace FixedSettings
'''

BUFFERPOOL_HPP = r'''#pragma once

#include "SteamAudioImpl.hpp"

#include <array>
#include <phonon.h>

namespace PhononPlayback {
class BufferPool {
public:
    BufferPool(IPLint32 channels, IPLint32 samples);
    ~BufferPool();
    BufferPool(const BufferPool&) = delete;
    BufferPool(BufferPool&&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;
    BufferPool& operator=(BufferPool&&) = delete;

    IPLAudioBuffer& GetCurrentBuffer() { return buffers_[bufferIdx_]; }
    IPLAudioBuffer& GetNextBuffer() { return buffers_[GetNextBufferIndex()]; }
    void SwitchToNextBuffer() { bufferIdx_ = GetNextBufferIndex(); }

private:
    unsigned GetNextBufferIndex() const;

    unsigned bufferIdx_ = 0;
    std::array<IPLAudioBuffer, 4> buffers_{};
};
} // namespace PhononPlayback
'''

PLAYBACK_ENVIRONMENT_HPP = r'''#pragma once

#include "SteamAudioImpl.hpp"
#include "bufferpool.hpp"

#include <chrono>
#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <phonon.h>
#include <ffi_interface.hpp>

namespace PhononPlayback {
struct Playback;

struct Environment {
    IPLAudioSettings audioSettings{};
    IPLHRTF hrtf = nullptr;
    IPLReflectionMixer reflectionMixer = nullptr;
    IPLAmbisonicsDecodeEffect ambisonicsDecodeEffect = nullptr;

    BufferPool bufferPool;
    BufferPool monoBufferPool;
    BufferPool ambisonicsBufferPool;
    IPLAudioBuffer frameBuffer{};

    explicit Environment(IPLint32 frameSize);
    ~Environment();

    Environment(const Environment&) = delete;
    Environment(Environment&&) = delete;
    Environment& operator=(const Environment&) = delete;
    Environment& operator=(Environment&&) = delete;
};

class RayTracer {
public:
    RayTracer() = default;
    ~RayTracer();

    RayTracer(const RayTracer&) = delete;
    RayTracer(RayTracer&&) = delete;
    RayTracer& operator=(const RayTracer&) = delete;
    RayTracer& operator=(RayTracer&&) = delete;

    bool Initialize();
    void Shutdown();

    // Must be called from the Unity/game thread because it invokes managed APIs.
    void UpdateScene();

    // Must be called while playbackQueue is locked.
    void Simulate(std::list<Playback>& playbacks);

    IPLSource CreateSource();
    bool IsInitialized() const { return simulator_ != nullptr && scene_ != nullptr; }
    bool ReflectionsReady() const { return reflectionsReady_; }

private:
    struct Geometry {
        IPLScene subScene = nullptr;
        IPLStaticMesh staticMesh = nullptr;
        unsigned references = 0;
    };

    struct Collider {
        FFIInterface::ObjectHandle object = FFIInterface::ObjectHandle::Invalid;
        FFIInterface::GCHandle gcHandle{};
        int meshId = 0;
        IPLInstancedMesh instance = nullptr;
        IPLMatrix4x4 transform{};
        bool active = false;
    };

    using Clock = std::chrono::steady_clock;

    bool ResolveMeshColliderType();
    void RescanColliders();
    void UpdateColliderTransforms();

    bool AddCollider(FFIInterface::ObjectHandle collider, int colliderId,
                     FFIInterface::ObjectHandle mesh, int meshId);
    void RemoveCollider(int colliderId);
    Geometry* AcquireGeometry(FFIInterface::ObjectHandle mesh, int meshId);
    void ReleaseGeometry(int meshId);

    bool SetColliderActive(Collider& collider, bool active, const IPLMatrix4x4* transform);
    void CommitScene();
    static bool MatricesDiffer(const IPLMatrix4x4& a, const IPLMatrix4x4& b);

    IPLScene scene_ = nullptr;
    IPLSimulator simulator_ = nullptr;
    FFIInterface::ObjectHandle meshColliderType_ = FFIInterface::ObjectHandle::Invalid;

    std::unordered_map<int, Geometry> geometries_;
    std::unordered_map<int, Collider> colliders_;

    Clock::time_point lastTransformUpdate_{};
    Clock::time_point lastColliderScan_{};
    Clock::time_point lastReflectionUpdate_{};

    bool sceneDirty_ = false;
    bool reflectionsReady_ = false;
};

extern std::optional<Environment> env;
extern RayTracer rayTracer;
} // namespace PhononPlayback
'''

PLAYBACK_ENVIRONMENT_CPP = r'''#include "playback_environment.hpp"

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

using namespace FFIInterface;
using namespace Helpers;

namespace PhononPlayback {
std::optional<Environment> env;
RayTracer rayTracer;

namespace {
void logIplError(const char* operation, IPLerror status) {
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(Utils::createErrorMessage(operation, status));
}

bool isValidObject(ObjectHandle object) {
    return object != ObjectHandle::Null && object != ObjectHandle::Invalid;
}

double secondsSince(RayTracer::Clock::time_point now, RayTracer::Clock::time_point then) {
    if (then.time_since_epoch().count() == 0)
        return 1.0e9;
    return std::chrono::duration<double>(now - then).count();
}
} // namespace

Environment::Environment(IPLint32 frameSize)
    : bufferPool(FixedSettings::outputChannels, frameSize),
      monoBufferPool(1, frameSize),
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
    status = iplReflectionMixerCreate(
        GlobalState::phononCtx, &audioSettings, &reflectionSettings, &reflectionMixer);
    logIplError("create the reflection mixer", status);

    IPLAmbisonicsDecodeEffectSettings decodeSettings{};
    decodeSettings.maxOrder = FixedSettings::ambisonicsOrder;
    decodeSettings.hrtf = hrtf;
    status = iplAmbisonicsDecodeEffectCreate(
        GlobalState::phononCtx, &audioSettings, &decodeSettings, &ambisonicsDecodeEffect);
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

RayTracer::~RayTracer() {
    Shutdown();
}

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

    IPLSimulationSettings simulationSettings{};
    simulationSettings.flags = static_cast<IPLSimulationFlags>(
        IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
    simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
    simulationSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    simulationSettings.maxNumOcclusionSamples = FixedSettings::maxOcclusionSamples;
    simulationSettings.maxNumRays = FixedSettings::maxReflectionRays;
    simulationSettings.numDiffuseSamples = 32;
    simulationSettings.maxDuration = FixedSettings::reflectionDurationSeconds;
    simulationSettings.maxOrder = FixedSettings::ambisonicsOrder;
    simulationSettings.maxNumSources = FixedSettings::maxRaytracedSources;

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
    return true;
}

void RayTracer::Shutdown() {
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

bool RayTracer::ResolveMeshColliderType() {
    if (isValidObject(meshColliderType_))
        return true;

    static constexpr const char* typeNames[] = {
        "UnityEngine.MeshCollider, UnityEngine.PhysicsModule",
        "UnityEngine.MeshCollider, UnityEngine"
    };

    for (const char* typeName : typeNames) {
        ObjectHandle name = FFI toCsString(typeName);
        ObjectHandle type = call<"System.Type$$GetType", ObjectHandle>(name, nullptr);
        FFI dropObject(name);

        if (isValidObject(type)) {
            meshColliderType_ = type;
            return true;
        }
    }

    FFI logWarn(FFI toCsString(
        "Steam Audio could not resolve UnityEngine.MeshCollider. "
        "Collision-scene ray tracing has been disabled."));
    return false;
}

RayTracer::Geometry* RayTracer::AcquireGeometry(ObjectHandle mesh, int meshId) {
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
    Geometry* geometry = AcquireGeometry(mesh, meshId);
    if (!geometry)
        return false;

    Collider record{};
    record.object = collider;
    record.gcHandle = FFI gcCreateHandle(collider, true);
    record.meshId = meshId;

    ObjectHandle transform =
        call<"UnityEngine.Component$$get_transform", ObjectHandle>(collider, nullptr);
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

    IPLerror status =
        iplInstancedMeshCreate(scene_, &instanceSettings, &record.instance);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create an instanced collision mesh", status));
        FFI dropObject(record.object);
        FFI gcDeleteHandle(record.gcHandle);
        ReleaseGeometry(meshId);
        return false;
    }

    const bool enabled =
        call<"UnityEngine.Collider$$get_enabled", bool>(collider, nullptr);
    ObjectHandle gameObject =
        call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(collider, nullptr);
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
    if (!ResolveMeshColliderType())
        return;

    ObjectHandle array =
        call<"UnityEngine.Object$$FindObjectsOfType", ObjectHandle>(meshColliderType_, nullptr);
    if (!isValidObject(array))
        return;

    const int32_t count = call<"System.Array$$get_Length", int32_t>(array, nullptr);
    std::unordered_set<int> seen;
    seen.reserve(static_cast<std::size_t>(std::max(count, 0)));

    for (int32_t index = 0; index < count; ++index) {
        ObjectHandle collider =
            call<"System.Array$$GetValue", ObjectHandle>(array, index, nullptr);
        if (!isValidObject(collider))
            continue;

        const int colliderId =
            call<"UnityEngine.Object$$GetInstanceID", int32_t>(collider, nullptr);
        seen.insert(colliderId);

        ObjectHandle mesh =
            call<"UnityEngine.MeshCollider$$get_sharedMesh", ObjectHandle>(collider, nullptr);
        if (!isValidObject(mesh)) {
            FFI dropObject(collider);
            continue;
        }

        const int meshId =
            call<"UnityEngine.Object$$GetInstanceID", int32_t>(mesh, nullptr);

        auto existing = colliders_.find(colliderId);
        if (existing != colliders_.end() && existing->second.meshId != meshId) {
            RemoveCollider(colliderId);
            existing = colliders_.end();
        }

        if (existing == colliders_.end()) {
            if (!AddCollider(collider, colliderId, mesh, meshId))
                FFI dropObject(collider);
        } else {
            // GetValue returns a temporary managed handle. Keep the handle
            // already stored by the collider record.
            FFI dropObject(collider);
        }

        FFI dropObject(mesh);
    }

    FFI dropObject(array);

    std::vector<int> removed;
    removed.reserve(colliders_.size());
    for (const auto& [colliderId, collider] : colliders_) {
        if (!seen.contains(colliderId))
            removed.push_back(colliderId);
    }

    for (int colliderId : removed)
        RemoveCollider(colliderId);
}

bool RayTracer::MatricesDiffer(const IPLMatrix4x4& a, const IPLMatrix4x4& b) {
    const auto* af = reinterpret_cast<const float*>(&a);
    const auto* bf = reinterpret_cast<const float*>(&b);
    constexpr std::size_t count = sizeof(IPLMatrix4x4) / sizeof(float);

    for (std::size_t i = 0; i < count; ++i) {
        if (std::abs(af[i] - bf[i]) > 1.0e-5f)
            return true;
    }
    return false;
}

bool RayTracer::SetColliderActive(
    Collider& collider, bool active, const IPLMatrix4x4* transform) {
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
        if (!call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(
                collider.object, nullptr)) {
            if (SetColliderActive(collider, false, nullptr))
                sceneDirty_ = true;
            continue;
        }

        const bool enabled =
            call<"UnityEngine.Collider$$get_enabled", bool>(collider.object, nullptr);

        ObjectHandle gameObject =
            call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(
                collider.object, nullptr);
        const bool activeInHierarchy = isValidObject(gameObject)
            ? call<"UnityEngine.GameObject$$get_activeInHierarchy", bool>(
                  gameObject, nullptr)
            : false;
        if (isValidObject(gameObject))
            FFI dropObject(gameObject);

        const bool active = enabled && activeInHierarchy;
        if (!active) {
            if (SetColliderActive(collider, false, nullptr))
                sceneDirty_ = true;
            continue;
        }

        ObjectHandle transform =
            call<"UnityEngine.Component$$get_transform", ObjectHandle>(
                collider.object, nullptr);
        if (!isValidObject(transform)) {
            if (SetColliderActive(collider, false, nullptr))
                sceneDirty_ = true;
            continue;
        }

        const IPLMatrix4x4 matrix =
            PhononTools::TransformUtils::get_localToWorldMatrix(transform);
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

    const auto now = Clock::now();

    if (secondsSince(now, lastColliderScan_) >=
        FixedSettings::colliderRescanIntervalSeconds) {
        RescanColliders();
        lastColliderScan_ = now;
    }

    if (secondsSince(now, lastTransformUpdate_) >=
        FixedSettings::transformUpdateIntervalSeconds) {
        UpdateColliderTransforms();
        lastTransformUpdate_ = now;
    }

    CommitScene();
}

IPLSource RayTracer::CreateSource() {
    if (!IsInitialized())
        return nullptr;

    IPLSourceSettings sourceSettings{};
    sourceSettings.flags = static_cast<IPLSimulationFlags>(
        IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);

    IPLSource source = nullptr;
    const IPLerror status =
        iplSourceCreate(simulator_, &sourceSettings, &source);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage(
            "create a Steam Audio simulation source", status));
        return nullptr;
    }

    return source;
}

void RayTracer::Simulate(std::list<Playback>& playbacks) {
    if (!IsInitialized())
        return;

    IPLSharedSimulationInputs sharedInputs{};
    sharedInputs.listener = GlobalState::playerCoord;

    const std::size_t sourceCount = std::max<std::size_t>(1, playbacks.size());
    sharedInputs.numRays = static_cast<IPLint32>(
        std::clamp<std::size_t>(1024 * sourceCount, 1024,
                                FixedSettings::maxReflectionRays));
    sharedInputs.numBounces = FixedSettings::reflectionBounces;
    sharedInputs.duration = FixedSettings::reflectionDurationSeconds;
    sharedInputs.order = FixedSettings::ambisonicsOrder;
    sharedInputs.irradianceMinDistance = 1.0f;

    const auto allFlags = static_cast<IPLSimulationFlags>(
        IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
    iplSimulatorSetSharedInputs(simulator_, allFlags, &sharedInputs);

    for (Playback& playback : playbacks) {
        if (!playback.raySource)
            continue;

        IPLSimulationInputs inputs{};
        inputs.flags = allFlags;
        inputs.source.origin = playback.worldPosition;
        inputs.source.right = {1.0f, 0.0f, 0.0f};
        inputs.source.up = {0.0f, 1.0f, 0.0f};
        inputs.source.ahead = {0.0f, 0.0f, 1.0f};

        inputs.distanceAttenuationModel.type =
            IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
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

    iplSimulatorCommit(simulator_);
    iplSimulatorRunDirect(simulator_);

    const auto now = Clock::now();
    if (secondsSince(now, lastReflectionUpdate_) >=
        FixedSettings::reflectionUpdateIntervalSeconds) {
        iplSimulatorRunReflections(simulator_);
        lastReflectionUpdate_ = now;
        reflectionsReady_ = true;
    }
}
} // namespace PhononPlayback
'''

PLAYBACK_HPP = r'''#pragma once

#include "SteamAudioImpl.hpp"

#include <list>
#include <mutex>
#include <ffi_interface.hpp>

typedef struct ma_device ma_device;

namespace PhononPlayback {
enum AudioRolloffMode { Logarithmic = 0, Linear = 1, Custom = 2 };

struct Playback {
    FFIInterface::ObjectHandle audioSource;
    FFIInterface::GCHandle audioSourceGc;
    AudioRolloffMode rolloffMode = Linear;
    uint64_t delay = 0;
    float volumeScale = 1.0f;
    float volume = 0.5f;
    float spatialBlend = 1.0f;
    float maxDistance = 1000.0f;
    bool isOneShot = false;
    bool loop = false;
    uint64_t playPosition = 0;
    IPLVector3 worldPosition{};

    IPLAudioBuffer audioBuffer{};
    IPLAudioBuffer inputScratch{};
    IPLAudioBuffer monoScratch{};

    IPLBinauralEffect binauralEffect = nullptr;
    IPLDirectEffect directEffect = nullptr;
    IPLReflectionEffect reflectionEffect = nullptr;
    IPLSource raySource = nullptr;

    IPLint32 effectFrameSize = 0;

    Playback(FFIInterface::ObjectHandle audioSource,
             const IPLAudioBuffer& audioBuffer, uint64_t delay,
             float volumeScale, bool isOneShot);
    ~Playback();

    Playback(const Playback&) = delete;
    Playback(Playback&&) = delete;
    Playback& operator=(const Playback&) = delete;
    Playback& operator=(Playback&&) = delete;

    IPLAudioBuffer& operator=(const IPLAudioBuffer& audioBuffer);

    bool EnsureAudioResources(IPLint32 frameSize);
    bool hasReachedEnd() const {
        return !loop && playPosition >= static_cast<uint64_t>(audioBuffer.numSamples);
    }
};

extern std::list<Playback> playbackQueue;
extern std::mutex playbackMutex;

void dataCallback(ma_device* pDevice, float* pOutput,
                  const float* pInput, ma_uint32 sampleCount);
} // namespace PhononPlayback
'''

PLAYBACK_CPP = r'''#include "playback.hpp"

#include "SteamAudioImpl.hpp"
#include "playback_environment.hpp"
#include "fixedsettings.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <phonon.h>
#include <ffi_interface.hpp>

using namespace FFIInterface;

namespace PhononPlayback {
std::list<Playback> playbackQueue;
std::mutex playbackMutex;

namespace Math {
float distance(const IPLVector3& a, const IPLVector3& b) {
    const float x = b.x - a.x;
    const float y = b.y - a.y;
    const float z = b.z - a.z;
    return std::sqrt(x * x + y * y + z * z);
}

float maxDistanceAttenuation(float distance, float maxDistance) {
    if (maxDistance <= 0.0f)
        return distance <= 0.0f ? 1.0f : 0.0f;

    float normalizedDistance =
        std::clamp(distance / maxDistance, 0.0f, 1.0f);
    const float exponent = -normalizedDistance * normalizedDistance;
    const float numerator = std::exp(exponent) - std::exp(-1.0f);
    const float denominator = 1.0f - std::exp(-1.0f);
    return numerator / denominator;
}
} // namespace Math

namespace {
void clearBuffer(IPLAudioBuffer& buffer) {
    for (IPLint32 channel = 0; channel < buffer.numChannels; ++channel)
        std::fill_n(buffer.data[channel], buffer.numSamples, 0.0f);
}

bool allocateBuffer(IPLint32 channels, IPLint32 samples,
                    IPLAudioBuffer& buffer, const char* operation) {
    const IPLerror status = iplAudioBufferAllocate(
        GlobalState::phononCtx, channels, samples, &buffer);
    if (status == IPL_STATUS_SUCCESS)
        return true;

    FFI logError(Utils::createErrorMessage(operation, status));
    buffer = {};
    return false;
}
} // namespace

Playback::Playback(ObjectHandle audioSource,
                   const IPLAudioBuffer& audioBuffer, uint64_t delay,
                   float volumeScale, bool isOneShot)
    : audioSource(audioSource),
      delay(delay),
      volumeScale(volumeScale),
      isOneShot(isOneShot),
      audioBuffer(audioBuffer) {
    audioSourceGc = FFI gcCreateHandle(audioSource, true);
    raySource = rayTracer.CreateSource();
}

Playback::~Playback() {
    if (inputScratch.data)
        iplAudioBufferFree(GlobalState::phononCtx, &inputScratch);
    if (monoScratch.data)
        iplAudioBufferFree(GlobalState::phononCtx, &monoScratch);
    if (audioBuffer.data)
        iplAudioBufferFree(GlobalState::phononCtx, &audioBuffer);

    if (reflectionEffect)
        iplReflectionEffectRelease(&reflectionEffect);
    if (binauralEffect)
        iplBinauralEffectRelease(&binauralEffect);
    if (directEffect)
        iplDirectEffectRelease(&directEffect);
    if (raySource)
        iplSourceRelease(&raySource);

    if (audioSource != ObjectHandle::Null &&
        audioSource != ObjectHandle::Invalid) {
        FFI dropObject(audioSource);
        FFI gcDeleteHandle(audioSourceGc);
    }
}

bool Playback::EnsureAudioResources(IPLint32 frameSize) {
    if (!env || frameSize <= 0)
        return false;

    if (effectFrameSize == frameSize && inputScratch.data &&
        monoScratch.data && binauralEffect && directEffect &&
        reflectionEffect) {
        return true;
    }

    if (inputScratch.data)
        iplAudioBufferFree(GlobalState::phononCtx, &inputScratch);
    if (monoScratch.data)
        iplAudioBufferFree(GlobalState::phononCtx, &monoScratch);
    if (reflectionEffect)
        iplReflectionEffectRelease(&reflectionEffect);
    if (binauralEffect)
        iplBinauralEffectRelease(&binauralEffect);
    if (directEffect)
        iplDirectEffectRelease(&directEffect);

    effectFrameSize = 0;

    if (!allocateBuffer(audioBuffer.numChannels, frameSize, inputScratch,
                        "allocate an input audio scratch buffer"))
        return false;
    if (!allocateBuffer(1, frameSize, monoScratch,
                        "allocate a mono audio scratch buffer"))
        return false;

    IPLBinauralEffectSettings binauralSettings{};
    binauralSettings.hrtf = env->hrtf;
    IPLerror status = iplBinauralEffectCreate(
        GlobalState::phononCtx, &env->audioSettings,
        &binauralSettings, &binauralEffect);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage(
            "create a binaural effect", status));
        return false;
    }

    IPLDirectEffectSettings directSettings{};
    directSettings.numChannels = FixedSettings::outputChannels;
    status = iplDirectEffectCreate(
        GlobalState::phononCtx, &env->audioSettings,
        &directSettings, &directEffect);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage(
            "create a direct sound effect", status));
        return false;
    }

    IPLReflectionEffectSettings reflectionSettings{};
    reflectionSettings.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    reflectionSettings.irSize = static_cast<IPLint32>(
        std::ceil(env->audioSettings.samplingRate *
                  FixedSettings::reflectionDurationSeconds));
    reflectionSettings.numChannels = FixedSettings::ambisonicsChannels;

    status = iplReflectionEffectCreate(
        GlobalState::phononCtx, &env->audioSettings,
        &reflectionSettings, &reflectionEffect);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage(
            "create a reflection effect", status));
        return false;
    }

    effectFrameSize = frameSize;
    return true;
}

IPLAudioBuffer& Playback::operator=(const IPLAudioBuffer& newAudioBuffer) {
    if (audioBuffer.data)
        iplAudioBufferFree(GlobalState::phononCtx, &audioBuffer);

    if (inputScratch.data) {
        iplAudioBufferFree(GlobalState::phononCtx, &inputScratch);
        inputScratch = {};
    }
    if (monoScratch.data) {
        iplAudioBufferFree(GlobalState::phononCtx, &monoScratch);
        monoScratch = {};
    }

    effectFrameSize = 0;
    playPosition = 0;
    audioBuffer = newAudioBuffer;
    return audioBuffer;
}

void dataCallback(ma_device* pDevice, float* pOutput,
                  const float* pInput, ma_uint32 sampleCount) {
    (void)pDevice;
    (void)pInput;

    ++GlobalState::stats.framesStarted;

    if (!pOutput || sampleCount == 0) {
        ++GlobalState::stats.framesFinished;
        return;
    }

    if (!env.has_value() ||
        env->audioSettings.frameSize != static_cast<IPLint32>(sampleCount)) {
        env.emplace(static_cast<IPLint32>(sampleCount));
    }

    IPLAudioBuffer& frameBuffer = env->frameBuffer;
    clearBuffer(frameBuffer);

    bool reflectionInputApplied = false;

    {
        std::scoped_lock lock(playbackMutex);

        for (Playback& playback : playbackQueue) {
            ++GlobalState::stats.playbacksStarted;

            if (!playback.EnsureAudioResources(
                    static_cast<IPLint32>(sampleCount))) {
                continue;
            }

            if (playback.audioBuffer.numSamples <= 0 ||
                playback.audioBuffer.numChannels <= 0) {
                continue;
            }

            std::size_t silentPrefix = 0;
            if (playback.delay >= sampleCount) {
                playback.delay -= sampleCount;
                ++GlobalState::stats.playbacksFinished;
                continue;
            } else if (playback.delay > 0) {
                silentPrefix = static_cast<std::size_t>(playback.delay);
                playback.delay = 0;
            }

            if (playback.hasReachedEnd())
                continue;

            clearBuffer(playback.inputScratch);

            const float volume = std::max(
                0.0f, playback.volume * playback.volumeScale);
            const uint64_t samplesConsumed =
                sampleCount - static_cast<ma_uint32>(silentPrefix);

            for (IPLint32 channel = 0;
                 channel < playback.inputScratch.numChannels; ++channel) {
                for (ma_uint32 sample =
                         static_cast<ma_uint32>(silentPrefix);
                     sample < sampleCount; ++sample) {
                    const uint64_t sourceOffset =
                        sample - static_cast<ma_uint32>(silentPrefix);
                    const uint64_t position =
                        playback.playPosition + sourceOffset;

                    float value = 0.0f;
                    if (playback.loop) {
                        const uint64_t wrapped =
                            position % playback.audioBuffer.numSamples;
                        value = playback.audioBuffer.data[channel][wrapped];
                    } else if (position <
                               static_cast<uint64_t>(
                                   playback.audioBuffer.numSamples)) {
                        value = playback.audioBuffer.data[channel][position];
                    }

                    playback.inputScratch.data[channel][sample] =
                        value * volume;
                }
            }

            if (playback.inputScratch.numChannels == 1) {
                std::memcpy(
                    playback.monoScratch.data[0],
                    playback.inputScratch.data[0],
                    sampleCount * sizeof(float));
            } else {
                iplAudioBufferDownmix(
                    GlobalState::phononCtx,
                    &playback.inputScratch,
                    &playback.monoScratch);
            }

            IPLSimulationOutputs simulationOutputs{};
            bool haveSimulationOutput = false;
            if (playback.raySource && rayTracer.IsInitialized()) {
                iplSourceGetOutputs(
                    playback.raySource,
                    static_cast<IPLSimulationFlags>(
                        IPL_SIMULATIONFLAGS_DIRECT |
                        IPL_SIMULATIONFLAGS_REFLECTIONS),
                    &simulationOutputs);
                haveSimulationOutput = true;
            }

            if (haveSimulationOutput &&
                rayTracer.ReflectionsReady() &&
                playback.reflectionEffect &&
                env->reflectionMixer) {
                iplReflectionEffectApply(
                    playback.reflectionEffect,
                    &simulationOutputs.reflections,
                    &playback.monoScratch,
                    env->reflectionMixer);
                reflectionInputApplied = true;
            }

            IPLAudioBuffer& spatialized =
                env->bufferPool.GetCurrentBuffer();

            if (playback.spatialBlend > 0.0f) {
                IPLBinauralEffectParams binauralParams{};
                binauralParams.interpolation =
                    IPL_HRTFINTERPOLATION_BILINEAR;
                binauralParams.spatialBlend =
                    std::clamp(playback.spatialBlend, 0.0f, 1.0f);
                binauralParams.hrtf = env->hrtf;
                binauralParams.peakDelays = nullptr;
                binauralParams.direction =
                    iplCalculateRelativeDirection(
                        GlobalState::phononCtx,
                        playback.worldPosition,
                        GlobalState::playerCoord.origin,
                        GlobalState::playerCoord.ahead,
                        GlobalState::playerCoord.up);
                binauralParams.direction.x =
                    -binauralParams.direction.x;

                iplBinauralEffectApply(
                    playback.binauralEffect,
                    &binauralParams,
                    &playback.monoScratch,
                    &spatialized);
            } else if (
                playback.inputScratch.numChannels ==
                FixedSettings::outputChannels) {
                for (unsigned channel = 0;
                     channel < FixedSettings::outputChannels;
                     ++channel) {
                    std::memcpy(
                        spatialized.data[channel],
                        playback.inputScratch.data[channel],
                        sampleCount * sizeof(float));
                }
            } else if (
                playback.inputScratch.numChannels >
                FixedSettings::outputChannels) {
                iplAudioBufferDownmix(
                    GlobalState::phononCtx,
                    &playback.inputScratch,
                    &spatialized);
            } else {
                for (unsigned channel = 0;
                     channel < FixedSettings::outputChannels;
                     ++channel) {
                    std::memcpy(
                        spatialized.data[channel],
                        playback.inputScratch.data[
                            channel %
                            playback.inputScratch.numChannels],
                        sampleCount * sizeof(float));
                }
            }

            IPLDirectEffectParams directParams{};
            directParams.flags = FixedSettings::directEffectFlags;
            directParams.transmissionType =
                IPL_TRANSMISSIONTYPE_FREQDEPENDENT;
            directParams.distanceAttenuation = 1.0f;
            directParams.airAbsorption[0] = 1.0f;
            directParams.airAbsorption[1] = 1.0f;
            directParams.airAbsorption[2] = 1.0f;
            directParams.directivity = 1.0f;
            directParams.occlusion = 1.0f;
            directParams.transmission[0] = 1.0f;
            directParams.transmission[1] = 1.0f;
            directParams.transmission[2] = 1.0f;

            if (haveSimulationOutput) {
                directParams.distanceAttenuation =
                    simulationOutputs.direct.distanceAttenuation;
                std::memcpy(
                    directParams.airAbsorption,
                    simulationOutputs.direct.airAbsorption,
                    sizeof(directParams.airAbsorption));
                directParams.directivity =
                    simulationOutputs.direct.directivity;
                directParams.occlusion =
                    simulationOutputs.direct.occlusion;
                std::memcpy(
                    directParams.transmission,
                    simulationOutputs.direct.transmission,
                    sizeof(directParams.transmission));
            } else {
                IPLDistanceAttenuationModel distanceModel{};
                distanceModel.type =
                    IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
                directParams.distanceAttenuation =
                    iplDistanceAttenuationCalculate(
                        GlobalState::phononCtx,
                        playback.worldPosition,
                        GlobalState::playerCoord.origin,
                        &distanceModel);

                IPLAirAbsorptionModel absorptionModel{};
                absorptionModel.type =
                    IPL_AIRABSORPTIONTYPE_DEFAULT;
                iplAirAbsorptionCalculate(
                    GlobalState::phononCtx,
                    playback.worldPosition,
                    GlobalState::playerCoord.origin,
                    &absorptionModel,
                    directParams.airAbsorption);
            }

            if (playback.rolloffMode !=
                AudioRolloffMode::Logarithmic) {
                const float distance = Math::distance(
                    GlobalState::playerCoord.origin,
                    playback.worldPosition);
                directParams.distanceAttenuation *=
                    Math::maxDistanceAttenuation(
                        distance, playback.maxDistance);
            }

            iplDirectEffectApply(
                playback.directEffect,
                &directParams,
                &env->bufferPool.GetCurrentBuffer(),
                &env->bufferPool.GetNextBuffer());
            env->bufferPool.SwitchToNextBuffer();

            iplAudioBufferMix(
                GlobalState::phononCtx,
                &env->bufferPool.GetCurrentBuffer(),
                &frameBuffer);

            playback.playPosition += samplesConsumed;
            if (playback.loop &&
                playback.audioBuffer.numSamples > 0) {
                playback.playPosition %=
                    playback.audioBuffer.numSamples;
            }

            ++GlobalState::stats.playbacksFinished;
        }
    }

    if (reflectionInputApplied && env->reflectionMixer &&
        env->ambisonicsDecodeEffect) {
        IPLReflectionEffectParams mixerParams{};
        mixerParams.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
        mixerParams.numChannels = FixedSettings::ambisonicsChannels;
        mixerParams.irSize = static_cast<IPLint32>(
            std::ceil(env->audioSettings.samplingRate *
                      FixedSettings::reflectionDurationSeconds));

        IPLAudioBuffer& ambisonics =
            env->ambisonicsBufferPool.GetCurrentBuffer();
        clearBuffer(ambisonics);

        iplReflectionMixerApply(
            env->reflectionMixer, &mixerParams, &ambisonics);

        IPLAmbisonicsDecodeEffectParams decodeParams{};
        decodeParams.order = FixedSettings::ambisonicsOrder;
        decodeParams.hrtf = env->hrtf;
        decodeParams.orientation = GlobalState::playerCoord;
        decodeParams.binaural = IPL_TRUE;

        IPLAudioBuffer& decoded =
            env->bufferPool.GetNextBuffer();
        iplAmbisonicsDecodeEffectApply(
            env->ambisonicsDecodeEffect,
            &decodeParams,
            &ambisonics,
            &decoded);

        iplAudioBufferMix(
            GlobalState::phononCtx, &decoded, &frameBuffer);
    }

    iplAudioBufferInterleave(
        GlobalState::phononCtx, &frameBuffer, pOutput);

    ++GlobalState::stats.framesFinished;
}
} // namespace PhononPlayback
'''


def replace_once(text: str, old: str, new: str, description: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(
            f"Could not patch {description}: expected exactly one match, found {count}"
        )
    return text.replace(old, new, 1)


def locate(project: Path, name: str) -> Path:
    direct = project / name
    if direct.is_file():
        return direct

    ignored = {
        ".git",
        "build",
        "Build",
        "cmake-build-debug",
        "cmake-build-release",
        "Library",
        "Temp",
        "obj",
        "bin",
    }

    matches = []
    for root, dirs, files in os.walk(project):
        dirs[:] = [d for d in dirs if d not in ignored and not d.startswith(".steamaudio_raytracing_backup_")]
        if name in files:
            matches.append(Path(root) / name)

    if not matches:
        raise FileNotFoundError(f"Could not find required file: {name}")
    if len(matches) > 1:
        formatted = "\n  ".join(str(p) for p in matches)
        raise RuntimeError(f"Multiple copies of {name} were found:\n  {formatted}")

    return matches[0]


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=str(path.parent)
    )
    temp = Path(temp_name)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp, path)
    except Exception:
        try:
            temp.unlink(missing_ok=True)
        finally:
            raise


def patch_phonontools_cpp(text: str) -> str:
    text = replace_once(
        text,
        "    IPLAudioBuffer fres;\n",
        "    IPLAudioBuffer fres{};\n",
        "audio buffer initialization",
    )

    text = replace_once(
        text,
        "    std::vector<IPLint32> materials(triangles.size(), Materials::Brick);\n",
        "    std::vector<IPLint32> materials(triangles.size(), 0);\n",
        "collision mesh material indices",
    )

    text = replace_once(
        text,
        """    staticMeshSettings.numMaterials = 1;
    staticMeshSettings.vertices = vertices.data();
    staticMeshSettings.triangles = triangles.data();
    staticMeshSettings.materialIndices = materials.data();
    staticMeshSettings.materials = Materials::presets;
""",
        """    staticMeshSettings.numMaterials = 1;
    staticMeshSettings.vertices = vertices.data();
    staticMeshSettings.triangles = triangles.data();
    staticMeshSettings.materialIndices = materials.data();
    staticMeshSettings.materials = &Materials::presets[Materials::Brick];
""",
        "collision material assignment",
    )
    return text


def patch_utils_cpp(text: str) -> str:
    return replace_once(
        text,
        """std::string formatPos(IPLVector3 pos) {
    return '[' + std::to_string(pos.x) + ", " + std::to_string(pos.x) + ", " + std::to_string(pos.x) + ']';
}
""",
        """std::string formatPos(IPLVector3 pos) {
    return '[' + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ']';
}
""",
        "position formatting",
    )


def patch_steam_audio_impl(text: str) -> str:
    text = replace_once(
        text,
        """        auto& playback = playbackQueue.emplace_back(audioSource, audioBuffer, delay, volumeScale, isOneShot);
        PhononTools::updatePlayback(playback, true);
""",
        """        auto& playback = playbackQueue.emplace_back(audioSource, audioBuffer, delay, volumeScale, isOneShot);
        if (!PhononTools::updatePlayback(playback, true)) {
            playbackQueue.pop_back();
            return;
        }
""",
        "initial playback validation",
    )

    text = replace_once(
        text,
        """        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            it->volume = newLoop;
            break;
        }
""",
        """        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            it->loop = newLoop;
            break;
        }
""",
        "loop setter",
    )

    text = replace_once(
        text,
        """    // Free audio buffer again because it wasn't actually used
    iplAudioBufferFree(GlobalState::phononCtx, &*audioBuffer);
""",
        """    // Free the buffer again if it was created but not consumed.
    if (audioBuffer.has_value())
        iplAudioBufferFree(GlobalState::phononCtx, &*audioBuffer);
""",
        "optional clip buffer cleanup",
    )

    context_block = """    {
        IPLContextSettings contextSettings{};
        contextSettings.version = STEAMAUDIO_VERSION;
        const IPLerror status = iplContextCreate(&contextSettings, &GlobalState::phononCtx);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage("initialize Steam Audio context", status));
    }

"""
    text = replace_once(
        text,
        context_block,
        context_block
        + """    if (!PhononPlayback::rayTracer.Initialize())
        FFI logWarn("Steam Audio ray tracing initialization failed; playback will use the non-raytraced fallback."_cs);

""",
        "ray tracer initialization",
    )

    old_unload = """STEAMAUDIOIMPL_EXPORT void onUnload() {
    hooks.clear();

    FFI logInfo("Unloading miniaudio..."_cs);
    ma_device_uninit(&GlobalState::maDevice);

    FFI logInfo("Unloading Steam Audio..."_cs);
    PhononPlayback::env.reset();
    iplContextRelease(&GlobalState::phononCtx);
}
"""
    new_unload = """STEAMAUDIOIMPL_EXPORT void onUnload() {
    hooks.clear();

    FFI logInfo("Unloading miniaudio..."_cs);
    ma_device_uninit(&GlobalState::maDevice);

    FFI logInfo("Unloading Steam Audio..."_cs);
    {
        std::scoped_lock lock(PhononPlayback::playbackMutex);
        PhononPlayback::playbackQueue.clear();
    }
    PhononPlayback::rayTracer.Shutdown();
    PhononPlayback::env.reset();
    iplContextRelease(&GlobalState::phononCtx);
}
"""
    text = replace_once(text, old_unload, new_unload, "shutdown order")

    text = replace_once(
        text,
        """    // Display basic stats
    if (showStats) {
""",
        """    // Keep collision geometry synchronized with Unity. This runs outside
    // the audio queue lock so mesh extraction cannot block the audio callback.
    rayTracer.UpdateScene();

    // Display basic stats
    if (showStats) {
""",
        "scene synchronization",
    )

    text = replace_once(
        text,
        """        FFI ImGuiText(FFI toCsString(("Frame size: " + std::to_string(env->audioSettings.frameSize)).c_str()));
""",
        """        FFI ImGuiText(FFI toCsString(("Frame size: " +
            std::to_string(env ? env->audioSettings.frameSize : 0)).c_str()));
""",
        "safe frame-size statistics",
    )

    old_loop = """    std::scoped_lock L(playbackMutex);
    for (auto playback = playbackQueue.begin(); playback != playbackQueue.end(); ++playback) {
        if (PhononTools::updatePlayback(*playback, false)) {
            if (showStats) {
                FFI ImGuiSeparator();
                FFI ImGuiText(FFI toCsString(("Position: " + std::to_string(playback->playPosition)).c_str()));
                FFI ImGuiText(FFI toCsString(("Length: " + std::to_string(playback->audioBuffer.numSamples)).c_str()));
                FFI ImGuiText(FFI toCsString(("Volume: " + std::to_string(playback->volume)).c_str()));
                FFI ImGuiText(FFI toCsString(("Volume Scale: " + std::to_string(playback->volumeScale)).c_str()));
                FFI ImGuiText(FFI toCsString(("Spatial blend: " + std::to_string(playback->spatialBlend)).c_str()));
                FFI ImGuiText(FFI toCsString(("Max distance: " + std::to_string(playback->maxDistance)).c_str()));
                FFI ImGuiText(FFI toCsString(("Loop: " + std::string(call<"UnityEngine.AudioSource$$get_loop", bool>(
                                                                         playback->audioSource, nullptr)
                                                                         ? "yes"
                                                                         : "no"))
                                                 .c_str()));
                FFI ImGuiText(FFI toCsString(("World position: " + Utils::formatPos(playback->worldPosition)).c_str()));
            }
        } else {
            playbackQueue.erase(playback);
            break;
        }
    }

    if (showStats)
        FFI ImGuiEnd();
"""
    new_loop = """    std::scoped_lock L(playbackMutex);
    for (auto playback = playbackQueue.begin(); playback != playbackQueue.end();) {
        if (PhononTools::updatePlayback(*playback, false)) {
            if (showStats) {
                FFI ImGuiSeparator();
                FFI ImGuiText(FFI toCsString(("Position: " + std::to_string(playback->playPosition)).c_str()));
                FFI ImGuiText(FFI toCsString(("Length: " + std::to_string(playback->audioBuffer.numSamples)).c_str()));
                FFI ImGuiText(FFI toCsString(("Volume: " + std::to_string(playback->volume)).c_str()));
                FFI ImGuiText(FFI toCsString(("Volume Scale: " + std::to_string(playback->volumeScale)).c_str()));
                FFI ImGuiText(FFI toCsString(("Spatial blend: " + std::to_string(playback->spatialBlend)).c_str()));
                FFI ImGuiText(FFI toCsString(("Max distance: " + std::to_string(playback->maxDistance)).c_str()));
                FFI ImGuiText(FFI toCsString(("Loop: " + std::string(playback->loop ? "yes" : "no")).c_str()));
                FFI ImGuiText(FFI toCsString(("World position: " + Utils::formatPos(playback->worldPosition)).c_str()));
            }
            ++playback;
        } else {
            playback = playbackQueue.erase(playback);
        }
    }

    rayTracer.Simulate(playbackQueue);

    if (showStats)
        FFI ImGuiEnd();
"""
    text = replace_once(text, old_loop, new_loop, "playback update and simulation loop")

    return text


def validate_outputs(outputs: dict[Path, str]) -> None:
    required_tokens = {
        "playback_environment.cpp": [
            "iplInstancedMeshUpdateTransform",
            "iplSimulatorRunDirect",
            "iplSimulatorRunReflections",
            "UnityEngine.MeshCollider$$get_sharedMesh",
        ],
        "playback.cpp": [
            "IPL_TRANSMISSIONTYPE_FREQDEPENDENT",
            "iplReflectionEffectApply",
            "iplAmbisonicsDecodeEffectApply",
        ],
        "fixedsettings.hpp": [
            "IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION",
            "IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION",
        ],
    }

    by_name = {path.name: content for path, content in outputs.items()}
    for name, tokens in required_tokens.items():
        if name not in by_name:
            raise RuntimeError(f"Internal error: no generated output for {name}")
        for token in tokens:
            if token not in by_name[name]:
                raise RuntimeError(
                    f"Internal validation failed: {token!r} missing from {name}"
                )

    for path, content in outputs.items():
        if "\0" in content:
            raise RuntimeError(f"Generated output for {path} contains a NUL byte")
        if not content.endswith("\n"):
            outputs[path] = content + "\n"


def main() -> int:
    project = Path.cwd().resolve()

    required_names = [
        "fixedsettings.hpp",
        "bufferpool.hpp",
        "playback_environment.hpp",
        "playback_environment.cpp",
        "playback.hpp",
        "playback.cpp",
        "phonontools.cpp",
        "SteamAudioImpl.cpp",
        "utils.cpp",
    ]

    try:
        paths = {name: locate(project, name) for name in required_names}

        originals = {
            path: path.read_text(encoding="utf-8")
            for path in paths.values()
        }

        outputs: dict[Path, str] = {
            paths["fixedsettings.hpp"]: FIXEDSETTINGS_HPP,
            paths["bufferpool.hpp"]: BUFFERPOOL_HPP,
            paths["playback_environment.hpp"]: PLAYBACK_ENVIRONMENT_HPP,
            paths["playback_environment.cpp"]: PLAYBACK_ENVIRONMENT_CPP,
            paths["playback.hpp"]: PLAYBACK_HPP,
            paths["playback.cpp"]: PLAYBACK_CPP,
            paths["phonontools.cpp"]: patch_phonontools_cpp(
                originals[paths["phonontools.cpp"]]
            ),
            paths["SteamAudioImpl.cpp"]: patch_steam_audio_impl(
                originals[paths["SteamAudioImpl.cpp"]]
            ),
            paths["utils.cpp"]: patch_utils_cpp(
                originals[paths["utils.cpp"]]
            ),
        }

        validate_outputs(outputs)

        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_root = project / f".steamaudio_raytracing_backup_{timestamp}"
        backup_root.mkdir(parents=True, exist_ok=False)

        for path in paths.values():
            relative = path.relative_to(project)
            destination = backup_root / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, destination)

        written: list[Path] = []
        try:
            for path, content in outputs.items():
                atomic_write(path, content)
                written.append(path)
        except Exception:
            print("An error occurred while writing files; restoring backups...", file=sys.stderr)
            for path in written:
                relative = path.relative_to(project)
                shutil.copy2(backup_root / relative, path)
            raise

        print("Steam Audio ray tracing patch applied successfully.")
        print(f"Backup directory: {backup_root}")
        print("Modified files:")
        for path in sorted(outputs):
            print(f"  - {path.relative_to(project)}")

        print()
        print("Important:")
        print("  1. Rebuild the native project.")
        print("  2. Ensure the installed Steam Audio SDK exposes the simulator,")
        print("     instanced-mesh, reflection-mixer, and ambisonics APIs.")
        print("  3. Test against the exact Unity version used by the game; the")
        print("     runtime resolves MeshCollider through Unity's reflection API.")
        return 0

    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

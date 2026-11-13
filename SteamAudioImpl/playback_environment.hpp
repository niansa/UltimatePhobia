#pragma once

#include "SteamAudioImpl.hpp"
#include "bufferpool.hpp"

#include <chrono>
#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <phonon.h>
#include <ffi_interface.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <thread>
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
    using Clock = std::chrono::steady_clock;

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
    void DestroySource(IPLSource source);
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

    bool ResolveColliderTypes();
    void RescanColliders();
    void UpdateColliderTransforms();

    bool AddCollider(FFIInterface::ObjectHandle collider, int colliderId, FFIInterface::ObjectHandle mesh, int meshId);
    Geometry *AcquireBoxGeometry(FFIInterface::ObjectHandle boxCollider, int colliderId);
    bool AddBoxCollider(FFIInterface::ObjectHandle collider, int colliderId);
    void RemoveCollider(int colliderId);
    Geometry *AcquireGeometry(FFIInterface::ObjectHandle mesh, int meshId);
    void ReleaseGeometry(int meshId);

    bool SetColliderActive(Collider& collider, bool active, const IPLMatrix4x4 *transform);
    void CommitScene();
    bool StartReflectionWorker();
    void StopReflectionWorker();
    void QueueReflectionSimulation();
    void ReflectionWorkerMain();
    static bool MatricesDiffer(const IPLMatrix4x4& a, const IPLMatrix4x4& b);

    IPLScene scene_ = nullptr;
    IPLSimulator simulator_ = nullptr;
    FFIInterface::ObjectHandle meshColliderType_ = FFIInterface::ObjectHandle::Invalid;
    FFIInterface::ObjectHandle boxColliderType_ = FFIInterface::ObjectHandle::Invalid;
    FFIInterface::ObjectHandle meshRendererType_ = FFIInterface::ObjectHandle::Invalid;

    std::unordered_map<int, Geometry> geometries_;
    std::unordered_map<int, Collider> colliders_;
    std::unordered_set<int> unreadableMeshes_;

    Clock::time_point lastTransformUpdate_{};
    Clock::time_point lastColliderScan_{};
    Clock::time_point lastReflectionUpdate_{};

    bool sceneDirty_ = false;
    std::shared_mutex simulationMutex_;
    std::mutex reflectionMutex_;
    std::condition_variable reflectionCv_;
    std::thread reflectionThread_;
    bool reflectionStop_ = false;
    bool reflectionPending_ = false;

    std::atomic_bool reflectionsReady_{false};
};

extern std::optional<Environment> env;
extern RayTracer rayTracer;
} // namespace PhononPlayback

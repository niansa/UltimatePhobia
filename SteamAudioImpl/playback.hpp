#pragma once

#include "SteamAudioImpl.hpp"

#include <list>
#include <mutex>
#include <optional>
#include <ffi_interface.hpp>

typedef struct ma_device ma_device;

namespace PhononPlayback {
struct Playback {
    FFIInterface::ObjectHandle audioSource;
    FFIInterface::GCHandle audioSourceGc;
    uint64_t delay;
    float volumeScale;
    float volume = 0.5f;
    float spatialBlend = 1.0f;
    float maxDistance = 1000.0f;
    bool isOneShot;
    bool loop = false;
    uint64_t playPosition = 0;
    IPLVector3 worldPosition{};
    std::optional<IPLSimulationOutputs> simulationOutputsCache;

    IPLAudioBuffer audioBuffer;
    IPLBinauralEffect binauralEffect = nullptr;
    IPLDirectEffect directEffect = nullptr;
    IPLReflectionEffect reflectionEffect = nullptr;
    IPLAmbisonicsBinauralEffect ambisonicsBinauralEffect = nullptr;
    IPLSource source = nullptr;

    Playback(FFIInterface::ObjectHandle audioSource, const IPLAudioBuffer& audioBuffer, uint64_t delay, float volumeScale, bool isOneShot);
    ~Playback();
    Playback(const Playback&) = delete;
    Playback(Playback&& o)
        : audioSource(o.audioSource), audioSourceGc(o.audioSourceGc), audioBuffer(o.audioBuffer), delay(o.delay), volumeScale(o.volumeScale),
          playPosition(o.playPosition) {
        o.audioSource = FFIInterface::ObjectHandle::Invalid;
    }

    IPLAudioBuffer& operator=(const IPLAudioBuffer& audioBuffer);

    void updateSimulationOutputs();

    bool hasReachedEnd() const { return playPosition >= audioBuffer.numSamples; }
};
extern std::list<Playback> playbackQueue;
extern std::mutex playbackMutex;

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput,
                  ma_uint32 sampleCount);
} // namespace PhononPlayback

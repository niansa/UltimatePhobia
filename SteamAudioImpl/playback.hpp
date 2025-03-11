#pragma once

#include "SteamAudioImpl.hpp"

#include <list>
#include <mutex>
#include <ffi_interface.hpp>

typedef struct ma_device ma_device;

namespace PhononPlayback {
struct Playback {
    FFIInterface::ObjectHandle audioSourceObject;
    FFIInterface::ObjectHandle cachedAudioSource = FFIInterface::ObjectHandle::Null;
    FFIInterface::GCHandle audioSourceObjectGc;
    FFIInterface::GCHandle cachedAudioSourceGc;
    uint64_t delay;
    float volumeScale;
    float volume = 0.5f;
    float spatialBlend = 1.0f;
    float maxDistance = 1000.0f;
    bool isOneShot;
    uint64_t playPosition = 0;
    IPLVector3 worldPosition{};

    IPLAudioBuffer audioBuffer;
    IPLBinauralEffect binauralEffect = nullptr;
    IPLDirectEffect directEffect = nullptr;

    Playback(FFIInterface::ObjectHandle audioSourceObject, const IPLAudioBuffer& audioBuffer, uint64_t delay, float volumeScale, bool isOneShot);
    ~Playback();
    Playback(const Playback&) = delete;
    Playback(Playback&& o)
        : audioSourceObject(o.audioSourceObject), audioSourceObjectGc(o.audioSourceObjectGc), audioBuffer(o.audioBuffer), delay(o.delay),
          volumeScale(o.volumeScale), playPosition(o.playPosition) {
        o.audioSourceObject = FFIInterface::ObjectHandle::Invalid;
    }

    IPLAudioBuffer& operator=(const IPLAudioBuffer& audioBuffer);

    FFIInterface::ObjectHandle getAudioSource();

    bool hasReachedEnd() const { return playPosition >= audioBuffer.numSamples; }
};
extern std::list<Playback> playbackQueue;
extern std::mutex playbackQueueMutex;

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput,
                  ma_uint32 sampleCount);
} // namespace PhononPlayback

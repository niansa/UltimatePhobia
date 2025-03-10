#pragma once

#include "SteamAudioImpl.hpp"

#include <list>
#include <mutex>
#include <ffi_interface.hpp>

typedef struct ma_device ma_device;

namespace PhononPlayback {
struct Playback {
    FFIInterface::ObjectHandle audioSource;
    FFIInterface::GCHandle audioSourceGc;
    uint64_t delay;
    float volumeScale;
    float volume = 0.5f;
    bool isOneShot;
    uint64_t playPosition = 0;
    IPLVector3 worldPosition{};

    IPLAudioBuffer audioBuffer;
    IPLBinauralEffect binauralEffect = nullptr;
    IPLDirectEffect directEffect = nullptr;

    Playback(FFIInterface::ObjectHandle audioSource, IPLAudioBuffer audioBuffer,
             uint64_t delay, float volumeScale, bool isOneShot);
    ~Playback();
    Playback(const Playback&) = delete;
    Playback(Playback&& o)
        : audioSource(o.audioSource), audioSourceGc(o.audioSourceGc),
          audioBuffer(o.audioBuffer), delay(o.delay),
          volumeScale(o.volumeScale), playPosition(o.playPosition) {
        o.audioSource = FFIInterface::ObjectHandle::Invalid;
    }

    bool hasReachedEnd() const {
        return playPosition >= audioBuffer.numSamples;
    }
};
extern std::list<Playback> playbackQueue;
extern std::mutex playbackQueueMutex;

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput,
                  ma_uint32 sampleCount);
} // namespace PhononPlayback

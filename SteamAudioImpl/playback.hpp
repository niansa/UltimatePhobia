#pragma once

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

    Playback(FFIInterface::ObjectHandle audioSource, const IPLAudioBuffer& audioBuffer, uint64_t delay,
             float volumeScale, bool isOneShot);
    ~Playback();

    Playback(const Playback&) = delete;
    Playback(Playback&&) = delete;
    Playback& operator=(const Playback&) = delete;
    Playback& operator=(Playback&&) = delete;

    IPLAudioBuffer& operator=(const IPLAudioBuffer& audioBuffer);

    bool EnsureAudioResources(IPLint32 frameSize);
    bool hasReachedEnd() const { return !loop && playPosition >= static_cast<uint64_t>(audioBuffer.numSamples); }
};

extern std::list<Playback> playbackQueue;
extern std::mutex playbackMutex;

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput, ma_uint32 sampleCount);
} // namespace PhononPlayback

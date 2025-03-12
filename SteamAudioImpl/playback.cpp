#include "playback.hpp"
#include "SteamAudioImpl.hpp"
#include "playback_environment.hpp"
#include "simulation_environment.hpp"
#include "fixedsettings.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <phonon.h>
#include <ffi_interface.hpp>

using namespace FFIInterface;

namespace PhononPlayback {
std::list<Playback> playbackQueue;
std::mutex playbackQueueMutex;

namespace Math {
float distance(const IPLVector3& a, const IPLVector3& b) { return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2) + pow(b.z - a.z, 2)); }

float maxDistanceAttenuation(float distance, float maxDistance) {
    if (maxDistance <= 0.0f) {
        // Handle invalid parameters
        return (distance <= 0.0f) ? 1.0f : 0.0f;
    }

    float normalizedDistance = distance / maxDistance;
    normalizedDistance = std::clamp(normalizedDistance, 0.0f, 1.0f);

    // Square the normalized distance to make falloff increase with distance
    float exponent = -1.0f * normalizedDistance * normalizedDistance;
    float numerator = expf(exponent) - expf(-1.0f);
    float denominator = 1.0f - expf(-1.0f);

    return numerator / denominator;
}
} // namespace Math

Playback::Playback(FFIInterface::ObjectHandle audioSource, const IPLAudioBuffer& audioBuffer, uint64_t delay, float volumeScale, bool isOneShot)
    : audioSource(audioSource), audioBuffer(audioBuffer), delay(delay), volumeScale(volumeScale), isOneShot(isOneShot) {
    audioSourceGc = FFI gcCreateHandle(audioSource, true);

    {
        IPLBinauralEffectSettings effectSettings{.hrtf = env->hrtf};
        iplBinauralEffectCreate(GlobalState::phononCtx, &env->audioSettings, &effectSettings, &binauralEffect);
    }

    {
        IPLDirectEffectSettings directEffectSettings{.numChannels = FixedSettings::outputChannels};
        iplDirectEffectCreate(GlobalState::phononCtx, const_cast<IPLAudioSettings *>(&env->audioSettings), &directEffectSettings, &directEffect);
    }

    if (PhononSimulation::env.has_value()) {
        FFI logInfo(FFI toCsString("Adding simulator source..."));
        using PhononSimulation::env;
        IPLSourceSettings sourceSettings{.flags = IPL_SIMULATIONFLAGS_DIRECT};
        iplSourceCreate(env->getSimulator(), &sourceSettings, &source);
        iplSourceAdd(source, env->getSimulator());
    }
}
Playback::~Playback() {
    if (audioSource == ObjectHandle::Null)
        return;
    iplAudioBufferFree(GlobalState::phononCtx, &audioBuffer);
    iplBinauralEffectRelease(&binauralEffect);
    iplDirectEffectRelease(&directEffect);
    if (source) {
        FFI logInfo(FFI toCsString("Removing simulator source..."));
        if (PhononSimulation::env.has_value()) {
            using PhononSimulation::env;
            iplSourceRemove(source, env->getSimulator());
        }
        iplSourceRelease(&source);
    }

    FFI dropObject(audioSource);
    FFI gcDeleteHandle(audioSourceGc);
}

IPLAudioBuffer& Playback::operator=(const IPLAudioBuffer& audioBuffer) {
    iplAudioBufferFree(GlobalState::phononCtx, &this->audioBuffer);
    playPosition = 0;
    return this->audioBuffer = audioBuffer;
}

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput, ma_uint32 sampleCount) {
    // Reconstruct environment if frame size is wrong
    if (!env.has_value() || env->audioSettings.frameSize != sampleCount)
        env.emplace(sampleCount);

    // Allocate frame buffer
    IPLAudioBuffer frameBuffer{};
    IPLerror status = iplAudioBufferAllocate(GlobalState::phononCtx, FixedSettings::outputChannels, sampleCount, &frameBuffer);
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(Utils::createErrorMessage("create output frame buffer", status));

    // Clear frame buffer
    for (unsigned channel = 0; channel != frameBuffer.numChannels; channel++)
        for (unsigned sample = 0; sample != frameBuffer.numSamples; sample++)
            frameBuffer.data[channel][sample] = 0.0f;

    {
        std::scoped_lock L(playbackQueueMutex);
        for (auto& playback : playbackQueue) {
            // Process delay
            if (playback.delay > 0) {
                playback.delay -= sampleCount;
                continue;
            }

            // Skip buffers that have reached the end
            if (playback.hasReachedEnd())
                continue;

            // Calculate available samples from input
            const auto availableSamples = playback.audioBuffer.numSamples - playback.playPosition;

            // Allocate input buffer
            IPLAudioBuffer inputBuffer;
            IPLerror status = iplAudioBufferAllocate(GlobalState::phononCtx, playback.audioBuffer.numChannels, sampleCount, &inputBuffer);
            if (status != IPL_STATUS_SUCCESS)
                FFI logError(Utils::createErrorMessage("create input snippet buffer", status));

            // Copy audio data while applying volume
            const float volume = playback.volume <= 1.0f ? std::min<float>(playback.volume * playback.volumeScale, 1.0f) * 4.0f : 0.0f;
            for (unsigned channel = 0; channel != inputBuffer.numChannels; channel++) {
                for (unsigned sample = 0; sample != inputBuffer.numSamples; ++sample) {
                    const auto position = playback.playPosition + sample;
                    float value;
                    if (playback.loop)
                        value = playback.audioBuffer.data[channel][position % playback.audioBuffer.numSamples];
                    if (sample < availableSamples)
                        value = playback.audioBuffer.data[channel][position];
                    else
                        value = 0.0f;
                    inputBuffer.data[channel][sample] = value * volume;
                }
            }

            if (playback.spatialBlend != 0.0f) {
                // Apply binaural effect
                IPLBinauralEffectParams effectParams{
                    .interpolation = IPL_HRTFINTERPOLATION_BILINEAR, .spatialBlend = playback.spatialBlend, .hrtf = env->hrtf, .peakDelays = nullptr};
                effectParams.direction = iplCalculateRelativeDirection(GlobalState::phononCtx, playback.worldPosition, GlobalState::playerCoord.origin,
                                                                       GlobalState::playerCoord.ahead, GlobalState::playerCoord.up);
                effectParams.direction.x = -effectParams.direction.x;
                iplBinauralEffectApply(playback.binauralEffect, &effectParams, &inputBuffer, &env->bufferPool.GetCurrentBuffer());
            } else {
                // Copy input to output without spatialization, up/down mix as needed
                if (inputBuffer.numChannels > FixedSettings::outputChannels) {
                    // Downmix
                    iplAudioBufferDownmix(GlobalState::phononCtx, &inputBuffer, &env->bufferPool.GetCurrentBuffer());
                } else if (inputBuffer.numChannels == FixedSettings::outputChannels) {
                    // Copy
                    auto& nbuf = env->bufferPool.GetCurrentBuffer();
                    for (unsigned channel = 0; channel != FixedSettings::outputChannels; ++channel)
                        memcpy(nbuf.data[channel], inputBuffer.data[channel], nbuf.numSamples * sizeof(float));
                } else {
                    // Upmix
                    auto& nbuf = env->bufferPool.GetCurrentBuffer();
                    for (unsigned channel = 0; channel != FixedSettings::outputChannels; ++channel)
                        memcpy(nbuf.data[channel], inputBuffer.data[channel % inputBuffer.numChannels], nbuf.numSamples * sizeof(float));
                }
            }

            // Calculate distance
            const float distance = Math::distance(GlobalState::playerCoord.origin, playback.worldPosition);

            // Apply direct effect without simulation
            {
                IPLDistanceAttenuationModel distanceAttenuationModel{IPL_DISTANCEATTENUATIONTYPE_DEFAULT};
                IPLAirAbsorptionModel airAbsorptionModel{IPL_AIRABSORPTIONTYPE_DEFAULT};
                IPLDirectEffectParams directEffectParams{
                    .flags = static_cast<IPLDirectEffectFlags>(IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION | IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION),
                    .distanceAttenuation = iplDistanceAttenuationCalculate(GlobalState::phononCtx, playback.worldPosition, GlobalState::playerCoord.origin,
                                                                           &distanceAttenuationModel) *
                                           Math::maxDistanceAttenuation(distance, playback.maxDistance)};
                iplAirAbsorptionCalculate(GlobalState::phononCtx, playback.worldPosition, GlobalState::playerCoord.origin, &airAbsorptionModel,
                                          directEffectParams.airAbsorption);
                iplDirectEffectApply(playback.directEffect, &directEffectParams, &env->bufferPool.GetCurrentBuffer(), &env->bufferPool.GetNextBuffer());
                env->bufferPool.SwitchToNextBuffer();
            }

            // Mix into frame buffer
            iplAudioBufferMix(GlobalState::phononCtx, &env->bufferPool.GetCurrentBuffer(), &frameBuffer);

            // Deallocate input buffer
            iplAudioBufferFree(GlobalState::phononCtx, &inputBuffer);

            // Increment play position
            playback.playPosition += sampleCount;

            // Wrap position over if looping
            if (playback.loop && playback.hasReachedEnd())
                playback.playPosition = playback.playPosition % playback.audioBuffer.numSamples;
        }
    }

    iplAudioBufferInterleave(GlobalState::phononCtx, &frameBuffer, pOutput);
    iplAudioBufferFree(GlobalState::phononCtx, &frameBuffer);
}
} // namespace PhononPlayback

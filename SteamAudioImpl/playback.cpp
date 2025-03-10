#include "playback.hpp"
#include "SteamAudioImpl.hpp"
#include "environment.hpp"
#include "fixedsettings.hpp"
#include "utils.hpp"

using namespace FFIInterface;

namespace PhononPlayback {
std::list<Playback> playbackQueue;
std::mutex playbackQueueMutex;

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
}
Playback::~Playback() {
    if (audioSource == ObjectHandle::Invalid || audioSource == ObjectHandle::Null)
        return;
    iplAudioBufferFree(GlobalState::phononCtx, &audioBuffer);
    iplBinauralEffectRelease(&binauralEffect);
    iplDirectEffectRelease(&directEffect);

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

            // Copy audio data while applying volume scale
            for (unsigned channel = 0; channel != inputBuffer.numChannels; channel++) {
                for (unsigned sample = 0; sample != inputBuffer.numSamples; ++sample) {
                    if (sample < availableSamples)
                        inputBuffer.data[channel][sample] =
                            playback.audioBuffer.data[channel][playback.playPosition + sample] * (playback.volume * playback.volumeScale);
                    else
                        inputBuffer.data[channel][sample] = 0.0f;
                }
            }

            if (playback.spatialBlend != 0.0f) {
                // Apply binaural effect
                IPLBinauralEffectParams effectParams{
                    .interpolation = IPL_HRTFINTERPOLATION_BILINEAR, .spatialBlend = playback.spatialBlend, .hrtf = env->hrtf, .peakDelays = nullptr};
                effectParams.direction = iplCalculateRelativeDirection(GlobalState::phononCtx, playback.worldPosition, GlobalState::playerPos,
                                                                       GlobalState::playerAhead, GlobalState::playerUp);
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

            // Apply direct effect without simulation
            {
                IPLDistanceAttenuationModel distanceAttenuationModel{IPL_DISTANCEATTENUATIONTYPE_DEFAULT};
                IPLAirAbsorptionModel airAbsorptionModel{IPL_AIRABSORPTIONTYPE_DEFAULT};
                IPLDirectEffectParams directEffectParams{
                    .flags = static_cast<IPLDirectEffectFlags>(IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION | IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION),
                    .distanceAttenuation =
                        iplDistanceAttenuationCalculate(GlobalState::phononCtx, playback.worldPosition, GlobalState::playerPos, &distanceAttenuationModel)};
                iplAirAbsorptionCalculate(GlobalState::phononCtx, playback.worldPosition, GlobalState::playerPos, &airAbsorptionModel,
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
        }
    }

    iplAudioBufferInterleave(GlobalState::phononCtx, &frameBuffer, pOutput);
    iplAudioBufferFree(GlobalState::phononCtx, &frameBuffer);
}
} // namespace PhononPlayback

#include "playback.hpp"

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

    float normalizedDistance = std::clamp(distance / maxDistance, 0.0f, 1.0f);
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

bool allocateBuffer(IPLint32 channels, IPLint32 samples, IPLAudioBuffer& buffer, const char *operation) {
    const IPLerror status = iplAudioBufferAllocate(GlobalState::phononCtx, channels, samples, &buffer);
    if (status == IPL_STATUS_SUCCESS)
        return true;

    FFI logError(Utils::createErrorMessage(operation, status));
    buffer = {};
    return false;
}
} // namespace

Playback::Playback(ObjectHandle audioSource, const IPLAudioBuffer& audioBuffer, uint64_t delay, float volumeScale,
                   bool isOneShot)
    : audioSource(audioSource), delay(delay), volumeScale(volumeScale), isOneShot(isOneShot), audioBuffer(audioBuffer) {
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
        rayTracer.DestroySource(raySource);

    if (audioSource != ObjectHandle::Null && audioSource != ObjectHandle::Invalid) {
        FFI dropObject(audioSource);
        FFI gcDeleteHandle(audioSourceGc);
    }
}

bool Playback::EnsureAudioResources(IPLint32 frameSize) {
    if (!env || frameSize <= 0)
        return false;

    if (effectFrameSize == frameSize && inputScratch.data && monoScratch.data && binauralEffect && directEffect &&
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

    if (!allocateBuffer(audioBuffer.numChannels, frameSize, inputScratch, "allocate an input audio scratch buffer"))
        return false;
    if (!allocateBuffer(1, frameSize, monoScratch, "allocate a mono audio scratch buffer"))
        return false;

    IPLBinauralEffectSettings binauralSettings{};
    binauralSettings.hrtf = env->hrtf;
    IPLerror status =
        iplBinauralEffectCreate(GlobalState::phononCtx, &env->audioSettings, &binauralSettings, &binauralEffect);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create a binaural effect", status));
        return false;
    }

    IPLDirectEffectSettings directSettings{};
    directSettings.numChannels = FixedSettings::outputChannels;
    status = iplDirectEffectCreate(GlobalState::phononCtx, &env->audioSettings, &directSettings, &directEffect);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create a direct sound effect", status));
        return false;
    }

    IPLReflectionEffectSettings reflectionSettings{};
    reflectionSettings.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    reflectionSettings.irSize =
        static_cast<IPLint32>(std::ceil(env->audioSettings.samplingRate * FixedSettings::reflectionDurationSeconds));
    reflectionSettings.numChannels = FixedSettings::ambisonicsChannels;

    status =
        iplReflectionEffectCreate(GlobalState::phononCtx, &env->audioSettings, &reflectionSettings, &reflectionEffect);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create a reflection effect", status));
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

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput, ma_uint32 sampleCount) {
    (void)pDevice;
    (void)pInput;

    ++GlobalState::stats.framesStarted;

    if (!pOutput || sampleCount == 0) {
        ++GlobalState::stats.framesFinished;
        return;
    }

    if (!env.has_value() || env->audioSettings.frameSize != static_cast<IPLint32>(sampleCount)) {
        env.emplace(static_cast<IPLint32>(sampleCount));
    }

    IPLAudioBuffer& frameBuffer = env->frameBuffer;
    clearBuffer(frameBuffer);

    bool reflectionInputApplied = false;

    {
        std::scoped_lock lock(playbackMutex);

        for (Playback& playback : playbackQueue) {
            ++GlobalState::stats.playbacksStarted;

            if (!playback.EnsureAudioResources(static_cast<IPLint32>(sampleCount))) {
                continue;
            }

            if (playback.audioBuffer.numSamples <= 0 || playback.audioBuffer.numChannels <= 0) {
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

            const float volume = std::max(0.0f, playback.volume * playback.volumeScale);
            const uint64_t samplesConsumed = sampleCount - static_cast<ma_uint32>(silentPrefix);

            for (IPLint32 channel = 0; channel < playback.inputScratch.numChannels; ++channel) {
                for (ma_uint32 sample = static_cast<ma_uint32>(silentPrefix); sample < sampleCount; ++sample) {
                    const uint64_t sourceOffset = sample - static_cast<ma_uint32>(silentPrefix);
                    const uint64_t position = playback.playPosition + sourceOffset;

                    float value = 0.0f;
                    if (playback.loop) {
                        const uint64_t wrapped = position % playback.audioBuffer.numSamples;
                        value = playback.audioBuffer.data[channel][wrapped];
                    } else if (position < static_cast<uint64_t>(playback.audioBuffer.numSamples)) {
                        value = playback.audioBuffer.data[channel][position];
                    }

                    playback.inputScratch.data[channel][sample] = value * volume;
                }
            }

            if (playback.inputScratch.numChannels == 1) {
                std::memcpy(playback.monoScratch.data[0], playback.inputScratch.data[0], sampleCount * sizeof(float));
            } else {
                iplAudioBufferDownmix(GlobalState::phononCtx, &playback.inputScratch, &playback.monoScratch);
            }

            IPLSimulationOutputs simulationOutputs{};
            bool haveSimulationOutput = false;
            if (playback.raySource && rayTracer.IsInitialized()) {
                iplSourceGetOutputs(
                    playback.raySource,
                    static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS),
                    &simulationOutputs);
                haveSimulationOutput = true;
            }

            if (haveSimulationOutput && rayTracer.ReflectionsReady() && playback.reflectionEffect &&
                env->reflectionMixer) {
                iplReflectionEffectApply(playback.reflectionEffect, &simulationOutputs.reflections,
                                         &playback.monoScratch, &env->ambisonicsBufferPool.GetCurrentBuffer(),
                                         env->reflectionMixer);
                reflectionInputApplied = true;
            }

            IPLAudioBuffer& spatialized = env->bufferPool.GetCurrentBuffer();

            if (playback.spatialBlend > 0.0f) {
                IPLBinauralEffectParams binauralParams{};
                binauralParams.interpolation = IPL_HRTFINTERPOLATION_BILINEAR;
                binauralParams.spatialBlend = std::clamp(playback.spatialBlend, 0.0f, 1.0f);
                binauralParams.hrtf = env->hrtf;
                binauralParams.peakDelays = nullptr;
                binauralParams.direction = iplCalculateRelativeDirection(
                    GlobalState::phononCtx, playback.worldPosition, GlobalState::playerCoord.origin,
                    GlobalState::playerCoord.ahead, GlobalState::playerCoord.up);
                binauralParams.direction.x = -binauralParams.direction.x;

                iplBinauralEffectApply(playback.binauralEffect, &binauralParams, &playback.monoScratch, &spatialized);
            } else if (playback.inputScratch.numChannels == FixedSettings::outputChannels) {
                for (unsigned channel = 0; channel < FixedSettings::outputChannels; ++channel) {
                    std::memcpy(spatialized.data[channel], playback.inputScratch.data[channel],
                                sampleCount * sizeof(float));
                }
            } else if (playback.inputScratch.numChannels > FixedSettings::outputChannels) {
                iplAudioBufferDownmix(GlobalState::phononCtx, &playback.inputScratch, &spatialized);
            } else {
                for (unsigned channel = 0; channel < FixedSettings::outputChannels; ++channel) {
                    std::memcpy(spatialized.data[channel],
                                playback.inputScratch.data[channel % playback.inputScratch.numChannels],
                                sampleCount * sizeof(float));
                }
            }

            IPLDirectEffectParams directParams{};
            directParams.flags = FixedSettings::directEffectFlags;
            directParams.transmissionType = IPL_TRANSMISSIONTYPE_FREQDEPENDENT;
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
                directParams.distanceAttenuation = simulationOutputs.direct.distanceAttenuation;
                std::memcpy(directParams.airAbsorption, simulationOutputs.direct.airAbsorption,
                            sizeof(directParams.airAbsorption));
                directParams.directivity = simulationOutputs.direct.directivity;
                directParams.occlusion = simulationOutputs.direct.occlusion;
                std::memcpy(directParams.transmission, simulationOutputs.direct.transmission,
                            sizeof(directParams.transmission));
            } else {
                IPLDistanceAttenuationModel distanceModel{};
                distanceModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
                directParams.distanceAttenuation = iplDistanceAttenuationCalculate(
                    GlobalState::phononCtx, playback.worldPosition, GlobalState::playerCoord.origin, &distanceModel);

                IPLAirAbsorptionModel absorptionModel{};
                absorptionModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;
                iplAirAbsorptionCalculate(GlobalState::phononCtx, playback.worldPosition,
                                          GlobalState::playerCoord.origin, &absorptionModel,
                                          directParams.airAbsorption);
            }

            // if (playback.rolloffMode != AudioRolloffMode::Logarithmic) {
            //     const float distance = Math::distance(GlobalState::playerCoord.origin, playback.worldPosition);
            //     directParams.distanceAttenuation *= Math::maxDistanceAttenuation(distance, playback.maxDistance);
            // }

            iplDirectEffectApply(playback.directEffect, &directParams, &env->bufferPool.GetCurrentBuffer(),
                                 &env->bufferPool.GetNextBuffer());
            env->bufferPool.SwitchToNextBuffer();

            iplAudioBufferMix(GlobalState::phononCtx, &env->bufferPool.GetCurrentBuffer(), &frameBuffer);

            playback.playPosition += samplesConsumed;
            if (playback.loop && playback.audioBuffer.numSamples > 0) {
                playback.playPosition %= playback.audioBuffer.numSamples;
            }

            ++GlobalState::stats.playbacksFinished;
        }
    }

    if (reflectionInputApplied && env->reflectionMixer && env->ambisonicsDecodeEffect) {
        IPLReflectionEffectParams mixerParams{};
        mixerParams.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
        mixerParams.numChannels = FixedSettings::ambisonicsChannels;
        mixerParams.irSize = static_cast<IPLint32>(
            std::ceil(env->audioSettings.samplingRate * FixedSettings::reflectionDurationSeconds));

        IPLAudioBuffer& ambisonics = env->ambisonicsBufferPool.GetCurrentBuffer();
        clearBuffer(ambisonics);

        iplReflectionMixerApply(env->reflectionMixer, &mixerParams, &ambisonics);

        IPLAmbisonicsDecodeEffectParams decodeParams{};
        decodeParams.order = FixedSettings::ambisonicsOrder;
        decodeParams.hrtf = env->hrtf;
        decodeParams.orientation = GlobalState::playerCoord;
        decodeParams.binaural = IPL_TRUE;

        IPLAudioBuffer& decoded = env->bufferPool.GetNextBuffer();
        iplAmbisonicsDecodeEffectApply(env->ambisonicsDecodeEffect, &decodeParams, &ambisonics, &decoded);

        iplAudioBufferMix(GlobalState::phononCtx, &decoded, &frameBuffer);
    }

    iplAudioBufferInterleave(GlobalState::phononCtx, &frameBuffer, pOutput);

    ++GlobalState::stats.framesFinished;
}
} // namespace PhononPlayback

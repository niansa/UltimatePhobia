#include "SteamAudioImpl.hpp"
#include "miniaudio.h"

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <array>
#include <mutex>
#include <optional>
#include <ffi_interface.hpp>
#include <phonon.h>

using namespace FFIInterface;
using namespace Helpers;
using namespace Literals;
#define FFI FFI_USE_FTABLE

const FFIInterface::Imports *imports;

namespace {
IPLContext phononCtx;
ma_device maDevice;

namespace FixedSettings {
constexpr unsigned outputChannels = 2;
}

struct Playback {
    ObjectHandle audioSource;
    GCHandle audioSourceGc;
    IPLAudioBuffer audioBuffer;
    uint64_t delay = 0;
    float volumeScale = 1.0f;
    uint64_t playPosition = 0;
    IPLVector3 worldPosition;

    Playback(ObjectHandle audioSource, IPLAudioBuffer audioBuffer,
             uint64_t delay, float volumeScale)
        : audioSource(audioSource), audioBuffer(audioBuffer), delay(delay),
          volumeScale(volumeScale) {
        audioSourceGc = FFI gcCreateHandle(audioSource, true);
    }
    ~Playback() {
        if (audioSource == ObjectHandle::Invalid ||
            audioSource == ObjectHandle::Null)
            return;
        FFI dropObject(audioSource);
        FFI gcDeleteHandle(audioSourceGc);
    }
    Playback(const Playback&) = delete;
    Playback(Playback&& o)
        : audioSource(o.audioSource), audioSourceGc(o.audioSourceGc),
          audioBuffer(o.audioBuffer), delay(o.delay),
          volumeScale(o.volumeScale), playPosition(o.playPosition) {
        o.audioSource = ObjectHandle::Invalid;
    }

    bool hasReachedEnd() const {
        return playPosition >= audioBuffer.numSamples;
    }
};
std::list<Playback> playbackQueue;
std::mutex playbackQueueMutex;

IPLVector3 playerPos, playerAhead, playerUp;

namespace Utils {
const char *errorToMessage(IPLerror error) {
    switch (error) {
    case IPL_STATUS_SUCCESS:
        return "The operation completed successfully.";
    case IPL_STATUS_FAILURE:
        return "An unspecified error occurred.";
    case IPL_STATUS_OUTOFMEMORY:
        return "The system ran out of memory.";
    case IPL_STATUS_INITIALIZATION:
        return "An error occurred while initializing an external dependency.";
    }
    return "Invalid error.";
}
const char *errorToMessage(ma_result error) {
    return ma_result_description(error);
}

ObjectHandle createErrorMessage(const char *what, auto why) {
    std::ostringstream fress("Failed to ");
    fress << what << "! Error was: " << errorToMessage(why);
    const std::string fres = fress.str();
    return FFI toCsStringWithLength(fres.data(), fres.size());
}
} // namespace Utils

namespace SteamAudioTools {
class TransformUtils {
    const IPLVector3 (*Transform$$get_position)(uintptr_t __this,
                                                uintptr_t method) =
        reinterpret_cast<decltype(Transform$$get_position)>(
            reinterpret_cast<void *>(FFI getMethodAddresss(
                getMethodCached<"UnityEngine.Transform$$get_position">())));
    const IPLVector3 (*Transform$$get_forward)(uintptr_t __this,
                                               uintptr_t method) =
        reinterpret_cast<decltype(Transform$$get_forward)>(
            reinterpret_cast<void *>(FFI getMethodAddresss(
                getMethodCached<"UnityEngine.Transform$$get_forward">())));
    const IPLVector3 (*Transform$$get_up)(uintptr_t __this, uintptr_t method) =
        reinterpret_cast<decltype(Transform$$get_up)>(
            reinterpret_cast<void *>(FFI getMethodAddresss(
                getMethodCached<"UnityEngine.Transform$$get_up">())));

public:
    IPLVector3 get_position(ObjectHandle transform) {
        return Transform$$get_position(FFI getObjectAddress(transform), 0);
    }
    IPLVector3 get_forward(ObjectHandle transform) {
        return Transform$$get_forward(FFI getObjectAddress(transform), 0);
    }
    IPLVector3 get_up(ObjectHandle transform) {
        return Transform$$get_up(FFI getObjectAddress(transform), 0);
    }
};
std::optional<TransformUtils> transformUtils;

IPLAudioBuffer audioBufferFromAudioClip(ObjectHandle audioClip) {
    IPLAudioBuffer fres;

    // Get basic properties
    fres.numSamples =
        call<"UnityEngine.AudioClip$$get_samples", int32_t>(audioClip, nullptr);
    fres.numChannels = call<"UnityEngine.AudioClip$$get_channels", int32_t>(
        audioClip, nullptr);

    // Get data
    static ObjectHandle singleArrayClass = []() {
        ObjectHandle corlib = FFI getImageCorlib();
        ObjectHandle single = FFI getClassFromName(corlib, "System", "Single");
        ObjectHandle singleArray = FFI getArrayFromClass(single, 1);
        FFI dropObject(corlib);
        FFI dropObject(single);
        return singleArray;
    }();
    const size_t csDataLen = fres.numSamples * fres.numChannels;
    ObjectHandle csData = FFI createArray(singleArrayClass, csDataLen);
    std::vector<float> data(csDataLen);
    call<"bool UnityEngine_AudioClip__GetData (UnityEngine_AudioClip_o* "
         "__this, System_Single_array* data, int32_t offsetSamples, const "
         "MethodInfo* method);">(audioClip, csData, 0, nullptr);
    FFI copyArrayBytes(csData, 0, csDataLen * sizeof(float), data.data());
    FFI dropObject(csData);

    // Allocate buffer
    IPLerror status = iplAudioBufferAllocate(phononCtx, fres.numChannels,
                                             fres.numSamples, &fres);
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(
            Utils::createErrorMessage("allocate input audio buffer", status));

    // Deinterleave Unity provided buffer
    iplAudioBufferDeinterleave(phononCtx, data.data(), &fres);

    // Ensure buffer is no more than stereo
    if (fres.numChannels > 2) {
        IPLAudioBuffer stereoFres;
        status =
            iplAudioBufferAllocate(phononCtx, 2, fres.numSamples, &stereoFres);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage(
                "allocate downmixed input audio buffer", status));
        iplAudioBufferDownmix(phononCtx, &fres, &stereoFres);
        iplAudioBufferFree(phononCtx, &fres);
        fres = stereoFres;
    }

    return fres;
}
} // namespace SteamAudioTools

namespace SteamAudioImpl {
class BufferPool {
public:
    BufferPool(IPLint32 channels, IPLint32 samples) {
        for (auto& buffer : buffers_) {
            IPLerror status =
                iplAudioBufferAllocate(phononCtx, channels, samples, &buffer);
            if (status != IPL_STATUS_SUCCESS)
                FFI logError(Utils::createErrorMessage(
                    "create buffer for buffer pool", status));
        }
    }
    ~BufferPool() {
        for (auto& buffer : buffers_)
            iplAudioBufferFree(phononCtx, &buffer);
    }
    BufferPool(const BufferPool&) = delete;
    BufferPool(BufferPool&&) = delete;

    IPLAudioBuffer& GetCurrentBuffer() { return buffers_[bufferIdx_]; }
    IPLAudioBuffer& GetNextBuffer() { return buffers_[GetNextBufferIndex()]; }
    void SwitchToNextBuffer() { bufferIdx_ = GetNextBufferIndex(); }

private:
    // Returns next buffer index.
    unsigned GetNextBufferIndex() const {
        auto bufferIdx = bufferIdx_;
        if (++bufferIdx == buffers_.size())
            return 0;
        return bufferIdx;
    }

    // Current buffer index.
    unsigned bufferIdx_ = 0;
    // Different audio buffers for processing.
    std::array<IPLAudioBuffer, 4> buffers_;
};

struct Environment {
    IPLAudioSettings audioSettings;
    IPLHRTF hrtf = nullptr;
    IPLBinauralEffect binauralEffect = nullptr;
    BufferPool bufferPool;

    Environment(IPLint32 frameSize)
        : bufferPool(FixedSettings::outputChannels, frameSize) {
        audioSettings.frameSize = frameSize;
        audioSettings.samplingRate = maDevice.sampleRate;

        {
            IPLHRTFSettings hrtfSettings{.type = IPL_HRTFTYPE_DEFAULT,
                                         .volume = 1.0f};
            iplHRTFCreate(phononCtx, &audioSettings, &hrtfSettings, &hrtf);
        }

        {
            IPLBinauralEffectSettings effectSettings{.hrtf = hrtf};
            iplBinauralEffectCreate(phononCtx, &audioSettings, &effectSettings,
                                    &binauralEffect);
        }
    }
};
std::optional<Environment> env;

void dataCallback(ma_device *pDevice, float *pOutput, const float *pInput,
                  ma_uint32 sampleCount) {
    // Reconstruct environment if frame size is wrong
    if (!env.has_value() || env->audioSettings.frameSize != sampleCount)
        env.emplace(sampleCount);

    // Allocate frame buffer
    IPLAudioBuffer frameBuffer{};
    IPLerror status = iplAudioBufferAllocate(
        phononCtx, FixedSettings::outputChannels, sampleCount, &frameBuffer);
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(
            Utils::createErrorMessage("create output frame buffer", status));

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
            const auto availableSamples =
                playback.audioBuffer.numSamples - playback.playPosition;

            // Allocate input buffer
            IPLAudioBuffer inputBuffer;
            IPLerror status = iplAudioBufferAllocate(
                phononCtx, playback.audioBuffer.numChannels, sampleCount,
                &inputBuffer);
            if (status != IPL_STATUS_SUCCESS)
                FFI logError(Utils::createErrorMessage(
                    "create input snippet buffer", status));

            // Copy audio data while applying volume scale
            for (unsigned channel = 0; channel != inputBuffer.numChannels;
                 channel++) {
                for (unsigned sample = 0; sample != inputBuffer.numSamples;
                     ++sample) {
                    if (sample < availableSamples)
                        inputBuffer.data[channel][sample] =
                            playback.audioBuffer
                                .data[channel][playback.playPosition + sample] *
                            playback.volumeScale;
                    else
                        inputBuffer.data[channel][sample] = 0.0f;
                }
            }

            // Apply binaural effect
            {
                IPLBinauralEffectParams effectParams{
                    .interpolation = IPL_HRTFINTERPOLATION_BILINEAR,
                    .spatialBlend = 1.0f,
                    .hrtf = env->hrtf,
                    .peakDelays = nullptr};
                effectParams.direction = iplCalculateRelativeDirection(
                    phononCtx, playback.worldPosition, playerPos, playerAhead,
                    playerUp);
                effectParams.direction.x = -effectParams.direction.x;
                iplBinauralEffectApply(env->binauralEffect, &effectParams,
                                       &inputBuffer,
                                       &env->bufferPool.GetCurrentBuffer());
            }

            // Mix into frame buffer
            iplAudioBufferMix(phononCtx, &env->bufferPool.GetCurrentBuffer(),
                              &frameBuffer);

            // Deallocate input buffer
            iplAudioBufferFree(phononCtx, &inputBuffer);

            // Increment play position
            playback.playPosition += sampleCount;
        }
    }

    iplAudioBufferInterleave(phononCtx, &frameBuffer, pOutput);
    iplAudioBufferFree(phononCtx, &frameBuffer);
}
} // namespace SteamAudioImpl

STEAMAUDIOIMPL_EXPORT void onAudioSourcePlay() {
    // Get audio source
    const auto audioSource = FFI getValueObject(0);

    {
        // Make sure source isn't already playing
        std::scoped_lock L(playbackQueueMutex);
        for (auto& playback : playbackQueue) {
            if (!playback.hasReachedEnd() &&
                call<"UnityEngine.Object$$Equals", bool>(
                    audioSource, playback.audioSource, nullptr))
                return;
        }
    }

    // Get default properties
    uint64_t delay = 0.0;
    float volumeScale = 1.0f;
    ObjectHandle audioClip = ObjectHandle::Invalid;

    // Get properties from arguments
    if (getMethodCached<
            "void UnityEngine_AudioSource__PlayOneShot "
            "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
            "clip, const MethodInfo* method);">() == FFI getOriginal()) {
        audioClip = FFI getValueObject(1);
    }
    if (getMethodCached<
            "void UnityEngine_AudioSource__PlayOneShot "
            "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
            "clip, float volumeScale, const MethodInfo* method);">() ==
        FFI getOriginal()) {
        audioClip = FFI getValueObject(1);
        volumeScale = FFI getValueFloat(2);
    }
    if (getMethodCached<
            "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
            "__this, double delay, const MethodInfo* method);">() ==
        FFI getOriginal()) {
        delay = FFI getValueDouble(1);
    }
    if (getMethodCached<
            "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
            "__this, uint64_t delay, const MethodInfo* method);">() ==
        FFI getOriginal()) {
        delay = FFI getValueI64(1);
    }

    // Get audio clip if not passed by argument
    if (audioClip == ObjectHandle::Invalid) {
        audioClip = call<"UnityEngine.AudioSource$$get_clip", ObjectHandle>(
            audioSource, nullptr);
    }

    // Get Steam Audio buffer from audio clip
    const IPLAudioBuffer audioBuffer =
        SteamAudioTools::audioBufferFromAudioClip(audioClip);
    {
        std::scoped_lock L(playbackQueueMutex);
        playbackQueue.emplace_back(audioSource, audioBuffer, delay,
                                   volumeScale);
    }

    // Clean up
    FFI dropObject(audioClip);
}
} // namespace

namespace Interface {
STEAMAUDIOIMPL_EXPORT void initImports(const FFIInterface::Imports *imports) {
    ::imports = imports;
}

STEAMAUDIOIMPL_EXPORT void onLoad() {
    FFI logInfo("Initializing Steam Audio..."_cs);
    {
        IPLContextSettings contextSettings{};
        contextSettings.version = STEAMAUDIO_VERSION;
        const IPLerror status = iplContextCreate(&contextSettings, &phononCtx);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage(
                "initialize Steam Audio context", status));
    }

    FFI logInfo("Initializing miniaudio..."_cs);
    {
        ma_device_config config =
            ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = FixedSettings::outputChannels;
        config.sampleRate = 0;
        config.dataCallback =
            reinterpret_cast<ma_device_data_proc>(SteamAudioImpl::dataCallback);

        const ma_result status = ma_device_init(NULL, &config, &maDevice);
        if (status != MA_SUCCESS)
            FFI logError(
                Utils::createErrorMessage("initialize miniaudio", status));

        ma_device_start(&maDevice);
    }

    SteamAudioTools::transformUtils.emplace();

    FFI hook(
        getMethodCached<
            "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
            "__this, const MethodInfo* method);">(),
        "onAudioSourcePlay");
    FFI hook(getMethodCached<
                 "void UnityEngine_AudioSource__PlayOneShot "
                 "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
                 "clip, const MethodInfo* method);">(),
             "onAudioSourcePlay");
    FFI hook(getMethodCached<
                 "void UnityEngine_AudioSource__PlayOneShot "
                 "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
                 "clip, float volumeScale, const MethodInfo* method);">(),
             "onAudioSourcePlay");
    FFI hook(
        getMethodCached<
            "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
            "__this, double delay, const MethodInfo* method);">(),
        "onAudioSourcePlay");
    FFI hook(
        getMethodCached<
            "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
            "__this, uint64_t delay, const MethodInfo* method);">(),
        "onAudioSourcePlay");
}

STEAMAUDIOIMPL_EXPORT void onUnload() {
    FFI logInfo("Unloading Steam Audio..."_cs);
    SteamAudioImpl::env.reset();
    iplContextRelease(&phononCtx);

    FFI logInfo("Unloading miniaudio..."_cs);
    ma_device_uninit(&maDevice);

    FFI unhook(getMethodCached<
               "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
               "__this, const MethodInfo* method);">());
    FFI unhook(getMethodCached<
               "void UnityEngine_AudioSource__PlayOneShot "
               "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
               "clip, const MethodInfo* method);">());
    FFI unhook(getMethodCached<
               "void UnityEngine_AudioSource__PlayOneShot "
               "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
               "clip, float volumeScale, const MethodInfo* method);">());
    FFI unhook(getMethodCached<
               "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
               "__this, double delay, const MethodInfo* method);">());
    FFI unhook(getMethodCached<
               "void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
               "__this, uint64_t delay, const MethodInfo* method);">());
}

STEAMAUDIOIMPL_EXPORT void onUiUpdate() {
    // Slowly clean up queue
    std::scoped_lock L(playbackQueueMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (it->hasReachedEnd()) {
            playbackQueue.erase(it);
            break;
        }
    }

    // Get current camera transform
    ObjectHandle camera =
        call<"UnityEngine.Camera$$get_current", ObjectHandle>(nullptr);
    if (camera != ObjectHandle::Null) {
        ObjectHandle cameraObject =
            call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(
                camera, nullptr);
        if (cameraObject != ObjectHandle::Null) {
            ObjectHandle cameraTransform =
                call<"UnityEngine.GameObject$$get_transform", ObjectHandle>(
                    cameraObject, nullptr);

            if (cameraTransform != ObjectHandle::Null) {
                // Update player position and direction
                playerPos = SteamAudioTools::transformUtils->get_position(
                    cameraTransform);
                playerAhead = SteamAudioTools::transformUtils->get_forward(
                    cameraTransform);
                playerUp =
                    SteamAudioTools::transformUtils->get_up(cameraTransform);
                FFI dropObject(cameraTransform);
            }
            FFI dropObject(cameraObject);
        }
        FFI dropObject(camera);
    }

    // Update playback positions
    for (auto& playback : playbackQueue) {
        if (!playback.hasReachedEnd() &&
            playback.audioSource != ObjectHandle::Null &&
            call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(
                playback.audioSource, nullptr)) {
            ObjectHandle sourceObject =
                call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(
                    playback.audioSource, nullptr);
            if (sourceObject != ObjectHandle::Null) {
                ObjectHandle sourceTransform =
                    call<"UnityEngine.GameObject$$get_transform", ObjectHandle>(
                        sourceObject, nullptr);
                if (sourceTransform != ObjectHandle::Null) {
                    playback.worldPosition =
                        SteamAudioTools::transformUtils->get_position(
                            sourceTransform);
                    FFI dropObject(sourceTransform);
                }
                FFI dropObject(sourceObject);
            }
        }
    }
}
} // namespace Interface

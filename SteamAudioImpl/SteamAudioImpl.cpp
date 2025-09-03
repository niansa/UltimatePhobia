#include "SteamAudioImpl.hpp"
#include "playback_environment.hpp"
#include "playback.hpp"
#include "phonontools.hpp"
#include "playback_environment.hpp"
#include "utils.hpp"
#include "fixedsettings.hpp"
#include "miniaudio.h"

#include <string>
#include <optional>
#include <cmath>
#include <ffi_interface.hpp>
#include <phonon.h>

using namespace FFIInterface;
using namespace Helpers;
using namespace Literals;

const FFIInterface::Imports *imports;

namespace GlobalState {
IPLContext phononCtx;
ma_device maDevice;

IPLCoordinateSpace3 playerCoord;
Statistics stats;
} // namespace GlobalState

GameHookPool hooks;

namespace Interface {
STEAMAUDIOIMPL_EXPORT void onAudioSourcePlay() {
    using namespace PhononPlayback;

    // Get original function
    const MethodHandle original = FFI getOriginal();

    // Get audio source
    const auto audioSource = FFI getValueObject(0);

    // Get default properties
    uint64_t delay = 0.0;
    float volumeScale = 1.0f;
    ObjectHandle audioClip = ObjectHandle::Invalid;
    bool isOneShot = false;

    // Get properties from arguments
    if (getMethodCached<
            "void UnityEngine_AudioSource__PlayOneShot (UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* clip, const MethodInfo* method);">() ==
        original) {
        audioClip = FFI getValueObject(1);
        isOneShot = true;
    } else if (getMethodCached<
                   "void UnityEngine_AudioSource__PlayOneShot (UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* clip, float volumeScale, const "
                   "MethodInfo* method);">() == original) {
        audioClip = FFI getValueObject(1);
        volumeScale = FFI getValueFloat(2);
        isOneShot = true;
    } else if (getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, double delay, const MethodInfo* method);">() ==
               original) {
        delay = FFI getValueDouble(1);
    } else if (getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, uint64_t delay, const MethodInfo* method);">() ==
               original) {
        delay = FFI getValueI64(1);
    }

    // Call original method
    FFI call(original, unknownArgCount);

    {
        // Make sure source isn't already playing
        std::scoped_lock L(playbackMutex);
        for (auto& playback : playbackQueue) {
            if (!playback.hasReachedEnd() && call<"UnityEngine.Object$$Equals", bool>(audioSource, playback.audioSource, nullptr)) {
                FFI logDebug("Not playing audio because it is already being played"_cs);
                return;
            }
        }
    }

    // Get audio clip if not passed by argument
    if (audioClip == ObjectHandle::Invalid)
        audioClip = call<"UnityEngine.AudioSource$$get_clip", ObjectHandle>(audioSource, nullptr);

    // Stop here if we still don't have an audio clip
    if (audioClip == ObjectHandle::Null) {
        FFI logWarn("Not playing audio because there is no clip to play"_cs);
        return;
    }

    // Get Steam Audio buffer from audio clip
    const IPLAudioBuffer audioBuffer = PhononTools::audioBufferFromAudioClip(audioClip);

    // Add audio to playback queue
    bool fallback = false;
    {
        std::scoped_lock L(playbackMutex);

        // Try to handle playback
        auto& playback = playbackQueue.emplace_back(audioSource, audioBuffer, delay, volumeScale, isOneShot);
        PhononTools::updatePlayback(playback, true);
    }

    // Prevent unity from playing audio itself
    call<"UnityEngine.AudioSource$$set_mute">(audioSource, true, nullptr);

    // Clean up
    FFI dropObject(audioClip);
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceStop() {
    using namespace PhononPlayback;

    // Get original function
    const MethodHandle original = FFI getOriginal();

    // Get audio source
    const auto audioSource = FFI getValueObject(0);

    bool stopOneShots = true;
    if (getMethodCached<"void UnityEngine_AudioSource__Stop (UnityEngine_AudioSource_o* __this, bool stopOneShots, const MethodInfo* method);">() == original)
        stopOneShots = FFI getValueI32(1) & 0xff;

    // Call original method
    FFI call(original, unknownArgCount);

    // Remove audio source from queue
    {
        std::scoped_lock L(playbackMutex);
        for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
            if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr) && (!it->isOneShot || stopOneShots)) {
                playbackQueue.erase(it);
                break;
            }
        }
    }
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceGetIsPlaying() {
    using namespace PhononPlayback;

    // Get audio source
    const ObjectHandle audioSource = FFI getValueObject(0);

    // Check playback queue
    std::scoped_lock L(playbackMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            FFI addArgI32(!it->hasReachedEnd());
            FFI moveArg(-1);
            return;
        }
    }

    // Call original function
    FFI clearArgs();
    FFI addArgObject(audioSource);
    FFI addArgNull();
    FFI call(FFI getOriginal(), 2);
}

// This is broken for some reason. The original method isn't called correctly. Probably because of the float?
// STEAMAUDIOIMPL_EXPORT void onAudioSourceSetVolume() {
//     using namespace PhononPlayback;

//     // Get audio source and new volume
//     const ObjectHandle audioSource = FFI getValueObject(0);
//     const float newVolume = FFI getValueFloat(1);

//     // Call original function
//     FFI call(FFI getOriginal(), 3);

//     // Update audio source volume
//     std::scoped_lock L(playbackMutex);
//     for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
//         if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
//             it->volume = newVolume;
//             break;
//         }
//     }
// }

STEAMAUDIOIMPL_EXPORT void onAudioSourceSetLoop() {
    using namespace PhononPlayback;

    // Get audio source and new volume
    const ObjectHandle audioSource = FFI getValueObject(0);
    const bool newLoop = FFI getValueI32(1) & 0xff;

    // Call original function
    FFI call(FFI getOriginal(), 3);

    // Update audio source volume
    std::scoped_lock L(playbackMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            it->volume = newLoop;
            break;
        }
    }
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceSetRolloffMode() {
    using namespace PhononPlayback;

    // Get audio source and new volume
    const ObjectHandle audioSource = FFI getValueObject(0);
    const auto mode = static_cast<PhononPlayback::AudioRolloffMode>(FFI getValueI32(1));

    // Call original function
    FFI call(FFI getOriginal(), 3);

    // Apply mode
    std::scoped_lock L(playbackMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            it->rolloffMode = mode;
            break;
        }
    }
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceSetClip() {
    using namespace PhononPlayback;

    // Get audio source and clip
    const ObjectHandle audioSource = FFI getValueObject(0);
    const ObjectHandle audioClip = FFI getValueObject(1);

    // Get audio buffer
    std::optional<IPLAudioBuffer> audioBuffer;
    if (audioClip != ObjectHandle::Null)
        audioBuffer = PhononTools::audioBufferFromAudioClip(audioClip);

    // Call original function
    FFI clearArgs();
    FFI addArgObject(audioSource);
    FFI addArgObject(audioClip);
    FFI addArgNull();
    FFI call(FFI getOriginal(), 3);

    // Update audio source clip
    std::scoped_lock L(playbackMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            if (audioBuffer.has_value())
                *it = *audioBuffer;
            else
                playbackQueue.erase(it);
            return;
        }
    }

    // Free audio buffer again because it wasn't actually used
    iplAudioBufferFree(GlobalState::phononCtx, &*audioBuffer);
}

STEAMAUDIOIMPL_EXPORT void initImports(const FFIInterface::Imports *imports) { ::imports = imports; }

STEAMAUDIOIMPL_EXPORT void onLoad() {
    if (FFI getFtableItemCount() < getLocalFtableItemCount())
        FFI abort("UltimatePhobia runtime too old!", __FILE_NAME__, __LINE__, 0);

    FFI logInfo("Initializing Steam Audio..."_cs);
    {
        IPLContextSettings contextSettings{};
        contextSettings.version = STEAMAUDIO_VERSION;
        const IPLerror status = iplContextCreate(&contextSettings, &GlobalState::phononCtx);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage("initialize Steam Audio context", status));
    }

    FFI logInfo("Initializing miniaudio..."_cs);
    {
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = FixedSettings::outputChannels;
        config.sampleRate = 0;
        config.dataCallback = reinterpret_cast<ma_device_data_proc>(PhononPlayback::dataCallback);

        const ma_result status = ma_device_init(NULL, &config, &GlobalState::maDevice);
        if (status != MA_SUCCESS)
            FFI logError(Utils::createErrorMessage("initialize miniaudio", status));

        ma_device_start(&GlobalState::maDevice);
    }

    hooks.add<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, const MethodInfo* method);">("onAudioSourcePlay");
    hooks.add<"void UnityEngine_AudioSource__PlayOneShot (UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* clip, const MethodInfo* method);">(
        "onAudioSourcePlay");
    hooks.add<"void UnityEngine_AudioSource__PlayOneShot (UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* clip, float volumeScale, "
              "const MethodInfo* method);">("onAudioSourcePlay");
    hooks.add<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, double delay, const MethodInfo* method);">("onAudioSourcePlay");
    hooks.add<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, uint64_t delay, const MethodInfo* method);">("onAudioSourcePlay");

    hooks.add<"void UnityEngine_AudioSource__Stop (UnityEngine_AudioSource_o* __this, const MethodInfo* method);">("onAudioSourceStop");
    hooks.add<"void UnityEngine_AudioSource__Stop (UnityEngine_AudioSource_o* __this, bool stopOneShots, const MethodInfo* method);">("onAudioSourceStop");

    hooks.add<"UnityEngine.AudioSource$$get_isPlaying">("onAudioSourceGetIsPlaying");
    // hooks.add<"UnityEngine.AudioSource$$set_volume">(), "onAudioSourceSetVolume");
    hooks.add<"UnityEngine.AudioSource$$set_clip">("onAudioSourceSetClip");
    hooks.add<"UnityEngine.AudioSource$$set_loop">("onAudioSourceSetLoop");
    hooks.add<"UnityEngine.AudioSource$$set_rolloffMode">("onAudioSourceSetRolloffMode");
}

STEAMAUDIOIMPL_EXPORT void onUnload() {
    hooks.clear();

    FFI logInfo("Unloading miniaudio..."_cs);
    ma_device_uninit(&GlobalState::maDevice);

    FFI logInfo("Unloading Steam Audio..."_cs);
    PhononPlayback::env.reset();
    iplContextRelease(&GlobalState::phononCtx);
}

STEAMAUDIOIMPL_EXPORT void onUiUpdate() {
    using namespace PhononPlayback;

    // Display settings
    FFI ImGuiBegin("Steam Audio");
    static bool showStats = true;
    ImGuiCheckbox("Show stats", &showStats);
    FFI ImGuiEnd();

    // Get current camera position and direction
    ObjectHandle camera = call<"UnityEngine.Camera$$get_current", ObjectHandle>(nullptr);
    if (camera != ObjectHandle::Null) {
        ObjectHandle cameraObject = call<"UnityEngine.Component$$get_gameObject", ObjectHandle>(camera, nullptr);
        if (cameraObject != ObjectHandle::Null) {
            ObjectHandle cameraTransform = call<"UnityEngine.GameObject$$get_transform", ObjectHandle>(cameraObject, nullptr);

            if (cameraTransform != ObjectHandle::Null) {
                // Update player position and direction
                GlobalState::playerCoord = {.right = PhononTools::TransformUtils::get_right(cameraTransform),
                                            .up = PhononTools::TransformUtils::get_up(cameraTransform),
                                            .ahead = PhononTools::TransformUtils::get_forward(cameraTransform),
                                            .origin = PhononTools::TransformUtils::get_position(cameraTransform)};
                FFI dropObject(cameraTransform);
            }
            FFI dropObject(cameraObject);
        }
        FFI dropObject(camera);
    }

    // Display basic stats
    if (showStats) {
        FFI ImGuiBegin("Steam Audio Stats");
        FFI ImGuiText(FFI toCsString(("Active sources: " + std::to_string(playbackQueue.size())).c_str()));
        FFI ImGuiText(FFI toCsString(("Frame size: " + std::to_string(env->audioSettings.frameSize)).c_str()));
        FFI ImGuiText(FFI toCsString(
            ("Frames processed: " + std::to_string(GlobalState::stats.framesFinished) + '/' + std::to_string(GlobalState::stats.framesStarted)).c_str()));
        FFI ImGuiText(FFI toCsString(
            ("Playbacks processed: " + std::to_string(GlobalState::stats.playbacksFinished) + '/' + std::to_string(GlobalState::stats.playbacksStarted))
                .c_str()));
        FFI ImGuiText(FFI toCsString(("World transform: " + Utils::formatCoord(GlobalState::playerCoord)).c_str()));
    }

    // Update playbacks and display stats
    std::scoped_lock L(playbackMutex);
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
                FFI ImGuiText(FFI toCsString(
                    ("Loop: " + std::string(call<"UnityEngine.AudioSource$$get_loop", bool>(playback->audioSource, nullptr) ? "yes" : "no")).c_str()));
                FFI ImGuiText(FFI toCsString(("World position: " + Utils::formatPos(playback->worldPosition)).c_str()));
            }
        } else {
            playbackQueue.erase(playback);
            break;
        }
    }

    if (showStats)
        FFI ImGuiEnd();
}
} // namespace Interface

#include "SteamAudioImpl.hpp"
#include "playback_environment.hpp"
#include "playback.hpp"
#include "simulation_environment.hpp"
#include "simulation.hpp"
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
} // namespace GlobalState

namespace Interface {
STEAMAUDIOIMPL_EXPORT void onAudioSourcePlay() {
    using namespace PhononPlayback;

    // Get audio source
    const auto audioSource = FFI getValueObject(0);

    {
        // Make sure source isn't already playing
        std::scoped_lock L(playbackQueueMutex);
        for (auto& playback : playbackQueue) {
            if (!playback.hasReachedEnd() && call<"UnityEngine.Object$$Equals", bool>(audioSource, playback.audioSource, nullptr)) {
                FFI logDebug("Not playing audio because it is already being played"_cs);
                return;
            }
        }
    }

    // Get default properties
    uint64_t delay = 0.0;
    float volumeScale = 1.0f;
    ObjectHandle audioClip = ObjectHandle::Invalid;
    bool isOneShot = false;

    // Get properties from arguments
    if (getMethodCached<"void UnityEngine_AudioSource__PlayOneShot "
                        "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
                        "clip, const MethodInfo* method);">() == FFI getOriginal()) {
        audioClip = FFI getValueObject(1);
        isOneShot = true;
    }
    if (getMethodCached<"void UnityEngine_AudioSource__PlayOneShot "
                        "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
                        "clip, float volumeScale, const MethodInfo* method);">() == FFI getOriginal()) {
        audioClip = FFI getValueObject(1);
        volumeScale = FFI getValueFloat(2);
        isOneShot = true;
    }
    if (getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
                        "__this, double delay, const MethodInfo* method);">() == FFI getOriginal()) {
        delay = FFI getValueDouble(1);
    }
    if (getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
                        "__this, uint64_t delay, const MethodInfo* method);">() == FFI getOriginal()) {
        delay = FFI getValueI64(1);
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
    {
        std::scoped_lock L(playbackQueueMutex);
        auto& playback = playbackQueue.emplace_back(audioSource, audioBuffer, delay, volumeScale, isOneShot);
        PhononTools::updatePlayback(playback, true);
    }

    // Clean up
    FFI dropObject(audioClip);
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceStop() {
    using namespace PhononPlayback;

    // Get audio source
    const auto audioSource = FFI getValueObject(0);

    bool stopOneShots = true;
    if (getMethodCached<"void UnityEngine_AudioSource__Stop "
                        "(UnityEngine_AudioSource_o* "
                        "__this, bool stopOneShots, const MethodInfo* "
                        "method);">() == FFI getOriginal()) {
        stopOneShots = FFI getValueI32(1) & 0xff;
    }

    // Remove audio source from qeue
    std::scoped_lock L(playbackQueueMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr) && (!it->isOneShot || stopOneShots)) {
            playbackQueue.erase(it);
            break;
        }
    }
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceGetIsPlaying() {
    using namespace PhononPlayback;

    // Get audio source
    const ObjectHandle audioSource = FFI getValueObject(0);

    // Check playback queue
    std::scoped_lock L(playbackQueueMutex);
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

STEAMAUDIOIMPL_EXPORT void onAudioSourceSetVolume() {
    using namespace PhononPlayback;

    // Get audio source and new volume
    const ObjectHandle audioSource = FFI getValueObject(0);
    const float newVolume = std::fabs(FFI getValueFloat(1));

    // Call original function
    FFI call(FFI getOriginal(), 3);

    // Update audio source volume
    std::scoped_lock L(playbackQueueMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            it->volume = newVolume;
            break;
        }
    }
}

STEAMAUDIOIMPL_EXPORT void onAudioSourceSetLoop() {
    using namespace PhononPlayback;

    // Get audio source and new volume
    const ObjectHandle audioSource = FFI getValueObject(0);
    const bool newLoop = FFI getValueI32(1) & 0xff;

    // Call original function
    FFI call(FFI getOriginal(), 3);

    // Update audio source volume
    std::scoped_lock L(playbackQueueMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (call<"UnityEngine.Object$$Equals", bool>(audioSource, it->audioSource, nullptr)) {
            it->volume = newLoop;
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
    std::scoped_lock L(playbackQueueMutex);
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

    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, const MethodInfo* method);">(), "onAudioSourcePlay");
    FFI hook(getMethodCached<
                 "void UnityEngine_AudioSource__PlayOneShot (UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* clip, const MethodInfo* method);">(),
             "onAudioSourcePlay");
    FFI hook(getMethodCached<"void UnityEngine_AudioSource__PlayOneShot (UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* clip, float volumeScale, "
                             "const MethodInfo* method);">(),
             "onAudioSourcePlay");
    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, double delay, const MethodInfo* method);">(),
             "onAudioSourcePlay");
    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* __this, uint64_t delay, const MethodInfo* method);">(),
             "onAudioSourcePlay");

    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Stop (UnityEngine_AudioSource_o* __this, const MethodInfo* method);">(), "onAudioSourceStop");
    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Stop (UnityEngine_AudioSource_o* __this, bool stopOneShots, const MethodInfo* method);">(),
             "onAudioSourceStop");

    FFI hook(getMethodCached<"UnityEngine.AudioSource$$get_isPlaying">(), "onAudioSourceGetIsPlaying");
    FFI hook(getMethodCached<"UnityEngine.AudioSource$$set_volume">(), "onAudioSourceSetVolume");
    FFI hook(getMethodCached<"UnityEngine.AudioSource$$set_clip">(), "onAudioSourceSetClip");
    FFI hook(getMethodCached<"UnityEngine.AudioSource$$set_loop">(), "onAudioSourceSetLoop");
}

STEAMAUDIOIMPL_EXPORT void onUnload() {
    FFI logInfo("Unloading miniaudio..."_cs);
    ma_device_uninit(&GlobalState::maDevice);

    FFI logInfo("Unloading Steam Audio..."_cs);
    PhononPlayback::env.reset();
    PhononSimulation::env.reset();
    iplContextRelease(&GlobalState::phononCtx);

    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
                               "__this, const MethodInfo* method);">());
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__PlayOneShot "
                               "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
                               "clip, const MethodInfo* method);">());
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__PlayOneShot "
                               "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
                               "clip, float volumeScale, const MethodInfo* method);">());
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
                               "__this, double delay, const MethodInfo* method);">());
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Play (UnityEngine_AudioSource_o* "
                               "__this, uint64_t delay, const MethodInfo* method);">());

    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Stop "
                               "(UnityEngine_AudioSource_o* __this, const "
                               "MethodInfo* method);">());
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Stop "
                               "(UnityEngine_AudioSource_o* "
                               "__this, bool stopOneShots, const MethodInfo* "
                               "method);">());

    FFI unhook(getMethodCached<"UnityEngine.AudioSource$$get_isPlaying">());
    FFI unhook(getMethodCached<"UnityEngine.AudioSource$$set_volume">());
    FFI unhook(getMethodCached<"UnityEngine.AudioSource$$set_clip">());
    FFI unhook(getMethodCached<"UnityEngine.AudioSource$$set_loop">());
}

STEAMAUDIOIMPL_EXPORT void onUiUpdate() {
    using namespace PhononPlayback;

    // Display settings
    FFI ImGuiBegin("Steam Audio");
    static bool enableSimulation = false;
    FFI ImGuiCheckbox("Enable reflection simulation", &enableSimulation);
    static bool showStats = true;
    FFI ImGuiCheckbox("Show stats", &showStats);
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

    // Run simulation
    if (enableSimulation)
        PhononSimulation::run();

    // Display basic stats
    if (showStats) {
        FFI ImGuiBegin("Steam Audio Stats");
        FFI ImGuiText(FFI toCsString(("Active sources: " + std::to_string(playbackQueue.size())).c_str()));
        if (enableSimulation)
            FFI ImGuiText(FFI toCsString(("Scene meshes: " + std::to_string(PhononSimulation::env->getMeshCount())).c_str()));
    }

    // Update playbacks and display stats
    std::scoped_lock L(playbackQueueMutex);
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
            }
        } else {
            playbackQueue.erase(playback);
            break;
        }
    }

    if (showStats) {
        FFI ImGuiEnd();
    }
}
} // namespace Interface

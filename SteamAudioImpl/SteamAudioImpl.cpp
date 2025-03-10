#include "SteamAudioImpl.hpp"
#include "environment.hpp"
#include "playback.hpp"
#include "phonontools.hpp"
#include "environment.hpp"
#include "utils.hpp"
#include "fixedsettings.hpp"
#include "miniaudio.h"

#include <string>
#include <ffi_interface.hpp>
#include <phonon.h>

using namespace FFIInterface;
using namespace Helpers;
using namespace Literals;

const FFIInterface::Imports *imports;

namespace GlobalState {
IPLContext phononCtx;
ma_device maDevice;

IPLVector3 playerPos, playerAhead, playerUp;
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
    bool isOneShot = false;

    // Get properties from arguments
    if (getMethodCached<
            "void UnityEngine_AudioSource__PlayOneShot "
            "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
            "clip, const MethodInfo* method);">() == FFI getOriginal()) {
        audioClip = FFI getValueObject(1);
        isOneShot = true;
    }
    if (getMethodCached<
            "void UnityEngine_AudioSource__PlayOneShot "
            "(UnityEngine_AudioSource_o* __this, UnityEngine_AudioClip_o* "
            "clip, float volumeScale, const MethodInfo* method);">() ==
        FFI getOriginal()) {
        audioClip = FFI getValueObject(1);
        volumeScale = FFI getValueFloat(2);
        isOneShot = true;
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
    if (audioClip == ObjectHandle::Invalid)
        audioClip = call<"UnityEngine.AudioSource$$get_clip", ObjectHandle>(
            audioSource, nullptr);

    // Stop here if we still don't have an audio clip
    if (audioClip == ObjectHandle::Null)
        return;

    // Get Steam Audio buffer from audio clip
    const IPLAudioBuffer audioBuffer =
        PhononTools::audioBufferFromAudioClip(audioClip);
    {
        std::scoped_lock L(playbackQueueMutex);
        auto& playback = playbackQueue.emplace_back(
            audioSource, audioBuffer, delay, volumeScale, isOneShot);
        PhononTools::updatePlayback(playback);
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
        if (call<"UnityEngine.Object$$Equals", bool>(
                audioSource, it->audioSource, nullptr) &&
            (!it->isOneShot || stopOneShots)) {
            playbackQueue.erase(it);
            break;
        }
    }
}

STEAMAUDIOIMPL_EXPORT void initImports(const FFIInterface::Imports *imports) {
    ::imports = imports;
}

STEAMAUDIOIMPL_EXPORT void onLoad() {
    FFI logInfo("Initializing Steam Audio..."_cs);
    {
        IPLContextSettings contextSettings{};
        contextSettings.version = STEAMAUDIO_VERSION;
        const IPLerror status =
            iplContextCreate(&contextSettings, &GlobalState::phononCtx);
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
            reinterpret_cast<ma_device_data_proc>(PhononPlayback::dataCallback);

        const ma_result status =
            ma_device_init(NULL, &config, &GlobalState::maDevice);
        if (status != MA_SUCCESS)
            FFI logError(
                Utils::createErrorMessage("initialize miniaudio", status));

        ma_device_start(&GlobalState::maDevice);
    }

    PhononTools::transformUtils.emplace();

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
    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Stop "
                             "(UnityEngine_AudioSource_o* __this, const "
                             "MethodInfo* method);">(),
             "onAudioSourceStop");
    FFI hook(getMethodCached<"void UnityEngine_AudioSource__Stop "
                             "(UnityEngine_AudioSource_o* "
                             "__this, bool stopOneShots, const MethodInfo* "
                             "method);">(),
             "onAudioSourceStop");
}

STEAMAUDIOIMPL_EXPORT void onUnload() {
    FFI logInfo("Unloading Steam Audio..."_cs);
    PhononPlayback::env.reset();
    iplContextRelease(&GlobalState::phononCtx);

    FFI logInfo("Unloading miniaudio..."_cs);
    ma_device_uninit(&GlobalState::maDevice);

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
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Stop "
                               "(UnityEngine_AudioSource_o* __this, const "
                               "MethodInfo* method);">());
    FFI unhook(getMethodCached<"void UnityEngine_AudioSource__Stop "
                               "(UnityEngine_AudioSource_o* "
                               "__this, bool stopOneShots, const MethodInfo* "
                               "method);">());
}

STEAMAUDIOIMPL_EXPORT void onUiUpdate() {
    using namespace PhononPlayback;

    // Display stats
    FFI ImGuiBegin("Steam Audio");
    FFI ImGuiText(FFI toCsString(
        ("Active sources: " + std::to_string(playbackQueue.size())).c_str()));

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
                GlobalState::playerPos =
                    PhononTools::transformUtils->get_position(cameraTransform);
                GlobalState::playerAhead =
                    PhononTools::transformUtils->get_forward(cameraTransform);
                GlobalState::playerUp =
                    PhononTools::transformUtils->get_up(cameraTransform);
                FFI dropObject(cameraTransform);
            }
            FFI dropObject(cameraObject);
        }
        FFI dropObject(camera);
    }

    // Slowly clean up queue
    std::scoped_lock L(playbackQueueMutex);
    for (auto it = playbackQueue.begin(); it != playbackQueue.end(); ++it) {
        if (it->hasReachedEnd()) {
            playbackQueue.erase(it);
            break;
        }
    }

    // Update playbacks
    for (auto& playback : playbackQueue) {
        if (PhononTools::updatePlayback(playback)) {
            FFI ImGuiText(FFI toCsString(
                ("Volume: " + std::to_string(playback.volume)).c_str()));
            FFI ImGuiText(FFI toCsString(
                ("Volume Scale: " + std::to_string(playback.volumeScale))
                    .c_str()));
        }
    }

    FFI ImGuiEnd();
}
} // namespace Interface

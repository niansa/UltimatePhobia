#pragma once

#include "SteamAudioImpl.hpp"

#include <optional>
#include <stdint.h>
#include <phonon.h>
#include <ffi_interface.hpp>

namespace PhononPlayback {
class Playback;
}

namespace PhononTools {
class TransformUtils {
    const IPLVector3 (*Transform$$get_position)(uintptr_t __this,
                                                uintptr_t method) =
        reinterpret_cast<decltype(Transform$$get_position)>(
            reinterpret_cast<void *>(FFI getMethodAddresss(
                Helpers::getMethodCached<
                    "UnityEngine.Transform$$get_position">())));
    const IPLVector3 (*Transform$$get_forward)(uintptr_t __this,
                                               uintptr_t method) =
        reinterpret_cast<decltype(Transform$$get_forward)>(
            reinterpret_cast<void *>(FFI getMethodAddresss(
                Helpers::getMethodCached<
                    "UnityEngine.Transform$$get_forward">())));
    const IPLVector3 (*Transform$$get_up)(uintptr_t __this, uintptr_t method) =
        reinterpret_cast<decltype(Transform$$get_up)>(
            reinterpret_cast<void *>(FFI getMethodAddresss(
                Helpers::getMethodCached<"UnityEngine.Transform$$get_up">())));

public:
    IPLVector3 get_position(FFIInterface::ObjectHandle transform) {
        return Transform$$get_position(FFI getObjectAddress(transform), 0);
    }
    IPLVector3 get_forward(FFIInterface::ObjectHandle transform) {
        return Transform$$get_forward(FFI getObjectAddress(transform), 0);
    }
    IPLVector3 get_up(FFIInterface::ObjectHandle transform) {
        return Transform$$get_up(FFI getObjectAddress(transform), 0);
    }
};
extern std::optional<TransformUtils> transformUtils;

IPLAudioBuffer audioBufferFromAudioClip(FFIInterface::ObjectHandle audioClip);
bool updatePlayback(PhononPlayback::Playback& p);
} // namespace PhononTools

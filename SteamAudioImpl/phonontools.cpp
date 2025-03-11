#include "phonontools.hpp"
#include "SteamAudioImpl.hpp"
#include "utils.hpp"
#include "playback.hpp"

#include <vector>
#include <ffi_interface.hpp>

using namespace FFIInterface;
using namespace Helpers;

namespace PhononTools {
std::optional<TransformUtils> transformUtils;

IPLAudioBuffer audioBufferFromAudioClip(FFIInterface::ObjectHandle audioClip) {
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
    IPLerror status = iplAudioBufferAllocate(
        GlobalState::phononCtx, fres.numChannels, fres.numSamples, &fres);
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(
            Utils::createErrorMessage("allocate input audio buffer", status));

    // Deinterleave Unity provided buffer
    iplAudioBufferDeinterleave(GlobalState::phononCtx, data.data(), &fres);

    // Ensure buffer is no more than stereo
    if (fres.numChannels > 2) {
        IPLAudioBuffer stereoFres;
        status = iplAudioBufferAllocate(GlobalState::phononCtx, 2,
                                        fres.numSamples, &stereoFres);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage(
                "allocate downmixed input audio buffer", status));
        iplAudioBufferDownmix(GlobalState::phononCtx, &fres, &stereoFres);
        iplAudioBufferFree(GlobalState::phononCtx, &fres);
        fres = stereoFres;
    }

    return fres;
}

bool updatePlayback(PhononPlayback::Playback& p, bool initial) {
    // Make sure playback hasn't expired
    if (p.audioSource == ObjectHandle::Null || !call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(p.audioSource, nullptr)) {
        FFI logDebug(FFI toCsString("Playback ended because native source object died!"));
        return false;
    }

    // Update position
    {
        ObjectHandle sourceTransform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(p.audioSource, nullptr);
        if (sourceTransform != ObjectHandle::Null) {
            p.worldPosition = transformUtils->get_position(sourceTransform);
            FFI dropObject(sourceTransform);
        }
    }

    // Update initial properties
    if (initial) {
        p.volume = call<"UnityEngine.AudioSource$$get_volume", float>(p.audioSource, nullptr);
        if (call<"UnityEngine.AudioSource$$get_spatialize", bool>(p.audioSource, nullptr))
            p.spatialBlend = call<"UnityEngine.AudioSource$$get_spatialBlend", float>(p.audioSource, nullptr);
        else
            p.spatialBlend = 0.0f;
        p.maxDistance = call<"UnityEngine.AudioSource$$get_maxDistance", float>(p.audioSource, nullptr);
        p.loop = call<"UnityEngine.AudioSource$$get_loop", bool>(p.audioSource, nullptr);
    }

    // Loop if requested
    if (p.hasReachedEnd()) {
        if (p.loop)
            p.playPosition = p.playPosition % p.audioBuffer.numSamples;
        else {
            FFI logDebug(FFI toCsString("Non-looped playback ended!"));
            return false;
        }
    }

    // We're all good
    return true;
}
} // namespace PhononTools

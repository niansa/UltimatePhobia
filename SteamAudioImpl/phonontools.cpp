#include "phonontools.hpp"
#include "SteamAudioImpl.hpp"
#include "utils.hpp"
#include "playback.hpp"
#include "simulation_environment.hpp"
#include "fixedsettings.hpp"

#include <vector>
#include <cstring>
#include <cmath>
#include <ffi_interface.hpp>

using namespace FFIInterface;
using namespace Helpers;

namespace {
namespace Materials {
static IPLMaterial presets[11] = {{0.10f, 0.20f, 0.30f, 0.05f, 0.100f, 0.050f, 0.030f}, {0.03f, 0.04f, 0.07f, 0.05f, 0.015f, 0.015f, 0.015f},
                                  {0.05f, 0.07f, 0.08f, 0.05f, 0.015f, 0.002f, 0.001f}, {0.01f, 0.02f, 0.02f, 0.05f, 0.060f, 0.044f, 0.011f},
                                  {0.60f, 0.70f, 0.80f, 0.05f, 0.031f, 0.012f, 0.008f}, {0.24f, 0.69f, 0.73f, 0.05f, 0.020f, 0.005f, 0.003f},
                                  {0.06f, 0.03f, 0.02f, 0.05f, 0.060f, 0.044f, 0.011f}, {0.12f, 0.06f, 0.04f, 0.05f, 0.056f, 0.056f, 0.004f},
                                  {0.11f, 0.07f, 0.06f, 0.05f, 0.070f, 0.014f, 0.005f}, {0.20f, 0.07f, 0.06f, 0.05f, 0.200f, 0.025f, 0.010f},
                                  {0.13f, 0.20f, 0.24f, 0.05f, 0.015f, 0.002f, 0.001f}};
enum { Generic, Brick, Concrete, Ceramic, Gravel, Carpet, Glass, Plaster, Wood, Metal, Rock };
} // namespace Materials
} // namespace

namespace PhononTools {
namespace TransformUtils {
IPLVector3 get_position(FFIInterface::ObjectHandle transform) { return callRetStruct<"UnityEngine.Transform$$get_position", IPLVector3>(transform, nullptr); }
IPLVector3 get_forward(FFIInterface::ObjectHandle transform) { return callRetStruct<"UnityEngine.Transform$$get_forward", IPLVector3>(transform, nullptr); }
IPLVector3 get_up(FFIInterface::ObjectHandle transform) { return callRetStruct<"UnityEngine.Transform$$get_up", IPLVector3>(transform, nullptr); }
IPLVector3 get_right(FFIInterface::ObjectHandle transform) { return callRetStruct<"UnityEngine.Transform$$get_right", IPLVector3>(transform, nullptr); }
IPLMatrix4x4 get_localToWorldMatrix(FFIInterface::ObjectHandle transform) {
    return callRetStruct<"UnityEngine.Transform$$get_localToWorldMatrix", IPLMatrix4x4>(transform, nullptr);
}
} // namespace TransformUtils

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

IPLStaticMesh staticMeshFromMesh(IPLScene scene, FFIInterface::ObjectHandle mesh) {
    // Get vertices
    const auto verticeArray = call<"UnityEngine.Mesh$$get_vertices", ObjectHandle>(mesh, nullptr);
    if (call_error)
        return nullptr;
    if (verticeArray == ObjectHandle::Null)
        return nullptr;
    std::vector<IPLVector3> vertices(call<"System.Array$$get_Length", int32_t>(verticeArray, nullptr));
    FFI copyArrayBytes(verticeArray, 0, vertices.size() * sizeof(IPLVector3), vertices.data());
    FFI dropObject(verticeArray);

    // Get triangles
    const auto triangleArray = call<"UnityEngine.Mesh$$get_triangles", ObjectHandle>(mesh, nullptr);
    if (call_error)
        return nullptr;
    if (triangleArray == ObjectHandle::Null)
        return nullptr;
    std::vector<IPLTriangle> triangles(call<"System.Array$$get_Length", int32_t>(triangleArray, nullptr) / 3);
    FFI copyArrayBytes(triangleArray, 0, triangles.size() * sizeof(IPLTriangle), triangles.data());
    FFI dropObject(triangleArray);

    // Get materials (TODO: Just a dummy for now)
    std::vector<IPLint32> materials(triangles.size(), Materials::Brick);

    // Create settings
    IPLStaticMeshSettings staticMeshSettings{};
    staticMeshSettings.numVertices = vertices.size();
    staticMeshSettings.numTriangles = triangles.size();
    staticMeshSettings.numMaterials = 1;
    staticMeshSettings.vertices = vertices.data();
    staticMeshSettings.triangles = triangles.data();
    staticMeshSettings.materialIndices = materials.data();
    staticMeshSettings.materials = Materials::presets;

    // Create final static mesh
    IPLStaticMesh fres;
    const auto status = iplStaticMeshCreate(scene, &staticMeshSettings, &fres);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create static mesh from Unity mesh", status));
        return nullptr;
    }
    return fres;
}

bool updatePlayback(PhononPlayback::Playback& p, bool initial) {
    // Make sure playback hasn't expired
    if (p.audioSource == ObjectHandle::Null || !call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(p.audioSource, nullptr) || call_error) {
        FFI logDebug(FFI toCsString("Playback ended because native source object died!"));
        return false;
    }

    // Make sure playback hasn't ended
    if (p.hasReachedEnd())
        return false;

    // Update position
    bool positionChanged = initial;
    ObjectHandle sourceTransform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(p.audioSource, nullptr);
    if (sourceTransform != ObjectHandle::Null) {
        const auto newPosition = TransformUtils::get_position(sourceTransform);
        if (std::memcmp(&p.worldPosition, &newPosition, sizeof(newPosition)) != 0) {
            p.worldPosition = newPosition;
            positionChanged = true;
        }
    }

    // Update initial properties
    if (initial) {
        p.volume = std::fabs(call<"UnityEngine.AudioSource$$get_volume", float>(p.audioSource, nullptr));
        if (call<"UnityEngine.AudioSource$$get_spatialize", bool>(p.audioSource, nullptr))
            p.spatialBlend = call<"UnityEngine.AudioSource$$get_spatialBlend", float>(p.audioSource, nullptr);
        else
            p.spatialBlend = 0.0f;
        p.maxDistance = call<"UnityEngine.AudioSource$$get_maxDistance", float>(p.audioSource, nullptr);
        p.loop = call<"UnityEngine.AudioSource$$get_loop", bool>(p.audioSource, nullptr);
    }

    // Update simulation
    if (PhononSimulation::env.has_value()) {
        // Update simulation outputs
        p.updateSimulationOutputs();

        // Update simulation inputs
        if (positionChanged && p.source) {
            IPLSimulationInputs inputs{.flags = FixedSettings::simulationFlags,
                                       .directFlags = FixedSettings::directSimulationFlags,
                                       .source = {.right = TransformUtils::get_right(sourceTransform),
                                                  .up = TransformUtils::get_up(sourceTransform),
                                                  .ahead = TransformUtils::get_forward(sourceTransform),
                                                  .origin = p.worldPosition},
                                       .directivity = {.dipoleWeight = 1.0f},
                                       .occlusionType = IPL_OCCLUSIONTYPE_RAYCAST,
                                       .occlusionRadius = 0.25f,
                                       .numOcclusionSamples = 8,
                                       .reverbScale = {1.0f, 1.0f, 1.0f},
                                       .hybridReverbTransitionTime = 1.0f,
                                       .hybridReverbOverlapPercent = 0.25f,
                                       .numTransmissionRays = 16};
            iplSourceSetInputs(p.source, FixedSettings::simulationFlags, &inputs);
            PhononSimulation::env->markSimulatorDirty();
        }
    }

    // We're all good
    FFI dropObject(sourceTransform);
    return true;
}
} // namespace PhononTools

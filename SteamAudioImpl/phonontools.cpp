#include "phonontools.hpp"
#include "SteamAudioImpl.hpp"
#include "utils.hpp"
#include "playback.hpp"
#include "fixedsettings.hpp"

#include <vector>
#include <cstring>
#include <cmath>
#include <ffi_interface.hpp>

using namespace FFIInterface;
using namespace Helpers;

namespace {
namespace Materials {
static IPLMaterial presets[11] = {
    {0.10f, 0.20f, 0.30f, 0.05f, 0.100f, 0.050f, 0.030f}, {0.03f, 0.04f, 0.07f, 0.05f, 0.015f, 0.015f, 0.015f},
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
IPLVector3 get_position(FFIInterface::ObjectHandle transform) {
    return callRetStruct<"UnityEngine.Transform$$get_position", IPLVector3>(transform, nullptr);
}
IPLVector3 get_forward(FFIInterface::ObjectHandle transform) {
    return callRetStruct<"UnityEngine.Transform$$get_forward", IPLVector3>(transform, nullptr);
}
IPLVector3 get_up(FFIInterface::ObjectHandle transform) {
    return callRetStruct<"UnityEngine.Transform$$get_up", IPLVector3>(transform, nullptr);
}
IPLVector3 get_right(FFIInterface::ObjectHandle transform) {
    return callRetStruct<"UnityEngine.Transform$$get_right", IPLVector3>(transform, nullptr);
}
IPLMatrix4x4 get_localToWorldMatrix(FFIInterface::ObjectHandle transform) {
    return callRetStruct<"UnityEngine.Transform$$get_localToWorldMatrix", IPLMatrix4x4>(transform, nullptr);
}
} // namespace TransformUtils

IPLAudioBuffer audioBufferFromAudioClip(FFIInterface::ObjectHandle audioClip) {
    IPLAudioBuffer fres{};

    // Get basic properties
    fres.numSamples = call<"UnityEngine.AudioClip$$get_samples", int32_t>(audioClip, nullptr);
    fres.numChannels = call<"UnityEngine.AudioClip$$get_channels", int32_t>(audioClip, nullptr);

    // Get data
    static ClassHandle singleArrayClass = []() {
        ImageHandle corlib = FFI getImageCorlib();
        ClassHandle single = FFI getClassFromName(corlib, "System", "Single");
        ClassHandle singleArray = FFI getArrayFromClass(single, 1);
        FFI dropImage(corlib);
        FFI dropClass(single);
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
    IPLerror status = iplAudioBufferAllocate(GlobalState::phononCtx, fres.numChannels, fres.numSamples, &fres);
    if (status != IPL_STATUS_SUCCESS)
        FFI logError(Utils::createErrorMessage("allocate input audio buffer", status));

    // Deinterleave Unity provided buffer
    iplAudioBufferDeinterleave(GlobalState::phononCtx, data.data(), &fres);

    // Ensure buffer is no more than stereo
    if (fres.numChannels > 2) {
        IPLAudioBuffer stereoFres;
        status = iplAudioBufferAllocate(GlobalState::phononCtx, 2, fres.numSamples, &stereoFres);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage("allocate downmixed input audio buffer", status));
        iplAudioBufferDownmix(GlobalState::phononCtx, &fres, &stereoFres);
        iplAudioBufferFree(GlobalState::phononCtx, &fres);
        fres = stereoFres;
    }

    return fres;
}

IPLStaticMesh staticMeshFromMesh(IPLScene scene, FFIInterface::ObjectHandle mesh) {
    if (mesh == ObjectHandle::Null || mesh == ObjectHandle::Invalid ||
        !call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(mesh, nullptr) || call_error)
        return nullptr;

    if (!call<"UnityEngine.Mesh$$get_isReadable", bool>(mesh, nullptr) || call_error)
        return nullptr;

    ObjectHandle nameObj = call<"UnityEngine.Object$$get_name", ObjectHandle>(mesh, nullptr);
    if (nameObj == ObjectHandle::Null || nameObj == ObjectHandle::Invalid)
        return nullptr;

    FFI logError(FFI toCsString(("Extracting mesh: " + std::string(getCString(nameObj))).c_str()));

    // Get vertices
    const auto verticeArray = call<"UnityEngine.Mesh$$get_vertices", ObjectHandle>(mesh, nullptr);
    if (call_error || verticeArray == ObjectHandle::Null || verticeArray == ObjectHandle::Invalid)
        return nullptr;

    const int32_t arrayVertCount = call<"System.Array$$get_Length", int32_t>(verticeArray, nullptr);
    if (call_error || arrayVertCount <= 0) {
        FFI dropObject(verticeArray);
        return nullptr;
    }

    std::vector<IPLVector3> vertices(arrayVertCount);
    FFI copyArrayBytes(verticeArray, 0, vertices.size() * sizeof(IPLVector3), vertices.data());
    FFI dropObject(verticeArray);

    // Get triangles
    const auto triangleArray = call<"UnityEngine.Mesh$$get_triangles", ObjectHandle>(mesh, nullptr);
    if (call_error || triangleArray == ObjectHandle::Null || triangleArray == ObjectHandle::Invalid)
        return nullptr;

    const int32_t triIndexCount = call<"System.Array$$get_Length", int32_t>(triangleArray, nullptr);
    if (call_error || triIndexCount < 3) {
        FFI dropObject(triangleArray);
        return nullptr;
    }

    std::vector<IPLTriangle> triangles(triIndexCount / 3);
    FFI copyArrayBytes(triangleArray, 0, triangles.size() * sizeof(IPLTriangle), triangles.data());
    FFI dropObject(triangleArray);

    // Get materials (TODO: Just a dummy for now)
    std::vector<IPLint32> materials(triangles.size(), 0);

    // Create settings
    IPLStaticMeshSettings staticMeshSettings{};
    staticMeshSettings.numVertices = vertices.size();
    staticMeshSettings.numTriangles = triangles.size();
    staticMeshSettings.numMaterials = 1;
    staticMeshSettings.vertices = vertices.data();
    staticMeshSettings.triangles = triangles.data();
    staticMeshSettings.materialIndices = materials.data();
    staticMeshSettings.materials = &Materials::presets[Materials::Brick];

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
    if (p.audioSource == ObjectHandle::Null ||
        !call<"UnityEngine.Object$$IsNativeObjectAlive", bool>(p.audioSource, nullptr) || call_error) {
        FFI logDebug(FFI toCsString("Playback ended because native source object died!"));
        return false;
    }

    // Make sure playback hasn't ended
    if (p.hasReachedEnd())
        return false;

    // Update position
    ObjectHandle sourceTransform = call<"UnityEngine.Component$$get_transform", ObjectHandle>(p.audioSource, nullptr);
    if (sourceTransform != ObjectHandle::Null) {
        const auto newPosition = TransformUtils::get_position(sourceTransform);
        if (std::memcmp(&p.worldPosition, &newPosition, sizeof(newPosition)) != 0) {
            p.worldPosition = newPosition;
        }
    }

    // Update initial properties
    p.volume = call<"UnityEngine.AudioSource$$get_volume", float>(p.audioSource, nullptr); // Meant to be initial
    if (initial) {
        if (call<"UnityEngine.AudioSource$$get_spatialize", bool>(p.audioSource, nullptr))
            p.spatialBlend = call<"UnityEngine.AudioSource$$get_spatialBlend", float>(p.audioSource, nullptr);
        else
            p.spatialBlend = 0.0f;
        p.maxDistance = call<"UnityEngine.AudioSource$$get_maxDistance", float>(p.audioSource, nullptr);
        p.loop = call<"UnityEngine.AudioSource$$get_loop", bool>(p.audioSource, nullptr);
    }

    // We're all good
    FFI dropObject(sourceTransform);
    return true;
}

IPLStaticMesh staticMeshFromBoxCollider(IPLScene scene, FFIInterface::ObjectHandle boxCollider) {
    if (boxCollider == ObjectHandle::Null || boxCollider == ObjectHandle::Invalid)
        return nullptr;

    const IPLVector3 center = callRetStruct<"UnityEngine.BoxCollider$$get_center", IPLVector3>(boxCollider, nullptr);
    const IPLVector3 size = callRetStruct<"UnityEngine.BoxCollider$$get_size", IPLVector3>(boxCollider, nullptr);

    const float hx = size.x * 0.5f;
    const float hy = size.y * 0.5f;
    const float hz = size.z * 0.5f;

    // Define 8 corners of the box in local space
    IPLVector3 vertices[8] = {
        {center.x - hx, center.y - hy, center.z - hz}, // 0: -X, -Y, -Z
        {center.x + hx, center.y - hy, center.z - hz}, // 1: +X, -Y, -Z
        {center.x - hx, center.y + hy, center.z - hz}, // 2: -X, +Y, -Z
        {center.x + hx, center.y + hy, center.z - hz}, // 3: +X, +Y, -Z
        {center.x - hx, center.y - hy, center.z + hz}, // 4: -X, -Y, +Z
        {center.x + hx, center.y - hy, center.z + hz}, // 5: +X, -Y, +Z
        {center.x - hx, center.y + hy, center.z + hz}, // 6: -X, +Y, +Z
        {center.x + hx, center.y + hy, center.z + hz}  // 7: +X, +Y, +Z
    };

    // 12 triangles (2 per face)
    IPLTriangle triangles[12] = {
        {0, 2, 1}, {1, 2, 3}, // Front (-Z)
        {5, 7, 4}, {4, 7, 6}, // Back (+Z)
        {4, 6, 0}, {0, 6, 2}, // Left (-X)
        {1, 3, 5}, {5, 3, 7}, // Right (+X)
        {2, 6, 3}, {3, 6, 7}, // Top (+Y)
        {4, 0, 5}, {5, 0, 1}  // Bottom (-Y)
    };

    IPLint32 materials[12] = {0};

    IPLStaticMeshSettings staticMeshSettings{};
    staticMeshSettings.numVertices = 8;
    staticMeshSettings.numTriangles = 12;
    staticMeshSettings.numMaterials = 1;
    staticMeshSettings.vertices = vertices;
    staticMeshSettings.triangles = triangles;
    staticMeshSettings.materialIndices = materials;
    staticMeshSettings.materials = &Materials::presets[Materials::Wood]; // Map to Wood/Plaster or desired default

    IPLStaticMesh fres;
    const auto status = iplStaticMeshCreate(scene, &staticMeshSettings, &fres);
    if (status != IPL_STATUS_SUCCESS) {
        FFI logError(Utils::createErrorMessage("create static mesh from BoxCollider", status));
        return nullptr;
    }
    return fres;
}
} // namespace PhononTools

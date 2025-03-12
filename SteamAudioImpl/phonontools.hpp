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
namespace TransformUtils {
IPLVector3 get_position(FFIInterface::ObjectHandle transform);
IPLVector3 get_forward(FFIInterface::ObjectHandle transform);
IPLVector3 get_up(FFIInterface::ObjectHandle transform);
IPLMatrix4x4 get_localToWorldMatrix(FFIInterface::ObjectHandle transform);
} // namespace TransformUtils

IPLAudioBuffer audioBufferFromAudioClip(FFIInterface::ObjectHandle audioClip);
IPLStaticMesh staticMeshFromMesh(IPLScene scene, FFIInterface::ObjectHandle mesh);
bool updatePlayback(PhononPlayback::Playback& p, bool initial);
} // namespace PhononTools

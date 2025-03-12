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
} // namespace TransformUtils

IPLAudioBuffer audioBufferFromAudioClip(FFIInterface::ObjectHandle audioClip);
IPLStaticMesh staticMeshfromMesh(IPLScene scene, FFIInterface::ObjectHandle mesh);
bool updatePlayback(PhononPlayback::Playback& p, bool initial);
} // namespace PhononTools

#pragma once

#include "SteamAudioImpl_global.hpp"
#include "miniaudio.h"

#include <phonon.h>

#define FFI_FTABLE *imports

namespace FFIInterface {
struct Imports;
}
extern const FFIInterface::Imports *imports;

namespace GlobalState {
extern IPLContext phononCtx;
extern ma_device maDevice;

extern IPLCoordinateSpace3 playerCoord;
} // namespace GlobalState

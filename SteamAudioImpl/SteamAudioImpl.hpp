#pragma once

#include "SteamAudioImpl_global.hpp"
#include "miniaudio.h"

#include <atomic>
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

struct Statistics {
    std::atomic<unsigned> framesStarted = 0, framesFinished = 0, playbacksStarted = 0, playbacksFinished = 0;
} extern stats;
} // namespace GlobalState

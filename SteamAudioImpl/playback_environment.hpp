#pragma once

#include "bufferpool.hpp"

#include <optional>
#include <phonon.h>

namespace PhononPlayback {
struct Environment {
    IPLAudioSettings audioSettings;
    IPLHRTF hrtf = nullptr;
    BufferPool bufferPool;

    Environment(IPLint32 frameSize);
};
extern std::optional<Environment> env;
} // namespace PhononPlayback

#pragma once

#include "SteamAudioImpl.hpp"

#include <array>
#include <phonon.h>

namespace PhononPlayback {
class BufferPool {
public:
    BufferPool(IPLint32 channels, IPLint32 samples);
    ~BufferPool();
    BufferPool(const BufferPool&) = delete;
    BufferPool(BufferPool&&) = delete;

    IPLAudioBuffer& GetCurrentBuffer() { return buffers_[bufferIdx_]; }
    IPLAudioBuffer& GetNextBuffer() { return buffers_[GetNextBufferIndex()]; }
    void SwitchToNextBuffer() { bufferIdx_ = GetNextBufferIndex(); }

private:
    // Returns next buffer index.
    unsigned GetNextBufferIndex() const;

    // Current buffer index.
    unsigned bufferIdx_ = 0;
    // Different audio buffers for processing.
    std::array<IPLAudioBuffer, 4> buffers_;
};
} // namespace PhononPlayback

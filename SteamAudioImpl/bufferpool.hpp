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
    BufferPool& operator=(const BufferPool&) = delete;
    BufferPool& operator=(BufferPool&&) = delete;

    IPLAudioBuffer& GetCurrentBuffer() { return buffers_[bufferIdx_]; }
    IPLAudioBuffer& GetNextBuffer() { return buffers_[GetNextBufferIndex()]; }
    void SwitchToNextBuffer() { bufferIdx_ = GetNextBufferIndex(); }

private:
    unsigned GetNextBufferIndex() const;

    unsigned bufferIdx_ = 0;
    std::array<IPLAudioBuffer, 4> buffers_{};
};
} // namespace PhononPlayback

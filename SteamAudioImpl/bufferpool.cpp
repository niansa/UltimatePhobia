#include "bufferpool.hpp"
#include "SteamAudioImpl.hpp"
#include "utils.hpp"

#include <ffi_interface.hpp>

namespace PhononPlayback {
BufferPool::BufferPool(IPLint32 channels, IPLint32 samples) {
    for (auto& buffer : buffers_) {
        IPLerror status = iplAudioBufferAllocate(GlobalState::phononCtx,
                                                 channels, samples, &buffer);
        if (status != IPL_STATUS_SUCCESS)
            FFI logError(Utils::createErrorMessage(
                "create buffer for buffer pool", status));
    }
}
BufferPool::~BufferPool() {
    for (auto& buffer : buffers_)
        iplAudioBufferFree(GlobalState::phononCtx, &buffer);
}

unsigned BufferPool::GetNextBufferIndex() const {
    auto bufferIdx = bufferIdx_;
    if (++bufferIdx == buffers_.size())
        return 0;
    return bufferIdx;
}
} // namespace PhononPlayback

#include "environment.hpp"
#include "SteamAudioImpl.hpp"
#include "fixedsettings.hpp"

namespace PhononPlayback {
std::optional<Environment> env;

Environment::Environment(IPLint32 frameSize)
    : bufferPool(FixedSettings::outputChannels, frameSize) {
    audioSettings.frameSize = frameSize;
    audioSettings.samplingRate = GlobalState::maDevice.sampleRate;

    {
        IPLHRTFSettings hrtfSettings{.type = IPL_HRTFTYPE_DEFAULT,
                                     .volume = 1.0f};
        iplHRTFCreate(GlobalState::phononCtx, &audioSettings, &hrtfSettings,
                      &hrtf);
    }
}
} // namespace PhononPlayback

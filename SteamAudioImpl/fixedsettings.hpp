#pragma once

#include <phonon.h>

namespace FixedSettings {
constexpr unsigned outputChannels = 2;
constexpr IPLDirectEffectFlags directEffectFlags =
    static_cast<IPLDirectEffectFlags>(IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION | IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION);
}

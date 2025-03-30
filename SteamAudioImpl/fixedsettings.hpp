#pragma once

#include <phonon.h>

namespace FixedSettings {
constexpr unsigned outputChannels = 2;
constexpr IPLSimulationFlags simulationFlags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
constexpr IPLDirectSimulationFlags directSimulationFlags =
    static_cast<IPLDirectSimulationFlags>(IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION | IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
                                          IPL_DIRECTSIMULATIONFLAGS_OCCLUSION | IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
constexpr IPLDirectEffectFlags directEffectFlags =
    static_cast<IPLDirectEffectFlags>(IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION | IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION);
constexpr IPLDirectEffectFlags directEffectSimulationFlags =
    static_cast<IPLDirectEffectFlags>(directEffectFlags | IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION | IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
constexpr float irDuration = 4.0f;
constexpr unsigned numRays = 4096;
constexpr unsigned reflectionAmbisonicsOrder = 2;
}

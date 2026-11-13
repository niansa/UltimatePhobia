#pragma once

#include <phonon.h>

namespace FixedSettings {
constexpr unsigned outputChannels = 2;
constexpr unsigned ambisonicsOrder = 1;
constexpr unsigned ambisonicsChannels = 4;

constexpr float transformUpdateIntervalSeconds = 0.05f;
constexpr float colliderRescanIntervalSeconds = 1.0f;
constexpr float reflectionUpdateIntervalSeconds = 0.20f;
constexpr float reflectionDurationSeconds = 1.0f;

constexpr IPLint32 maxOcclusionSamples = 16;
constexpr IPLint32 maxTransmissionRays = 8;
constexpr IPLint32 maxReflectionRays = 4096;
constexpr IPLint32 reflectionBounces = 4;
constexpr IPLint32 maxRaytracedSources = 256;

constexpr IPLDirectEffectFlags directEffectFlags = static_cast<IPLDirectEffectFlags>(
    IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION | IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION |
    IPL_DIRECTEFFECTFLAGS_APPLYDIRECTIVITY | IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
    IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
} // namespace FixedSettings

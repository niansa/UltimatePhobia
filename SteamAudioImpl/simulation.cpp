#include "SteamAudioImpl.hpp"
#include "simulation.hpp"
#include "simulation_environment.hpp"
#include "fixedsettings.hpp"

#include <optional>
#include <future>

namespace PhononSimulation {
namespace {
std::optional<std::future<void>> simulation;
}

void run() {
    using namespace std::chrono_literals;

    // Make sure environment exists
    if (!env.has_value())
        env.emplace();

    // Leave active simulation running
    if (simulation.has_value() && simulation->wait_for(0ms) != std::future_status::ready)
        return;

    // Update scene used for simulation
    env->updateScene();

    // Prepare simulation
    {
        IPLSimulationSharedInputs sharedInputs{.listener = GlobalState::playerCoord,
                                               .numRays = FixedSettings::numRays,
                                               .numBounces = 16,
                                               .duration = FixedSettings::irDuration,
                                               .order = FixedSettings::reflectionAmbisonicsOrder,
                                               .irradianceMinDistance = 1.0f};
        iplSimulatorSetSharedInputs(env->getSimulator(), FixedSettings::simulationFlags, &sharedInputs);

        std::scoped_lock L(env->getSimulatorMutex());
        env->undirtySimulator();
    }

    // Run new simulation
    simulation = std::async(std::launch::async, [] {
        std::scoped_lock L(env->getSimulatorMutex());
        iplSimulatorRunDirect(env->getSimulator());
        iplSimulatorRunReflections(env->getSimulator());
    });
}
} // namespace PhononSimulation

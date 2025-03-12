#include "SteamAudioImpl.hpp"
#include "simulation.hpp"
#include "simulation_environment.hpp"

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

    // Update scene used for simulation
    env->updateScene();

    // Leave active simulation running
    if (simulation.has_value() && simulation->wait_for(0ms) != std::future_status::ready)
        return;

    // Prepare simulation
    {
        IPLSimulationSharedInputs sharedInputs{};
        sharedInputs.listener = GlobalState::playerCoord;

        iplSimulatorSetSharedInputs(env->getSimulator(), IPL_SIMULATIONFLAGS_DIRECT, &sharedInputs);
    }
    std::scoped_lock L(env->getSimulatorMutex());
    env->undirtySimulator();

    // Run new simulation
    simulation = std::async(std::launch::async, [] {
        std::scoped_lock L(env->getSimulatorMutex());
        iplSimulatorRunDirect(env->getSimulator());
    });
}
} // namespace PhononSimulation

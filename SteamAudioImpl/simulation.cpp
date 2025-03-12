#include "simulation.hpp"
#include "simulation_environment.hpp"

#include <future>

namespace PhononSimulation {
namespace {
std::future<void> simulation;
}

void run() {
    // Make sure environment exists
    if (!env.has_value())
        env.emplace();

    // Update scene used for simulation
    env->updateScene();

    simulation = std::async(std::launch::async, [] {
        //
    });
}
} // namespace PhononSimulation

#include "simulation.hpp"
#include "simulation_environment.hpp"

namespace PhononSimulation {
void run() {
    // Make sure environment exists
    if (!env.has_value())
        env.emplace();

    // Update scene used for simulation
    env->updateScene();
}
} // namespace PhononSimulation

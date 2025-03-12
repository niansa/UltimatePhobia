#pragma once

#include <optional>
#include <mutex>
#include <phonon.h>

namespace PhononSimulation {
class Environment {
    struct Impl;
    Impl *i;

public:
    Environment();
    ~Environment();
    Environment(const Environment&) = delete;
    Environment(Environment&&) = delete;

    IPLScene getScene();
    IPLSimulator getSimulator();
    unsigned getMeshCount();
    std::mutex& getSimulatorMutex();

    void updateScene();
    void markSimulatorDirty();
    void undirtySimulator();

    bool getSimulationOutputs(IPLSource source, IPLSimulationOutputs& outputs);
};
extern std::optional<Environment> env;
} // namespace PhononSimulation

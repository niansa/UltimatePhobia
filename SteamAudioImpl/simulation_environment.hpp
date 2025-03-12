#pragma once

#include <optional>
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

    void updateScene();
    IPLScene getScene();
    unsigned getMeshCount();
};
extern std::optional<Environment> env;
} // namespace PhononSimulation

#pragma once

#include "CLI11.hpp"

class CommandHandler {
public:
    virtual ~CommandHandler() = default;

    virtual void updateApp(CLI::App& app);
};

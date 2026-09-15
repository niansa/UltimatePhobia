#pragma once

#include "base.hpp"

class MiscCommands : public Mod {
public:
    virtual std::unique_ptr<CommandHandler> getCommandHandler();
};

extern ModInfo miscCommandsInfo;

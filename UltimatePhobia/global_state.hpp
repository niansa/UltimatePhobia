#pragma once

#include <memory>
#include <spdlog/logger.h>

struct GlobalState {
    uintptr_t base;
    std::shared_ptr<spdlog::logger> logger;
} extern g;

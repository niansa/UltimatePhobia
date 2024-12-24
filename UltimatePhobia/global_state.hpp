#pragma once

#include <memory>
#include <spdlog/logger.h>

struct GlobalState {
    void *base;
    std::shared_ptr<spdlog::logger> logger;
} extern g;

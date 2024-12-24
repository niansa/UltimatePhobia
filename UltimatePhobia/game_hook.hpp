#pragma once

#include <array>
#include <cstdint>


class GameHook {
    friend class GameHookRelease;

    void *fnc;
    void *hook;

    static constexpr unsigned original_len = 14;
    std::array<uint8_t, original_len> original;
    bool released;

    bool release();
    void restore();

public:
    GameHook(void *fnc, void *hook);
    ~GameHook() {release();}
    GameHook(const GameHook&) = delete;
    GameHook(GameHook&&) = delete;

    template<typename fncT>
    fncT *getFunction() {
        return reinterpret_cast<fncT*>(fnc);
    }
};

class GameHookRelease {
    GameHook& hook;
    bool active;

public:
    GameHookRelease(GameHook &hook);
    ~GameHookRelease();
    GameHookRelease(const GameHookRelease &) = delete;
    GameHookRelease(GameHookRelease &&) = delete;

    bool isValid() const {
        return active;
    }
};

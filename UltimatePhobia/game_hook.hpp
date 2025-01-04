#pragma once

#include <array>
#include <cstdint>


extern void *GameHookCallerRip;

class GameHook {
    friend class GameHookRelease;

    void *fnc;
    void *hook;
    bool useTrampoline;

    static constexpr unsigned original_len = 14;
    std::array<uint8_t, original_len> original;
    bool released;

    bool release();
    bool restore();

public:
    static void noop();

    GameHook(void *fnc, void *hook, bool useTrampoline = false);
    ~GameHook() {release();}
    GameHook(const GameHook&) = delete;
    GameHook(GameHook&&) = delete;

    static void *getTrampolineCaller();

    void *getAddr() const {
        return fnc;
    }

    template<typename fncT>
    fncT *getFunction() const {
        return reinterpret_cast<fncT*>(fnc);
    }

    bool isActive() const {
        return !released;
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


#define GAMEHOOK_TRAMPOLINE(hook) \
    __attribute__((naked)) \
    static void hookTrampoline_##hook () { \
        __asm pop r10 \
        __asm mov GameHookCallerRip, r10 \
        __asm jmp hook \
    }

#pragma once

#include "global_state.hpp"
#include "mods/base.hpp"

#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <optional>
#include <cstdint>

extern void *GameHookCallerRip;

class GameHook {
    friend class GameHookRelease;

    void *fnc;
    void *hook;
    bool use2NdTrampoline;

    static constexpr unsigned original_len = 14;
    std::array<uint8_t, original_len> original;
    bool released;

    bool release();
    bool restore();

    GameHook(void *fnc, void *hook, bool useTrampoline = false);

public:
    static void noop();

    ~GameHook() {
        if (!released)
            release();
    }
    GameHook(const GameHook&) = delete;
    GameHook(GameHook&& o) : fnc(o.fnc), hook(o.hook), use2NdTrampoline(o.use2NdTrampoline), original(o.original), released(o.released) { o.released = true; }

    static std::optional<GameHook> safeCreate(void *fnc, void *hook, bool useTrampoline = false);
    static void safeCreate(std::optional<GameHook>& fres, void *fnc, void *hook, bool useTrampoline = false);
    static GameHook safeCreateOrPanic(ModInfo& mod, void *fnc, void *hook, bool useTrampoline = false);
    static GameHook unsafeCreate(void *fnc, void *hook, bool useTrampoline = false);
    static void *getTrampolineCaller();
    static void *getHookAt(void *fnc);
    static inline bool isHookAt(void *fnc) { return getHookAt(fnc) != nullptr; }

    void *getAddr() const { return fnc; }

    template <typename fncT> fncT *getFunction() const { return reinterpret_cast<fncT *>(fnc); }

    bool isActive() const { return !released; }
};

class GameHookRelease {
    GameHook& hook;
    bool active;

public:
    GameHookRelease(GameHook& hook);
    ~GameHookRelease();
    GameHookRelease(const GameHookRelease&) = delete;
    GameHookRelease(GameHookRelease&&) = delete;

    bool isValid() const { return active; }
};

class GameHookPool {
    std::vector<std::shared_ptr<GameHook>> hooks;

public:
    template <typename fncT> std::shared_ptr<GameHook> add(void *fnc, fncT *hook, bool useTrampoline = false) {
        if (get(fnc) != nullptr) {
            g.logger->warn("Not adding function to hook pool more than once");
            return nullptr;
        }
        auto fres = GameHook::safeCreate(fnc, reinterpret_cast<void *>(hook), useTrampoline);
        if (!fres.has_value() || !fres->isActive())
            return nullptr;
        return hooks.emplace_back(std::make_shared<GameHook>(std::move(*fres)));
    }
    std::shared_ptr<GameHook> get(void *fnc) {
        for (auto& hook : hooks) {
            if (hook->getAddr() == fnc)
                return hook;
        }
        return nullptr;
    }
    void remove(const std::shared_ptr<GameHook>& hook) {
        if (hook == nullptr)
            return;
        hooks.erase(std::remove(hooks.begin(), hooks.end(), hook), hooks.end());
    }
};

#ifdef _MSC_VER
#define GAMEHOOK_TRAMPOLINE(hook)                                                                                                                              \
    __attribute__((naked)) static void hookTrampoline_##hook() { __asm pop r10 __asm mov GameHookCallerRip, r10 __asm jmp hook }
#else
#define GAMEHOOK_TRAMPOLINE(hook)                                                                                                                              \
    __attribute__((naked)) static void hookTrampoline_##hook() {                                                                                               \
        asm volatile("pop %%r10\n"                                                                                                                             \
                     "lea GameHookCallerRip(%%rip), %%r11\n"                                                                                                   \
                     "mov %%r10, (%%r11)\n"                                                                                                                    \
                     "jmp %P0\n"                                                                                                                               \
                     :                                                                                                                                         \
                     : "i"(hook)                                                                                                                               \
                     : "r10", "r11");                                                                                                                          \
    }
#endif

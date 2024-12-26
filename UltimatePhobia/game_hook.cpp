#include "game_hook.hpp"
#include "global_state.hpp"

#include <cstring>
#include <cstdint>
#include <windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>


__attribute__((used))
void *GameHookCallerRip;



GameHook::GameHook(void *fnc, void *hook, bool useTrampoline)
    : fnc(fnc), hook(hook), useTrampoline(useTrampoline) {
    // Back up instructions from original function
    memcpy(original.data(), fnc, original_len);

    // Initial hook "restore"
    restore();
}

void *GameHook::getTrampolineCaller() {
    return reinterpret_cast<uint8_t *>(GameHookCallerRip) - 6;
}

bool GameHook::release() {
    // Restore original instructions
    SIZE_T writ = 0;
    WriteProcessMemory(GetCurrentProcess(), fnc, original.data(), original_len, &writ);

    if (writ != original_len) {
        g.logger->error("Failed to release hook at {}", fnc);
        return false;
    }

    released = true;
    return true;
}

bool GameHook::restore() {
    // Generate trampoline instructions
    uint8_t test[] = {
        0xff, static_cast<uint8_t>(useTrampoline?0x15:0x25), 0x00, 0x00, 0x00, 0x00,
        0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde
    };
    static_assert(sizeof(hook) == 8);
    memcpy(test+sizeof(test)-8, &hook, 8);

    // Write new instructions
    static_assert(original_len == sizeof(test));
    SIZE_T writ = 0;
    WriteProcessMemory(GetCurrentProcess(), fnc, test, sizeof(test), &writ);

    if (writ != sizeof(test)) {
        g.logger->error("Failed to set hook at {}", fnc);
        return false;
    }

    released = false;
    return true;
}


GameHookRelease::GameHookRelease(GameHook& hook)
    : hook(hook) {
    if (!hook.isActive()) {
        active = false;
        return;
    }
    hook.release();
    active = !hook.isActive();
}

GameHookRelease::~GameHookRelease() {
    if (active)
        hook.restore();
}


void GameHook::noop() {}

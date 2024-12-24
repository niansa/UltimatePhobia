#include "game_hook.hpp"
#include "global_state.hpp"

#include <cstring>
#include <cstdint>
#include <windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>



GameHook::GameHook(void *fnc, void *hook)
    : fnc(fnc), hook(hook) {
    // Back up instructions from original function
    memcpy(original.data(), fnc, original_len);

    // Initial hook "restore"
    restore();
}

bool GameHook::release() {
    if (released)
        return false;
    released = true;

    // Restore original instructions
    SIZE_T writ = 0;
    WriteProcessMemory(GetCurrentProcess(), fnc, original.data(), original_len, &writ);

    if (writ != original_len) {
        g.logger->error("Failed to release hook at {}", fnc);
        return false;
    }

    return true;
}

void GameHook::restore() {
    released = false;

    // Generate trampoline instructions
    uint8_t test[] = {
        0xff, 0x25, 0x00, 0x00, 0x00, 0x00,
        0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde
    };
    static_assert(sizeof(hook) == 8);
    memcpy(test+sizeof(test)-8, &hook, 8);

    // Write new instructions
    static_assert(original_len == sizeof(test));
    SIZE_T writ = 0;
    WriteProcessMemory(GetCurrentProcess(), fnc, test, sizeof(test), &writ);

    if (writ != sizeof(test))
        g.logger->error("Failed to set hook at {}", fnc);
}


GameHookRelease::GameHookRelease(GameHook& hook)
    : hook(hook) {
    active = hook.release();
}

GameHookRelease::~GameHookRelease() {
    if (active)
        hook.restore();
}

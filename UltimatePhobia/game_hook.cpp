#include "game_hook.hpp"
#include "global_state.hpp"
#include "mods/base.hpp"

#include <array>
#include <cstring>
#include <cstdint>
#include <windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>

__attribute__((used)) void *GameHookCallerRip;

static std::array<uint8_t, 14> generate1StTrampolinue(bool has2NdTrampoline) {
    return {0xff, static_cast<uint8_t>(has2NdTrampoline ? 0x15 : 0x25), 0x00, 0x00, 0x00, 0x00, 0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};
}

GameHook::GameHook(void *fnc, void *hook, bool useTrampoline) : fnc(fnc), hook(hook), use2NdTrampoline(useTrampoline) {
    g.logger->debug("Setting up hook at {} to {} {}second stage trampoline...", fnc, hook, useTrampoline ? "with " : "without ");

    // Back up instructions from original function
    memcpy(original.data(), fnc, original_len);

    // Initial hook "restore"
    restore();
}

std::optional<GameHook> GameHook::safeCreate(void *fnc, void *hook, bool useTrampoline) {
    std::optional<GameHook> fres;
    safeCreate(fres, fnc, hook, useTrampoline);
    return fres;
}

void GameHook::safeCreate(std::optional<GameHook>& fres, void *fnc, void *hook, bool useTrampoline) {
    fres.reset();
    if (isHookAt(fnc)) {
        g.logger->warn("Prevented safe hook creation because target function "
                       "already has a hook set");
        return;
    }
    GameHook maybeFres(fnc, hook, useTrampoline);
    if (maybeFres.isActive())
        fres.emplace(std::move(maybeFres));
    else
        g.logger->warn("Prevented safe hook creation because hook failed to "
                       "set initialize");
}

GameHook GameHook::safeCreateOrPanic(ModInfo& mod, void *fnc, void *hook, bool useTrampoline) {
    auto fres = safeCreate(fnc, hook, useTrampoline);
    if (!fres.has_value())
        throw ModPanic(mod, fmt::format("Failed to set mandatory hook at {}", fnc));
    return std::move(*fres);
}

GameHook GameHook::unsafeCreate(void *fnc, void *hook, bool useTrampoline) { return GameHook(fnc, hook, useTrampoline); }

void *GameHook::getTrampolineCaller() { return reinterpret_cast<uint8_t *>(GameHookCallerRip) - 6; }

void *GameHook::getHookAt(void *fnc) {
    HANDLE hProcess = GetCurrentProcess();

    // Read relevant data from memory
    std::array<uint8_t, original_len - 8> buf;
    SIZE_T reat = 0;
    ReadProcessMemory(hProcess, fnc, buf.data(), buf.size(), &reat);
    if (reat != sizeof(buf))
        return nullptr;

    // Try with and without 2nd stage trampoline
    for (const bool has2NdTrampoline : {false, true}) {
        const auto fullTrampoline = generate1StTrampolinue(has2NdTrampoline);

        // Extract instructions from trampoline
        decltype(buf) trampoline;
        std::copy(fullTrampoline.begin(), fullTrampoline.end() - 8, trampoline.begin());

        // Check if read instructions match
        if (trampoline == buf) {
            // Read trampoline jump destination
            void *fres;
            ReadProcessMemory(hProcess, reinterpret_cast<uint8_t *>(fnc) + trampoline.size(), &fres, sizeof(fres), &reat);
            if (reat != sizeof(fres))
                return nullptr;
            return fres;
        }
    }

    // There's no trampoline here
    return nullptr;
}

bool GameHook::release() {
    // Restore original datadelete
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
    // Generate first stage trampoline
    auto buf = generate1StTrampolinue(use2NdTrampoline);
    static_assert(sizeof(hook) == 8);
    memcpy(reinterpret_cast<void *>(buf.data() + buf.size() - 8), &hook, 8);

    // Write new data
    static_assert(original_len == buf.size());
    SIZE_T writ = 0;
    WriteProcessMemory(GetCurrentProcess(), fnc, buf.data(), buf.size(), &writ);

    if (writ != buf.size()) {
        g.logger->error("Failed to set hook at {}", fnc);
        return false;
    }

    released = false;
    return true;
}

GameHookRelease::GameHookRelease(GameHook& hook) : hook(hook) {
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

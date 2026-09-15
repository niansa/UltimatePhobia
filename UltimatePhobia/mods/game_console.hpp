#pragma once

#include "mods/base.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include <d3d11_1.h>
#include <vterm.h>

class GameConsole final : public Mod {
public:
    GameConsole();
    ~GameConsole() override;

    void uiUpdate() override;
    void onDX11Present(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) override;

    void print(std::string_view message);
    void println(std::string_view message);

private:
    struct RenderSnapshot;

    void writeTerminalLocked(std::string_view text);
    void resizeTerminalLocked(int rows, int columns);
    RenderSnapshot buildRenderSnapshotLocked() const;

    void resetKeyboardState();
    void updateKeyboard();
    void insertTextLocked(std::u32string_view text);
    void navigateHistoryLocked(int direction);

    static int scrollbackPushLine(int columns, const VTermScreenCell *cells, void *user);
    static int scrollbackPopLine(int columns, VTermScreenCell *cells, void *user);
    static int scrollbackClear(void *user);

    static constexpr std::size_t kMaximumScrollbackLines = 4096;
    static constexpr std::size_t kMaximumHistoryEntries = 100;

    std::atomic_bool isOpen_{false};
    bool tildeWasPressed_ = false;

    std::array<bool, 256> keyDown_{};
    std::array<std::chrono::steady_clock::time_point, 256> nextKeyRepeat_{};

    mutable std::mutex mutex_;

    VTerm *vterm_ = nullptr;
    VTermScreen *screen_ = nullptr;
    VTermState *state_ = nullptr;
    VTermScreenCallbacks screenCallbacks_{};

    int rows_ = 31;
    int columns_ = 87;

    std::vector<std::vector<VTermScreenCell>> scrollback_;
    std::size_t scrollOffset_ = 0;

    std::u32string input_;
    std::size_t cursorPosition_ = 0;

    std::vector<std::u32string> history_;
    std::size_t historyPosition_ = 0;
    std::u32string savedHistoryInput_;
};

extern ModInfo gameConsoleInfo;

#include "game_console.hpp"
#include "slugger_manager.hpp"
#include "misc_utils.hpp"
#include "console.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <windows.h>
#include <wrl/client.h>

namespace {

constexpr float kConsoleX = 10.0f;
constexpr float kConsoleY = 10.0f;
constexpr float kMaximumConsoleWidth = 800.0f;
constexpr float kMaximumConsoleHeight = 600.0f;
constexpr float kConsolePadding = 8.0f;

constexpr float kCellWidth = 10.0f;
constexpr float kCellHeight = 18.0f;
constexpr float kFontScale = 14.0f;

constexpr std::chrono::milliseconds kInitialKeyRepeatDelay{400};
constexpr std::chrono::milliseconds kKeyRepeatInterval{35};

constexpr std::array<float, 4> kConsoleBackground{0.0f, 0.0f, 0.0f, 1.0f};
constexpr std::array<float, 4> kDefaultForeground{1.0f, 1.0f, 1.0f, 1.0f};
constexpr std::array<float, 4> kPromptForeground{0.30f, 0.85f, 1.0f, 1.0f};
constexpr std::array<float, 4> kScrollIndicatorForeground{1.0f, 0.80f, 0.20f, 1.0f};

bool colorsEqual(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs) {
    constexpr float epsilon = 1.0f / 1024.0f;

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (std::fabs(lhs[i] - rhs[i]) > epsilon)
            return false;
    }

    return true;
}

bool isPrintableVirtualKey(int virtualKey) {
    if (virtualKey == VK_SPACE || virtualKey == VK_OEM_102)
        return true;

    if (virtualKey >= '0' && virtualKey <= 'Z')
        return true;

    if (virtualKey >= VK_NUMPAD0 && virtualKey <= VK_DIVIDE)
        return true;

    return virtualKey >= VK_OEM_1 && virtualKey <= VK_OEM_8;
}

std::array<float, 4> convertColor(VTermState *state, VTermColor color) {
    vterm_state_convert_color_to_rgb(state, &color);

    return {static_cast<float>(color.rgb.red) / 255.0f, static_cast<float>(color.rgb.green) / 255.0f, static_cast<float>(color.rgb.blue) / 255.0f, 1.0f};
}

std::string cellText(const VTermScreenCell& cell) {
    std::string output;

    for (std::size_t i = 0; i < VTERM_MAX_CHARS_PER_CELL; ++i) {
        if (cell.chars[i] == 0)
            break;
        appendCodepointUtf8(output, static_cast<char32_t>(cell.chars[i]));
    }

    return output;
}

} // namespace

struct GameConsole::RenderSnapshot {
    struct Cell {
        std::string text;
        std::array<float, 4> foreground = kDefaultForeground;
        std::array<float, 4> background = kConsoleBackground;
        bool underline = false;
        bool strike = false;
    };

    struct InputGlyph {
        int column = 0;
        std::string text;
        std::array<float, 4> foreground = kDefaultForeground;
    };

    int rows = 0;
    int columns = 0;
    std::vector<Cell> cells;
    std::vector<InputGlyph> inputGlyphs;

    int cursorColumn = 0;
    int cursorRow = 0;
    bool cursorVisible = true;
};

GameConsole::GameConsole() {
    vterm_ = vterm_new(rows_, columns_);

    if (!vterm_)
        throw std::runtime_error("Failed to create libvterm terminal");

    vterm_set_utf8(vterm_, 1);

    screen_ = vterm_obtain_screen(vterm_);
    state_ = vterm_obtain_state(vterm_);

    if (!screen_ || !state_) {
        vterm_free(vterm_);
        vterm_ = nullptr;
        throw std::runtime_error("Failed to initialize libvterm screen");
    }

    screenCallbacks_.sb_pushline = &GameConsole::scrollbackPushLine;
    screenCallbacks_.sb_popline = &GameConsole::scrollbackPopLine;
    screenCallbacks_.sb_clear = &GameConsole::scrollbackClear;

    vterm_screen_set_callbacks(screen_, &screenCallbacks_, this);

    vterm_screen_enable_altscreen(screen_, 1);
    vterm_screen_reset(screen_, 1);

    historyPosition_ = history_.size();
    resetKeyboardState();

    println("\x1b[36mGame console loaded\x1b[0m");

    DX11Hook::registerMod(this);
}

GameConsole::~GameConsole() {
    DX11Hook::unregisterMod(this);

    std::lock_guard<std::mutex> lock(mutex_);

    if (vterm_) {
        vterm_free(vterm_);
        vterm_ = nullptr;
        screen_ = nullptr;
        state_ = nullptr;
    }
}

void GameConsole::writeTerminalLocked(std::string_view text) {
    if (!vterm_ || text.empty())
        return;

    vterm_input_write(vterm_, text.data(), text.size());
}

void GameConsole::print(std::string_view string) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!vterm_)
        return;

    writeTerminalLocked(string);
}

void GameConsole::println(std::string_view string) { print(std::format("{}\r\n", string)); }

void GameConsole::resetKeyboardState() {
    const auto now = std::chrono::steady_clock::now();

    for (int virtualKey = 0; virtualKey < 256; ++virtualKey) {
        keyDown_[virtualKey] = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;

        nextKeyRepeat_[virtualKey] = now + kInitialKeyRepeatDelay;
    }
}

void GameConsole::insertTextLocked(std::u32string_view text) {
    if (text.empty())
        return;

    cursorPosition_ = std::min(cursorPosition_, input_.size());

    input_.insert(input_.begin() + static_cast<std::ptrdiff_t>(cursorPosition_), text.begin(), text.end());

    cursorPosition_ += text.size();
    historyPosition_ = history_.size();
    savedHistoryInput_.clear();
}

void GameConsole::navigateHistoryLocked(int direction) {
    if (history_.empty())
        return;

    if (direction < 0) {
        if (historyPosition_ == history_.size())
            savedHistoryInput_ = input_;

        if (historyPosition_ > 0)
            --historyPosition_;
    } else if (direction > 0) {
        if (historyPosition_ < history_.size())
            ++historyPosition_;
    }

    if (historyPosition_ < history_.size())
        input_ = history_[historyPosition_];
    else
        input_ = savedHistoryInput_;

    cursorPosition_ = input_.size();
}

void GameConsole::updateKeyboard() {
    const auto now = std::chrono::steady_clock::now();

    std::array<bool, 256> pressed{};
    std::array<bool, 256> repeated{};

    for (int virtualKey = 0; virtualKey < 256; ++virtualKey) {
        const bool down = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;

        pressed[virtualKey] = down && !keyDown_[virtualKey];

        if (down) {
            if (!keyDown_[virtualKey]) {
                nextKeyRepeat_[virtualKey] = now + kInitialKeyRepeatDelay;
            } else if (now >= nextKeyRepeat_[virtualKey]) {
                repeated[virtualKey] = true;

                do {
                    nextKeyRepeat_[virtualKey] += kKeyRepeatInterval;
                } while (nextKeyRepeat_[virtualKey] <= now);
            }
        }

        keyDown_[virtualKey] = down;
    }

    const auto activated = [&](int virtualKey) { return pressed[virtualKey] || repeated[virtualKey]; };

    const bool control = keyDown_[VK_CONTROL] || keyDown_[VK_LCONTROL] || keyDown_[VK_RCONTROL];

    const bool alt = keyDown_[VK_MENU] || keyDown_[VK_LMENU] || keyDown_[VK_RMENU];

    bool requestTab = pressed[VK_TAB];
    bool requestSubmit = pressed[VK_RETURN];

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (activated(VK_PRIOR)) {
            const std::size_t pageAmount = static_cast<std::size_t>(std::max(1, rows_ - 1));
            scrollOffset_ = std::min(scrollback_.size(), scrollOffset_ + pageAmount);
        }

        if (activated(VK_NEXT)) {
            const std::size_t pageAmount = static_cast<std::size_t>(std::max(1, rows_ - 1));

            if (scrollOffset_ > pageAmount)
                scrollOffset_ -= pageAmount;
            else
                scrollOffset_ = 0;
        }

        if (activated(VK_LEFT)) {
            if (control) {
                while (cursorPosition_ > 0 && !isWordCharacter(input_[cursorPosition_ - 1])) {
                    --cursorPosition_;
                }

                while (cursorPosition_ > 0 && isWordCharacter(input_[cursorPosition_ - 1])) {
                    --cursorPosition_;
                }
            } else if (cursorPosition_ > 0) {
                --cursorPosition_;
            }
        }

        if (activated(VK_RIGHT)) {
            if (control) {
                while (cursorPosition_ < input_.size() && isWordCharacter(input_[cursorPosition_]))
                    ++cursorPosition_;

                while (cursorPosition_ < input_.size() && !isWordCharacter(input_[cursorPosition_]))
                    ++cursorPosition_;
            } else if (cursorPosition_ < input_.size()) {
                ++cursorPosition_;
            }
        }

        if (activated(VK_HOME))
            cursorPosition_ = 0;

        if (activated(VK_END))
            cursorPosition_ = input_.size();

        if (activated(VK_BACK) && cursorPosition_ > 0) {
            input_.erase(input_.begin() + static_cast<std::ptrdiff_t>(cursorPosition_ - 1));

            --cursorPosition_;
            historyPosition_ = history_.size();
        }

        if (activated(VK_DELETE) && cursorPosition_ < input_.size()) {
            input_.erase(input_.begin() + static_cast<std::ptrdiff_t>(cursorPosition_));
            historyPosition_ = history_.size();
        }

        if (activated(VK_UP))
            navigateHistoryLocked(-1);

        if (activated(VK_DOWN))
            navigateHistoryLocked(1);

        if (control && pressed['A'])
            cursorPosition_ = 0;

        if (control && pressed['E'])
            cursorPosition_ = input_.size();

        if (control && pressed['U']) {
            input_.erase(input_.begin(), input_.begin() + static_cast<std::ptrdiff_t>(cursorPosition_));

            cursorPosition_ = 0;
            historyPosition_ = history_.size();
        }

        if (control && pressed['K']) {
            input_.erase(input_.begin() + static_cast<std::ptrdiff_t>(cursorPosition_), input_.end());

            historyPosition_ = history_.size();
        }

        if (control && pressed['W'] && cursorPosition_ > 0) {
            const std::size_t end = cursorPosition_;

            while (cursorPosition_ > 0 && !isWordCharacter(input_[cursorPosition_ - 1])) {
                --cursorPosition_;
            }

            while (cursorPosition_ > 0 && isWordCharacter(input_[cursorPosition_ - 1])) {
                --cursorPosition_;
            }

            input_.erase(input_.begin() + static_cast<std::ptrdiff_t>(cursorPosition_), input_.begin() + static_cast<std::ptrdiff_t>(end));

            historyPosition_ = history_.size();
        }
    }

    // Ctrl+Alt can represent AltGr, so allow ToUnicodeEx in that case.
    const bool suppressPrintableInput = control && !keyDown_[VK_RMENU];

    if (!suppressPrintableInput && !alt) {
        BYTE keyboardState[256]{};

        if (GetKeyboardState(keyboardState)) {
            const HKL keyboardLayout = GetKeyboardLayout(0);

            for (int virtualKey = 0; virtualKey < 256; ++virtualKey) {
                if (!activated(virtualKey) || !isPrintableVirtualKey(virtualKey) || virtualKey == VK_OEM_3) {
                    continue;
                }

                WCHAR characters[8]{};

                const UINT scanCode = MapVirtualKeyExW(static_cast<UINT>(virtualKey), MAPVK_VK_TO_VSC, keyboardLayout);

                const int result =
                    ToUnicodeEx(static_cast<UINT>(virtualKey), scanCode, keyboardState, characters, static_cast<int>(std::size(characters)), 0, keyboardLayout);

                if (result <= 0)
                    continue;

                const std::u32string decoded = utf16ToUtf32(std::wstring_view(characters, static_cast<std::size_t>(result)));

                std::lock_guard<std::mutex> lock(mutex_);
                insertTextLocked(decoded);
            }
        }
    }

    if (requestTab) {
        std::string completionInput;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            completionInput = utf32ToUtf8(input_);
        }

        try {
            console::onTab(completionInput);

            const std::u32string completed = utf8ToUtf32(completionInput);

            std::lock_guard<std::mutex> lock(mutex_);
            input_ = completed;
            cursorPosition_ = input_.size();
            historyPosition_ = history_.size();
            savedHistoryInput_.clear();
        } catch (const std::exception& exception) {
            println(std::string("\x1b[31mTab completion error: ") + exception.what() + "\x1b[0m");
        } catch (...) {
            println("\x1b[31mTab completion failed with an unknown error\x1b[0m");
        }
    }

    if (requestSubmit) {
        std::string submittedInput;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            submittedInput = utf32ToUtf8(input_);

            writeTerminalLocked("\x1b[36m> \x1b[0m");
            writeTerminalLocked(submittedInput);
            writeTerminalLocked("\r\n");

            if (!input_.empty()) {
                if (history_.empty() || history_.back() != input_) {
                    history_.push_back(input_);

                    if (history_.size() > kMaximumHistoryEntries) {
                        history_.erase(history_.begin());
                    }
                }
            }

            input_.clear();
            cursorPosition_ = 0;
            historyPosition_ = history_.size();
            savedHistoryInput_.clear();
            scrollOffset_ = 0;
        }

        try {
            console::onInput(submittedInput);
        } catch (const std::exception& exception) {
            println(std::string("\x1b[31mCommand error: ") + exception.what() + "\x1b[0m");
        } catch (...) {
            println("\x1b[31mCommand failed with an unknown error\x1b[0m");
        }
    }
}

void GameConsole::uiUpdate() {
    const bool tildePressed = (GetAsyncKeyState(VK_OEM_3) & 0x8000) != 0;

    if (tildePressed && !tildeWasPressed_) {
        const bool opening = !isOpen_.load(std::memory_order_relaxed);

        isOpen_.store(opening, std::memory_order_release);
        resetKeyboardState();
    }

    tildeWasPressed_ = tildePressed;

    if (!isOpen_.load(std::memory_order_acquire))
        return;

    updateKeyboard();

    auto slugger = sluggerManagerInfo.get<SluggerManager>();

    if (!slugger)
        return;

    RenderSnapshot snapshot;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot = buildRenderSnapshotLocked();
    }

    Slugger::SlugTextBuildOptions options;
    options.scale = kFontScale;
    options.centerOrigin = false;

    // Render cells
    for (int row = 0; row < snapshot.rows; ++row) {
        for (int column = 0; column < snapshot.columns; ++column) {
            const auto& cell = snapshot.cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(snapshot.columns) + static_cast<std::size_t>(column)];

            if (cell.text.empty())
                continue;

            options.minYColor = {cell.foreground[0], cell.foreground[1], cell.foreground[2], cell.foreground[3]};
            options.maxYColor = options.minYColor;

            const float x = kConsoleX + kConsolePadding + static_cast<float>(column) * kCellWidth;
            const float y = kConsoleY + kConsolePadding + static_cast<float>(row) * kCellHeight;

            slugger->drawScreenText(cell.text, x, y, options, false, false, FontType::Monospace);
        }
    }

    // Render input line glyphs
    for (const auto& glyph : snapshot.inputGlyphs) {
        if (glyph.text.empty())
            continue;

        options.minYColor = {glyph.foreground[0], glyph.foreground[1], glyph.foreground[2], glyph.foreground[3]};
        options.maxYColor = options.minYColor;

        const float x = kConsoleX + kConsolePadding + static_cast<float>(glyph.column) * kCellWidth;
        const float y = kConsoleY + kConsolePadding + static_cast<float>(snapshot.rows) * kCellHeight;

        slugger->drawScreenText(glyph.text, x, y, options, false, false, FontType::Monospace);
    }
}

void GameConsole::resizeTerminalLocked(int rows, int columns) {
    rows = std::max(rows, 2);
    columns = std::max(columns, 10);

    if (!vterm_ || (rows == rows_ && columns == columns_))
        return;

    vterm_set_size(vterm_, rows, columns);

    rows_ = rows;
    columns_ = columns;
    scrollOffset_ = std::min(scrollOffset_, scrollback_.size());
}

GameConsole::RenderSnapshot GameConsole::buildRenderSnapshotLocked() const {
    RenderSnapshot snapshot;

    snapshot.rows = rows_;
    snapshot.columns = columns_;
    snapshot.cursorRow = rows_;
    snapshot.cursorVisible = true;

    const std::size_t cellCount = static_cast<std::size_t>(rows_) * static_cast<std::size_t>(columns_);

    snapshot.cells.resize(cellCount);

    const std::size_t historySize = scrollback_.size();
    const std::size_t scrollOffset = std::min(scrollOffset_, historySize);

    const std::size_t totalRows = historySize + static_cast<std::size_t>(rows_);
    const std::size_t firstVisibleRow = totalRows - static_cast<std::size_t>(rows_) - scrollOffset;

    for (int outputRow = 0; outputRow < rows_; ++outputRow) {
        const std::size_t sourceRow = firstVisibleRow + static_cast<std::size_t>(outputRow);

        for (int column = 0; column < columns_; ++column) {
            VTermScreenCell sourceCell{};
            bool hasCell = false;

            if (sourceRow < historySize) {
                const auto& historyLine = scrollback_[sourceRow];

                if (static_cast<std::size_t>(column) < historyLine.size()) {
                    sourceCell = historyLine[static_cast<std::size_t>(column)];
                    hasCell = true;
                }
            } else {
                const std::size_t screenRow = sourceRow - historySize;

                if (screenRow < static_cast<std::size_t>(rows_)) {
                    const VTermPos position{static_cast<int>(screenRow), column};
                    hasCell = vterm_screen_get_cell(screen_, position, &sourceCell) != 0;
                }
            }

            auto& destination = snapshot.cells[static_cast<std::size_t>(outputRow) * static_cast<std::size_t>(columns_) + static_cast<std::size_t>(column)];

            if (!hasCell)
                continue;

            VTermColor foreground = sourceCell.fg;
            VTermColor background = sourceCell.bg;

            if (sourceCell.attrs.reverse)
                std::swap(foreground, background);

            destination.foreground = convertColor(state_, foreground);
            destination.background = convertColor(state_, background);
            destination.underline = sourceCell.attrs.underline != 0;
            destination.strike = sourceCell.attrs.strike != 0;

            if (sourceCell.width != 0)
                destination.text = cellText(sourceCell);
        }
    }

    snapshot.inputGlyphs.push_back({0, ">", kPromptForeground});

    constexpr int promptWidth = 2;
    const int availableInputColumns = std::max(1, columns_ - promptWidth);

    const std::size_t cursorCell = displayWidth(input_, cursorPosition_);
    std::size_t firstVisibleCell = 0;

    if (cursorCell >= static_cast<std::size_t>(availableInputColumns))
        firstVisibleCell = cursorCell - static_cast<std::size_t>(availableInputColumns) + 1;

    std::size_t logicalCell = 0;

    for (const char32_t codepoint : input_) {
        const int width = std::max(codepointWidth(codepoint), 0);
        const std::size_t nextCell = logicalCell + static_cast<std::size_t>(width);

        if (nextCell > firstVisibleCell && logicalCell < firstVisibleCell + static_cast<std::size_t>(availableInputColumns)) {
            const int displayColumn = promptWidth + static_cast<int>(logicalCell >= firstVisibleCell ? logicalCell - firstVisibleCell : 0);

            if (displayColumn < columns_) {
                std::string text;
                appendCodepointUtf8(text, codepoint);

                snapshot.inputGlyphs.push_back({displayColumn, std::move(text), kDefaultForeground});
            }
        }

        logicalCell = nextCell;
    }

    snapshot.cursorColumn = promptWidth + static_cast<int>(cursorCell >= firstVisibleCell ? cursorCell - firstVisibleCell : 0);
    snapshot.cursorColumn = std::clamp(snapshot.cursorColumn, 0, std::max(0, columns_ - 1));

    if (scrollOffset_ > 0) {
        const std::string indicator = "[SCROLL: " + std::to_string(scrollOffset_) + "]";
        const int indicatorColumn = std::max(promptWidth, columns_ - static_cast<int>(indicator.size()));
        snapshot.inputGlyphs.push_back({indicatorColumn, indicator, kScrollIndicatorForeground});
    }

    return snapshot;
}

void GameConsole::onDX11Present(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) {
    (void)syncInterval;
    (void)flags;

    if (!isOpen_.load(std::memory_order_acquire) || !swapChain)
        return;

    Microsoft::WRL::ComPtr<ID3D11Device> device;

    if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&device))))
        return;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(&context);

    if (!context)
        return;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context1;

    if (FAILED(context.As(&context1)))
        return;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
        return;

    D3D11_TEXTURE2D_DESC backBufferDescription{};
    backBuffer->GetDesc(&backBufferDescription);

    if (backBufferDescription.Width <= 20 || backBufferDescription.Height <= 20)
        return;

    const float availableWidth = std::min(kMaximumConsoleWidth, static_cast<float>(backBufferDescription.Width) - 2.0f * kConsoleX);
    const float availableHeight = std::min(kMaximumConsoleHeight, static_cast<float>(backBufferDescription.Height) - 2.0f * kConsoleY);

    const int columns = std::max(10, static_cast<int>((availableWidth - 2.0f * kConsolePadding) / kCellWidth));
    const int totalRows = std::max(3, static_cast<int>((availableHeight - 2.0f * kConsolePadding) / kCellHeight));
    const int terminalRows = totalRows - 1;

    RenderSnapshot snapshot;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        resizeTerminalLocked(terminalRows, columns);
        snapshot = buildRenderSnapshotLocked();
    }

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

    if (FAILED(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView)))
        return;

    const LONG consoleLeft = static_cast<LONG>(kConsoleX);
    const LONG consoleTop = static_cast<LONG>(kConsoleY);
    const LONG consoleRight = std::min<LONG>(static_cast<LONG>(backBufferDescription.Width),
                                             static_cast<LONG>(kConsoleX + 2.0f * kConsolePadding + static_cast<float>(snapshot.columns) * kCellWidth));
    const LONG consoleBottom = std::min<LONG>(static_cast<LONG>(backBufferDescription.Height),
                                              static_cast<LONG>(kConsoleY + 2.0f * kConsolePadding + static_cast<float>(snapshot.rows + 1) * kCellHeight));

    if (consoleRight <= consoleLeft || consoleBottom <= consoleTop)
        return;

    const D3D11_RECT consoleRectangle{consoleLeft, consoleTop, consoleRight, consoleBottom};

    context1->ClearView(renderTargetView.Get(), kConsoleBackground.data(), &consoleRectangle, 1);

    for (int row = 0; row < snapshot.rows; ++row) {
        int column = 0;

        while (column < snapshot.columns) {
            const auto& firstCell =
                snapshot.cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(snapshot.columns) + static_cast<std::size_t>(column)];

            if (colorsEqual(firstCell.background, kConsoleBackground)) {
                ++column;
                continue;
            }

            const int runStart = column;
            const auto runColor = firstCell.background;

            while (++column < snapshot.columns) {
                const auto& nextCell =
                    snapshot.cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(snapshot.columns) + static_cast<std::size_t>(column)];

                if (!colorsEqual(nextCell.background, runColor))
                    break;
            }

            const D3D11_RECT rectangle{static_cast<LONG>(kConsoleX + kConsolePadding + static_cast<float>(runStart) * kCellWidth),
                                       static_cast<LONG>(kConsoleY + kConsolePadding + static_cast<float>(row) * kCellHeight),
                                       static_cast<LONG>(kConsoleX + kConsolePadding + static_cast<float>(column) * kCellWidth),
                                       static_cast<LONG>(kConsoleY + kConsolePadding + static_cast<float>(row + 1) * kCellHeight)};

            context1->ClearView(renderTargetView.Get(), runColor.data(), &rectangle, 1);
        }
    }

    for (int row = 0; row < snapshot.rows; ++row) {
        for (int column = 0; column < snapshot.columns; ++column) {
            const auto& cell = snapshot.cells[static_cast<std::size_t>(row) * static_cast<std::size_t>(snapshot.columns) + static_cast<std::size_t>(column)];

            if (!cell.underline && !cell.strike)
                continue;

            const LONG left = static_cast<LONG>(kConsoleX + kConsolePadding + static_cast<float>(column) * kCellWidth);
            const LONG right = static_cast<LONG>(kConsoleX + kConsolePadding + static_cast<float>(column + 1) * kCellWidth);

            if (cell.underline) {
                const LONG bottom = static_cast<LONG>(kConsoleY + kConsolePadding + static_cast<float>(row + 1) * kCellHeight - 2.0f);
                const D3D11_RECT underline{left, bottom - 1, right, bottom};
                context1->ClearView(renderTargetView.Get(), cell.foreground.data(), &underline, 1);
            }

            if (cell.strike) {
                const LONG middle = static_cast<LONG>(kConsoleY + kConsolePadding + static_cast<float>(row) * kCellHeight + kCellHeight * 0.55f);
                const D3D11_RECT strike{left, middle, right, middle + 1};
                context1->ClearView(renderTargetView.Get(), cell.foreground.data(), &strike, 1);
            }
        }
    }

    if (snapshot.cursorVisible) {
        const LONG cursorLeft = static_cast<LONG>(kConsoleX + kConsolePadding + static_cast<float>(snapshot.cursorColumn) * kCellWidth);
        const LONG cursorRight = static_cast<LONG>(kConsoleX + kConsolePadding + static_cast<float>(snapshot.cursorColumn + 1) * kCellWidth);
        const LONG cursorBottom = static_cast<LONG>(kConsoleY + kConsolePadding + static_cast<float>(snapshot.cursorRow + 1) * kCellHeight - 2.0f);

        const D3D11_RECT cursorRectangle{cursorLeft, cursorBottom - 2, cursorRight, cursorBottom};

        context1->ClearView(renderTargetView.Get(), kDefaultForeground.data(), &cursorRectangle, 1);
    }
}

int GameConsole::scrollbackPushLine(int columns, const VTermScreenCell *cells, void *user) {
    auto *self = static_cast<GameConsole *>(user);

    if (!self || !cells || columns <= 0)
        return 0;

    self->scrollback_.emplace_back(cells, cells + columns);

    if (self->scrollback_.size() > kMaximumScrollbackLines) {
        self->scrollback_.erase(self->scrollback_.begin());

        if (self->scrollOffset_ > 0)
            --self->scrollOffset_;
    } else if (self->scrollOffset_ > 0) {
        ++self->scrollOffset_;
    }

    self->scrollOffset_ = std::min(self->scrollOffset_, self->scrollback_.size());

    return 1;
}

int GameConsole::scrollbackPopLine(int columns, VTermScreenCell *cells, void *user) {
    auto *self = static_cast<GameConsole *>(user);

    if (!self || !cells || columns <= 0)
        return 0;

    if (self->scrollback_.empty())
        return 0;

    VTermColor defaultForeground{};
    VTermColor defaultBackground{};

    vterm_state_get_default_colors(self->state_, &defaultForeground, &defaultBackground);

    for (int column = 0; column < columns; ++column) {
        cells[column] = {};
        cells[column].width = 1;
        cells[column].fg = defaultForeground;
        cells[column].bg = defaultBackground;
    }

    const auto& source = self->scrollback_.back();

    const int copyCount = std::min(columns, static_cast<int>(source.size()));
    std::copy_n(source.begin(), copyCount, cells);

    self->scrollback_.pop_back();
    self->scrollOffset_ = std::min(self->scrollOffset_, self->scrollback_.size());

    return 1;
}

int GameConsole::scrollbackClear(void *user) {
    auto *self = static_cast<GameConsole *>(user);

    if (!self)
        return 0;

    self->scrollback_.clear();
    self->scrollOffset_ = 0;
    return 1;
}

ModInfo gameConsoleInfo{"Game Console", true, []() { return std::make_unique<GameConsole>(); }, []() { gameConsoleInfo.load(); }};

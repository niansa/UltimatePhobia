#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <cwchar>
#include <cfloat>
#include <windows.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <winuser.h>
#include <Slugger/slugger.hpp>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

using namespace Slugger;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

static std::vector<uint8_t> LoadFileBytes(const wchar_t *path) {
    std::ifstream file(std::filesystem::path(path), std::ios::binary | std::ios::ate);
    if (!file)
        throw std::runtime_error("Failed to open TTF file.");

    const auto size = file.tellg();
    if (size <= 0)
        throw std::runtime_error("TTF file is empty.");

    std::vector<uint8_t> data(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(data.data()), size);

    if (!file)
        throw std::runtime_error("Failed to read TTF file.");

    return data;
}

static HWND FindNotepadTxtWindow() {
    struct SearchData {
        const wchar_t *needle;
        HWND result = nullptr;
    } data{L" - Notepad", nullptr};

    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            auto *d = reinterpret_cast<SearchData *>(lParam);
            if (!IsWindowVisible(hwnd))
                return TRUE;

            wchar_t title[512]{};
            GetWindowTextW(hwnd, title, 512);

            if (title[0] != L'\0' && wcsstr(title, d->needle)) {
                d->result = hwnd;
                return FALSE;
            }

            return TRUE;
        },
        reinterpret_cast<LPARAM>(&data));

    // Fallback: any classic Notepad window
    if (!data.result)
        data.result = FindWindowW(L"Notepad", nullptr);

    return data.result;
}

int main() {
    try {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        HWND target = FindNotepadTxtWindow();
        if (!target) {
            MessageBoxW(nullptr,
                        L"Could not find a Notepad window with title containing ' - Notepad'.\n"
                        L"Open Notepad first.",
                        L"Overlay sample", MB_ICONWARNING);
            return 0;
        }

        RECT rc{};
        GetClientRect(target, &rc);
        POINT p0{rc.left, rc.top};
        POINT p1{rc.right, rc.bottom};
        ClientToScreen(target, &p0);
        ClientToScreen(target, &p1);

        const int width = std::max(1L, p1.x - p0.x);
        const int height = std::max(1L, p1.y - p0.y);

        auto gfx = SlugDeviceResources::Create(
#ifdef _DEBUG
            true
#else
            false
#endif
        );

        OverlayWindow overlay(width, height, target);
        overlay.SetupSwapchain(gfx.factory, gfx.device);

        auto ttf = LoadFileBytes(L"C:\\Windows\\Fonts\\comic.ttf");

        const std::string text = "ROTATING 3D TEXT";
        SlugFontBuilder fontBuilder(ttf.data());
        fontBuilder.BakeGlyphsFromString(text);

        std::unique_ptr<SlugFont> font = fontBuilder.CreateFont(gfx.device);
        SlugRenderer renderer(gfx.device, gfx.context);

        SlugTextBuildOptions textOpts;
        textOpts.minYColor = {1.00f, 0.85f, 0.20f, 1.0f};
        textOpts.maxYColor = {0.25f, 0.85f, 1.00f, 1.0f};
        textOpts.centerOrigin = true;

        std::vector<SlugVertex> textVertices = fontBuilder.BuildTextVertices(text, textOpts);

        auto t0 = std::chrono::high_resolution_clock::now();

        bool running = true;
        while (running) {
            MSG msg{};
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT)
                    running = false;

                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }

            if (!running || !IsWindow(target))
                break;

            if (!overlay.SyncAndResizeToTarget(target, gfx.device, gfx.context, true)) {
                Sleep(16);
                continue;
            }

            constexpr float clearColor[4] = {0.f, 0.f, 0.f, 0.f};
            overlay.Bind(gfx.context);
            overlay.Clear(gfx.context, clearColor);

            const auto now = std::chrono::high_resolution_clock::now();
            const float t = std::chrono::duration<float>(now - t0).count();

            const float aspect = static_cast<float>(overlay.GetWidth()) / static_cast<float>(overlay.GetHeight());

            XMMATRIX world = XMMatrixScaling(0.18f, 0.18f, 0.18f) * XMMatrixRotationX(0.35f) * XMMatrixRotationY(t) * XMMatrixRotationZ(0.20f * sinf(t * 1.7f));
            XMMATRIX view = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -5500.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), aspect, 0.1f, 20000.0f);
            XMMATRIX mvp = world * view * proj;

            renderer.BeginRender(*font, overlay.GetWidth(), overlay.GetHeight(), mvp);
            renderer.DrawVertices(textVertices);

            overlay.Present(1, 0);
        }

        return 0;
    } catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Overlay sample error", MB_ICONERROR);
        return -1;
    }
}

#include "dx11_hook.hpp"
#include "game_hook.hpp"

#include <algorithm>
#include <optional>
#include <windows.h>

// Define function pointers for DX11 functions
typedef HRESULT(__stdcall *Present_t)(IDXGISwapChain *, UINT, UINT);
typedef HRESULT(__stdcall *ResizeBuffers_t)(IDXGISwapChain *, UINT, UINT, UINT, DXGI_FORMAT, UINT);

namespace {
std::vector<Mod *> s_mods;
std::optional<GameHook> s_presentHook;
std::optional<GameHook> s_resizeBuffersHook;

HRESULT __stdcall PresentFnc(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) {
    // Notify all registered mods
    for (auto mod : s_mods) {
        if (mod)
            mod->onDX11Present(pSwapChain, SyncInterval, Flags);
    }

    // Call original using your GameHook trampoline
    GameHookRelease GHR(*s_presentHook);
    return s_presentHook->getFunction<std::remove_pointer_t<Present_t>>()(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall ResizeBuffersFnc(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    for (auto mod : s_mods) {
        if (mod)
            mod->onDX11ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    GameHookRelease GHR(*s_resizeBuffersHook);
    return s_resizeBuffersHook->getFunction<std::remove_pointer_t<ResizeBuffers_t>>()(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}
} // namespace

void DX11Hook::init() {
    g.logger->info("Initializing DX11 Hooks...");

    // Create dummy window for device creation
    HWND hwnd = CreateWindowA("STATIC", "Dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
    if (!hwnd) {
        g.logger->error("Failed to create dummy window for DX11 hook.");
        return;
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain *swapChain = nullptr;
    ID3D11Device *device = nullptr;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11DeviceContext *context = nullptr;

    // Create dummy device and swapchain to grab vtable
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &swapChain, &device,
                                               &featureLevel, &context);

    if (SUCCEEDED(hr)) {
        void **vtable = *reinterpret_cast<void ***>(swapChain);
        void *presentFunc = vtable[8];
        void *resizeFunc = vtable[13];

        g.logger->info("Found DX11 Present at {}, ResizeBuffers at {}", presentFunc, resizeFunc);

        GameHook::safeCreate(s_presentHook, presentFunc, reinterpret_cast<void *>(PresentFnc), false);
        GameHook::safeCreate(s_resizeBuffersHook, resizeFunc, reinterpret_cast<void *>(ResizeBuffersFnc), false);

        swapChain->Release();
        device->Release();
        context->Release();
    } else {
        g.logger->error("D3D11CreateDeviceAndSwapChain failed! HRESULT {}", hr);
    }

    DestroyWindow(hwnd);
}

void DX11Hook::deinit() {
    g.logger->info("Deinitializing DX11 Hooks safely...");

    s_presentHook.reset();
    s_resizeBuffersHook.reset();
}

void DX11Hook::registerMod(Mod *mod) {
    if (std::find(s_mods.begin(), s_mods.end(), mod) == s_mods.end())
        s_mods.push_back(mod);
}

void DX11Hook::unregisterMod(Mod *mod) { s_mods.erase(std::remove(s_mods.begin(), s_mods.end(), mod), s_mods.end()); }

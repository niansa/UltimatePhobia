#include "imgui_man.hpp"
#include "global_state.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ImGuiMan {
namespace {
constexpr ImVec4 clear_color = ImVec4(0.1372549019607843f, 0.1215686274509804f, 0.1254901960784314f, 1.00f); // #231f20

ID3D11Device *pd3dDevice = nullptr;
ID3D11DeviceContext *pd3dDeviceContext = nullptr;
IDXGISwapChain *pSwapChain = nullptr;
bool swapChainOccluded = false;
UINT resizeWidth = 0, resizeHeight = 0;
ID3D11RenderTargetView *mainRenderTargetView = nullptr;

WNDCLASSEXW wc;
HWND hwnd;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
} // namespace

void init() {
    g.logger->info("Initializing GUI...");

    // Create application window
    // ImGui_ImplWin32_EnableDpiAwareness();
    wc = {sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr};
    ::RegisterClassExW(&wc);
    hwnd = ::CreateWindowW(wc.lpszClassName, L"UltimatePhasmo", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        g.logger->error("Failed to create D3D11 device");
        return;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);
}
void deinit() {
    g.logger->info("Deinitializing GUI...");

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool pre_update() {
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32
    // backend.
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    // Handle window being minimized or screen locked
    if (swapChainOccluded && pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
        return false;
    }
    swapChainOccluded = false;

    // Handle window resize (we don't resize directly in the WM_SIZE handler)
    if (resizeWidth != 0 && resizeHeight != 0) {
        CleanupRenderTarget();
        pSwapChain->ResizeBuffers(0, resizeWidth, resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        resizeWidth = resizeHeight = 0;
        CreateRenderTarget();
    }

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    return true;
}
void post_update() {
    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
    pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
    pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present
    HRESULT hr = pSwapChain->Present(0, 0); // Present without vsync
    swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

namespace {
bool CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
                                                &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software
                                       // driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
                                            &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (pSwapChain) {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }
    if (pd3dDeviceContext) {
        pd3dDeviceContext->Release();
        pd3dDeviceContext = nullptr;
    }
    if (pd3dDevice) {
        pd3dDevice->Release();
        pd3dDevice = nullptr;
    }
}

void CreateRenderTarget() {
    ID3D11Texture2D *pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (mainRenderTargetView) {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        resizeWidth = (UINT)LOWORD(lParam); // Queue resize
        resizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
} // namespace
} // namespace ImGuiMan

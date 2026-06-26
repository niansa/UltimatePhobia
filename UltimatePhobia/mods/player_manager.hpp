#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "dx11_hook.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <Slugger/slugger.hpp>

struct Player_o;

struct PlayerTextData {
    std::string text;

    float screenX = 0.0f; // left -> right
    float screenY = 0.0f; // top -> bottom

    // Camera-space forward depth
    float depth = -1.0f;
};

class PlayerManager final : public Mod {
public:
    std::map<Player_o *, PlayerTextData> trackedPlayers;
    mutable std::mutex m_drawMutex;

    GameHook player$$UpdateHook;
    GameHook player$$OnDisableHook;

    PlayerManager();
    ~PlayerManager() override;

    Player_o *getLocalPlayer() const;

    void onDX11Present(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) override;
    void onDX11ResizeBuffers(IDXGISwapChain *swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) override;

private:
    void InitializeSlugger(IDXGISwapChain *swapChain);
    void RefreshBackbufferDimensions(IDXGISwapChain *swapChain);
    void EnsureRenderTarget(IDXGISwapChain *swapChain);

    bool m_sluggerInitialized = false;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_mainRTV;

    std::vector<uint8_t> m_fontData;
    std::unique_ptr<Slugger::SlugFontBuilder> m_fontBuilder;
    std::unique_ptr<Slugger::SlugFont> m_font;
    std::unique_ptr<Slugger::SlugRenderer> m_renderer;

    int m_backbufferWidth = 0;
    int m_backbufferHeight = 0;
};

extern ModInfo playerManagerInfo;

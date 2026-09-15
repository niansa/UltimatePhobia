#pragma once

#include "mods/base.hpp"
#include "dx11_hook.hpp"
#include "bindings/unityengine.hpp"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <d3d11_1.h>
#include <wrl/client.h>

#include <Slugger/slugger.hpp>

enum class FontType { Proportional, Monospace };

struct TextRequest2D {
    std::string text;
    float x = 0.0f;
    float y = 0.0f;
    Slugger::SlugTextBuildOptions options{};
    bool centerHorizontal = false;
    bool centerVertical = false;
    FontType fontType = FontType::Proportional;
};

struct BillboardRenderRequest {
    std::string text;
    float screenX = 0.0f;
    float screenY = 0.0f;
    float depth = 0.0f;
    float baseScale = 1.0f;
    float minScale = 0.0f;
    float maxScale = 1.0f;
    Slugger::SlugTextBuildOptions options{};
    FontType fontType = FontType::Proportional;
};

struct WorldRenderRequest {
    std::string text;
    UnityEngine_Vector3_o position{};
    UnityEngine_Quaternion_o rotation{};
    float scale = 1.0f;
    Slugger::SlugTextBuildOptions options{};
    bool centerHorizontal = true;
    bool centerVertical = true;
    FontType fontType = FontType::Proportional;
};

class SluggerManager final : public Mod {
public:
    SluggerManager();
    ~SluggerManager() override;

    // Draw text directly in Unity screen coordinates
    void drawScreenText(const std::string& text, float x, float y, const Slugger::SlugTextBuildOptions& options = {}, bool centerHorizontal = false,
                        bool centerVertical = false, FontType fontType = FontType::Proportional);

    // Draw screen-facing text anchored at a 3D world position
    void drawBillboardText(const std::string& text, UnityEngine_Vector3_o worldPos, float baseScale, float minScale, float maxScale,
                           const Slugger::SlugTextBuildOptions& options = {}, FontType fontType = FontType::Proportional);

    // Draw text on a plane in the 3D world
    void drawWorldText(const std::string& text, UnityEngine_Vector3_o position, UnityEngine_Quaternion_o rotation, float scale,
                       const Slugger::SlugTextBuildOptions& options = {}, bool centerHorizontal = true, bool centerVertical = true,
                       FontType fontType = FontType::Proportional);

    void onDX11Present(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) override;

    void onDX11ResizeBuffers(IDXGISwapChain *swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) override;

private:
    void InitializeSlugger(IDXGISwapChain *swapChain);
    void RefreshBackbufferDimensions(IDXGISwapChain *swapChain);
    void EnsureRenderTarget(IDXGISwapChain *swapChain);

    bool initializationAttempted_ = false;
    bool sluggerInitialized_ = false;

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11Device1> device1_;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context1_;

    Microsoft::WRL::ComPtr<ID3DDeviceContextState> sluggerContextState_;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mainRTV_;

private:
    struct FontSlot {
        std::vector<std::uint8_t> fontData;
        std::unique_ptr<Slugger::SlugFontBuilder> builder;
        std::unique_ptr<Slugger::SlugFont> font;
    };

    FontSlot proportionalFont_;
    FontSlot monospaceFont_;
    FontSlot *GetFontSlot(FontType type) { return (type == FontType::Monospace && monospaceFont_.font) ? &monospaceFont_ : &proportionalFont_; }

    std::unique_ptr<Slugger::SlugRenderer> renderer_;

    int backbufferWidth_ = 0;
    int backbufferHeight_ = 0;

    std::mutex queueMutex_;
    std::vector<TextRequest2D> _2dQueue_;
    std::vector<BillboardRenderRequest> billboardQueue_;
    std::vector<WorldRenderRequest> worldQueue_;
};

extern ModInfo sluggerManagerInfo;

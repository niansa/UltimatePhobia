#pragma once

#include <string_view>
#include <span>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <dxgi1_2.h>
#include <windows.h>
#include <dwmapi.h>
#include <dcomp.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "../../src/stb_truetype.h"

namespace Slugger {

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct SlugVertex {
    XMFLOAT4 pos;
    XMFLOAT4 tex;
    XMFLOAT4 jac;
    XMFLOAT4 bnd;
    XMFLOAT4 col;
};

struct alignas(16) SlugConstantBuffer {
    XMFLOAT4 slug_matrix[4];
    XMFLOAT4 slug_viewport;
};

struct SlugDeviceResources {
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGIFactory2> factory;

    [[nodiscard]]
    static SlugDeviceResources Create(bool enableDebugLayer = false);
};

struct SlugTextBuildOptions {
    XMFLOAT4 minYColor{1.0f, 1.0f, 1.0f, 1.0f};
    XMFLOAT4 maxYColor{1.0f, 1.0f, 1.0f, 1.0f};
    bool centerOrigin = true;
    float scale = 1.0f;
};

class SlugFont {
public:
    SlugFont(ComPtr<ID3D11Device> device, std::span<const uint16_t> curveDataRGBA16F, int curveWidth, int curveHeight, std::span<const uint16_t> bandDataRG16U,
             int bandWidth, int bandHeight);

    // Prevent copying
    SlugFont(const SlugFont&) = delete;
    SlugFont& operator=(const SlugFont&) = delete;

    [[nodiscard]]
    ID3D11ShaderResourceView *GetCurveSRV() const noexcept {
        return m_curveSRV.Get();
    }
    [[nodiscard]]
    ID3D11ShaderResourceView *GetBandSRV() const noexcept {
        return m_bandSRV.Get();
    }

private:
    ComPtr<ID3D11ShaderResourceView> m_curveSRV;
    ComPtr<ID3D11ShaderResourceView> m_bandSRV;
};

struct SlugGlyphInfo {
    int codepoint;
    float advanceWidth;
    float leftSideBearing;
    float bboxMinX, bboxMinY;
    float bboxMaxX, bboxMaxY;
    uint16_t glyphLocX;
    uint16_t glyphLocY;
    uint16_t numBands;

    // Helper to extract banding boundaries and packed shader coordinates
    void GetBandingAndTexData(XMFLOAT4& outBnd, float& outTexZ, float& outTexW) const noexcept {
        float width = std::max(bboxMaxX - bboxMinX, 1.0f);
        float height = std::max(bboxMaxY - bboxMinY, 1.0f);

        outBnd.x = static_cast<float>(numBands) / width;
        outBnd.y = static_cast<float>(numBands) / height;
        outBnd.z = -bboxMinX * outBnd.x;
        outBnd.w = -bboxMinY * outBnd.y;

        uint32_t z_uint = (glyphLocX & 0xFFFF) | ((glyphLocY & 0xFFFF) << 16);
        uint32_t flags = 0; // 0x10 for Even-Odd
        uint32_t maxBand = std::max(0, static_cast<int>(numBands) - 1);
        uint32_t w_uint = (maxBand & 0xFF) | ((maxBand & 0xFF) << 16) | (flags << 24);

        memcpy(&outTexZ, &z_uint, sizeof(float));
        memcpy(&outTexW, &w_uint, sizeof(float));
    }
};

class SlugFontBuilder {
public:
    explicit SlugFontBuilder(const uint8_t *ttfBuffer);

    SlugFontBuilder(const SlugFontBuilder&) = delete;
    SlugFontBuilder& operator=(const SlugFontBuilder&) = delete;

    void BakeGlyph(int codepoint, int numBands = 8);
    void BakeGlyphsFromString(std::string_view string);
    void BakeGlyphsFromString(std::string_view string, int numBands);

    [[nodiscard]]
    std::unique_ptr<SlugFont> CreateFont(ComPtr<ID3D11Device> device);

    [[nodiscard]]
    std::vector<SlugVertex> BuildTextVertices(std::string_view string, SlugTextBuildOptions options = {}) const;

    [[nodiscard]]
    const SlugGlyphInfo *GetGlyphInfo(int codepoint) const noexcept;
    [[nodiscard]]
    int GetFontVMetrics() const noexcept;

private:
    stbtt_fontinfo m_fontInfo;
    std::vector<SlugGlyphInfo> m_glyphs;

    std::vector<uint16_t> m_curveData;
    std::vector<uint16_t> m_bandData;

    int m_capHeight = 0;
    uint16_t m_unitsPerEm = 2048;

    int m_currentCurveX = 0;
    int m_currentCurveY = 0;

    static inline const int MAX_TEXTURE_WIDTH = 4096; // Strictly enforced by shaders kLogBandTextureWidth

    void WriteCurveTexel(int x, int y, float v0, float v1, float v2, float v3);
    void AlignBandDirectory(int size);
    uint32_t FindOrAddBandSequence(const std::vector<std::pair<uint16_t, uint16_t>>& seq, uint32_t searchStartIdx);
};

class SlugRenderer {
public:
    SlugRenderer(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);

    SlugRenderer(const SlugRenderer&) = delete;
    SlugRenderer& operator=(const SlugRenderer&) = delete;

    void BeginRender(SlugFont& font, int viewportWidth, int viewportHeight, const XMMATRIX& mvp);
    void DrawVertices(std::span<const SlugVertex> vertices);

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11RasterizerState> m_rasterizerState;

    UINT m_vertexCapacity = 0;

    void InitializePipeline();
};

class OverlayWindow {
public:
    HWND hwnd = nullptr;
    ComPtr<IDXGISwapChain1> swapchain;

    OverlayWindow(int width, int height, HWND targetOwner = NULL);
    ~OverlayWindow();

    OverlayWindow(const OverlayWindow&) = delete;
    OverlayWindow& operator=(const OverlayWindow&) = delete;

    void SetupSwapchain(ComPtr<IDXGIFactory2> factory, ComPtr<ID3D11Device> device);

    bool SyncToTarget(HWND target, bool clientArea = false, bool topmost = false);
    bool SyncAndResizeToTarget(HWND target, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, bool clientArea = false, bool topmost = false);

    void Resize(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, int width, int height);

    void SetOwner(HWND owner);

    void Bind(ComPtr<ID3D11DeviceContext> context) const;
    void Clear(ComPtr<ID3D11DeviceContext> context, const float clearColor[4]) const;
    void Present(UINT syncInterval = 1, UINT flags = 0);

    [[nodiscard]]
    int GetWidth() const noexcept {
        return m_width;
    }
    [[nodiscard]]
    int GetHeight() const noexcept {
        return m_height;
    }
    [[nodiscard]]
    ID3D11RenderTargetView *GetRenderTargetView() const noexcept {
        return m_rtv.Get();
    }

private:
    ComPtr<ID3D11RenderTargetView> m_rtv;
    int m_width = 0;
    int m_height = 0;

    void CreateBackbufferRTV(ComPtr<ID3D11Device> device);
};

} // namespace Slugger

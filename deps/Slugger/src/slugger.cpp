#include "Slugger/slugger.hpp"

#include <d3dcompiler.h>
#include <stdexcept>
#include <DirectXPackedVector.h>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "SlugVertexShader.h"
#include "SlugPixelShader.h"

namespace Slugger {

using namespace DirectX::PackedVector;

static constexpr wchar_t kOverlayWindowClassName[] = L"SlugOverlay";

inline void ThrowIfFailed(HRESULT hr, const char *msg = "Direct3D11 Error") {
    if (FAILED(hr))
        throw std::runtime_error(std::string(msg) + " (HRESULT: " + std::to_string(hr) + ")");
}

static uint16_t FloatToHalf(float f) { return XMConvertFloatToHalf(f); }

SlugDeviceResources SlugDeviceResources::Create(bool enableDebugLayer) {
    SlugDeviceResources out;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (enableDebugLayer)
        flags |= D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL flOut{};
    const D3D_FEATURE_LEVEL levels11_1[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    const D3D_FEATURE_LEVEL levels11_0[] = {D3D_FEATURE_LEVEL_11_0};

    auto tryCreate = [&](UINT createFlags, const D3D_FEATURE_LEVEL *levels, UINT levelCount) {
        return D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags, levels, levelCount, D3D11_SDK_VERSION, &out.device, &flOut,
                                 &out.context);
    };

    HRESULT hr = tryCreate(flags, levels11_1, ARRAYSIZE(levels11_1));
    if (hr == E_INVALIDARG) {
        out.device.Reset();
        out.context.Reset();
        hr = tryCreate(flags, levels11_0, ARRAYSIZE(levels11_0));
    }

    if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG)) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;

        out.device.Reset();
        out.context.Reset();

        hr = tryCreate(flags, levels11_1, ARRAYSIZE(levels11_1));
        if (hr == E_INVALIDARG) {
            out.device.Reset();
            out.context.Reset();
            hr = tryCreate(flags, levels11_0, ARRAYSIZE(levels11_0));
        }
    }

    ThrowIfFailed(hr, "D3D11CreateDevice failed");

    ComPtr<IDXGIDevice> dxgiDevice;
    ThrowIfFailed(out.device.As(&dxgiDevice), "Query IDXGIDevice failed");

    ComPtr<IDXGIAdapter> adapter;
    ThrowIfFailed(dxgiDevice->GetAdapter(&adapter), "GetAdapter failed");

    ThrowIfFailed(adapter->GetParent(IID_PPV_ARGS(&out.factory)), "GetParent(factory) failed");

    return out;
}

// Binary helpers for reading TrueType tables
static uint16_t ReadU16(const uint8_t *data, size_t offset) { return (data[offset] << 8) | data[offset + 1]; }
static int16_t ReadS16(const uint8_t *data, size_t offset) { return static_cast<int16_t>(ReadU16(data, offset)); }
static uint32_t ReadU32(const uint8_t *data, size_t offset) {
    return (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
}

static HWND GetInsertAfterForAbove(HWND target) {
    const bool targetTopmost = (GetWindowLongPtrW(target, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;

    for (HWND w = GetWindow(target, GW_HWNDPREV); w; w = GetWindow(w, GW_HWNDPREV)) {
        const bool wTopmost = (GetWindowLongPtrW(w, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
        if (wTopmost == targetTopmost)
            return w;
    }

    return targetTopmost ? HWND_TOPMOST : HWND_TOP;
}

static RECT GetTargetRectOnScreen(HWND target, bool clientArea) {
    RECT rc{};

    if (!clientArea) {
        GetWindowRect(target, &rc);
        return rc;
    }

    GetClientRect(target, &rc);

    POINT p0{rc.left, rc.top};
    POINT p1{rc.right, rc.bottom};

    ClientToScreen(target, &p0);
    ClientToScreen(target, &p1);

    rc.left = p0.x;
    rc.top = p0.y;
    rc.right = p1.x;
    rc.bottom = p1.y;
    return rc;
}

// SlugFont

SlugFont::SlugFont(ComPtr<ID3D11Device> device, std::span<const uint16_t> curveDataRGBA16F, int curveWidth, int curveHeight,
                   std::span<const uint16_t> bandDataRG16U, int bandWidth, int bandHeight) {

    D3D11_TEXTURE2D_DESC curveDesc = {};
    curveDesc.Width = curveWidth;
    curveDesc.Height = curveHeight;
    curveDesc.MipLevels = 1;
    curveDesc.ArraySize = 1;
    curveDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    curveDesc.SampleDesc.Count = 1;
    curveDesc.Usage = D3D11_USAGE_IMMUTABLE;
    curveDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA curveInitData = {};
    curveInitData.pSysMem = curveDataRGBA16F.data();
    curveInitData.SysMemPitch = curveWidth * 4 * sizeof(uint16_t);

    ComPtr<ID3D11Texture2D> curveTex;
    ThrowIfFailed(device->CreateTexture2D(&curveDesc, &curveInitData, &curveTex), "Failed to create curve texture");
    ThrowIfFailed(device->CreateShaderResourceView(curveTex.Get(), nullptr, &m_curveSRV), "Failed to create curve SRV");

    D3D11_TEXTURE2D_DESC bandDesc = curveDesc;
    bandDesc.Width = bandWidth;
    bandDesc.Height = bandHeight;
    bandDesc.Format = DXGI_FORMAT_R16G16_UINT;

    D3D11_SUBRESOURCE_DATA bandInitData = {};
    bandInitData.pSysMem = bandDataRG16U.data();
    bandInitData.SysMemPitch = bandWidth * 2 * sizeof(uint16_t);

    ComPtr<ID3D11Texture2D> bandTex;
    ThrowIfFailed(device->CreateTexture2D(&bandDesc, &bandInitData, &bandTex), "Failed to create band texture");
    ThrowIfFailed(device->CreateShaderResourceView(bandTex.Get(), nullptr, &m_bandSRV), "Failed to create band SRV");
}

// SlugFontBuilder

struct BezierCurve {
    float x0, y0, x1, y1, x2, y2;
    uint16_t texX, texY;

    float getMinX() const noexcept { return std::min({x0, x1, x2}); }
    float getMaxX() const noexcept { return std::max({x0, x1, x2}); }
    float getMinY() const noexcept { return std::min({y0, y1, y2}); }
    float getMaxY() const noexcept { return std::max({y0, y1, y2}); }

    bool isHorizontal() const noexcept { return std::abs(y0 - y1) < 1e-4f && std::abs(y1 - y2) < 1e-4f; }
    bool isVertical() const noexcept { return std::abs(x0 - x1) < 1e-4f && std::abs(x1 - x2) < 1e-4f; }
};

struct Contour {
    std::vector<BezierCurve> curves;
};

SlugFontBuilder::SlugFontBuilder(const uint8_t *ttfBuffer) {
    if (!stbtt_InitFont(&m_fontInfo, ttfBuffer, 0))
        throw std::runtime_error("Failed to initialize stb_truetype font.");

    uint32_t start = m_fontInfo.fontstart;
    int numTables = ReadU16(ttfBuffer, start + 4);

    for (int i = 0; i < numTables; ++i) {
        uint32_t tag = ReadU32(ttfBuffer, start + 12 + i * 16);
        uint32_t offset = ReadU32(ttfBuffer, start + 12 + i * 16 + 8);

        if (tag == 0x4F532F32) { // "OS/2"
            uint16_t version = ReadU16(ttfBuffer, offset);
            if (version >= 2)
                m_capHeight = ReadS16(ttfBuffer, offset + 74);
        } else if (tag == 0x68656164) { // "head"
            m_unitsPerEm = ReadU16(ttfBuffer, offset + 18);
        }
    }

    if (m_capHeight == 0) {
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
        m_capHeight = ascent;
    }
}

int SlugFontBuilder::GetFontVMetrics() const noexcept { return m_capHeight; }

const SlugGlyphInfo *SlugFontBuilder::GetGlyphInfo(int codepoint) const noexcept {
    for (const auto& g : m_glyphs) {
        if (g.codepoint == codepoint)
            return &g;
    }
    return nullptr;
}

void SlugFontBuilder::WriteCurveTexel(int x, int y, float v0, float v1, float v2, float v3) {
    size_t index = (y * MAX_TEXTURE_WIDTH + x) * 4;
    if (index + 4 > m_curveData.size())
        m_curveData.resize(index + 4, 0);
    m_curveData[index] = FloatToHalf(v0);
    m_curveData[index + 1] = FloatToHalf(v1);
    m_curveData[index + 2] = FloatToHalf(v2);
    m_curveData[index + 3] = FloatToHalf(v3);
}

void SlugFontBuilder::AlignBandDirectory(int size) {
    int currentX = (m_bandData.size() / 2) % MAX_TEXTURE_WIDTH;
    if (currentX + size > MAX_TEXTURE_WIDTH) {
        int pad = MAX_TEXTURE_WIDTH - currentX;
        m_bandData.resize(m_bandData.size() + pad * 2, 0);
    }
}

// Band matching
uint32_t SlugFontBuilder::FindOrAddBandSequence(const std::vector<std::pair<uint16_t, uint16_t>>& seq, uint32_t searchStartIdx) {
    if (seq.empty())
        return searchStartIdx;

    int seqLen = static_cast<int>(seq.size());
    int totalElements = static_cast<int>(m_bandData.size() / 2);

    for (int i = searchStartIdx; i <= totalElements - seqLen; ++i) {
        bool match = true;
        for (int j = 0; j < seqLen; ++j) {
            if (m_bandData[(i + j) * 2] != seq[j].first || m_bandData[(i + j) * 2 + 1] != seq[j].second) {
                match = false;
                break;
            }
        }
        if (match)
            return static_cast<uint32_t>(i);
    }

    uint32_t offset = totalElements;
    for (const auto& p : seq) {
        m_bandData.push_back(p.first);
        m_bandData.push_back(p.second);
    }
    return offset;
}

void SlugFontBuilder::BakeGlyph(int codepoint, int numBands) {
    int glyphIndex = stbtt_FindGlyphIndex(&m_fontInfo, codepoint);
    if (glyphIndex == 0)
        return;

    stbtt_vertex *vertices;
    int num_verts = stbtt_GetGlyphShape(&m_fontInfo, glyphIndex, &vertices);

    std::vector<Contour> contours;
    Contour currentContour;
    float cx = 0.0f, cy = 0.0f;

    for (int i = 0; i < num_verts; ++i) {
        switch (vertices[i].type) {
        case STBTT_vmove:
            if (!currentContour.curves.empty()) {
                contours.push_back(currentContour);
                currentContour.curves.clear();
            }
            cx = static_cast<float>(vertices[i].x);
            cy = static_cast<float>(vertices[i].y);
            break;
        case STBTT_vline: {
            float nx = static_cast<float>(vertices[i].x);
            float ny = static_cast<float>(vertices[i].y);
            // Encode straight line as {p1, p2, p2}
            currentContour.curves.push_back({cx, cy, nx, ny, nx, ny, 0, 0});
            cx = nx;
            cy = ny;
            break;
        }
        case STBTT_vcurve: {
            float mx = static_cast<float>(vertices[i].cx);
            float my = static_cast<float>(vertices[i].cy);
            float nx = static_cast<float>(vertices[i].x);
            float ny = static_cast<float>(vertices[i].y);
            currentContour.curves.push_back({cx, cy, mx, my, nx, ny, 0, 0});
            cx = nx;
            cy = ny;
            break;
        }
        case STBTT_vcubic:
            break;
        }
    }
    if (!currentContour.curves.empty())
        contours.push_back(currentContour);
    stbtt_FreeShape(&m_fontInfo, vertices);

    if (contours.empty())
        return;

    float minX = contours[0].curves[0].getMinX(), maxX = contours[0].curves[0].getMaxX();
    float minY = contours[0].curves[0].getMinY(), maxY = contours[0].curves[0].getMaxY();

    for (auto& contour : contours) {
        int reqTexels = static_cast<int>(contour.curves.size() + 1);

        // Wrap horizontally if contour exceeds maximum width
        if (m_currentCurveX + reqTexels > MAX_TEXTURE_WIDTH) {
            m_currentCurveX = 0;
            m_currentCurveY++;
        }

        for (size_t i = 0; i < contour.curves.size(); ++i) {
            auto& c = contour.curves[i];
            c.texX = m_currentCurveX + static_cast<uint16_t>(i);
            c.texY = m_currentCurveY;

            minX = std::min(minX, c.getMinX());
            maxX = std::max(maxX, c.getMaxX());
            minY = std::min(minY, c.getMinY());
            maxY = std::max(maxY, c.getMaxY());

            WriteCurveTexel(c.texX, c.texY, c.x0, c.y0, c.x1, c.y1);
        }

        auto& lastC = contour.curves.back();
        WriteCurveTexel(m_currentCurveX + static_cast<int>(contour.curves.size()), m_currentCurveY, lastC.x2, lastC.y2, 0, 0);
        m_currentCurveX += reqTexels;
    }

    // Allocate directory layout for band on a single row
    AlignBandDirectory(numBands * 2);

    uint32_t dirIndex = static_cast<uint32_t>(m_bandData.size() / 2);
    uint32_t glyphLocX = dirIndex % MAX_TEXTURE_WIDTH;
    uint32_t glyphLocY = dirIndex / MAX_TEXTURE_WIDTH;

    // Pad space for directory
    m_bandData.resize(m_bandData.size() + (numBands * 2 * 2), 0);

    const float epsilon = static_cast<float>(m_unitsPerEm) / 1024.0f;

    auto processBands = [&](bool isHorizontal, int dirOffset) {
        float minBound = isHorizontal ? minY : minX;
        float maxBound = isHorizontal ? maxY : maxX;
        float bandSize = (maxBound - minBound) / numBands;

        for (int i = 0; i < numBands; ++i) {
            float bandStart = minBound + (i * bandSize) - epsilon;
            float bandEnd = minBound + ((i + 1) * bandSize) + epsilon;

            std::vector<const BezierCurve *> bandCurves;
            for (const auto& contour : contours) {
                for (const auto& c : contour.curves) {
                    if (isHorizontal && c.isHorizontal())
                        continue;
                    if (!isHorizontal && c.isVertical())
                        continue;

                    float cMin = isHorizontal ? c.getMinY() : c.getMinX();
                    float cMax = isHorizontal ? c.getMaxY() : c.getMaxX();

                    if (cMax >= bandStart && cMin <= bandEnd)
                        bandCurves.push_back(&c);
                }
            }

            std::sort(bandCurves.begin(), bandCurves.end(), [isHorizontal](const BezierCurve *a, const BezierCurve *b) {
                return isHorizontal ? (a->getMaxX() > b->getMaxX()) : (a->getMaxY() > b->getMaxY());
            });

            std::vector<std::pair<uint16_t, uint16_t>> seq;
            for (const auto *c : bandCurves)
                seq.push_back({c->texX, c->texY});

            uint32_t offset = FindOrAddBandSequence(seq, dirIndex + numBands * 2);

            m_bandData[(dirIndex + dirOffset + i) * 2 + 0] = static_cast<uint16_t>(seq.size());
            m_bandData[(dirIndex + dirOffset + i) * 2 + 1] = static_cast<uint16_t>(offset - dirIndex);
        }
    };

    processBands(true, 0);
    processBands(false, numBands);

    int advanceWidth, leftSideBearing;
    stbtt_GetGlyphHMetrics(&m_fontInfo, glyphIndex, &advanceWidth, &leftSideBearing);

    SlugGlyphInfo info = {};
    info.codepoint = codepoint;
    info.advanceWidth = static_cast<float>(advanceWidth);
    info.leftSideBearing = static_cast<float>(leftSideBearing);
    info.bboxMinX = minX;
    info.bboxMinY = minY;
    info.bboxMaxX = maxX;
    info.bboxMaxY = maxY;
    info.glyphLocX = static_cast<uint16_t>(glyphLocX);
    info.glyphLocY = static_cast<uint16_t>(glyphLocY);
    info.numBands = static_cast<uint16_t>(numBands);

    m_glyphs.push_back(info);
}

void SlugFontBuilder::BakeGlyphsFromString(std::string_view string) { BakeGlyphsFromString(string, 8); }

void SlugFontBuilder::BakeGlyphsFromString(std::string_view string, int numBands) {
    bool baked[256] = {};

    for (unsigned char c : string) {
        if (!baked[c]) {
            BakeGlyph(c, numBands);
            baked[c] = true;
        }
    }

    if (!baked[(unsigned char)' '])
        BakeGlyph(' ', numBands);
}

std::unique_ptr<SlugFont> SlugFontBuilder::CreateFont(ComPtr<ID3D11Device> device) {
    int curveHeight = std::max(1, m_currentCurveY + (m_currentCurveX > 0 ? 1 : 0));
    m_curveData.resize(MAX_TEXTURE_WIDTH * curveHeight * 4, 0);

    int bandHeight = std::max(1, static_cast<int>((m_bandData.size() / 2 + MAX_TEXTURE_WIDTH - 1) / MAX_TEXTURE_WIDTH));
    m_bandData.resize(MAX_TEXTURE_WIDTH * bandHeight * 2, 0);

    return std::make_unique<SlugFont>(device, m_curveData, MAX_TEXTURE_WIDTH, curveHeight, m_bandData, MAX_TEXTURE_WIDTH, bandHeight);
}

std::vector<SlugVertex> SlugFontBuilder::BuildTextVertices(std::string_view text, SlugTextBuildOptions options) const {
    std::vector<SlugVertex> vertices;
    float penX = 0.0f;
    float penY = 0.0f;

    const SlugGlyphInfo *spaceGlyph = GetGlyphInfo(' ');
    const float spaceAdvance = spaceGlyph ? spaceGlyph->advanceWidth : 400.0f;
    const float lineAdvance = m_capHeight > 0 ? static_cast<float>(m_capHeight) : 1000.0f;

    auto emitGlyph = [&](const SlugGlyphInfo& g) {
        XMFLOAT4 bnd{};
        float texZ = 0.0f;
        float texW = 0.0f;
        g.GetBandingAndTexData(bnd, texZ, texW);

        // Handle DX11 Y-axis flip and scaling
        const float x0 = (penX + g.bboxMinX) * options.scale;
        const float y0 = (penY + g.bboxMinY) * -options.scale;
        const float x1 = (penX + g.bboxMaxX) * options.scale;
        const float y1 = (penY + g.bboxMaxY) * -options.scale;

        // Calculate the shader derivatives mapping screen -> texture
        XMFLOAT4 computedJacobian = {1.0f / options.scale, 0.0f, 0.0f, 1.0f / -options.scale};

        auto makeVertex = [&](float px, float py, float tx, float ty, const XMFLOAT4& col) {
            SlugVertex v{};
            v.pos = {px, py, 0.0f, 1.0f};
            v.tex = {tx, ty, texZ, texW};
            v.jac = computedJacobian;
            v.bnd = bnd;
            v.col = col;
            return v;
        };

        vertices.push_back(makeVertex(x0, y0, g.bboxMinX, g.bboxMinY, options.minYColor));
        vertices.push_back(makeVertex(x1, y0, g.bboxMaxX, g.bboxMinY, options.minYColor));
        vertices.push_back(makeVertex(x1, y1, g.bboxMaxX, g.bboxMaxY, options.maxYColor));

        vertices.push_back(makeVertex(x0, y0, g.bboxMinX, g.bboxMinY, options.minYColor));
        vertices.push_back(makeVertex(x1, y1, g.bboxMaxX, g.bboxMaxY, options.maxYColor));
        vertices.push_back(makeVertex(x0, y1, g.bboxMinX, g.bboxMaxY, options.maxYColor));

        penX += g.advanceWidth;
    };

    for (unsigned char c : text) {
        switch (c) {
        case ' ':
            penX += spaceAdvance;
            break;
        case '\t':
            penX += spaceAdvance * 4.0f;
            break;
        case '\n':
            penX = 0.0f;
            penY -= lineAdvance;
            break;
        default:
            if (const SlugGlyphInfo *g = GetGlyphInfo(c)) {
                emitGlyph(*g);
            }
            break;
        }
    }

    if (options.centerOrigin && !vertices.empty()) {
        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX;

        for (const auto& v : vertices) {
            minX = std::min(minX, v.pos.x);
            minY = std::min(minY, v.pos.y);
            maxX = std::max(maxX, v.pos.x);
            maxY = std::max(maxY, v.pos.y);
        }

        const float cx = (minX + maxX) * 0.5f;
        const float cy = (minY + maxY) * 0.5f;

        for (auto& v : vertices) {
            v.pos.x -= cx;
            v.pos.y -= cy;
        }
    }

    return vertices;
}

// SlugRenderer

SlugRenderer::SlugRenderer(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context) : m_device(device), m_context(context) { InitializePipeline(); }

void SlugRenderer::InitializePipeline() {
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr =
        D3DCompile(g_SlugVertexShader, sizeof(g_SlugVertexShader) - 1, "SlugVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        std::string errStr = errorBlob ? static_cast<char *>(errorBlob->GetBufferPointer()) : "Unknown Error";
        throw std::runtime_error("Failed to compile Vertex Shader: " + errStr);
    }
    ThrowIfFailed(m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader));

    hr = D3DCompile(g_SlugPixelShader, sizeof(g_SlugPixelShader) - 1, "SlugPixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        std::string errStr = errorBlob ? static_cast<char *>(errorBlob->GetBufferPointer()) : "Unknown Error";
        throw std::runtime_error("Failed to compile Pixel Shader: " + errStr);
    }
    ThrowIfFailed(m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader));

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {{"ATTRIB", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                                             {"ATTRIB", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                                             {"ATTRIB", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                                             {"ATTRIB", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                                             {"ATTRIB", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};
    ThrowIfFailed(m_device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout));

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(SlugConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ThrowIfFailed(m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer));

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ThrowIfFailed(m_device->CreateBlendState(&blendDesc, &m_blendState));

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthClipEnable = TRUE;

    ThrowIfFailed(m_device->CreateRasterizerState(&rsDesc, &m_rasterizerState), "Failed to create rasterizer state");
}

void SlugRenderer::BeginRender(SlugFont& font, int viewportWidth, int viewportHeight, const XMMATRIX& mvp) {
    SlugConstantBuffer cb = {};
    cb.slug_viewport = XMFLOAT4(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), 0.0f, 0.0f);

    XMMATRIX mvpTransposed = XMMatrixTranspose(mvp);
    XMStoreFloat4(&cb.slug_matrix[0], mvpTransposed.r[0]);
    XMStoreFloat4(&cb.slug_matrix[1], mvpTransposed.r[1]);
    XMStoreFloat4(&cb.slug_matrix[2], mvpTransposed.r[2]);
    XMStoreFloat4(&cb.slug_matrix[3], mvpTransposed.r[3]);

    D3D11_MAPPED_SUBRESOURCE mapped;
    ThrowIfFailed(m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped), "Failed to map constant buffer");
    memcpy(mapped.pData, &cb, sizeof(SlugConstantBuffer));
    m_context->Unmap(m_constantBuffer.Get(), 0);

    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_context->RSSetState(m_rasterizerState.Get());

    ID3D11ShaderResourceView *srvs[] = {font.GetCurveSRV(), font.GetBandSRV()};
    m_context->PSSetShaderResources(0, 2, srvs);

    float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);
}

void SlugRenderer::DrawVertices(std::span<const SlugVertex> vertices) {
    if (vertices.empty())
        return;

    UINT vertexCount = static_cast<UINT>(vertices.size());

    if (!m_vertexBuffer || m_vertexCapacity < vertexCount) {
        m_vertexCapacity = std::max(m_vertexCapacity * 2, vertexCount + 1024);

        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.ByteWidth = m_vertexCapacity * sizeof(SlugVertex);
        vbDesc.Usage = D3D11_USAGE_DYNAMIC;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        ThrowIfFailed(m_device->CreateBuffer(&vbDesc, nullptr, &m_vertexBuffer), "Failed to create dynamic vertex buffer");
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    ThrowIfFailed(m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped), "Failed to map vertex buffer");

    memcpy(mapped.pData, vertices.data(), vertexCount * sizeof(SlugVertex));
    m_context->Unmap(m_vertexBuffer.Get(), 0);

    UINT stride = sizeof(SlugVertex);
    UINT offset = 0;

    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->Draw(vertexCount, 0);
}

// OverlayWindow

OverlayWindow::OverlayWindow(int width, int height, HWND targetOwner) : m_width(width), m_height(height) {
    WNDCLASSEXW wc = {sizeof(WNDCLASSEXW), CS_CLASSDC, DefWindowProcW, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, kOverlayWindowClassName, NULL};
    RegisterClassExW(&wc);

    hwnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, wc.lpszClassName, L"Slug Overlay", WS_POPUP, 0, 0, width,
                           height, targetOwner, NULL, wc.hInstance, NULL);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
}

OverlayWindow::~OverlayWindow() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void OverlayWindow::SetupSwapchain(ComPtr<IDXGIFactory2> factory, ComPtr<ID3D11Device> device) {
    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    ThrowIfFailed(factory->CreateSwapChainForHwnd(device.Get(), hwnd, &sd, nullptr, nullptr, &swapchain), "Failed to create Swapchain");

    RECT rc{};
    GetClientRect(hwnd, &rc);
    m_width = std::max(1L, rc.right - rc.left);
    m_height = std::max(1L, rc.bottom - rc.top);

    CreateBackbufferRTV(device);
}

bool OverlayWindow::SyncToTarget(HWND target, bool clientArea, bool topmost) {
    if (!target || !IsWindow(target)) {
        ShowWindow(hwnd, SW_HIDE);
        return false;
    }

    if (!IsWindowVisible(target) || IsIconic(target)) {
        ShowWindow(hwnd, SW_HIDE);
        return false;
    }

    RECT rc = GetTargetRectOnScreen(target, clientArea);
    const int w = std::max(1L, rc.right - rc.left);
    const int h = std::max(1L, rc.bottom - rc.top);

    if (topmost) {
        SetWindowLongPtrW(hwnd, GWLP_HWNDPARENT, 0);
        SetWindowPos(hwnd, HWND_TOPMOST, rc.left, rc.top, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    } else {
        SetWindowLongPtrW(hwnd, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(target));
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        SetWindowPos(hwnd, nullptr, rc.left, rc.top, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }

    return true;
}

bool OverlayWindow::SyncAndResizeToTarget(HWND target, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, bool clientArea, bool topmost) {
    if (!SyncToTarget(target, clientArea, topmost))
        return false;

    RECT rc = GetTargetRectOnScreen(target, clientArea);
    Resize(device, context, std::max(1L, rc.right - rc.left), std::max(1L, rc.bottom - rc.top));

    return true;
}

void OverlayWindow::Resize(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, int width, int height) {
    width = std::max(1, width);
    height = std::max(1, height);

    if (!swapchain) {
        throw std::runtime_error("OverlayWindow swapchain has not been created.");
    }

    if (m_rtv && width == m_width && height == m_height) {
        return;
    }

    context->OMSetRenderTargets(0, nullptr, nullptr);
    m_rtv.Reset();

    ThrowIfFailed(swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0), "ResizeBuffers failed");

    m_width = width;
    m_height = height;

    CreateBackbufferRTV(device);
}

void OverlayWindow::SetOwner(HWND owner) { SetWindowLongPtrW(hwnd, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(owner)); }

void OverlayWindow::Bind(ComPtr<ID3D11DeviceContext> context) const {
    ID3D11RenderTargetView *rtv = m_rtv.Get();
    context->OMSetRenderTargets(1, &rtv, nullptr);

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);
}

void OverlayWindow::Clear(ComPtr<ID3D11DeviceContext> context, const float clearColor[4]) const {
    if (m_rtv)
        context->ClearRenderTargetView(m_rtv.Get(), clearColor);
}

void OverlayWindow::Present(UINT syncInterval, UINT flags) { ThrowIfFailed(swapchain->Present(syncInterval, flags), "Present failed"); }

void OverlayWindow::CreateBackbufferRTV(ComPtr<ID3D11Device> device) {
    ComPtr<ID3D11Texture2D> backbuffer;
    ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)), "GetBuffer(backbuffer) failed");
    ThrowIfFailed(device->CreateRenderTargetView(backbuffer.Get(), nullptr, &m_rtv), "CreateRenderTargetView failed");
}

} // namespace Slugger

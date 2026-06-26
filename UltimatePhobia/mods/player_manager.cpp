#include <fstream>
#include <algorithm>
#include <limits>

#include "player_manager.hpp"
#include "il2cpp_cppinterop.hpp"
#include "global_instance_manager.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"

using namespace Il2Cpp::CppInterop;
using namespace Il2Cpp;

namespace {
constexpr float kHeadWorldYOffset = 0.2f;
constexpr float kTargetFontSize = 26.0f;
} // namespace

void player$$UpdateFnc(Player_o *__this, const MethodInfo *method) {
    auto self = playerManagerInfo.get<PlayerManager>();
    if (!self || !__this)
        return;

    std::string nickName;
    bool shouldProject = false;

    // Register player
    {
        std::lock_guard<std::mutex> lock(self->m_drawMutex);

        auto it = self->trackedPlayers.find(__this);
        if (it == self->trackedPlayers.end()) {
            if (__this->fields.photonView) {
                if (__this->fields.photonView->fields._AmOwner_k__BackingField) {
                    // Cache local player as empty label
                    self->trackedPlayers[__this] = PlayerTextData{};
                } else {
                    Photon_Realtime_Player_o *photonPlayer = __this->fields.photonView->fields._Owner_k__BackingField;
                    if (photonPlayer) {
                        System_String_o *nickNameCs = Photon::Realtime::Player::get_NickName(photonPlayer, nullptr);
                        if (nickNameCs) {
                            nickName = ToCppString(nickNameCs);
                            self->trackedPlayers[__this] = PlayerTextData{nickName, 0.0f, 0.0f, -1.0f};
                            shouldProject = true;
                        }
                    }
                }
            }
        } else {
            nickName = it->second.text;
            shouldProject = !nickName.empty();
        }
    }

    // Update remote players screen position
    if (shouldProject) {
        UnityEngine_Camera_o *mainCam = UnityEngine::Camera::get_main();

        if (mainCam && __this->fields.pcPlayerHead) {
            UnityEngine_Transform_o *headTrans = UnityEngine::GameObject::get_transform(__this->fields.pcPlayerHead);
            if (headTrans) {
                UnityEngine_Vector3_o headPos = UnityEngine::Transform::get_position(headTrans);
                headPos.fields.y += kHeadWorldYOffset;

                UnityEngine_Vector3_o screenPos = UnityEngine::Camera::WorldToScreenPoint(mainCam, headPos);
                float unityHeight = static_cast<float>(UnityEngine::Screen::get_height());

                std::lock_guard<std::mutex> lock(self->m_drawMutex);
                auto it = self->trackedPlayers.find(__this);
                if (it != self->trackedPlayers.end()) {
                    it->second.screenX = screenPos.fields.x;
                    it->second.screenY = unityHeight - screenPos.fields.y;
                    it->second.depth = screenPos.fields.z;
                }
            } else {
                std::lock_guard<std::mutex> lock(self->m_drawMutex);
                auto it = self->trackedPlayers.find(__this);
                if (it != self->trackedPlayers.end())
                    it->second.depth = -1.0f;
            }
        } else {
            std::lock_guard<std::mutex> lock(self->m_drawMutex);
            auto it = self->trackedPlayers.find(__this);
            if (it != self->trackedPlayers.end())
                it->second.depth = -1.0f;
        }
    }

    // Call original
    GameHookRelease GHR(self->player$$UpdateHook);
    self->player$$UpdateHook.getFunction<decltype(player$$UpdateFnc)>()(__this, method);
}

void player$$OnDisableFnc(Player_o *__this, const MethodInfo *method) {
    auto self = playerManagerInfo.get<PlayerManager>();
    if (!self)
        return;

    {
        std::lock_guard<std::mutex> lock(self->m_drawMutex);
        self->trackedPlayers.erase(__this);
    }

    GameHookRelease GHR(self->player$$OnDisableHook);
    self->player$$OnDisableHook.getFunction<decltype(player$$OnDisableFnc)>()(__this, method);
}

PlayerManager::PlayerManager()
    : player$$UpdateHook(GameHook::safeCreateOrPanic(playerManagerInfo, Il2Cpp::Player::Update_getPtr(), reinterpret_cast<void *>(player$$UpdateFnc))),
      player$$OnDisableHook(
          GameHook::safeCreateOrPanic(playerManagerInfo, Il2Cpp::Player::OnDisable_getPtr(), reinterpret_cast<void *>(player$$OnDisableFnc))) {
    DX11Hook::registerMod(this);
}

PlayerManager::~PlayerManager() { DX11Hook::unregisterMod(this); }

Player_o *PlayerManager::getLocalPlayer() const {
    std::lock_guard<std::mutex> lock(m_drawMutex);

    for (const auto& [player, data] : trackedPlayers) {
        if (!player || !player->fields.photonView)
            continue;

        if (player->fields.photonView->fields._AmOwner_k__BackingField)
            return player;
    }

    return nullptr;
}

void PlayerManager::RefreshBackbufferDimensions(IDXGISwapChain *swapChain) {
    DXGI_SWAP_CHAIN_DESC desc{};
    if (SUCCEEDED(swapChain->GetDesc(&desc))) {
        if (desc.BufferDesc.Width != 0)
            m_backbufferWidth = static_cast<int>(desc.BufferDesc.Width);
        if (desc.BufferDesc.Height != 0)
            m_backbufferHeight = static_cast<int>(desc.BufferDesc.Height);
    }
}

void PlayerManager::EnsureRenderTarget(IDXGISwapChain *swapChain) {
    if (m_mainRTV)
        return;

    if (!m_device)
        return;

    ID3D11Texture2D *backBufferRaw = nullptr;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBufferRaw)))) {
        g.logger->error("EnsureRenderTarget: failed to get swap chain back buffer.");
        return;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    backBuffer.Attach(backBufferRaw);

    if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_mainRTV.ReleaseAndGetAddressOf()))) {
        g.logger->error("EnsureRenderTarget: failed to create RTV.");
        return;
    }
}

void PlayerManager::InitializeSlugger(IDXGISwapChain *swapChain) {
    if (m_sluggerInitialized) {
        return;
    }

    g.logger->info("Initializing Slugger...");

    if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void **>(m_device.ReleaseAndGetAddressOf())))) {
        g.logger->error("InitializeSlugger: failed to get D3D11 device.");
        m_sluggerInitialized = true;
        return;
    }

    m_device->GetImmediateContext(m_context.ReleaseAndGetAddressOf());
    if (!m_context) {
        g.logger->error("InitializeSlugger: failed to get immediate context.");
        m_sluggerInitialized = true;
        return;
    }

    std::ifstream fontFile("C:\\Windows\\Fonts\\arial.ttf", std::ios::binary | std::ios::ate);
    if (!fontFile.is_open()) {
        g.logger->error("InitializeSlugger: failed to open C:\\Windows\\Fonts\\arial.ttf");
        m_sluggerInitialized = true;
        return;
    }

    std::streamsize fileSize = fontFile.tellg();
    if (fileSize <= 0) {
        g.logger->error("InitializeSlugger: invalid font file size.");
        m_sluggerInitialized = true;
        return;
    }

    fontFile.seekg(0, std::ios::beg);

    m_fontData.resize(static_cast<size_t>(fileSize));
    if (!fontFile.read(reinterpret_cast<char *>(m_fontData.data()), fileSize)) {
        g.logger->error("InitializeSlugger: failed to read font data.");
        m_sluggerInitialized = true;
        return;
    }

    m_fontBuilder = std::make_unique<Slugger::SlugFontBuilder>(m_fontData.data());
    m_fontBuilder->BakeGlyphsFromString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_- [](){}.,:;!?/\\'\"");

    m_font = m_fontBuilder->CreateFont(m_device.Get());
    m_renderer = std::make_unique<Slugger::SlugRenderer>(m_device.Get(), m_context.Get());

    RefreshBackbufferDimensions(swapChain);

    m_sluggerInitialized = true;
}

void PlayerManager::onDX11Present(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) {
    if (!m_sluggerInitialized)
        InitializeSlugger(swapChain);

    if (!m_sluggerInitialized || !m_device || !m_context || !m_fontBuilder || !m_font || !m_renderer)
        return;

    RefreshBackbufferDimensions(swapChain);
    EnsureRenderTarget(swapChain);

    if (!m_mainRTV || m_backbufferWidth <= 0 || m_backbufferHeight <= 0)
        return;

    const float unityScreenWidth = static_cast<float>(UnityEngine::Screen::get_width());
    const float unityScreenHeight = static_cast<float>(UnityEngine::Screen::get_height());

    if (unityScreenWidth <= 0.0f || unityScreenHeight <= 0.0f)
        return;

    std::lock_guard<std::mutex> lock(m_drawMutex);

    ID3D11RenderTargetView *rtvs[] = {m_mainRTV.Get()};
    m_context->OMSetRenderTargets(1, rtvs, nullptr);

    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(m_backbufferWidth);
    vp.Height = static_cast<float>(m_backbufferHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    m_context->RSSetViewports(1, &vp);

    DirectX::XMMATRIX orthoProjection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, unityScreenWidth, unityScreenHeight, 0.0f, 0.0f, 1.0f);

    m_renderer->BeginRender(*m_font, static_cast<int>(unityScreenWidth), static_cast<int>(unityScreenHeight), orthoProjection);

    Slugger::SlugTextBuildOptions options;
    options.minYColor = {1.0f, 1.0f, 1.0f, 1.0f};
    options.maxYColor = {1.0f, 1.0f, 1.0f, 1.0f};
    options.centerOrigin = false;

    const float baseFontScale = kTargetFontSize / static_cast<float>(std::max(1, m_fontBuilder->GetFontVMetrics()));

    for (const auto& [player, data] : trackedPlayers) {
        if (data.text.empty())
            continue;

        if (data.depth <= 0.0f)
            continue;

        // Cull in same Unity screen-space coordinates as stored
        if (data.screenX < -64.0f || data.screenX > unityScreenWidth + 64.0f || data.screenY < -64.0f || data.screenY > unityScreenHeight + 64.0f)
            continue;

        // Clamped distance-based scale
        float scaledFontScale = baseFontScale * (3.0f / std::max(data.depth, 1.0f));
        options.scale = std::clamp(scaledFontScale, baseFontScale * 0.50f, baseFontScale * 1.50f);

        auto vertices = m_fontBuilder->BuildTextVertices(data.text, options);
        if (vertices.empty())
            continue;

        // Compute vertex bounds for anchoring
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();

        for (const auto& v : vertices) {
            minX = std::min(minX, v.pos.x);
            maxX = std::max(maxX, v.pos.x);
            minY = std::min(minY, v.pos.y);
            maxY = std::max(maxY, v.pos.y);
        }

        const float textWidth = maxX - minX;
        const float textHeight = maxY - minY;

        // Anchor point is projected head position
        const float anchorX = data.screenX;
        const float anchorY = data.screenY;

        // Center horizontally, place text above anchor vertically
        const float drawOffsetX = anchorX - (textWidth * 0.5f) - minX;
        const float drawOffsetY = anchorY - textHeight - minY;

        for (auto& v : vertices) {
            v.pos.x += drawOffsetX;
            v.pos.y += drawOffsetY;
        }

        m_renderer->DrawVertices(vertices);
    }
}

void PlayerManager::onDX11ResizeBuffers(IDXGISwapChain *swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
    m_mainRTV.Reset();

    m_backbufferWidth = static_cast<int>(width);
    m_backbufferHeight = static_cast<int>(height);
}

ModInfo playerManagerInfo{"Player Manager", false, []() { return std::make_unique<PlayerManager>(); }};

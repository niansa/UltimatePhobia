#include "slugger_manager.hpp"

#include "bindings/unityengine.hpp"

#include <windows.h>
#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <limits>
#include <string>
#include <utility>

namespace {

constexpr float kEpsilon = 1.0e-6f;
constexpr std::size_t kMaximumQueuedRequests = 16384;

bool FileExists(const std::string& path) {
    const DWORD attributes = GetFileAttributesA(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool IsFinite(float value) { return std::isfinite(value); }
bool IsFinite(const UnityEngine_Vector3_o& value) { return IsFinite(value.fields.x) && IsFinite(value.fields.y) && IsFinite(value.fields.z); }
bool IsFinite(const UnityEngine_Quaternion_o& value) {
    return IsFinite(value.fields.x) && IsFinite(value.fields.y) && IsFinite(value.fields.z) && IsFinite(value.fields.w);
}

UnityEngine_Vector3_o Add(const UnityEngine_Vector3_o& a, const UnityEngine_Vector3_o& b) {
    UnityEngine_Vector3_o result{};
    result.fields.x = a.fields.x + b.fields.x;
    result.fields.y = a.fields.y + b.fields.y;
    result.fields.z = a.fields.z + b.fields.z;
    return result;
}

UnityEngine_Vector3_o Subtract(const UnityEngine_Vector3_o& a, const UnityEngine_Vector3_o& b) {
    UnityEngine_Vector3_o result{};
    result.fields.x = a.fields.x - b.fields.x;
    result.fields.y = a.fields.y - b.fields.y;
    result.fields.z = a.fields.z - b.fields.z;
    return result;
}

UnityEngine_Vector3_o Multiply(const UnityEngine_Vector3_o& value, float scalar) {
    UnityEngine_Vector3_o result{};
    result.fields.x = value.fields.x * scalar;
    result.fields.y = value.fields.y * scalar;
    result.fields.z = value.fields.z * scalar;
    return result;
}

float LengthSquared(const UnityEngine_Vector3_o& value) {
    return value.fields.x * value.fields.x + value.fields.y * value.fields.y + value.fields.z * value.fields.z;
}

UnityEngine_Vector3_o Normalize(const UnityEngine_Vector3_o& value, const UnityEngine_Vector3_o& fallback) {
    const float lengthSquared = LengthSquared(value);
    if (!IsFinite(lengthSquared) || lengthSquared <= kEpsilon)
        return fallback;

    return Multiply(value, 1.0f / std::sqrt(lengthSquared));
}

UnityEngine_Vector3_o Cross(const UnityEngine_Vector3_o& a, const UnityEngine_Vector3_o& b) {
    UnityEngine_Vector3_o result{};
    result.fields.x = a.fields.y * b.fields.z - a.fields.z * b.fields.y;
    result.fields.y = a.fields.z * b.fields.x - a.fields.x * b.fields.z;
    result.fields.z = a.fields.x * b.fields.y - a.fields.y * b.fields.x;
    return result;
}

struct UnityQuaternionBasis {
    UnityEngine_Vector3_o right{};
    UnityEngine_Vector3_o up{};
    UnityEngine_Vector3_o forward{};
};

// Converts a Unity quaternion into its three rotated basis vectors
UnityQuaternionBasis GetUnityQuaternionBasis(const UnityEngine_Quaternion_o& rotation) {

    float x = rotation.fields.x;
    float y = rotation.fields.y;
    float z = rotation.fields.z;
    float w = rotation.fields.w;

    const float lengthSquared = x * x + y * y + z * z + w * w;
    if (!IsFinite(lengthSquared) || lengthSquared <= kEpsilon)
        return {UnityEngine_Vector3_o{{1.0f, 0.0f, 0.0f}}, UnityEngine_Vector3_o{{0.0f, 1.0f, 0.0f}}, UnityEngine_Vector3_o{{0.0f, 0.0f, 1.0f}}};

    const float inverseLength = 1.0f / std::sqrt(lengthSquared);
    x *= inverseLength;
    y *= inverseLength;
    z *= inverseLength;
    w *= inverseLength;

    const float xx = x * x;
    const float yy = y * y;
    const float zz = z * z;
    const float xy = x * y;
    const float xz = x * z;
    const float yz = y * z;
    const float xw = x * w;
    const float yw = y * w;
    const float zw = z * w;

    UnityQuaternionBasis basis{};

    // First column
    basis.right.fields.x = 1.0f - 2.0f * (yy + zz);
    basis.right.fields.y = 2.0f * (xy + zw);
    basis.right.fields.z = 2.0f * (xz - yw);

    // Second column
    basis.up.fields.x = 2.0f * (xy - zw);
    basis.up.fields.y = 1.0f - 2.0f * (xx + zz);
    basis.up.fields.z = 2.0f * (yz + xw);

    // Third column
    basis.forward.fields.x = 2.0f * (xz + yw);
    basis.forward.fields.y = 2.0f * (yz - xw);
    basis.forward.fields.z = 1.0f - 2.0f * (xx + yy);

    return basis;
}

struct TextBounds {
    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
};

template <typename TVertices> bool CalculateTextBounds(const TVertices& vertices, TextBounds& bounds) {
    if (vertices.empty())
        return false;

    bounds.minX = std::numeric_limits<float>::max();
    bounds.maxX = std::numeric_limits<float>::lowest();
    bounds.minY = std::numeric_limits<float>::max();
    bounds.maxY = std::numeric_limits<float>::lowest();

    for (const auto& vertex : vertices) {
        if (!IsFinite(vertex.pos.x) || !IsFinite(vertex.pos.y))
            return false;

        bounds.minX = std::min(bounds.minX, vertex.pos.x);
        bounds.maxX = std::max(bounds.maxX, vertex.pos.x);
        bounds.minY = std::min(bounds.minY, vertex.pos.y);
        bounds.maxY = std::max(bounds.maxY, vertex.pos.y);
    }

    return IsFinite(bounds.minX) && IsFinite(bounds.maxX) && IsFinite(bounds.minY) && IsFinite(bounds.maxY);
}

std::string GetArialFontPath() {
    std::array<char, MAX_PATH> windowsDirectory{};
    const UINT length = GetWindowsDirectoryA(windowsDirectory.data(), static_cast<UINT>(windowsDirectory.size()));

    if (length > 0 && length < windowsDirectory.size()) {
        std::string result(windowsDirectory.data(), length);

        if (!result.empty() && result.back() != '\\')
            result.push_back('\\');

        result += "Fonts\\arial.ttf";
        return result;
    }

    return "C:\\Windows\\Fonts\\arial.ttf";
}

std::string GetMonospacedFontPath() {
    static const std::array<const char *, 7> kLinuxMonospacePaths = {
        {"Z:\\usr\\share\\fonts\\truetype\\dejavu\\DejaVuSansMono.ttf", "Z:\\usr\\share\\fonts\\TTF\\DejaVuSansMono.ttf",
         "Z:\\usr\\share\\fonts\\truetype\\liberation\\LiberationMono-Regular.ttf", "Z:\\usr\\share\\fonts\\truetype\\liberation2\\LiberationMono-Regular.ttf",
         "Z:\\usr\\share\\fonts\\liberation-mono\\LiberationMono-Regular.ttf", "Z:\\usr\\share\\fonts\\truetype\\noto\\NotoSansMono-Regular.ttf",
         "Z:\\usr\\share\\fonts\\noto\\NotoMono-Regular.ttf"}};

    for (const char *path : kLinuxMonospacePaths)
        if (FileExists(path))
            return path;

    // Windows font directory lookup
    std::array<char, MAX_PATH> windowsDirectory{};
    const UINT length = GetWindowsDirectoryA(windowsDirectory.data(), static_cast<UINT>(windowsDirectory.size()));

    if (length > 0 && length < windowsDirectory.size()) {
        std::string winDir(windowsDirectory.data(), length);
        if (!winDir.empty() && winDir.back() != '\\')
            winDir.push_back('\\');

        const std::string consola = winDir + "Fonts\\consola.ttf";
        if (FileExists(consola))
            return consola;

        const std::string courier = winDir + "Fonts\\cour.ttf";
        if (FileExists(courier))
            return courier;
    }

    // Fallback to standard C: Windows paths
    if (FileExists("C:\\Windows\\Fonts\\consola.ttf"))
        return "C:\\Windows\\Fonts\\consola.ttf";
    if (FileExists("C:\\Windows\\Fonts\\cour.ttf"))
        return "C:\\Windows\\Fonts\\cour.ttf";

    // Or if nothing else works, just use Arial
    return GetArialFontPath();
}

// Preserve pipeline state
class ScopedDeviceContextState final {
public:
    ScopedDeviceContextState(ID3D11DeviceContext1 *context, ID3DDeviceContextState *sluggerState) : m_context(context) {
        if (!m_context || !sluggerState)
            return;

        m_context->SwapDeviceContextState(sluggerState, m_previousState.ReleaseAndGetAddressOf());

        m_active = m_previousState != nullptr;
    }

    ~ScopedDeviceContextState() {
        if (m_active && m_context)
            m_context->SwapDeviceContextState(m_previousState.Get(), nullptr);
    }

    ScopedDeviceContextState(const ScopedDeviceContextState&) = delete;
    ScopedDeviceContextState& operator=(const ScopedDeviceContextState&) = delete;

private:
    ID3D11DeviceContext1 *m_context = nullptr;
    Microsoft::WRL::ComPtr<ID3DDeviceContextState> m_previousState;
    bool m_active = false;
};

} // namespace

SluggerManager::SluggerManager() { DX11Hook::registerMod(this); }

SluggerManager::~SluggerManager() {
    DX11Hook::unregisterMod(this);

    std::lock_guard<std::mutex> lock(queueMutex_);
    _2dQueue_.clear();
    billboardQueue_.clear();
    worldQueue_.clear();
}

void SluggerManager::drawScreenText(const std::string& text, float x, float y, const Slugger::SlugTextBuildOptions& options, bool centerHorizontal,
                                    bool centerVertical, FontType fontType) {
    if (text.empty() || !IsFinite(x) || !IsFinite(y))
        return;

    std::lock_guard<std::mutex> lock(queueMutex_);

    if (_2dQueue_.size() >= kMaximumQueuedRequests)
        return;

    _2dQueue_.push_back({text, x, y, options, centerHorizontal, centerVertical, fontType});
}

void SluggerManager::drawBillboardText(const std::string& text, UnityEngine_Vector3_o worldPos, float baseScale, float minScale, float maxScale,
                                       const Slugger::SlugTextBuildOptions& options, FontType fontType) {
    if (text.empty() || !IsFinite(worldPos) || !IsFinite(baseScale) || !IsFinite(minScale) || !IsFinite(maxScale))
        return;

    if (baseScale <= 0.0f || maxScale <= 0.0f)
        return;

    if (minScale > maxScale)
        std::swap(minScale, maxScale);

    minScale = std::max(0.0f, minScale);

    auto *mainCamera = Il2Cpp::UnityEngine::Camera::get_main();
    if (!mainCamera)
        return;

    const UnityEngine_Vector3_o screenPosition = Il2Cpp::UnityEngine::Camera::WorldToScreenPoint(mainCamera, worldPos);

    if (!IsFinite(screenPosition))
        return;

    // Do not render stuff that's behind the camera
    if (screenPosition.fields.z <= 0.0f)
        return;

    const float unityHeight = static_cast<float>(Il2Cpp::UnityEngine::Screen::get_height());

    if (!IsFinite(unityHeight) || unityHeight <= 0.0f)
        return;

    float trueDistance = screenPosition.fields.z;

    auto *cameraTransform = Il2Cpp::UnityEngine::Component::get_transform(reinterpret_cast<UnityEngine_Component_o *>(mainCamera));

    if (cameraTransform) {
        const UnityEngine_Vector3_o cameraPosition = Il2Cpp::UnityEngine::Transform::get_position(cameraTransform);

        if (IsFinite(cameraPosition)) {
            const UnityEngine_Vector3_o difference = Subtract(worldPos, cameraPosition);

            const float distanceSquared = LengthSquared(difference);
            if (IsFinite(distanceSquared) && distanceSquared > kEpsilon)
                trueDistance = std::sqrt(distanceSquared);
        }
    }

    std::lock_guard<std::mutex> lock(queueMutex_);

    if (billboardQueue_.size() >= kMaximumQueuedRequests)
        return;

    billboardQueue_.push_back(
        {text, screenPosition.fields.x, unityHeight - screenPosition.fields.y, trueDistance, baseScale, minScale, maxScale, options, fontType});
}

void SluggerManager::drawWorldText(const std::string& text, UnityEngine_Vector3_o position, UnityEngine_Quaternion_o rotation, float scale,
                                   const Slugger::SlugTextBuildOptions& options, bool centerHorizontal, bool centerVertical, FontType fontType) {
    if (text.empty() || !IsFinite(position) || !IsFinite(rotation) || !IsFinite(scale) || scale <= 0.0f)
        return;

    std::lock_guard<std::mutex> lock(queueMutex_);

    if (worldQueue_.size() >= kMaximumQueuedRequests)
        return;

    worldQueue_.push_back({text, position, rotation, scale, options, centerHorizontal, centerVertical, fontType});
}

void SluggerManager::RefreshBackbufferDimensions(IDXGISwapChain *swapChain) {
    if (!swapChain)
        return;

    DXGI_SWAP_CHAIN_DESC swapChainDescription{};
    if (SUCCEEDED(swapChain->GetDesc(&swapChainDescription))) {
        if (swapChainDescription.BufferDesc.Width != 0)
            backbufferWidth_ = static_cast<int>(swapChainDescription.BufferDesc.Width);

        if (swapChainDescription.BufferDesc.Height != 0)
            backbufferHeight_ = static_cast<int>(swapChainDescription.BufferDesc.Height);
    }
}

void SluggerManager::EnsureRenderTarget(IDXGISwapChain *swapChain) {
    if (mainRTV_ || !device_ || !swapChain)
        return;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.ReleaseAndGetAddressOf())))) {

        g.logger->error("SluggerManager::EnsureRenderTarget: "
                        "failed to get the swap-chain back buffer.");
        return;
    }

    D3D11_TEXTURE2D_DESC backBufferDescription{};
    backBuffer->GetDesc(&backBufferDescription);

    if (backBufferDescription.Width != 0)
        backbufferWidth_ = static_cast<int>(backBufferDescription.Width);

    if (backBufferDescription.Height != 0)
        backbufferHeight_ = static_cast<int>(backBufferDescription.Height);

    if (FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, mainRTV_.ReleaseAndGetAddressOf())))
        g.logger->error("SluggerManager::EnsureRenderTarget: "
                        "failed to create the back-buffer render-target view.");
}

void SluggerManager::InitializeSlugger(IDXGISwapChain *swapChain) {
    if (initializationAttempted_ || sluggerInitialized_)
        return;

    initializationAttempted_ = true;

    if (!swapChain) {
        g.logger->error("SluggerManager::InitializeSlugger: swap chain is null.");
        return;
    }

    g.logger->info("Initializing Slugger Manager...");

    try {
        if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(device_.ReleaseAndGetAddressOf()))))
            return;

        device_->GetImmediateContext(context_.ReleaseAndGetAddressOf());
        if (!context_)
            return;

        device_.As(&device1_);
        context_.As(&context1_);

        if (device1_ && context1_) {
            const D3D_FEATURE_LEVEL featureLevel = device_->GetFeatureLevel();
            D3D_FEATURE_LEVEL selectedFeatureLevel{};
            device1_->CreateDeviceContextState(0, &featureLevel, 1, D3D11_SDK_VERSION, __uuidof(ID3D11Device), &selectedFeatureLevel,
                                               sluggerContextState_.ReleaseAndGetAddressOf());
        }

        // Get all printable characters
        static constexpr auto printableAsciiArray = []() {
            std::array<char, 95> arr{};
            for (int i = 0; i < 95; ++i)
                arr[i] = static_cast<char>(i + 32);
            return arr;
        }();
        constexpr std::string_view asciiView{printableAsciiArray.data(), printableAsciiArray.size()};

        auto loadFontSlot = [this, asciiView](FontSlot& slot, const std::string& path, const char *name) {
            std::ifstream fontFile(path, std::ios::binary | std::ios::ate);
            if (!fontFile.is_open()) {
                g.logger->error(std::string("SluggerManager::InitializeSlugger: failed to open ") + name + " font file.");
                return false;
            }

            const std::streamsize fileSize = fontFile.tellg();
            if (fileSize <= 0)
                return false;

            fontFile.seekg(0, std::ios::beg);
            slot.fontData.resize(static_cast<std::size_t>(fileSize));

            if (!fontFile.read(reinterpret_cast<char *>(slot.fontData.data()), fileSize))
                return false;

            slot.builder = std::make_unique<Slugger::SlugFontBuilder>(slot.fontData.data());
            slot.builder->BakeGlyphsFromString(asciiView);
            slot.font = slot.builder->CreateFont(device_.Get());

            return slot.font != nullptr;
        };

        // Load proportional font
        if (!loadFontSlot(proportionalFont_, GetArialFontPath(), "Arial")) {
            g.logger->error("SluggerManager::InitializeSlugger: failed to load proportional font.");
            return;
        }

        // Load monospaced font
        if (!loadFontSlot(monospaceFont_, GetMonospacedFontPath(), "Monospace"))
            g.logger->warn("SluggerManager::InitializeSlugger: monospaced font failed to load, falling back to proportional font.");

        renderer_ = std::make_unique<Slugger::SlugRenderer>(device_.Get(), context_.Get());

        RefreshBackbufferDimensions(swapChain);
        EnsureRenderTarget(swapChain);

        sluggerInitialized_ = device_ && context_ && proportionalFont_.font && renderer_;

        if (sluggerInitialized_)
            g.logger->info("Slugger Manager initialized successfully.");
    } catch (const std::exception& exception) {
        g.logger->error(std::string("SluggerManager::InitializeSlugger: exception: ") + exception.what());
        sluggerInitialized_ = false;
    }
}

void SluggerManager::onDX11Present(IDXGISwapChain *swapChain, UINT /*syncInterval*/, UINT /*flags*/) {
    if (!sluggerInitialized_ && !initializationAttempted_)
        InitializeSlugger(swapChain);

    std::vector<TextRequest2D> render2D;
    std::vector<BillboardRenderRequest> renderBillboard;
    std::vector<WorldRenderRequest> renderWorld;

    {
        std::lock_guard<std::mutex> lock(queueMutex_);

        render2D = std::move(_2dQueue_);
        renderBillboard = std::move(billboardQueue_);
        renderWorld = std::move(worldQueue_);

        _2dQueue_.clear();
        billboardQueue_.clear();
        worldQueue_.clear();
    }

    if (!sluggerInitialized_ || !device_ || !context_ || !proportionalFont_.font || !proportionalFont_.builder || !renderer_ || !swapChain)
        return;

    RefreshBackbufferDimensions(swapChain);
    EnsureRenderTarget(swapChain);

    if (!mainRTV_ || backbufferWidth_ <= 0 || backbufferHeight_ <= 0)
        return;

    const float unityScreenWidth = static_cast<float>(Il2Cpp::UnityEngine::Screen::get_width());
    const float unityScreenHeight = static_cast<float>(Il2Cpp::UnityEngine::Screen::get_height());

    if (!IsFinite(unityScreenWidth) || !IsFinite(unityScreenHeight) || unityScreenWidth <= 0.0f || unityScreenHeight <= 0.0f)
        return;

    ScopedDeviceContextState contextStateGuard(context1_.Get(), sluggerContextState_.Get());

    ID3D11RenderTargetView *renderTargets[] = {mainRTV_.Get()};

    context_->OMSetRenderTargets(1, renderTargets, nullptr);

    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(backbufferWidth_);
    viewport.Height = static_cast<float>(backbufferHeight_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    context_->RSSetViewports(1, &viewport);

    // Render world-space text
    if (!renderWorld.empty()) {
        auto *mainCamera = Il2Cpp::UnityEngine::Camera::get_main();

        if (mainCamera) {
            auto *cameraTransform = Il2Cpp::UnityEngine::Component::get_transform(reinterpret_cast<UnityEngine_Component_o *>(mainCamera));

            if (cameraTransform) {
                const UnityEngine_Vector3_o cameraPosition = Il2Cpp::UnityEngine::Transform::get_position(cameraTransform);
                const UnityEngine_Vector3_o rawCameraForward = Il2Cpp::UnityEngine::Transform::get_forward(cameraTransform);
                const UnityEngine_Vector3_o rawCameraUp = Il2Cpp::UnityEngine::Transform::get_up(cameraTransform);
                const UnityEngine_Vector3_o fallbackForward{{0.0f, 0.0f, 1.0f}};
                const UnityEngine_Vector3_o fallbackUp{{0.0f, 1.0f, 0.0f}};
                const UnityEngine_Vector3_o cameraForward = Normalize(rawCameraForward, fallbackForward);
                const UnityEngine_Vector3_o cameraUp = Normalize(rawCameraUp, fallbackUp);

                float fieldOfView = Il2Cpp::UnityEngine::Camera::get_fieldOfView(mainCamera);
                float nearClip = Il2Cpp::UnityEngine::Camera::get_nearClipPlane(mainCamera);
                float farClip = Il2Cpp::UnityEngine::Camera::get_farClipPlane(mainCamera);

                if (!IsFinite(fieldOfView))
                    fieldOfView = 60.0f;
                if (!IsFinite(nearClip))
                    nearClip = 0.01f;
                if (!IsFinite(farClip))
                    farClip = 1000.0f;

                fieldOfView = std::clamp(fieldOfView, 1.0f, 179.0f);
                nearClip = std::max(nearClip, 0.001f);
                farClip = std::max(farClip, nearClip + 0.001f);

                const float cameraAspect = unityScreenWidth / unityScreenHeight;

                const DirectX::XMMATRIX viewMatrix =
                    DirectX::XMMatrixLookToLH(DirectX::XMVectorSet(cameraPosition.fields.x, cameraPosition.fields.y, cameraPosition.fields.z, 1.0f),
                                              DirectX::XMVectorSet(cameraForward.fields.x, cameraForward.fields.y, cameraForward.fields.z, 0.0f),
                                              DirectX::XMVectorSet(cameraUp.fields.x, cameraUp.fields.y, cameraUp.fields.z, 0.0f));

                const DirectX::XMMATRIX projectionMatrix =
                    DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fieldOfView), cameraAspect, nearClip, farClip);

                const DirectX::XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;

                for (const auto& request : renderWorld) {
                    if (request.text.empty() || !IsFinite(request.position) || !IsFinite(request.rotation) || !IsFinite(request.scale) ||
                        request.scale <= 0.0f) {
                        continue;
                    }

                    FontSlot *slot = GetFontSlot(request.fontType);
                    const float verticalMetrics = static_cast<float>(std::max(1, slot->builder->GetFontVMetrics()));

                    Slugger::SlugTextBuildOptions options = request.options;
                    options.scale = request.scale / verticalMetrics;
                    options.centerOrigin = false;

                    auto vertices = slot->builder->BuildTextVertices(request.text, options);
                    if (vertices.empty())
                        continue;

                    TextBounds bounds{};
                    if (!CalculateTextBounds(vertices, bounds))
                        continue;

                    float horizontalOrigin = bounds.minX;
                    float verticalOrigin = bounds.minY;

                    if (request.centerHorizontal)
                        horizontalOrigin = (bounds.minX + bounds.maxX) * 0.5f;
                    if (request.centerVertical)
                        verticalOrigin = (bounds.minY + bounds.maxY) * 0.5f;

                    // Build model matrix
                    DirectX::XMVECTOR qRot =
                        DirectX::XMVectorSet(request.rotation.fields.x, request.rotation.fields.y, request.rotation.fields.z, request.rotation.fields.w);
                    DirectX::XMVECTOR vPos = DirectX::XMVectorSet(request.position.fields.x, request.position.fields.y, request.position.fields.z, 1.0f);

                    // Flip Y-scale so font renders upright
                    DirectX::XMMATRIX modelMatrix =
                        DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMVectorZero(), qRot, vPos);

                    // Pass combined MVP matrix to GPU
                    DirectX::XMMATRIX mvp = modelMatrix * viewProjectionMatrix;
                    renderer_->BeginRender(*slot->font, static_cast<int>(unityScreenWidth), static_cast<int>(unityScreenHeight), mvp);

                    // Apply centering offsets
                    for (auto& vertex : vertices) {
                        vertex.pos.x -= horizontalOrigin;
                        vertex.pos.y -= verticalOrigin;
                        vertex.pos.z = 0.0f;
                        vertex.pos.w = 1.0f;
                    }

                    renderer_->DrawVertices(vertices);
                }
            }
        }
    }

    const DirectX::XMMATRIX orthographicProjection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, unityScreenWidth, unityScreenHeight, 0.0f, 0.0f, 1.0f);
    const Slugger::SlugFont *activeFont = nullptr;

    // Render billboard requests
    for (const auto& request : renderBillboard) {
        if (request.text.empty() || !IsFinite(request.screenX) || !IsFinite(request.screenY) || !IsFinite(request.depth) || !IsFinite(request.baseScale) ||
            !IsFinite(request.minScale) || !IsFinite(request.maxScale)) {
            continue;
        }

        if (request.screenX < -64.0f || request.screenX > unityScreenWidth + 64.0f || request.screenY < -64.0f || request.screenY > unityScreenHeight + 64.0f)
            continue;

        FontSlot *slot = GetFontSlot(request.fontType);
        const float verticalMetrics = static_cast<float>(std::max(1, slot->builder->GetFontVMetrics()));
        const float baseFontScale = request.baseScale / verticalMetrics;
        const float minimumFontScale = request.minScale / verticalMetrics;
        const float maximumFontScale = request.maxScale / verticalMetrics;
        const float distanceScale = baseFontScale * (3.0f / std::max(request.depth, 1.0f));
        const float finalScale = std::clamp(distanceScale, minimumFontScale, maximumFontScale);

        Slugger::SlugTextBuildOptions options = request.options;
        options.scale = finalScale;
        options.centerOrigin = false;

        auto vertices = slot->builder->BuildTextVertices(request.text, options);
        if (vertices.empty())
            continue;

        TextBounds bounds{};
        if (!CalculateTextBounds(vertices, bounds))
            continue;

        const float textWidth = bounds.maxX - bounds.minX;
        const float textHeight = bounds.maxY - bounds.minY;
        const float offsetX = request.screenX - textWidth * 0.5f - bounds.minX;
        const float offsetY = request.screenY - textHeight - bounds.minY;

        for (auto& vertex : vertices) {
            vertex.pos.x += offsetX;
            vertex.pos.y += offsetY;
        }

        if (activeFont != slot->font.get()) {
            renderer_->BeginRender(*slot->font, static_cast<int>(unityScreenWidth), static_cast<int>(unityScreenHeight), orthographicProjection);
            activeFont = slot->font.get();
        }

        renderer_->DrawVertices(vertices);
    }

    // Render direct screen-space text
    for (const auto& request : render2D) {
        if (request.text.empty() || !IsFinite(request.x) || !IsFinite(request.y))
            continue;

        FontSlot *slot = GetFontSlot(request.fontType);
        const float verticalMetrics = static_cast<float>(std::max(1, slot->builder->GetFontVMetrics()));

        Slugger::SlugTextBuildOptions options = request.options;
        if (options.scale > 0.0f)
            options.scale /= verticalMetrics;

        auto vertices = slot->builder->BuildTextVertices(request.text, options);
        if (vertices.empty())
            continue;

        float offsetX = request.x;
        float offsetY = request.y;

        if (request.centerHorizontal || request.centerVertical) {
            TextBounds bounds{};
            if (CalculateTextBounds(vertices, bounds)) {
                if (request.centerHorizontal)
                    offsetX -= (bounds.minX + bounds.maxX) * 0.5f;
                if (request.centerVertical)
                    offsetY -= (bounds.minY + bounds.maxY) * 0.5f;
            }
        } else {
            auto refVertices = slot->builder->BuildTextVertices("M|", options);
            TextBounds refBounds{};
            if (CalculateTextBounds(refVertices, refBounds))
                offsetY -= refBounds.minY;
        }

        for (auto& vertex : vertices) {
            vertex.pos.x += offsetX;
            vertex.pos.y += offsetY;
        }

        if (activeFont != slot->font.get()) {
            renderer_->BeginRender(*slot->font, static_cast<int>(unityScreenWidth), static_cast<int>(unityScreenHeight), orthographicProjection);
            activeFont = slot->font.get();
        }

        renderer_->DrawVertices(vertices);
    }
}

void SluggerManager::onDX11ResizeBuffers(IDXGISwapChain * /*swapChain*/, UINT /*bufferCount*/, UINT width, UINT height, DXGI_FORMAT /*newFormat*/,
                                         UINT /*swapChainFlags*/) {
    // The old RTV must be released before the swap chain can resize it buffers
    // It will be recreated on the next Present
    mainRTV_.Reset();

    backbufferWidth_ = static_cast<int>(width);
    backbufferHeight_ = static_cast<int>(height);
}

ModInfo sluggerManagerInfo{"Slugger Manager", false, []() { return std::make_unique<SluggerManager>(); }};

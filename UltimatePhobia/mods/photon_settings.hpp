#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "il2cpp_api_cpp.hpp"
#include "signaling_client.hpp"

#include <string_view>
#include <optional>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <luxon/enet_peer.hpp>

namespace server {
class ServerManager;
class HandlerBase;
} // namespace server

class GameServerProxy;

class PhotonSettings final : public Mod {
    struct P2PSettings {        
        char stun_server_host[64] = "stun.l.google.com";
        uint16_t stun_server_port = 19302;

        char signaling_base[64] = "http://photon.tuxifan.net:5059";

        bool enable_ipv6 = true;
    } p2p_settings;

    struct GameServerProxy {
        luxon::enet::EnetEndpoint stun_server_ep;
        luxon::enet::UdpSocket socket;

        GameServerProxy(const PhotonSettings::P2PSettings& stun);
        void run_once();
        void sendStunKeepalive();

        void request_join(const luxon::enet::EnetEndpoint& remote_server_ep);
        void reset();

        bool proxyReady() const noexcept { return proxy_ready.load(); }
        bool hasActiveServer() const noexcept { return server_active.load(); }
        bool isClientNotified() const noexcept { return client_notified_flag.load(); }
        bool consumeJoinReady() noexcept { return join_ready.exchange(false); }
        std::optional<luxon::enet::EnetEndpoint> takeProxyEndpoint() const {
            std::lock_guard lock(state_mutex);
            return proxy_ep;
        }

    private:
        mutable std::mutex state_mutex;
        std::optional<luxon::enet::EnetEndpoint> proxy_ep, client_ep, server_ep;
        bool client_notified = false;
        bool kick_server_ep = false;

        std::atomic_bool proxy_ready{false};
        std::atomic_bool server_active{false};
        std::atomic_bool client_notified_flag{false};
        std::atomic_bool join_ready{false};
    };

    struct ClientSettings {
        char AppIdRealtime[64];
        bool override_AppIdRealtime;
        char AppIdChat[64];
        bool override_AppIdChat;
        char AppIdVoice[64];
        bool override_AppIdVoice;
        char AppVersion[64];
        bool override_AppVersion;
        bool UseNameServer;
        bool override_UseNameServer;
        char FixedRegion[64];
        bool override_FixedRegion;
        char BestRegionSummaryFromStorage[64];
        bool override_BestRegionSummaryFromStorage;
        char Server[64];
        bool override_Server;
        int Port;
        bool override_Port;
        char ProxyServer[64];
        bool override_ProxyServer;
        int Protocol;
        bool override_Protocol;
        bool EnableProtocolFallback;
        bool override_EnableProtocolFallback;
        int AuthMode;
        bool override_AuthMode;
        bool EnableLobbyStatistics;
        bool override_EnableLobbyStatistics;
        int NetworkLogging;
        bool override_NetworkLogging;

        bool override_SendRate;
        int SendRate;
        bool override_SerializationRate;
        int SerializationRate;
    } client_settings;

    std::unique_ptr<server::ServerManager> serman;
    std::optional<GameServerProxy> client_proxy;
    std::optional<signaling::SignalingClient> signaling;
    luxon::enet::EnetEndpoint server_stun_binding_ep;
    Il2Cpp::API::Class photonNetworkClass;

    std::optional<std::jthread> server_manager_thread;
    std::optional<std::jthread> client_proxy_thread;
    server::HandlerBase *pending_join_handler{};
    std::atomic_size_t current_game_id{0};

public:
    GameHook photonNetwork$$ConnectUsingSettingsHook;

    PhotonSettings();

    void uiUpdate() override;

    void setAppIdRealtime(std::string_view value);
    void setServer(std::string_view value);
    void fromIl2CppClass(const Photon_Realtime_AppSettings_Fields& o);
    void toIl2CppClass(Photon_Realtime_AppSettings_Fields& o);

    void startP2P();
    void stopP2P();

    void startPollingProxy();
    void startPollingServer(luxon::enet::EnetServer& server);

private:
    void flushPendingJoinResponse();
};

extern ModInfo photonSettingsInfo;

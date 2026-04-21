#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "signaling_client.hpp"

#include <string_view>
#include <optional>
#include <memory>
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
        std::optional<luxon::enet::EnetEndpoint> proxy_ep, client_ep, server_ep;
        luxon::enet::UdpSocket socket;
        server::HandlerBase *handler{};
        bool client_notified = false;

        GameServerProxy(const PhotonSettings::P2PSettings& stun);
        void run_once();
        void reset() {
            handler = nullptr;
            server_ep.reset();
            client_ep.reset();
            client_notified = false;
        }
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
    } client_settings;

    std::unique_ptr<server::ServerManager> serman;
    std::optional<GameServerProxy> client_proxy;
    std::optional<signaling::SignalingClient> signaling;
    luxon::enet::EnetEndpoint server_stun_binding_ep;

    std::size_t current_game_id{};

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
};

extern ModInfo photonSettingsInfo;

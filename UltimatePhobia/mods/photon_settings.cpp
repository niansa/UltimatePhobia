#include "photon_settings.hpp"
#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"

#include <string>
#include <string_view>
#include <format>
#include <charconv>
#include <utility>
#include <stdexcept>
#include <cstring>
#include <imgui.h>
#include <luxon/server/server_manager.hpp>
#include <luxon/server/logger.hpp>
#include <luxon/server/handler_masterserver.hpp>
#include <luxon/server/handler_gameserver.hpp>
#include <luxon/server/string_hash.hpp>
#include <luxon/common_codes.hpp>

namespace {
std::pair<std::string, uint16_t> endpoint_split_addr(const luxon::enet::EnetEndpoint& endpoint) {
    const std::string stun_binding_addr = endpoint.to_string();
    const auto colon_pos = stun_binding_addr.rfind(':');

    const std::string_view stun_binding_host = std::string_view{stun_binding_addr}.substr(0, colon_pos);
    uint16_t stun_binding_port = 0;
    if (colon_pos != std::string::npos)
        std::from_chars(stun_binding_addr.data() + colon_pos + 1, stun_binding_addr.data() + stun_binding_addr.size(), stun_binding_port);

    return {std::string(stun_binding_host), stun_binding_port};
}
} // namespace

#define CONVLIST                                                                                                                                               \
    CONVSTRFIELD(AppIdRealtime);                                                                                                                               \
    CONVSTRFIELD(AppIdChat);                                                                                                                                   \
    CONVSTRFIELD(AppIdVoice);                                                                                                                                  \
    CONVSTRFIELD(AppVersion);                                                                                                                                  \
    CONVBOOLFIELD(UseNameServer);                                                                                                                              \
    CONVSTRFIELD(FixedRegion);                                                                                                                                 \
    CONVSTRFIELD(BestRegionSummaryFromStorage);                                                                                                                \
    CONVSTRFIELD(Server);                                                                                                                                      \
    CONVINTFIELD(Port);                                                                                                                                        \
    CONVSTRFIELD(ProxyServer);                                                                                                                                 \
    CONVINTFIELD(Protocol);                                                                                                                                    \
    CONVBOOLFIELD(EnableProtocolFallback);                                                                                                                     \
    CONVINTFIELD(AuthMode);                                                                                                                                    \
    CONVBOOLFIELD(EnableLobbyStatistics);                                                                                                                      \
    CONVINTFIELD(NetworkLogging);

using namespace Il2Cpp::CppInterop;

static bool photonNetwork$$ConnectUsingSettingsFnc(Photon_Realtime_AppSettings_o *appSettings, bool startInOfflineMode, const MethodInfo *method) {
    auto self = photonSettingsInfo.get<PhotonSettings>();
    self->fromIl2CppClass(appSettings->fields);
    self->toIl2CppClass(appSettings->fields);
    GameHookRelease GHR(self->photonNetwork$$ConnectUsingSettingsHook);
    return self->photonNetwork$$ConnectUsingSettingsHook.getFunction<decltype(photonNetwork$$ConnectUsingSettingsFnc)>()(appSettings, startInOfflineMode,
                                                                                                                         method);
}

PhotonSettings::PhotonSettings()
    : photonNetwork$$ConnectUsingSettingsHook(GameHook::safeCreateOrPanic(
          photonSettingsInfo, Il2Cpp::Photon::Pun::PhotonNetwork::ConnectUsingSettings_getPtr<Photon_Realtime_AppSettings_o *, bool>(),
          reinterpret_cast<void *>(photonNetwork$$ConnectUsingSettingsFnc))) {
    memset(&client_settings, 0, sizeof(client_settings));
}

void PhotonSettings::uiUpdate() {
    if (serman)
        serman->run_once();
    if (client_proxy)
        client_proxy->run_once();

    using namespace ImGui;
    Begin("Photon Settings");

    BeginDisabled(!!serman);
#define OVERRIDER(field)                                                                                                                                       \
    PushID("override_" #field);                                                                                                                                \
    Checkbox("Override", &client_settings.override_##field);                                                                                                   \
    PopID();                                                                                                                                                   \
    SameLine();                                                                                                                                                \
    BeginDisabled(!client_settings.override_##field);
#define CONVBOOLFIELD(field)                                                                                                                                   \
    OVERRIDER(field) Checkbox(#field, &client_settings.field);                                                                                                 \
    EndDisabled()
#define CONVINTFIELD(field)                                                                                                                                    \
    OVERRIDER(field) InputInt(#field, &client_settings.field);                                                                                                 \
    EndDisabled()
#define CONVSTRFIELD(field)                                                                                                                                    \
    OVERRIDER(field)                                                                                                                                           \
    InputText(#field, client_settings.field, sizeof(client_settings.field));                                                                                   \
    EndDisabled()
    CONVLIST
#undef OVERRIDER
#undef CONVBOOLFIELD
#undef CONVINTFIELD
#undef CONVSTRFIELD
    EndDisabled();

    Separator();

    if (!serman) {
        if (Button("Start P2P Service"))
            startP2P();
    } else {
        if (Button("Stop P2P Service"))
            stopP2P();
    }
    End();
}

#define DEFINE_SETTER(field)                                                                                                                                   \
    void PhotonSettings::set##field(std::string_view value) {                                                                                                  \
        if (value.size() >= sizeof(client_settings.field))                                                                                                     \
            value = value.substr(0, sizeof(client_settings.field) - 1);                                                                                        \
        memcpy(client_settings.field, value.data(), value.size());                                                                                             \
        client_settings.field[value.size()] = '\0';                                                                                                            \
        client_settings.override_##field = true;                                                                                                               \
    }

DEFINE_SETTER(Server)

void PhotonSettings::fromIl2CppClass(const Photon_Realtime_AppSettings_Fields& o) {
#define OVERRIDER(field) if (!client_settings.override_##field)
#define CONVBOOLFIELD(field) OVERRIDER(field) client_settings.field = o.field
#define CONVINTFIELD(field) OVERRIDER(field) client_settings.field = o.field
#define CONVSTRFIELD(field) OVERRIDER(field) ToCString(o.field, client_settings.field, sizeof(client_settings.field))
    CONVLIST
#undef OVERRIDER
#undef CONVBOOLFIELD
#undef CONVINTFIELD
#undef CONVSTRFIELD
}

void PhotonSettings::toIl2CppClass(Photon_Realtime_AppSettings_Fields& o){
#define CONVBOOLFIELD(field) o.field = client_settings.field
#define CONVINTFIELD(field) o.field = client_settings.field
#define CONVSTRFIELD(field) o.field = ToCsString(client_settings.field)
    CONVLIST
#undef CONVBOOLFIELD
#undef CONVINTFIELD
#undef CONVSTRFIELD
}

void PhotonSettings::startP2P() {
    using namespace server;

    // Set initial settings
    setServer("127.0.0.1");

    // Create signaling
    signaling.emplace(p2p_settings.signaling_base);

    // Create proxy
    client_proxy.emplace(p2p_settings);

    // Configure luxon server logger
    custom_log_sink = [](log_level level, std::string message) {
        spdlog::level::level_enum spdlog_level;
        switch (level) {
        case server::log_level::trace:
            spdlog_level = spdlog::level::trace;
            break;
        case server::log_level::debug:
            spdlog_level = spdlog::level::debug;
            break;
        case server::log_level::info:
            spdlog_level = spdlog::level::info;
            break;
        case server::log_level::warn:
            spdlog_level = spdlog::level::warn;
            break;
        case server::log_level::err:
            spdlog_level = spdlog::level::err;
            break;
        case server::log_level::critical:
            spdlog_level = spdlog::level::critical;
            break;
        case server::log_level::off:
            spdlog_level = spdlog::level::off;
            break;
        default:
            // Help compiler optimize this switch away if possible
            spdlog_level = static_cast<spdlog::level::level_enum>(level);
        }
        g.logger->log(spdlog_level, "Luxon: {}", message);
    };

    // Create luxon server manager configuration
    ServerManagerConfig config;
    config.max_connections = 4;
    config.max_game_peers = 4;
    config.servers.emplace_back(ServerConfig{ServerType::NameServer, 5058});
    config.servers.emplace_back(ServerConfig{ServerType::MasterServer, 5055});
    config.servers.emplace_back(ServerConfig{ServerType::GameServer, 5056, true, p2p_settings.stun_server_host, p2p_settings.stun_server_port});
    config.add_endpoint(ServerType::MasterServer, ServerProtocol::UDP, "127.0.0.1:5055");
    config.add_endpoint(ServerType::GameServer, ServerProtocol::UDP, "127.0.0.1:5056");

    // Create luxon server manager
    serman = std::make_unique<server::ServerManager>(std::move(config));

    // Set up special callbacks
    for (const auto& [port, server] : serman->get_servers()) {
        // Catch STUN binding upon completion
        server->on_stun_bind = [this, server](luxon::enet::EnetEndpoint&& ep) {
            server_stun_binding_ep = ep;
            startPollingServer(*server);
            g.logger->info("Got server STUN binding: {}", ep.to_string());
        };
    }

    // Set up hookpoints
    serman->hookpoints.MasterServer_HandleOperationRequest_CreateGame = [this](MasterServerHandler& handler, const std::string& game_id) -> bool {
        // Only deal with default lobby
        if (handler.get_joined_lobby_name() != "")
            return false;

        // Get all the details we need to publish
        const auto game_id_hash = server::string_hash(game_id);
        const auto [stun_binding_host, stun_binding_port] = endpoint_split_addr(server_stun_binding_ep);

        g.logger->info("Attempting to P2P create game '{}' (Hash: {})", game_id, game_id_hash);

        // Signal host
        try {
            signaling->register_host(game_id_hash, stun_binding_host, stun_binding_port);
        } catch (const signaling::SignalingError& e) {
            const ser::OperationResponseMessage resp{.operation_code = luxon::OpCodes::Matchmaking::CreateGame,
                                                     .return_code = luxon::ErrorCodes::Matchmaking::ServerCheckFailed,
                                                     .debug_message = std::format("Signaling error: {}", e.what())};
            handler.send(handler.get_peer()->protocol->Serialize(resp));
            return true;
        }

        // Continue game creation flow
        current_game_id = game_id_hash;

        g.logger->info("P2P game create done!", game_id, game_id_hash);
        return false;
    };

    serman->hookpoints.MasterServer_HandleOperationRequest_JoinGame = [this](MasterServerHandler& handler, const std::string& game_id, bool or_create) -> bool {
        // Only deal with default lobby
        if (handler.get_joined_lobby_name() != "")
            return false;

        // Reset proxy
        client_proxy->reset();

        // Make sure proxy has completed NAT punch
        if (!client_proxy->proxy_ep) {
            const ser::OperationResponseMessage resp{.operation_code = luxon::OpCodes::Matchmaking::JoinGame,
                                                     .return_code = luxon::ErrorCodes::Matchmaking::ServerCheckFailed,
                                                     .debug_message = "Proxy NAT punch not completed"};
            handler.send(handler.get_peer()->protocol->Serialize(resp));
            return true;
        }

        // Get all the details we need to publish
        const std::size_t game_id_hash = server::string_hash(game_id);
        const auto [stun_binding_host, stun_binding_port] = endpoint_split_addr(*client_proxy->proxy_ep);

        g.logger->info("Attempting to P2P join game '{}' (Hash: {})", game_id, game_id_hash);

        // Signal game join
        signaling::Endpoint server_signal_ep;
        try {
            if (const auto server_signal_ep_opt = signaling->join_room(game_id_hash, stun_binding_host, stun_binding_port)) {
                server_signal_ep = *server_signal_ep_opt;
            } else {
                const ser::OperationResponseMessage resp{.operation_code = luxon::OpCodes::Matchmaking::JoinGame,
                                                         .return_code = luxon::ErrorCodes::Matchmaking::ServerCheckFailed,
                                                         .debug_message = "Game not found"};
                handler.send(handler.get_peer()->protocol->Serialize(resp));
                return true;
            }
        } catch (const signaling::SignalingError& e) {
            const ser::OperationResponseMessage resp{.operation_code = luxon::OpCodes::Matchmaking::JoinGame,
                                                     .return_code = luxon::ErrorCodes::Matchmaking::ServerCheckFailed,
                                                     .debug_message = std::format("Signaling error: {}", e.what())};
            handler.send(handler.get_peer()->protocol->Serialize(resp));
            return true;
        }

        // Turn signaling endpoint into enet endpoint
        const auto server_ep_opt = enet::EnetEndpoint::from(server_signal_ep.ip.c_str(), server_signal_ep.port);
        if (!server_ep_opt) {
            const ser::OperationResponseMessage resp{.operation_code = luxon::OpCodes::Matchmaking::JoinGame,
                                                     .return_code = luxon::ErrorCodes::Matchmaking::ServerCheckFailed,
                                                     .debug_message = std::format("Signaling error: Got invalid IP address: {}", server_signal_ep.ip)};
            handler.send(handler.get_peer()->protocol->Serialize(resp));
            return true;
        }
        client_proxy->server_ep = server_ep_opt;

        // Open joiner-side NAT toward host server
        client_proxy->socket.send_to(reinterpret_cast<const uint8_t *>(""), 1, *client_proxy->server_ep);

        // Treat this request as handled for now, we'll respond later
        client_proxy->handler = &handler;

        g.logger->info("P2P game join pending...");
        return true;
    };

    startPollingProxy();
}

void PhotonSettings::stopP2P() {
    serman.reset();
    client_proxy.reset();
}

void PhotonSettings::startPollingProxy() {
    if (!serman || !client_proxy)
        return;

    // STUN keepalive
    if (!client_proxy->socket.send_to(reinterpret_cast<const uint8_t *>(""), 1, client_proxy->stun_server_ep))
        g.logger->warn("Failed to keep alive proxy STUN binding");

    serman->add_scheduled_task(16000, std::bind(&PhotonSettings::startPollingProxy, this));
}

void PhotonSettings::startPollingServer(luxon::enet::EnetServer& server) {
    if (current_game_id && serman->get_connection_count() != 0) {
        try {
            // Allow clients in
            for (const auto& client : signaling->poll_room(current_game_id).clients) {
                if (const auto client_ep_opt = luxon::enet::EnetEndpoint::from(client.ip.c_str(), client.port)) {
                    server.socket().send_to(reinterpret_cast<const uint8_t *>(""), 1, *client_ep_opt);
                    g.logger->info("P2P Hello to {}", client_ep_opt->to_string());
                }
            }
        } catch (const signaling::SignalingError& e) {
            g.logger->error("Failed to poll P2P signaling server: {}", e.what());
        }
    } else {
        current_game_id = 0;
    }

    serman->add_scheduled_task(12000, std::bind(&PhotonSettings::startPollingServer, this, std::ref(server)));
}

PhotonSettings::GameServerProxy::GameServerProxy(const PhotonSettings::P2PSettings& p2p_settings) {
    socket.bind_any(5059);
    socket.set_nonblocking(true);

    // Start STUN binding request
    const auto stun_server_ep_opt = socket.lookup_hostname(p2p_settings.stun_server_host, p2p_settings.stun_server_port);
    if (stun_server_ep_opt) {
        stun_server_ep = *stun_server_ep_opt;
        if (!socket.send_stun_binding_request(stun_server_ep))
            throw std::runtime_error("Failed to send STUN binding request");
    } else {
        throw std::runtime_error("Failed to resolve STUN server hostname");
    }
}

void PhotonSettings::GameServerProxy::run_once() {
    // Receive datagram
    luxon::enet::EnetEndpoint from_ep;
    luxon::enet::DatagramBuffer datagram_buf;
    size_t datagram_len = 0;
    if (!(datagram_len = socket.recv_from(datagram_buf.data(), datagram_buf.size(), from_ep)))
        return;

    const luxon::enet::DatagramView datagram{datagram_buf.begin(), datagram_buf.begin() + datagram_len};

    // Get proxy STUN binding response
    if (!proxy_ep && from_ep == stun_server_ep) {
        if (const auto proxy_ep_opt = socket.parse_stun_binding_response(datagram)) {
            proxy_ep = *proxy_ep_opt;
            g.logger->info("Got proxy STUN binding: {}", proxy_ep->to_string());
        }
        return;
    }

    // Stop here if we haven't processed JoinGame request yet
    if (!handler || !server_ep)
        return;

    // Handle incoming packet from remote server
    if (from_ep == *server_ep) {
        // The server has successfully punched its NAT
        if (!client_notified) {
            using namespace luxon;
            ser::OperationResponseMessage resp{.operation_code = OpCodes::Matchmaking::JoinGame, .return_code = ErrorCodes::Core::Ok};
            resp.parameters[DictKeyCodes::LoadBalancing::Address] = "127.0.0.1:5059";
            resp.parameters[DictKeyCodes::LoadBalancing::Token] = std::monostate{}; // Let client authenticate from scratch
            handler->send(handler->get_peer()->protocol->Serialize(resp));

            g.logger->info("P2P game join done!");
            g.logger->info("Server NAT hole punched. Instructing local client to connect.");
            client_notified = true;
        }

        // Forward server's packet to local client
        if (client_ep)
            socket.send_to(datagram.data(), datagram.size(), *client_ep);

        return;
    }

    // Handle incoming packet from local client
    if (!client_ep) {
        client_ep = from_ep;
        g.logger->info("Got local client endpoint: {}", client_ep->to_string());
    }

    // Forward client's packet to remote server
    if (client_ep && from_ep == *client_ep)
        socket.send_to(datagram.data(), datagram.size(), *server_ep);
}

ModInfo photonSettingsInfo {
    "Photon Settings",
    false,
    []() { return std::make_unique<PhotonSettings>(); },
    []() {
        auto mod = photonSettingsInfo.get<PhotonSettings>();
        mod->setServer("photon.tuxifan.net");
    }
};

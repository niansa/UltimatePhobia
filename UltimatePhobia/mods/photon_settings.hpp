#pragma once

#include "base.hpp"
#include "game_hook.hpp"

#include <string_view>

class PhotonSettings final : public Mod {
    struct Settings {
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
    } settings;

public:
    GameHook photonNetwork$$ConnectUsingSettingsHook;

    PhotonSettings();

    void uiUpdate() override;

    void setAppIdRealtime(std::string_view value);
    void setServer(std::string_view value);
    void fromIl2CppClass(const Photon_Realtime_AppSettings_Fields& o);
    void toIl2CppClass(Photon_Realtime_AppSettings_Fields& o);
};

extern ModInfo photonSettingsInfo;

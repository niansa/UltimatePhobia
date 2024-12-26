#include "photon_settings.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"

#include <cstring>
#include <imgui.h>

#define CONVLIST \
    CONVSTRFIELD(AppIdRealtime); \
    CONVSTRFIELD(AppIdChat); \
    CONVSTRFIELD(AppIdVoice); \
    CONVSTRFIELD(AppVersion); \
    CONVBOOLFIELD(UseNameServer); \
    CONVSTRFIELD(FixedRegion); \
    CONVSTRFIELD(BestRegionSummaryFromStorage); \
    CONVSTRFIELD(Server); \
    CONVINTFIELD(Port); \
    CONVSTRFIELD(ProxyServer); \
    CONVINTFIELD(Protocol); \
    CONVBOOLFIELD(EnableProtocolFallback); \
    CONVINTFIELD(AuthMode); \
    CONVBOOLFIELD(EnableLobbyStatistics); \
    CONVINTFIELD(NetworkLogging);




static bool photonNetworkConnectUsingSettingsFnc(Photon_Realtime_AppSettings_o* appSettings, bool startInOfflineMode, const MethodInfo* method) {
    auto self = photonSettingsInfo.get<PhotonSettings>();
    self->fromIl2CppClass(appSettings->fields);
    self->toIl2CppClass(appSettings->fields);
    GameHookRelease GHR(self->photonNetworkConnectUsingSettingsHook);
    return self->photonNetworkConnectUsingSettingsHook.getFunction<decltype(photonNetworkConnectUsingSettingsFnc)>()(appSettings, startInOfflineMode, method);
}


PhotonSettings::PhotonSettings()
    : photonNetworkConnectUsingSettingsHook(
          GameData::getMethod("bool Photon_Pun_PhotonNetwork__ConnectUsingSettings (Photon_Realtime_AppSettings_o* appSettings, bool startInOfflineMode, const MethodInfo* method);").address,
          reinterpret_cast<void*>(photonNetworkConnectUsingSettingsFnc)
          ) {
    memset(&settings, 0, sizeof(settings));
}

void PhotonSettings::uiUpdate() {
    using namespace ImGui;
    Begin("Photon Settings");
#   define OVERRIDER(field) PushID("override_" #field); Checkbox("Override", &settings.override_##field); PopID(); SameLine(); BeginDisabled(!settings.override_##field);
#   define CONVBOOLFIELD(field) OVERRIDER(field) Checkbox(#field, &settings.field); EndDisabled()
#   define CONVINTFIELD(field) OVERRIDER(field) InputInt(#field, &settings.field); EndDisabled()
#   define CONVSTRFIELD(field) OVERRIDER(field) InputText(#field, settings.field, sizeof(settings.field)); EndDisabled()
    CONVLIST
#   undef OVERRIDER
#   undef CONVBOOLFIELD
#   undef CONVINTFIELD
#   undef CONVSTRFIELD
    End();
}

void PhotonSettings::fromIl2CppClass(const Photon_Realtime_AppSettings_Fields& o) {
#   define OVERRIDER(field) if (!settings.override_##field)
#   define CONVBOOLFIELD(field) OVERRIDER(field) settings.field = o.field
#   define CONVINTFIELD(field) OVERRIDER(field) settings.field = o.field
#   define CONVSTRFIELD(field) OVERRIDER(field) GameTypes::toCString(o.field, settings.field, sizeof(settings.field))
    CONVLIST
#   undef OVERRIDER
#   undef CONVBOOLFIELD
#   undef CONVINTFIELD
#   undef CONVSTRFIELD
}

void PhotonSettings::toIl2CppClass(Photon_Realtime_AppSettings_Fields &o) {
#   define CONVBOOLFIELD(field) o.field = settings.field
#   define CONVINTFIELD(field) o.field = settings.field
#   define CONVSTRFIELD(field) o.field = GameTypes::createCsString(settings.field)
    CONVLIST
#   undef CONVBOOLFIELD
#   undef CONVINTFIELD
#   undef CONVSTRFIELD
}


ModInfo photonSettingsInfo {
    "Photon Settings",
    [] () {return std::make_unique<PhotonSettings>();}
};

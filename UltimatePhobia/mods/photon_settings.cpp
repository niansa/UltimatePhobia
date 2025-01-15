#include "photon_settings.hpp"
#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"

#include <fstream>
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

using namespace Il2Cpp::CppInterop;




static bool photonNetwork$$ConnectUsingSettingsFnc(Photon_Realtime_AppSettings_o* appSettings, bool startInOfflineMode, const MethodInfo* method) {
    auto self = photonSettingsInfo.get<PhotonSettings>();
    self->fromIl2CppClass(appSettings->fields);
    self->toIl2CppClass(appSettings->fields);
    GameHookRelease GHR(self->photonNetwork$$ConnectUsingSettingsHook);
    return self->photonNetwork$$ConnectUsingSettingsHook.getFunction<decltype(photonNetwork$$ConnectUsingSettingsFnc)>()(appSettings, startInOfflineMode, method);
}


PhotonSettings::PhotonSettings()
    : photonNetwork$$ConnectUsingSettingsHook(
          Il2Cpp::Photon::Pun::PhotonNetwork::ConnectUsingSettings_getPtr<Photon_Realtime_AppSettings_o *, bool>(),
          reinterpret_cast<void*>(photonNetwork$$ConnectUsingSettingsFnc)
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

void PhotonSettings::setAppIdRealtime(std::string_view value) {
    if (value.size() >= sizeof(settings.AppIdRealtime))
        value = value.substr(0, sizeof(settings.AppIdRealtime)-1);
    memcpy(settings.AppIdRealtime, value.data(), value.size());
    settings.AppIdRealtime[value.size()] = '\0';
    settings.override_AppIdRealtime = true;
}

void PhotonSettings::fromIl2CppClass(const Photon_Realtime_AppSettings_Fields& o) {
#   define OVERRIDER(field) if (!settings.override_##field)
#   define CONVBOOLFIELD(field) OVERRIDER(field) settings.field = o.field
#   define CONVINTFIELD(field) OVERRIDER(field) settings.field = o.field
#   define CONVSTRFIELD(field) OVERRIDER(field) ToCString(o.field, settings.field, sizeof(settings.field))
    CONVLIST
#   undef OVERRIDER
#   undef CONVBOOLFIELD
#   undef CONVINTFIELD
#   undef CONVSTRFIELD
}

void PhotonSettings::toIl2CppClass(Photon_Realtime_AppSettings_Fields &o) {
#   define CONVBOOLFIELD(field) o.field = settings.field
#   define CONVINTFIELD(field) o.field = settings.field
#   define CONVSTRFIELD(field) o.field = ToCsString(settings.field)
    CONVLIST
#   undef CONVBOOLFIELD
#   undef CONVINTFIELD
#   undef CONVSTRFIELD
}


ModInfo photonSettingsInfo {
    "Photon Settings",
    false,
    [] () {return std::make_unique<PhotonSettings>();},
    [] () {
        std::fstream file("..\\photon-realtime-app-id.txt");
        if (!file)
            return;
        std::string id;
        std::getline(file, id);
        photonSettingsInfo.get<PhotonSettings>()->setAppIdRealtime(id);
    }
};

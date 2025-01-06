#include "global_state.hpp"
#include "getbaseaddr.hpp"
#include "disable_antimod.hpp"
#include "disable_splashscreen.hpp"
//#include "crash_handler.hpp"
#include "imgui_man.hpp"
#include "application.hpp"

#include <filesystem>
#include <windows.h>
#include <Psapi.h>
#include <detours.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>



static void onLoad() {
    g.logger->info("Getting GameAssembly base address...");
    getBaseAddr([] () {
        //setupCrashHandler();
        g.logger->info("Found GameAssembly base address at {}", g.base);
        ImGuiMan::init();
        disableAntiMod();
        disableSplashscreen();
        new Application;
    });
}

static void onUnload() {
    ImGuiMan::deinit();
}


EXTERN_C BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
    if (DetourIsHelperProcess())
        return TRUE;

    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    if (std::filesystem::path(szFileName).filename() != "Phasmophobia.exe")
        return TRUE;

    if (dwReason == DLL_PROCESS_ATTACH) {
        try {
            std::filesystem::rename("up_log.txt", "up_log.prev.txt");
        } catch (...) {}
        g.logger = spdlog::basic_logger_mt("UltimatePhobia", "up_log.txt");
        g.logger->set_level(spdlog::level::debug);
        g.logger->flush_on(spdlog::level::info);
        g.logger->info("PID: {} - Module name: {}", GetCurrentProcessId(), szFileName);
        g.logger->info("Loading Microsoft Detours...", szFileName);
        DetourRestoreAfterWith();

        g.logger->info("Loading UltimatePhobia...", szFileName);
        onLoad();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        g.logger->info("UltimatePhobia is about to be unloaded.", szFileName);
        g.logger->warn("End of log.", szFileName);
        g.logger.reset();
    }

    return TRUE;
}

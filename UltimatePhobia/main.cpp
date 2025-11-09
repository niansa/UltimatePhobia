#include "global_state.hpp"
#include "getbaseaddr.hpp"
#include "misc_utils.hpp"
#include "disable_antimod.hpp"
#include "disable_splashscreen.hpp"
#include "configure_persistentdatapath.hpp"
#include "il2cpp_api.hpp"
// #include "crash_handler.hpp"
#include "safe_path.hpp"
#include "imgui_man.hpp"
#include "application.hpp"

#include <filesystem>
#include <windows.h>
#include <psapi.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int UnityMain(HINSTANCE hInstance, HINSTANCE hPrevInstanc, LPWSTR lpCmdLine, int nShowCmd);

static void onLoad() {
    g.logger->info("Getting GameAssembly base address...");
    getBaseAddr([]() {
        // setupCrashHandler();
        g.logger->info("Found GameAssembly base address at {}", reinterpret_cast<void *>(g.base));
        ImGuiMan::init();
        Il2Cpp::API::init();
        disableAntiMod();
        disableSplashscreen();
        configurePersistentDataPath();
        new Application;
    });
}

static void onUnload() { ImGuiMan::deinit(); }

static std::string lastErrorString() {
    auto errorMessageID = GetLastError();
    if (errorMessageID == 0) {
        return "No error";
    }
    LPSTR messageBuffer = nullptr;
    auto size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

#ifdef _MSC_VER
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#endif
    // Get file name
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);

    // Initialize and get safe path for logs
    SafePath::init();
    const auto safePath = SafePath::get();

    // Set up logger
    try {
        std::filesystem::rename(safePath / "up_log.txt", safePath / "up_log.prev.txt");
    } catch (...) {
    }
    g.logger = spdlog::basic_logger_mt("UltimatePhobia", (safePath / "up_log.txt").string());
    g.logger->set_level(spdlog::level::debug);
    g.logger->flush_on(spdlog::level::err);
    g.logger->info("PID: {} - Module name: {}", GetCurrentProcessId(), szFileName);
    g.logger->info("Compiled using {}", COMPILER_INFO);
    g.logger->info("Safe path determined to be at {}", safePath.string());

    // Set std::terminate handler
    std::set_terminate([]() {
        std::string reason;
        try {
            std::exception_ptr eptr{std::current_exception()};
            if (eptr)
                std::rethrow_exception(eptr);
            else
                reason = "None";
        } catch (const std::exception& ex) {
            reason = ex.what();
        } catch (...) {
            reason = "Unknown";
        }

        g.logger->critical("std::terminate called! Exception: " + reason);
        g.logger->flush();
        std::abort();
    });

    // Load UltimatePhobia
    g.logger->info("Loading UltimatePhobia...", szFileName);
    onLoad();

    // Load engine
    g.logger->info("Starting Unity Player...");
    HMODULE unityPlayer = LoadLibraryW(L"UnityPlayer.dll");
    if (unityPlayer == nullptr) {
        g.logger->critical("Failed to load Unity Player: {}", lastErrorString());
        abort();
    }
    auto unityMain = reinterpret_cast<decltype(UnityMain) *>(GetProcAddress(unityPlayer, "UnityMain"));
    if (unityMain == nullptr) {
        g.logger->critical("Failed to get Unity Player entry point: {}", lastErrorString());
    }
    std::wstring cmdLine =
#ifdef _MSC_VER
        lpCmdLine
#else
        utf8Decode(lpCmdLine).data()
#endif
        ;
    unityMain(hInstance, hPrevInstance, cmdLine.data(), nShowCmd);

    // Unload everything
    g.logger->info("UltimatePhobia is about to be unloaded.", szFileName);
    g.logger->warn("End of log.", szFileName);
    g.logger.reset();
    onUnload();
}

#include "crash_handler.hpp"

#include "global_state.hpp"
#include "gamedata.hpp"
#include "game_hook.hpp"

#include <iomanip>
#include <sstream>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>



static void logStackTraceDetail(CONTEXT c) {
    STACKFRAME64 s;
    s.AddrPC.Offset = c.Rip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Rsp;
    s.AddrStack.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Rbp;
    s.AddrFrame.Mode = AddrModeFlat;

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    g.logger->info("Generating stacktrace...");
    std::stringstream trace("\n");

    unsigned frame_number = 0;
    while (frame_number < 32) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &s, &c, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
            break;

        trace << std::setw(3) << "\n" << frame_number++ << "\t";
        void *addr = reinterpret_cast<void *>(s.AddrPC.Offset);
        if (addr == nullptr)
            break;

        const auto method = GameData::getMethod(addr);
        if (method.isValid())
            trace << '(' << method.name << ") " << method.signature;
        else
            trace << "(unknown) 0x" << addr;
        if (const void *hook = GameHook::getHookAt(addr))
            trace << " (hook at 0x" << hook << ")";
    }

    g.logger->debug("{}", trace.str());
    g.logger->flush();
}

static LONG WINAPI handleCrash(EXCEPTION_POINTERS *excp) {
    g.logger->critical("Crash intercepted!");
    logStackTraceDetail(*(excp->ContextRecord));
    return EXCEPTION_EXECUTE_HANDLER;
}


void setupCrashHandler() {
    g.logger->info("Setting up crash handler...");
    SymInitialize(GetCurrentProcess(), nullptr, true);
    SetUnhandledExceptionFilter(handleCrash);
}

void logStackTrace() {
    CONTEXT c;
    RtlCaptureContext(&c);
    logStackTraceDetail(c);
}

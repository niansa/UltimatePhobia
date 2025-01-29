#include "dll_ffi.hpp"
#include "global_state.hpp"
#include "ffi_interface.hpp"

#include <string>
#include <stdexcept>
#include <libloaderapi.h>



DLLFFI::DLLFFI(const std::filesystem::path &modPath) : modPathStr(modPath.string()) {
    chandle = LoadLibraryA(modPathStr.c_str());
    if (!chandle)
        throw std::runtime_error(fmt::format("dlopen(): {}", modPathStr));

    const static FFIInterface::Exports exports;
    void initImportsDummy(const FFIInterface::Exports *);
    auto initImports = reinterpret_cast<decltype(initImportsDummy)*>(GetProcAddress(chandle, "initImports"));
    if (!initImports)
        g.logger->error("DLL is missing 'initImports' function!");
    else
        initImports(&exports);
}

DLLFFI::~DLLFFI() {
    FreeLibrary(chandle);
}

void DLLFFI::simpleCall(const char *name) {
    void simpleCallDummy();
    auto func = reinterpret_cast<decltype(simpleCallDummy)*>(GetProcAddress(chandle, name));
    g.logger->flush();
    if (!func)
        g.logger->error("Failed to call DLL function {} in {} as it wasn't found", name, modPathStr);
    else
        func();
    g.logger->flush();
}

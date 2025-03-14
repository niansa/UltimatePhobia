namespace FFIInterface {
struct Imports;
}
const FFIInterface::Imports *imports;

#include <ffi_interface.hpp>

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define Q_DECL_EXPORT __declspec(dllexport)
#define Q_DECL_IMPORT __declspec(dllimport)
#else
#define Q_DECL_EXPORT __attribute__((visibility("default")))
#define Q_DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(SOCKFFICLIENTTEST_LIBRARY)
#define SOCKFFICLIENTTEST_EXPORT extern "C" Q_DECL_EXPORT
#else
#define SOCKFFICLIENTTEST_EXPORT extern "C" Q_DECL_IMPORT
#endif

using namespace FFIInterface;
using namespace Helpers;
using namespace Literals;

SOCKFFICLIENTTEST_EXPORT void initImports(const FFIInterface::Imports *imports) { ::imports = imports; }

SOCKFFICLIENTTEST_EXPORT void onLoad() { FFI logInfo("Hello from SockFFI module!"_cs); }

SOCKFFICLIENTTEST_EXPORT void onUnload() { FFI logInfo("Goodbye from SockFFI module."_cs); }

SOCKFFICLIENTTEST_EXPORT void onUiUpdate() {
    // Display settings
    FFI ImGuiBegin("SockFFI client test");
    FFI ImGuiText("Hi :D"_cs);
    FFI ImGuiEnd();
}

#include <wasm32sdk.h>
#include "../UltimatePhobia/wasm_interface.hpp"

using namespace WASMInterface;
using namespace Helpers::Literals;



WASM_EXPORT void onLoad() {
    logInfo("Hello from WASM module!"_cs);
    Helpers::call<"void UnityEngine_Debug__Log (Il2CppObject* message, const MethodInfo* method);">("Hello world from WASM module!"_cs, nullptr);
}
WASM_EXPORT void onUnload() {
    logInfo("Goodbye from WASM module :-("_cs);
    Helpers::call<"void UnityEngine_Application__Quit (int32_t exitCode, const MethodInfo* method);">(0, nullptr);
}

WASM_EXPORT void onUiUpdate() {}

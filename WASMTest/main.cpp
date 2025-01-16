#include <wasm32sdk.h>
#include "../UltimatePhobia/wasm_interface.h"

using namespace WASMInterface;



WASM_EXPORT void onLoad() {
    logInfo(toCsString("Hello from WASM module!"));
}
WASM_EXPORT void onUnload() {
    logInfo(toCsString("Goodbye from WASM module :-("));
}

WASM_EXPORT void onUiUpdate() {

}

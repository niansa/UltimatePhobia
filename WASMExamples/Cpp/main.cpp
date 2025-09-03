#include <stdio.h>
#include <wasm32sdk.h>

#include "ffi_interface.hpp"

using namespace FFIInterface;
using namespace Helpers;
using namespace Helpers::Literals;

namespace {
ObjectHandle fuseBox;
int useBoxCountDown = -1;
bool originalState = true;
} // namespace

void useBoxSoonIfNeeded() {
    printf("Countdown: %i\nOriginal state: %s\n", useBoxCountDown, originalState ? "true" : "false");
    if (!originalState) {
        logInfo("Retriggering fuse box soon..."_cs);
        useBoxCountDown = 200;
    } else {
        useBoxCountDown = -1;
    }
}

WASM_EXPORT("onLoad")
void onLoad() {
    if (getFtableItemCount() < getLocalFtableItemCount())
        logWarn("UltimatePhobia runtime version might be too old!"_cs);

    logInfo("Hello from WASM module!"_cs);
    puts("Hello from WASM puts!\nThis is a multi-line test...");
    printf("Hello from WASM printf at %p!\nThis is another multi-line test...\n", printf);
    call<"void UnityEngine_Debug__Log (Il2CppObject* message, const "
         "MethodInfo* method);">("Hello world from WASM module!"_cs, nullptr);

    hook(getMethodCached<"FuseBox$$UseNetworked">(), "onFuseBoxUseNetworked");
    hook(getMethodCached<"Photon.Pun.PhotonNetwork$$CreateRoom">(), "onCreateRoom");
}
WASM_EXPORT("onUnload")
void onUnload() {
    logInfo("Goodbye from WASM module :-("_cs);
    unhook(getMethodCached<"FuseBox$$UseNetworked">());
    unhook(getMethodCached<"Photon.Pun.PhotonNetwork$$CreateRoom">());
    dropObject(fuseBox);
}

WASM_EXPORT("onUiUpdate")
void onUiUpdate() {
    if (useBoxCountDown == 0) {
        logInfo("Triggering fuse box..."_cs);
        call<"FuseBox$$Use">(fuseBox, nullptr);
    } else if (useBoxCountDown > 0) {
        --useBoxCountDown;
    }

    ImGuiBegin("WebAssembly Test Mod");
    ImGuiText("Hello World through ImGui"_cs);
    ImGuiEnd();
}

WASM_EXPORT("onFuseBoxUseNetworked")
void onFuseBoxUse() {
    // Get new fuse box object (which is passed as first argument)
    dropObject(fuseBox);
    fuseBox = getArg<ObjectHandle>(0);

    // Call original function
    call(getOriginal(), getArgCount());
    logInfo("Fuse box triggered."_cs);

    // Use fuse box again later if not triggered manually
    originalState = !originalState;
    useBoxSoonIfNeeded();
}

WASM_EXPORT("onCreateRoom")
void onCreateRoom() {
    // Get original room name
    auto originalName = getArg<ObjectHandle>(0);

    // Replace room name
    addArg("WASM Test Mod user#0000"_cs);
    moveArg(0);

    // Call origin function
    call(getOriginal(), unknownArgCount);
    printf("Room created with replacement name instead of \"%s\"!\n", getCString(originalName));

    dropObject(originalName);
}

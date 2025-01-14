#include "disable_splashscreen.hpp"
#include "global_state.hpp"
#include "game_hook.hpp"
#include "generated/il2cpp.hpp"
#include "bindings/unityengine.hpp"



void splashScreen$$AwakeFnc(SplashScreen_o* __this, const MethodInfo* method) {
    // Load into next scene without waiting
    UnityEngine::SceneManagement::SceneManager::LoadScene("Menu_New");
}


void disableSplashscreen() {
    g.logger->info("Disabling splash screen...");
    new GameHook(Il2Cpp::Methods::SplashScreen__Awake_getPtr(), reinterpret_cast<void *>(splashScreen$$AwakeFnc));
}

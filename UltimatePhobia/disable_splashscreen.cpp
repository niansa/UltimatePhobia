#include "disable_splashscreen.hpp"
#include "global_state.hpp"
#include "game_hook.hpp"
#include "gamedata.hpp"
#include "bindings/unityengine.hpp"



void splashScreen$$AwakeFnc(SplashScreen_o* __this, const MethodInfo* method) {
    // Load into next scene without waiting
    UnityEngine::SceneManagement::SceneManager::LoadScene("Menu_New");
}


void disableSplashscreen() {
    g.logger->info("Disabling splash screen...");
    new GameHook(GameData::getMethod("SplashScreen$$Awake").address, reinterpret_cast<void *>(splashScreen$$AwakeFnc));
}

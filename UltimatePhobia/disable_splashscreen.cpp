#include "disable_splashscreen.hpp"
#include "global_state.hpp"
#include "game_hook.hpp"
#include "game_types.hpp"
#include "generated/il2cpp.hpp"



void splashScreen$$AwakeFnc(SplashScreen_o* __this, const MethodInfo* method) {
    // Load into next scene without waiting
    Il2Cpp::UnityEngine::SceneManagement::SceneManager::LoadScene(GameTypes::createCsString("Menu_New"));
}


void disableSplashscreen() {
    g.logger->info("Disabling splash screen...");
    new GameHook(Il2Cpp::SplashScreen::Awake_getPtr(), reinterpret_cast<void *>(splashScreen$$AwakeFnc));
}

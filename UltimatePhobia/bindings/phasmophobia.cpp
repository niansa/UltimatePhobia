#include "phasmophobia.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"



namespace Phasmophobia {
void Door::UnlockDoor(Door_o *__this){
    GameData::getMethod("Door$$UnlockDoor")
        .getFunction<void (Door_o *, const MethodInfo *)>()
        (__this, nullptr);
}

void Door::DisableOrEnableDoor(Door_o *__this, bool enable) {
    GameData::getMethod("Door$$DisableOrEnableDoor")
        .getFunction<void (Door_o *, bool, const MethodInfo *)>()
        (__this, enable, nullptr);
}

void Door::OpenDoor(Door_o *__this, float speed, bool obf1) {
    GameData::getMethod("Door$$OpenDoor")
        .getFunction<void (Door_o *, float, bool, const MethodInfo *)>()
        (__this, obf1, obf1, nullptr);
}
}

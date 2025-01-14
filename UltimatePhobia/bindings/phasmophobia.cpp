#include "phasmophobia.hpp"
#include "generated/il2cpp.hpp"

using namespace Il2Cpp::Methods;



namespace Phasmophobia {
void Door::UnlockDoor(Door_o *__this) {
    Door__UnlockDoor(__this);
}

void Door::DisableOrEnableDoor(Door_o *__this, bool enable) {
    Door__DisableOrEnableDoor(__this, enable);
}

void Door::OpenDoor(Door_o *__this, float speed, bool obf1) {
    Door__OpenDoor(__this, speed, obf1);
}


void GhostActivity::Interact(GhostActivity_o *__this) {
    GhostActivity__Interact(__this);
}


bool Player::IsLocal(Player_o *__this) {
    return __this->fields.photonView->fields._AmOwner_k__BackingField;
}
}

#pragma once


struct Door_o;


namespace Phasmophobia {
namespace Door {
void UnlockDoor(Door_o *__this);
void DisableOrEnableDoor(Door_o *__this, bool enable);
void OpenDoor(Door_o *__this, float speed, bool obf1);
}

namespace GhostActivity {
void Interact(GhostActivity_o *__this);
}
}

#pragma once

#include "mods/base.hpp"

#include <dxgi.h>
#include <d3d11.h>

namespace DX11Hook {
void init();
void deinit();

void registerMod(Mod *mod);
void unregisterMod(Mod *mod);
} // namespace DX11Hook

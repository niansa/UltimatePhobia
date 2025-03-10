#pragma once

#include "miniaudio.h"
#include "SteamAudioImpl.hpp"

#include <phonon.h>
#include <ffi_interface.hpp>

namespace Utils {
const char *errorToMessage(IPLerror error);
const char *errorToMessage(ma_result error);

FFIInterface::ObjectHandle createErrorMessage(const char *what, IPLerror why);
FFIInterface::ObjectHandle createErrorMessage(const char *what, ma_result why);
} // namespace Utils

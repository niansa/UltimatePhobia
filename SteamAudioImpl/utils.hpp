#pragma once

#include "miniaudio.h"
#include "SteamAudioImpl.hpp"

#include <string>
#include <phonon.h>
#include <ffi_interface.hpp>

namespace Utils {
const char *errorToMessage(IPLerror error);
const char *errorToMessage(ma_result error);

FFIInterface::ObjectHandle createErrorMessage(const char *what, IPLerror why);
FFIInterface::ObjectHandle createErrorMessage(const char *what, ma_result why);

std::string formatPos(IPLVector3 pos);
std::string formatCoord(IPLCoordinateSpace3 coord);
} // namespace Utils

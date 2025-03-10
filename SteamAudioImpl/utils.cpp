#include "utils.hpp"
#include "SteamAudioImpl.hpp"

#include <sstream>
#include <ffi_interface.hpp>

using namespace FFIInterface;

namespace Utils {
namespace {
ObjectHandle createErrorMessage_(const char *what, auto why) {
    std::ostringstream fress("Failed to ");
    fress << what << "! Error was: " << errorToMessage(why);
    const std::string fres = fress.str();
    return FFI toCsStringWithLength(fres.data(), fres.size());
}
} // namespace

ObjectHandle createErrorMessage(const char *what, IPLerror why) {
    return createErrorMessage_(what, why);
}
ObjectHandle createErrorMessage(const char *what, ma_result why) {
    return createErrorMessage_(what, why);
}

const char *errorToMessage(IPLerror error) {
    switch (error) {
    case IPL_STATUS_SUCCESS:
        return "The operation completed successfully.";
    case IPL_STATUS_FAILURE:
        return "An unspecified error occurred.";
    case IPL_STATUS_OUTOFMEMORY:
        return "The system ran out of memory.";
    case IPL_STATUS_INITIALIZATION:
        return "An error occurred while initializing an external dependency.";
    }
    return "Invalid error.";
}
const char *errorToMessage(ma_result error) {
    return ma_result_description(error);
}
} // namespace Utils

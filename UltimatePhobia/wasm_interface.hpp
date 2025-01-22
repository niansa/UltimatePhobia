#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef WASM
#define UP_API WASM_IMPORT
#else
#define UP_API
#endif


namespace WASMInterface {
#ifdef WASM
enum class ObjectHandle : int {
    Null = 0,
    Invalid = -1
};
enum class MethodHandle : int {
    Invalid = -1
};
#else
using ObjectHandle = int;
using MethodHandle = int;
#endif
using WIBool = int;
constexpr int unknownArgCount = 0x6D616E63;

/**
 * @brief Invalidates given handle to C# object allowing it to be garbage collected
 */
UP_API void dropObject(ObjectHandle);
/**
 * @brief Check if given handle is a valid C# object
 * @return 1 if handle is valid, otherwise 0
 */
UP_API WIBool isValidObject(ObjectHandle);
/**
 * @return C# null object (0 handle)
 */
UP_API ObjectHandle getNull();

/**
 * @brief Converts given C string to System.String object until null-terminator
 * @param str C string
 */
UP_API ObjectHandle toCsString(const char *str);
/**
 * @brief Converts given C string of specified length to System.String object
 * @param str C string
 * @param length Length of C string
 */
UP_API ObjectHandle toCsStringWithLength(const char *str, int length);
/**
 * @brief Converts given string to C string (including null terminator)
 * @param str System.String object
 * @param buf Buffer to write string to
 * @param maxlen Length of buffer
 */
UP_API void toCString(ObjectHandle str, char *buf, int maxlen);

/**
 * @brief Logs given message on trace level
 * @param message System.String object containing message
 */
UP_API void logTrace(ObjectHandle message);
/**
 * @brief Logs given message on debug level
 * @param message System.String object containing message
 */
UP_API void logDebug(ObjectHandle message);
/**
 * @brief Logs given message on info level
 * @param message System.String object containing message
 */
UP_API void logInfo(ObjectHandle message);
/**
 * @brief Logs given message on warn level
 * @param message System.String object containing message
 */
UP_API void logWarn(ObjectHandle message);
/**
 * @brief Logs given message on error level
 * @param message System.String object containing message
 */
UP_API void logError(ObjectHandle message);
/**
 * @brief Logs given message on critical level
 * @param message System.String object containing message
 */
UP_API void logCritical(ObjectHandle message);

/**
 * @brief Gets given method
 * @param identifier Name or signature to look up function by
 * @return First method found, or -1
 */
UP_API MethodHandle getMethodByIdentifier(const char *identifier);
/**
 * @brief Gets given method
 * @param addr Address to look up function by
 * @return First method found, or -1
 */
UP_API MethodHandle getMethodByAddress(int64_t addr);
/**
 * @brief Gets name of given method
 * @return System.String object
 */
UP_API ObjectHandle getMethodName(MethodHandle);
/**
 * @brief Gets signature of given method
 * @return System.String object
 */
UP_API ObjectHandle getMethodSignature(MethodHandle);

/**
 * @brief Adds 32 bit integer to argument list
 * @param v Value to add
 */
UP_API void addArgI32(int32_t v);
/**
 * @brief Adds 64 bit integer to argument list
 * @param v Value to add
 */
UP_API void addArgI64(int64_t v);
/**
 * @brief Adds float to argument list
 * @param v Value to add
 */
UP_API void addArgFloat(float v);
/**
 * @brief Adds double to argument list
 * @param v Value to add
 */
UP_API void addArgDouble(double v);
/**
 * @brief Adds C# object to argument list
 * @param v C# object to add
 */
UP_API void addArgObject(ObjectHandle v);
/**
 * @brief Adds null value to argument list
 */
UP_API void addArgNull();
/**
 * @brief Clears arguments
 */
UP_API void clearArgs();
/**
 * @brief Returns amount of arguments
 */
UP_API int getArgCount();
/**
 * @brief Moves last argument to specified index replacing its value
 * @param index New argument index or negative value to replace return value
 * @return 1 on success, 0 on error (if argument list empty or index out of range)
 */
UP_API WIBool moveArg(int index);
/**
 * @brief Gets 32 bit integer stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API int32_t getValueI32(int index = -1);
/**
 * @brief Gets 64 bit integer stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API int64_t getValueI64(int index = -1);
/**
 * @brief Gets float stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API float getValueFloat(int index = -1);
/**
 * @brief Gets double stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API double getValueDouble(int index = -1);
/**
 * @brief Gets C# object stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API ObjectHandle getValueObject(int index = -1);
/**
 * @brief Gets error message from call function
 * @return System.String object
 */
UP_API ObjectHandle getCallError();
/**
 * @brief Calls given method with previously added arguments (also clears them if function call was able to be initiated)
 * @param argCount Amount of arguments previously added (optional, may be unknownArgCount)
 * @return 1 on success, 0 on failure (use getCallError to get error string)
 */
UP_API WIBool call(MethodHandle, int argCount);

/**
 * @brief Hooks given method
 * @param callback Name of function to call with arguments passed by game added instead when game calls given method
 * @return 1 on success, 0 on failure (if method is already hooked or is invalid)
 */
UP_API WIBool hook(MethodHandle, const char *callback);
/**
 * @brief Unhooks given method. MUST NOT be called inside hook for given method!
 * @return 1 on success, 0 on failure (if method isn't hooked or is invalid)
 */
UP_API WIBool unhook(MethodHandle);
/**
 * @brief Gets original method. MUST be called inside hook!
 * @return Original method or bogus data if not called inside hook
 */
UP_API MethodHandle getOriginal();

UP_API void ImGuiBegin(const char *name = "");
UP_API void ImGuiEnd();
UP_API void ImGuiText(ObjectHandle text);
UP_API void ImGuiCheckbox(const char *label, bool *v);
UP_API WIBool ImGuiButton(const char *label);
UP_API void ImGuiSeparator();
UP_API void ImGuiSeparatorText(const char *label);

UP_API void abort(const char *message, const char *filename, int lineNumber, int columnNumber);
}

#ifdef WASM
namespace Helpers {
namespace {
template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        for (unsigned it = 0; it != N; ++it)
            value[it] = str[it];
    }

    char value[N];

    operator const char *() const {
        return value;
    }
};

WASMInterface::ObjectHandle createCsString() {
    return WASMInterface::toCsStringWithLength(nullptr, 0);
}

void addArg(int32_t v) {
    WASMInterface::addArgI32(v);
}
void addArg(int64_t v) {
    WASMInterface::addArgI64(v);
}
void addArg(bool v) {
    WASMInterface::addArgI32(v);
}
void addArg(float v) {
    WASMInterface::addArgFloat(v);
}
void addArg(double v) {
    WASMInterface::addArgDouble(v);
}
void addArg(WASMInterface::ObjectHandle v) {
    WASMInterface::addArgObject(v);
}
void addArg(decltype(nullptr)) {
    WASMInterface::addArgNull();
}

template<typename T>
T getArg(int idx) = delete;
template<>
void getArg<void>(int idx) {}
template<>
int32_t getArg<int32_t>(int idx) {
    return WASMInterface::getValueI32(idx);
}
template<>
int64_t getArg<int64_t>(int idx) {
    return WASMInterface::getValueI64(idx);
}
template<>
bool getArg<bool>(int idx) {
    return WASMInterface::getValueI32(idx);
}
template<>
float getArg<float>(int idx) {
    return WASMInterface::getValueFloat(idx);
}
template<>
double getArg<double>(int idx) {
    return WASMInterface::getValueDouble(idx);
}
template<>
WASMInterface::ObjectHandle getArg<WASMInterface::ObjectHandle>(int idx) {
    return WASMInterface::getValueObject(idx);
}

template<typename T>
T getReturnValue() {
    return getArg<T>(-1);
}
template<typename T>
void setReturnValue(T v) {
    addArg(v);
    WASMInterface::moveArg(-1);
}

inline void addArgs() {}
template <typename Arg0, typename... Args>
void addArgs(Arg0 arg0, Args... args) {
    addArg(arg0);
    addArgs(args...);
}

template<unsigned maxlen = 64>
const char *getCString(WASMInterface::ObjectHandle str) {
    static char buf[maxlen];
    WASMInterface::toCString(str, buf, maxlen);
    return buf;
}

WASMInterface::MethodHandle getMethod(const char *identifier) {
    return WASMInterface::getMethodByIdentifier(identifier);
}
WASMInterface::MethodHandle getMethod(int64_t address) {
    return WASMInterface::getMethodByAddress(address);
}

template<StringLiteral identifier>
WASMInterface::MethodHandle getMethodCached() {
    static WASMInterface::MethodHandle fres = WASMInterface::getMethodByIdentifier(identifier);
    return fres;
}

bool call_error;
template<StringLiteral identifier, typename returnT = void, typename... Args>
returnT call(Args... args) {
    WASMInterface::clearArgs();
    addArgs(args...);
    call_error = !WASMInterface::call(getMethodCached<identifier>(), WASMInterface::unknownArgCount);
    return getReturnValue<returnT>();
}

namespace Literals {
inline WASMInterface::ObjectHandle operator "" _cs(const char *str, size_t len) {
    return WASMInterface::toCsStringWithLength(str, len);
}
}
}
}
#endif

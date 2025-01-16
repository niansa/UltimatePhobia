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
struct ObjectHandle {int id;};
#else
using ObjectHandle = int;
#endif
constexpr int unknownArgCount = 0x6D616E63;

UP_API void dropObject(ObjectHandle id);
UP_API int isValidObject(ObjectHandle id);
UP_API ObjectHandle getNull();

UP_API ObjectHandle toCsString(const char *str);
UP_API ObjectHandle toCsStringWithLength(const char *str, int length);
UP_API void toCString(ObjectHandle str, char *buf, int maxlen);

UP_API void logTrace(ObjectHandle message);
UP_API void logDebug(ObjectHandle message);
UP_API void logInfo(ObjectHandle message);
UP_API void logWarn(ObjectHandle message);
UP_API void logError(ObjectHandle message);
UP_API void logCritical(ObjectHandle message);

UP_API int getMethodByIdentifier(const char *name);
UP_API int getMethodByAddress(int64_t addr);
UP_API ObjectHandle getMethodName(int index);
UP_API ObjectHandle getMethodSignature(int index);

UP_API void addArgI32(int32_t v);
UP_API void addArgI64(int64_t v);
UP_API void addArgFloat(float v);
UP_API void addArgDouble(double v);
UP_API void addArgObject(ObjectHandle v);
UP_API void addArgNull();
UP_API void clearArgs();
UP_API int32_t getValueI32(int index = -1);
UP_API int64_t getValueI64(int index = -1);
UP_API float getValueFloat(int index = -1);
UP_API double getValueDouble(int index = -1);
UP_API ObjectHandle getValueObject(int index = -1);
UP_API ObjectHandle getCallError();
UP_API int call(int index, int argCount);
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

void addArg(int32_t v) {
    ::WASMInterface::addArgI32(v);
}
void addArg(int64_t v) {
    ::WASMInterface::addArgI64(v);
}
void addArg(float v) {
    ::WASMInterface::addArgFloat(v);
}
void addArg(double v) {
    ::WASMInterface::addArgDouble(v);
}
void addArg(::WASMInterface::ObjectHandle v) {
    ::WASMInterface::addArgObject(v);
}
void addArg(decltype(nullptr)) {
    ::WASMInterface::addArgNull();
}

inline void addArgs() {}
template <typename Arg0, typename... Args>
void addArgs(Arg0 arg0, Args... args) {
    addArg(arg0);
    addArgs(args...);
}

template<StringLiteral identifier, typename... Args>
void call(Args... args) {
    ::WASMInterface::clearArgs();
    static int methodIndex = ::WASMInterface::getMethodByIdentifier(identifier);
    addArgs(args...);
    ::WASMInterface::call(methodIndex, ::WASMInterface::unknownArgCount);
}

namespace Literals {
inline ::WASMInterface::ObjectHandle operator "" _cs(const char *str, size_t len) {
    return ::WASMInterface::toCsStringWithLength(str, len);
}
}
}
}
#endif

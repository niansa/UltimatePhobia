#pragma once

#include <stdint.h>

#ifdef WASM
#define UP_API WASM_IMPORT
#else
#define UP_API
#endif


namespace WASMInterface {
using ObjectHandle = int;

UP_API void dropObject(ObjectHandle id);
UP_API int isValidObject(ObjectHandle id);

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
UP_API int getMethodByAddress(intptr_t addr);
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

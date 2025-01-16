#include "wasm_interface.h"
#include "global_state.hpp"
#include "il2cpp_dynamic.hpp"
#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"

#include <map>



namespace WASMInterface {
namespace {
std::map<ObjectHandle, void *> objects;
ObjectHandle addObject(void *ptr) {
    // Find empty handle ID
    ObjectHandle fres = objects.size()+1;
    while (objects.find(fres) != objects.end())
        ++fres;
    objects[fres] = ptr;
    return fres;
}
void *getObject(ObjectHandle id) {
    if (id == 0)
        return nullptr;
    auto res = objects.find(id);
    if (res == objects.end())
        return nullptr;
    return res->second;
}
}
void dropObject(ObjectHandle id) {
    if (id > 0)
        objects.erase(id);
}
int isValidObject(ObjectHandle id) {
    return objects.find(id) != objects.end();
}

ObjectHandle toCsString(const char *str) {
    return addObject(Il2Cpp::CppInterop::ToCsString(str));
}
ObjectHandle toCsStringWithLength(const char *str, int length) {
    return addObject(Il2Cpp::CppInterop::ToCsString({str, static_cast<size_t>(length)}));
}
void toCString(ObjectHandle str, char *buf, int maxlen) {
    Il2Cpp::CppInterop::ToCString(reinterpret_cast<System_String_o *>(getObject(str)), buf, maxlen);
}

void logTrace(ObjectHandle message) {
    g.logger->trace("{}", Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}
void logDebug(ObjectHandle message) {
    g.logger->debug("{}", Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}
void logInfo(ObjectHandle message) {
    g.logger->info("{}", Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}
void logWarn(ObjectHandle message) {
    g.logger->warn("{}", Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}
void logError(ObjectHandle message) {
    g.logger->error("{}", Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}
void logCritical(ObjectHandle message) {
    g.logger->critical("{}", Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}

namespace Dynamic = Il2Cpp::Dynamic;
int getMethodByIdentifier(const char *name) {
    return Dynamic::getMethod(name, true).index;
}
int getMethodByAddress(intptr_t addr) {
    return Dynamic::getMethod(reinterpret_cast<void *>(addr), true).index;
}
ObjectHandle getMethodName(int index) {
    if (index < 0)
        return 0;
    return addObject(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).name));
}
ObjectHandle getMethodSignature(int index) {
    if (index < 0)
        return 0;
    return addObject(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).signature));
}

namespace {
std::vector<void *> call_args;
void *return_value;
const char *call_error;

void *getValue(int index) {
    if (index < 0)
        return return_value;
    if (index > call_args.size())
        return nullptr;
    return call_args[index];
}
}
void addArgI32(int32_t v) {
    call_args.push_back(reinterpret_cast<void *>(v));
}
void addArgI64(int64_t v) {
    call_args.push_back(reinterpret_cast<void *>(v));
}
void addArgFloat(float v) {
    call_args.push_back(reinterpret_cast<void *>(*reinterpret_cast<uint32_t *>(&v)));
}
void addArgDouble(double v) {
    call_args.push_back(reinterpret_cast<void *>(*reinterpret_cast<uint64_t *>(&v)));
}
void addArgObject(ObjectHandle v) {
    call_args.push_back(getObject(v));
}
void addArgNull() {
    call_args.push_back(nullptr);
}
void clearArgs() {
    call_args.clear();
}
int32_t getValueI32(int index) {
    return reinterpret_cast<uintptr_t>(getValue(index));
}
int64_t getValueI64(int index) {
    return reinterpret_cast<uintptr_t>(getValue(index));
}
float getValueFloat(int index) {
    void *value = getValue(index);
    return *reinterpret_cast<float *>(&value);
}
double getValueDouble(int index) {
    void *value = getValue(index);
    return *reinterpret_cast<double *>(&value);
}
ObjectHandle getValueObject(int index) {
    return addObject(getValue(index));
}
ObjectHandle getCallError() {
    return toCsString(call_error);
}
int call(int index, int argCount) {
    // Make sure argCount matches argument count
    if (call_args.size() != argCount) {
        call_error = "Mismatched added vs. passed arg count";
        return false;
    }

    // Get method
    const auto method = Dynamic::getMethod(index);
    if (!method.isValid()) {
        call_error = "Bad method";
        return false;
    }

    // Check argument count
    if (argCount != method.signature.size()-1) {
        call_error = "Mismatched passed vs. actual arg count";
        return false;
    }

    // Call function
    const auto args = std::exchange(call_args, {});
    try {
        switch (argCount) {
        case 0: return_value = method.getFunction<void *()>()(); break;
        case 1: return_value = method.getFunction<void *(void *)>()(args[0]); break;
        case 2: return_value = method.getFunction<void *(void *, void *)>()(args[0], args[1]); break;
        case 3: return_value = method.getFunction<void *(void *, void *, void *)>()(args[0], args[1], args[2]); break;
        case 4: return_value = method.getFunction<void *(void *, void *, void *, void *)>()(args[0], args[1], args[2], args[3]); break;
        case 5: return_value = method.getFunction<void *(void *, void *, void *, void *, void *)>()(args[0], args[1], args[2], args[3], args[4]); break;
        case 6: return_value = method.getFunction<void *(void *, void *, void *, void *, void *, void *)>()(args[0], args[1], args[2], args[3], args[4], args[5]); break;
        default: {
            call_error = "Too many arguments (max. 6 supported)";
            return false;
        }
        }
    } catch (const std::exception& e) {
        static std::string error_buffer = fmt::format("Method has thrown an exception: {}", e.what());
        call_error = error_buffer.c_str();
        return false;
    } catch (...) {
        call_error = "Method has thrown an unknown exception";
        return false;
    }

    // Everything seems to have gone well
    return true;
}
}

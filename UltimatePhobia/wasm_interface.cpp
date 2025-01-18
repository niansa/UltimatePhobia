#include "wasm_interface.hpp"
#include "global_state.hpp"
#include "il2cpp_dynamic.hpp"
#include "il2cpp_cppinterop.hpp"
#include "game_hook.hpp"
#include "wasm_loader.hpp"
#include "mods/base.hpp"

#include <map>
#include <exception>



namespace WASMInterface {
namespace {
std::map<ObjectHandle, void *> objects;

ObjectHandle addObject(void *ptr) {
    if (ptr == nullptr)
        return 0;
    // Find empty handle ID
    static ObjectHandle last_id = 0;
    ObjectHandle fres = ++last_id;
    while (objects.find(fres) != objects.end())
        ++fres;
    if (fres <= 0) {
        g.logger->warn("Ran out of object handles! Please consider dropping some handles.");
        return -1;
    }
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
    return id == 0 || objects.find(id) != objects.end();
}
ObjectHandle getNull() {
    return 0;
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

namespace {
void log(spdlog::level::level_enum level, ObjectHandle message) {
    g.logger->log(level, "[{} (WASM)] {}", WASMLoader::WASMMod::getCurrent()->name, Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(getObject(message))));
}
}
void logTrace(ObjectHandle message) {
    log(spdlog::level::trace, message);
}
void logDebug(ObjectHandle message) {
    log(spdlog::level::debug, message);
}
void logInfo(ObjectHandle message) {
    log(spdlog::level::info, message);
}
void logWarn(ObjectHandle message) {
    log(spdlog::level::warn, message);
}
void logError(ObjectHandle message) {
    log(spdlog::level::err, message);
}
void logCritical(ObjectHandle message) {
    log(spdlog::level::critical, message);
}

namespace Dynamic = Il2Cpp::Dynamic;
MethodHandle getMethodByIdentifier(const char *identifier) {
    return Dynamic::getMethod(identifier, true).index;
}
MethodHandle getMethodByAddress(int64_t addr) {
    return Dynamic::getMethod(reinterpret_cast<void *>(addr), true).index;
}
ObjectHandle getMethodName(MethodHandle index) {
    if (index < 0)
        return 0;
    return addObject(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).name));
}
ObjectHandle getMethodSignature(MethodHandle index) {
    if (index < 0)
        return 0;
    return addObject(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).signature));
}

namespace {
std::vector<void *> call_args;
void *return_value;
std::string call_error;

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
int getArgCount() {
    return call_args.size();
}
int moveArg(int index) {
    if (call_args.empty() || call_args.size() < index)
        return false;
    if (index >= 0) {
        call_args[index] = call_args.back();
    } else {
        return_value = call_args.back();
    }
    call_args.pop_back();
    return true;
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
    return addObject(Il2Cpp::CppInterop::ToCsString(call_error));
}
namespace {
void logBadCall() {
    g.logger->warn("WebAssembly interface failed to call function: {}", call_error);
}
}
int call(MethodHandle index, int argCount) {
    // Handle unknown argument count
    if (argCount == unknownArgCount)
        argCount = call_args.size();

    // Make sure argCount matches argument count
    else if (argCount != call_args.size()) {
        call_error = fmt::format("Mismatched added ({}) vs. passed ({}) arg count", call_args.size(), argCount);
        logBadCall();
        return false;
    }

    // Get method
    const auto method = Dynamic::getMethod(index);
    if (!method.isValid()) {
        call_error = fmt::format("Bad method index ({})", index);
        logBadCall();
        return false;
    }

    // Check argument count
    const auto actualArgCount = method.getArgCount();
    if (argCount != actualArgCount) {
        call_error = fmt::format("Mismatched passed ({}) vs. actual ({}) arg count", argCount, actualArgCount);
        logBadCall();
        return false;
    }

    // Call function
    const auto args = std::move(std::exchange(call_args, {}));
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
            call_error = fmt::format("Too many arguments ({}) (max. 6 supported)", argCount);
            logBadCall();
            return false;
        }
        }
    } catch (const std::exception& e) {
        call_error = fmt::format("Method has thrown an exception: {}", e.what());
        logBadCall();
        return false;
    } catch (...) {
        call_error = "Method has thrown an unknown exception";
        logBadCall();
        return false;
    }

    // Everything seems to have gone well
    return true;
}

namespace {
struct WASMGameHookInfo {
    std::unique_ptr<GameHook> hook;
    ModInfo *modInfo;
    std::string callback;
};
std::map<MethodHandle, WASMGameHookInfo> hooks;
MethodHandle currentHookMethod;
void *wasmHook(void *a, void *b, void *c, void *d, void *e, void *f) noexcept {
    // Get method
    const auto method = Dynamic::getMethod(GameHook::getTrampolineCaller());
    if (!method.isValid()) {
        g.logger->error("WASM GameHook interface encountered an invalid hook!");
        return nullptr;
    }
    currentHookMethod = method.index;

    // Get hook
    auto res = hooks.find(currentHookMethod);
    if (res == hooks.end()) {
        g.logger->error("WASM GameHook interface encountered an unknown hook!");
        return nullptr;
    }
    WASMGameHookInfo& hookInfo = res->second;

    // Make sure mod is still loaded
    if (!hookInfo.modInfo->isLoaded()) {
        g.logger->error("WASM GameHook interface attempted to handle hook for unloaded mod!");
        hooks.erase(method.index);
        return nullptr;
    }

    // Set up argument list
    call_args = {a, b, c, d, e, f};
    call_args.resize(method.getArgCount());

    // Call hook callback
    try {
        GameHookRelease GHR(*hooks.at(currentHookMethod).hook);
        hookInfo.modInfo->get<WASMLoader::WASMMod>()->simpleCall(hookInfo.callback.c_str());
        return return_value;
    } catch (const std::exception& e) {
        g.logger->error("Exception while executing WASM GameHook callback '{}': {}", hookInfo.callback, e.what());
        return nullptr;
    } catch (...) {
        g.logger->error("Unknown exception while executing WASM GameHook callback '{}'!", hookInfo.callback);
        return nullptr;
    }
}
GAMEHOOK_TRAMPOLINE(wasmHook)
}
int hook(MethodHandle method, const char *callback) {
    const auto methodInfo = Dynamic::getMethod(method);
    if (!methodInfo.isValid())
        return false;
    if (methodInfo.getArgCount() > 6) {
        g.logger->warn("WASM Mod attempted to hook function with more than 6 arguments (which is unsupported)");
        return false;
    }
    return hooks.emplace(method, WASMGameHookInfo{
                                                  std::make_unique<GameHook>(methodInfo.getFullAddress(), reinterpret_cast<void *>(hookTrampoline_wasmHook), true),
                                                  WASMLoader::WASMMod::getCurrent(),
                                                  callback
                                 }).second;
}
int unhook(MethodHandle method) {
    return hooks.erase(method);
}
MethodHandle getOriginal() {
    return currentHookMethod;
}
}

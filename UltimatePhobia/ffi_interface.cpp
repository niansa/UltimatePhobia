#include "ffi_interface.hpp"
#include "global_state.hpp"
#include "il2cpp_dynamic.hpp"
#include "il2cpp_cppinterop.hpp"
#include "il2cpp_api.hpp"
#include "game_hook.hpp"
#include "ffi_loader.hpp"
#include "anycall.hpp"
#include "mods/base.hpp"

#include <map>
#include <exception>
#include <imgui.h>

using namespace AnyCall;

namespace FFIInterface {
int32_t getFtableItemCount() { return getLocalFtableItemCount(); }

namespace {
template <std::integral HandleT, typename PtrT> class HandleCollection {
    std::map<HandleT, PtrT *> objects;

public:
    HandleT add(PtrT *ptr) {
        if (ptr == nullptr)
            return 0;
        // Find empty handle ID
        static HandleT last_id = 0;
        HandleT fres = ++last_id;
        while (objects.find(fres) != objects.end())
            ++fres;
        if (fres <= 0) {
            g.logger->warn("Ran out of object handles! Please consider dropping "
                           "some handles.");
            return -1;
        }
        objects[fres] = ptr;
        return fres;
    }
    PtrT *get(HandleT id) const {
        if (id == 0)
            return nullptr;
        auto res = objects.find(id);
        if (res == objects.end())
            return nullptr;
        return res->second;
    }

    void drop(HandleT id) {
        if (id > 0)
            objects.erase(id);
    }

    WIBool isValid(HandleT id) const { return id == 0 || objects.find(id) != objects.end(); }
    int64_t getAddress(HandleT id) const { return reinterpret_cast<uintptr_t>(get(id)); }
    static inline HandleT getNull() { return 0; }
};

HandleCollection<ObjectHandle, void /*Il2Cpp::API::Il2CppObject*/> objectHandles;
HandleCollection<ImageHandle, Il2Cpp::API::Il2CppImage> imageHandles;
HandleCollection<ClassHandle, Il2Cpp::API::Il2CppClass> classHandles;
} // namespace

ObjectHandle getNull() { return 0; }

// Object handle management
void dropObject(ObjectHandle id) { objectHandles.drop(id); }
WIBool isValidObject(ObjectHandle id) { return objectHandles.isValid(id); }
int64_t getObjectAddress(ObjectHandle id) { return objectHandles.getAddress(id); }

// Misc handle management
void dropImage(ImageHandle id) { imageHandles.drop(id); }
void dropClass(ClassHandle id) { classHandles.drop(id); }

// String creation
ObjectHandle toCsString(const char *str) { return objectHandles.add(Il2Cpp::CppInterop::ToCsString(str)); }
ObjectHandle toCsStringWithLength(const char *str, int32_t length) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString({str, static_cast<size_t>(length)}));
}
void toCString(ObjectHandle str, char *buf, int32_t maxlen) {
    Il2Cpp::CppInterop::ToCString(reinterpret_cast<System_String_o *>(objectHandles.get(str)), buf, maxlen);
}

// C# runtime wrappers
ImageHandle getImageCorlib() {
    return imageHandles.add(const_cast<Il2Cpp::API::Il2CppImage *>(
        reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(reinterpret_cast<const void *>(Il2Cpp::API::il2cpp_get_corlib()))));
}
ClassHandle getClassFromName(ImageHandle image, const char *namespaze, const char *name) {
    return classHandles.add(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(
        Il2Cpp::API::il2cpp_class_from_name(reinterpret_cast<Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)), namespaze, name)));
}
ObjectHandle getArrayFromClass(ClassHandle elementClass, int32_t rank) {
    return objectHandles.add(
        reinterpret_cast<void *>(Il2Cpp::API::il2cpp_array_class_get(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(elementClass)), rank)));
}
ObjectHandle createArray(ClassHandle elementClass, int32_t length) {
    return objectHandles.add(
        reinterpret_cast<void *>(Il2Cpp::API::il2cpp_array_new(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(elementClass)), length)));
}
void copyArrayBytes(ObjectHandle array, int32_t offset, int32_t length, void *to) {
    auto csArray = reinterpret_cast<System_Byte_array *>(objectHandles.get(array));
    if (csArray)
        memcpy(to, csArray->m_Items + offset, length);
}

// Garbage collection control
GCHandle gcCreateHandle(ObjectHandle object, WIBool pinned) {
    auto ptr = objectHandles.get(object);
    if (ptr == nullptr)
        return -1;
    return Il2Cpp::API::il2cpp_gchandle_new(reinterpret_cast<Il2CppObject *>(ptr), pinned);
}
void gcDeleteHandle(GCHandle handle) { Il2Cpp::API::il2cpp_gchandle_free(handle); }

// Logging
namespace {
void log(spdlog::level::level_enum level, ObjectHandle message) {
    g.logger->log(level, "[{} (FFI)] {}", FFILoader::FFIMod::getCurrent()->name,
                  Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(objectHandles.get(message))));
}
} // namespace
void logTrace(ObjectHandle message) { log(spdlog::level::trace, message); }
void logDebug(ObjectHandle message) { log(spdlog::level::debug, message); }
void logInfo(ObjectHandle message) { log(spdlog::level::info, message); }
void logWarn(ObjectHandle message) { log(spdlog::level::warn, message); }
void logError(ObjectHandle message) { log(spdlog::level::err, message); }
void logCritical(ObjectHandle message) { log(spdlog::level::critical, message); }

// Method management
namespace Dynamic = Il2Cpp::Dynamic;
MethodHandle getMethodByIdentifier(const char *identifier) { return Dynamic::getMethod(identifier, true).index; }
MethodHandle getMethodByAddress(int64_t addr) { return Dynamic::getMethod(reinterpret_cast<void *>(addr), true).index; }
ObjectHandle getMethodName(MethodHandle index) {
    if (index < 0)
        return 0;
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).name));
}
ObjectHandle getMethodSignature(MethodHandle index) {
    if (index < 0)
        return 0;
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).signature));
}
int64_t getMethodAddresss(MethodHandle index) {
    if (index < 0)
        return 0;
    return reinterpret_cast<uintptr_t>(Dynamic::getMethod(index).getFullAddress());
}
WIBool registerICallMethod(const char *identifier, const char *typeSignature) {
    if (!Dynamic::getMethod(identifier, true).isValid())
        return Dynamic::registerICall(identifier, typeSignature);
    return true;
}

// Argument stack management
namespace {
std::vector<void *> call_args;
void *return_value;
std::string call_error;

void *getValue(int32_t index) {
    if (index < 0)
        return return_value;
    if (index > call_args.size())
        return nullptr;
    return call_args[index];
}
} // namespace

void addArgI32(int32_t v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgI64(int64_t v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgFloat(float v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgDouble(double v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgObject(ObjectHandle v) { call_args.push_back(objectHandles.get(v)); }
void addArgNull() { call_args.push_back(nullptr); }
void clearArgs() { call_args.clear(); }

int32_t getArgCount() { return call_args.size(); }
WIBool moveArg(int32_t index) {
    if (call_args.empty() || static_cast<int>(call_args.size()) < index)
        return false;
    if (index >= 0) {
        call_args[index] = call_args.back();
    } else {
        return_value = call_args.back();
    }
    call_args.pop_back();
    return true;
}

int32_t getValueI32(int32_t index) { return bit_cast<int32_t>(getValue(index)); }
int64_t getValueI64(int32_t index) { return bit_cast<int64_t>(getValue(index)); }
float getValueFloat(int32_t index) {
    void *value = getValue(index);
    return bit_cast<float>(value);
}
double getValueDouble(int32_t index) {
    void *value = getValue(index);
    return bit_cast<double>(value);
}
ObjectHandle getValueObject(int32_t index) { return objectHandles.add(getValue(index)); }

// Method calling
ObjectHandle getCallError() { return objectHandles.add(Il2Cpp::CppInterop::ToCsString(call_error)); }

namespace {
void logBadCall(MethodHandle index) {
    const auto& method = Il2Cpp::Dynamic::getMethod(index);
    g.logger->warn("FFI` interface failed to call function '{}': {}",
                   method.isValid() ? method.signature.empty() ? method.name : method.signature : "<invalid>", call_error);
}
} // namespace

WIBool call2(MethodHandle index, int32_t argCount, WIBool returnsStruct) {
    // Handle unknown argument count
    if (argCount == unknownArgCount)
        argCount = call_args.size();

    // Make sure argCount matches argument count
    else if (argCount != call_args.size()) {
        call_error = fmt::format("Mismatched added ({}) vs. passed ({}) arg count", call_args.size(), argCount);
        logBadCall(index);
        return false;
    }

    // Get method
    const auto method = Dynamic::getMethod(index);
    if (!method.isValid()) {
        call_error = fmt::format("Bad method index ({})", index);
        logBadCall(index);
        return false;
    }

    // Check argument count
    const auto actualArgCount = method.getArgCount();
    if (argCount != actualArgCount) {
        call_error = fmt::format("Mismatched passed ({}) vs. actual ({}) arg count", argCount, actualArgCount);
        logBadCall(index);
        return false;
    }

    // Back up current mod in case called function triggers hook from another
    // mod
    ModInfo *currentModBackup = FFILoader::FFIMod::getCurrent();

    // Call function
    const auto args = std::move(std::exchange(call_args, {}));
    bool fres = true;
    try {
        if (!returnsStruct) {
            return_value =
                reinterpret_cast<void *>(AnyCall::call(reinterpret_cast<const uintptr_t *>(args.data()), method.getFullAddress(), method.typeSignature));
        } else {
            auto *dataDest = reinterpret_cast<System_Byte_array *>(return_value);
            const AnyCall::Struct returnData =
                AnyCall::callStruct(reinterpret_cast<const uintptr_t *>(args.data()), method.getFullAddress(), method.typeSignature);
            std::memcpy(dataDest->m_Items, &returnData, sizeof(returnData));
            return_value = dataDest;
        }
    } catch (const std::exception& e) {
        call_error = fmt::format("Method has thrown an exception: {}", e.what());
        logBadCall(index);
        fres = false;
    } catch (...) {
        call_error = "Method has thrown an unknown exception";
        logBadCall(index);
        fres = false;
    }

    // Restore current mod (see comment near declaration)
    FFILoader::FFIMod::setCurrent(currentModBackup);

    // Clear arguments again in case called function ended up adding arguments
    // through hook
    call_args.clear();

    // Everything seems to have gone well
    return fres;
}
WIBool call(MethodHandle index, int32_t argCount) { return call2(index, argCount, false); }

// Hook management
namespace {
struct FFIGameHookInfo {
    std::shared_ptr<GameHook> hook;
    ModInfo *modInfo;
    std::string callback;
};
std::map<MethodHandle, FFIGameHookInfo> hooks;
MethodHandle currentHookMethod;
void *ffiHook(void *a, void *b, void *c, void *d, void *e, void *f) noexcept {
    // Get method
    const auto method = Dynamic::getMethod(GameHook::getTrampolineCaller());
    if (!method.isValid()) {
        g.logger->error("FFI GameHook interface encountered an invalid hook!");
        return nullptr;
    }
    currentHookMethod = method.index;

    // Get hook
    auto res = hooks.find(currentHookMethod);
    if (res == hooks.end()) {
        g.logger->error("FFI GameHook interface encountered an unknown hook!");
        return nullptr;
    }
    FFIGameHookInfo& hookInfo = res->second;
    auto hook = hookInfo.hook; // Prevents UB when hook is erased in callback

    // Make sure mod is still loaded
    if (!hookInfo.modInfo->isLoaded()) {
        g.logger->error("FFI GameHook interface attempted to handle hook for "
                        "unloaded mod!");
        hooks.erase(method.index);
        return nullptr;
    }

    // Set up argument list
    call_args = {a, b, c, d, e, f};
    call_args.resize(method.getArgCount());

    // Call hook callback
    try {
        GameHookRelease GHR(*hooks.at(currentHookMethod).hook);
        hookInfo.modInfo->get<FFILoader::FFIMod>()->simpleCall(hookInfo.callback.c_str());
        return return_value;
    } catch (const std::exception& e) {
        g.logger->error("Exception while executing FFI GameHook callback '{}': {}", hookInfo.callback, e.what());
        return nullptr;
    } catch (const ModPanic& e) {
        g.logger->error("FFI Mod '{}' has panicked: {}", e.where(), e.what());
        return nullptr;
    } catch (...) {
        g.logger->error("Unknown exception while executing FFI GameHook callback '{}'!", hookInfo.callback);
        return nullptr;
    }
}
GAMEHOOK_TRAMPOLINE(ffiHook)
} // namespace
WIBool hook(MethodHandle method, const char *callback) {
    const auto methodInfo = Dynamic::getMethod(method);
    if (!methodInfo.isValid())
        return false;
    if (methodInfo.getArgCount() > 6) {
        g.logger->warn("FFI Mod attempted to hook function with more than 6 "
                       "arguments (which is unsupported)");
        return false;
    }
    auto hook = GameHook::safeCreate(methodInfo.getFullAddress(), reinterpret_cast<void *>(hookTrampoline_ffiHook), true);
    if (!hook.has_value())
        return false;
    return hooks.emplace(method, FFIGameHookInfo{std::make_shared<GameHook>(std::move(*hook)), FFILoader::FFIMod::getCurrent(), callback}).second;
}
WIBool unhook(MethodHandle method) { return hooks.erase(method); }
MethodHandle getOriginal() { return currentHookMethod; }

// ImGui wrappers
void ImGuiBegin(const char *name) { ImGui::Begin(name); }
void ImGuiEnd() { ImGui::End(); }
void ImGuiText(ObjectHandle text) {
    ImGui::TextUnformatted(Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(objectHandles.get(text))).c_str());
}
void ImGuiCheckbox(const char *label, bool *v) { ImGui::Checkbox(label, v); }
WIBool ImGuiCheckbox2(const char *label, WIBool v) {
    bool fres = v;
    ImGui::Checkbox(label, &fres);
    return fres;
}
WIBool ImGuiCheckbox3(const char *label, bool *v) { return ImGui::Checkbox(label, v); }
int32_t ImGuiCheckbox4(const char *label, WIBool v) {
    bool state = v;
    bool fres = ImGui::Checkbox(label, &state);
    return (state ? 0b01 : 0) | (fres ? 0b10 : 0);
}
WIBool ImGuiButton(const char *label) { return ImGui::Button(label); }
void ImGuiSeparator() { ImGui::Separator(); }
void ImGuiSeparatorText(const char *label) { ImGui::SeparatorText(label); }

// WASM runtime support
void abort(const char *message, const char *filename, int32_t lineNumber, int32_t columnNumber) {
    auto modInfo = FFILoader::FFIMod::getCurrent();
    const auto msg = fmt::format("FFI module {} has called abort()!\n - Message: "
                                 "{}\n - Filename: {}\n - Line: {}\n - Column: {}",
                                 modInfo->name, message ? message : "none", filename ? filename : "unknown", lineNumber, columnNumber);
    g.logger->critical(msg);
    throw ModPanic(*modInfo, msg);
}
} // namespace FFIInterface

#pragma once

#include "global_state.hpp"
#include "il2cpp_api_cpp.hpp"

#include <string_view>
#include <vector>
#include <variant>
#include <functional>

namespace Il2Cpp::Dynamic {
using ApiMethod = API::Method;

struct Method {
    std::variant<void *, ApiMethod> method;
    std::string_view name, signature, typeSignature;
    int index = -1;

    bool isValid() const {
        if (auto *mi = std::get_if<ApiMethod>(&method)) [[likely]]
            return mi->ptr != nullptr;
        if (auto *addr = std::get_if<void *>(&method))
            return *addr != nullptr;
        __builtin_unreachable();
        return false;
    }

    void *getFullAddress() const {
        if (auto *mi = std::get_if<ApiMethod>(&method)) [[likely]]
            return mi->function_pointer();
        if (auto *addr = std::get_if<void *>(&method))
            return *addr;
        __builtin_unreachable();
        return nullptr;
    }

    template <typename fncT> fncT *getFunction() const { return reinterpret_cast<fncT *>(getFullAddress()); }

    unsigned getArgCount() const { return typeSignature.size() - 1; }
};

void init(std::function<void()> onComplete);
std::string dump();
bool isLoaded();
Method getMethod(std::string_view identifier, bool noError = false);
Method getMethod(void *addr, bool noError = false);
Method getMethod(unsigned idx, bool noError = false);
std::vector<Method> searchMethods(std::string_view identifier);
const std::vector<Method>& getMethods();
bool registerICall(const char *name, std::string_view typeSignature);
} // namespace Il2Cpp::Dynamic

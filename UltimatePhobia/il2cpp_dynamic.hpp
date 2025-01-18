#pragma once

#include "global_state.hpp"

#include <string_view>
#include <vector>


namespace Il2Cpp::Dynamic {
struct Method {
    uintptr_t address;
    std::string_view name, signature, typeSignature;
    int index = -1;

    bool isValid() const {
        return address != 0;
    }

    void *getFullAddress() const {
        return reinterpret_cast<void *>(g.base + address);
    }

    template<typename fncT>
    fncT *getFunction() const {
        return reinterpret_cast<fncT*>(getFullAddress());
    }

    unsigned getArgCount() const {
        return typeSignature.size() - 1;
    }
};


void init();
bool isLoaded();
Method getMethod(std::string_view identifier, bool noError = false);
Method getMethod(void *addr, bool noError = false);
Method getMethod(unsigned idx, bool noError = false);
std::vector<Method> searchMethods(std::string_view identifier);
const std::vector<Method>& getMethods();
}

#pragma once

#include <string_view>
#include <vector>


namespace Il2Cpp::Dynamic {
struct Method {
    void *address;
    std::string_view name, signature, typeSignature;

    bool isValid() const {
        return address != nullptr;
    }

    template<typename fncT>
    fncT *getFunction() const {
        return reinterpret_cast<fncT*>(address);
    }
};


void init();
bool isLoaded();
Method getMethod(std::string_view identifier, bool noError = false);
Method getMethod(void *addr, bool noError = false);
std::vector<Method> searchMethods(std::string_view identifier);
}

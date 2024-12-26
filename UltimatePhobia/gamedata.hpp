#pragma once

#include <string_view>
#include <vector>


namespace GameData {
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


Method getMethod(std::string_view identifier);
Method getMethod(void *addr);
std::vector<Method> searchMethods(std::string_view identifier);
}

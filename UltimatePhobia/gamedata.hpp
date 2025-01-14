#ifdef ENABLE_OLD_GAMEDATA
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


Method getMethod(std::string_view identifier, bool noError = false);
Method getMethod(void *addr, bool noError = false);
std::vector<Method> searchMethods(std::string_view identifier);
}
#endif

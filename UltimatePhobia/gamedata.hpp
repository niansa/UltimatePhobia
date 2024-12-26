#pragma once

#include <string_view>
#include <vector>


struct GameMethod {
    void *address;
    std::string_view name, signature, typeSignature;

    bool isValid() const {
        return address != nullptr;
    }

    template<typename fncT>
    fncT *getFunction() {
        return reinterpret_cast<fncT*>(address);
    }
};


GameMethod getGameMethod(std::string_view identifier);
GameMethod getGameMethod(void *addr);
std::vector<GameMethod> searchGameMethods(std::string_view identifier);

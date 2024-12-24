#pragma once

#include <string_view>


struct GameMethod {
    void *address;
    std::string_view name, signature, typeSignature;

    bool isValid() const {
        return address != nullptr;
    }
};


GameMethod getGameMethod(std::string_view identifier);

#include "gamedata.hpp"
#include "generated/game_data.inc"
#include "global_state.hpp"

#include <cstdint>



namespace {
inline void *calculateAddress(void *addr) {
    return reinterpret_cast<uint8_t *>(g.base) + reinterpret_cast<intptr_t>(addr);
}
}


GameMethod getGameMethod(std::string_view identifier) {
    for (const GameMethod& method : methods) {
        if (method.signature == identifier) {
            GameMethod fres(method);
            fres.address = calculateAddress(fres.address);
            return fres;
        }
    }

    for (const GameMethod& method : methods) {
        if (method.name == identifier) {
            GameMethod fres(method);
            fres.address = calculateAddress(fres.address);
            return fres;
        }
    }

    return {nullptr};
}

GameMethod getGameMethod(void *addr) {
    for (GameMethod method : methods) {
        method.address = calculateAddress(method.address);
        if (method.address == addr) {
            return method;
        }
    }

    return {nullptr};
}

std::vector<GameMethod> searchGameMethods(std::string_view identifier) {
    std::vector<GameMethod> fres;

    const auto isDup = [&fres] (const GameMethod& method) {
        for (const auto& other_method : fres)
            if (method.signature == other_method.signature)
                return true;
        return false;
    };

    // Try exact search first
    {
        const auto result = getGameMethod(identifier);
        if (result.isValid())
            fres.emplace_back(std::move(result));
    }

    // Then try name search
    for (const GameMethod& method : methods) {
        if (isDup(method))
            continue;
        if (method.name.find(identifier) != std::string_view::npos) {
            GameMethod result(method);
            result.address = calculateAddress(result.address);
            fres.emplace_back(std::move(result));
        }
    }

    // Finally try signature search
    for (const GameMethod& method : methods) {
        if (isDup(method))
            continue;
        if (method.signature.find(identifier) != std::string_view::npos) {
            GameMethod result(method);
            result.address = calculateAddress(result.address);
            fres.emplace_back(std::move(result));
        }
    }

    return fres;
}

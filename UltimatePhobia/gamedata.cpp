#ifdef ENABLE_OLD_GAMEDATA
#include "gamedata.hpp"
#include "generated/game_data.inc"
#include "global_state.hpp"

#include <cstdint>



namespace GameData {
namespace {
inline void *calculateAddress(void *addr) {
    return reinterpret_cast<uint8_t *>(g.base) + reinterpret_cast<intptr_t>(addr);
}
}


Method getMethod(std::string_view identifier, bool noError) {
    for (const Method& method : methods) {
        if (method.signature == identifier) {
            Method fres(method);
            fres.address = calculateAddress(fres.address);
            return fres;
        }
    }

    for (const Method& method : methods) {
        if (method.name == identifier) {
            Method fres(method);
            fres.address = calculateAddress(fres.address);
            return fres;
        }
    }

    if (!noError)
        g.logger->error("Failed to find method by identifier: {}", identifier);
    return {nullptr};
}

Method getMethod(void *addr, bool noError) {
    for (Method method : methods) {
        method.address = calculateAddress(method.address);
        if (method.address == addr) {
            return method;
        }
    }

    if (!noError)
        g.logger->error("Failed to find method by address: {}", addr);
    return {nullptr};
}

std::vector<Method> searchMethods(std::string_view identifier) {
    std::vector<Method> fres;

    const auto isDup = [&fres] (const Method& method) {
        for (const auto& other_method : fres)
            if (method.signature == other_method.signature)
                return true;
        return false;
    };

    // Try exact search first
    {
        const auto result = getMethod(identifier, true);
        if (result.isValid())
            fres.emplace_back(std::move(result));
    }

    // Then try name search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.name.find(identifier) != std::string_view::npos) {
            Method result(method);
            result.address = calculateAddress(result.address);
            fres.emplace_back(std::move(result));
        }
    }

    // Finally try signature search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.signature.find(identifier) != std::string_view::npos) {
            Method result(method);
            result.address = calculateAddress(result.address);
            fres.emplace_back(std::move(result));
        }
    }

    return fres;
}
}
#endif

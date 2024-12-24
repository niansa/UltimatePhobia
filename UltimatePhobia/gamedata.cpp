#include "gamedata.hpp"
#include "generated/game_data.inc"
#include "global_state.hpp"



GameMethod getGameMethod(std::string_view identifier) {
    for (const GameMethod& method : methods) {
        if (method.signature == identifier) {
            GameMethod fres(method);
            fres.address = reinterpret_cast<char *>(g.base) + reinterpret_cast<intptr_t>(fres.address);
            return fres;
        }
    }

    for (const GameMethod& method : methods) {
        if (method.name == identifier) {
            GameMethod fres(method);
            fres.address = reinterpret_cast<char *>(g.base) + reinterpret_cast<intptr_t>(fres.address);
            return fres;
        }
    }

    return {nullptr};
}

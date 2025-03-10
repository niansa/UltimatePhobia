#include "il2cpp_api.hpp"
#include "global_state.hpp"

#include <windows.h>

namespace Il2Cpp {
namespace API {
#define DO_API(r, n, p) r(*n) p
#include "il2cpp_api.inc"
#undef DO_API

void init() {
    const auto module = GetModuleHandleA("GameAssembly.dll");
    if (module == NULL) {
        g.logger->error("Failed to initialize il2cpp API");
        return;
    }

#define DO_API(r, n, p)                                                                                                                                        \
    n = reinterpret_cast<decltype(n)>(GetProcAddress(module, #n));                                                                                             \
    if (n == NULL)                                                                                                                                             \
        g.logger->error("Failed to get '" #n "' function from il2cpp API");
#include "il2cpp_api.inc"
#undef DO_API
}
} // namespace API
} // namespace Il2Cpp

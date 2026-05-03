#include "il2cpp_api.hpp"
#include "global_state.hpp"

#include <cstdlib>
#include <windows.h>
#include <tracy/Tracy.hpp>

namespace Il2Cpp {
namespace API {
#define DO_API(r, n, p) r(*n) p
#include "il2cpp_api.inc"
#undef DO_API

namespace {
void *stdMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr)
        TracyAlloc(ptr, size);
    return ptr;
}

void *stdAlignedMalloc(size_t size, size_t alignment) {
    void *ptr = _aligned_malloc(alignment, size);
    if (ptr)
        TracyAlloc(ptr, size);
    return ptr;
}

void stdFree(void *ptr) {
    if (ptr)
        TracyFree(ptr);
    free(ptr);
}

void stdAlignedFree(void *ptr) {
    if (ptr)
        TracyFree(ptr);
    _aligned_free(ptr);
}

void *stdCalloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (ptr)
        TracyAlloc(ptr, nmemb * size);
    return ptr;
}

void *stdRealloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (new_ptr || size == 0) {
        if (ptr)
            TracyFree(ptr);
        if (new_ptr)
            TracyAlloc(new_ptr, size);
    }
    return new_ptr;
}

void *stdAlignedRealloc(void *ptr, size_t size, size_t alignment) {
    void *new_ptr = _aligned_realloc(ptr, size, alignment);
    if (new_ptr || size == 0) {
        if (ptr)
            TracyFree(ptr);
        if (new_ptr)
            TracyAlloc(new_ptr, size);
    }
    return new_ptr;
}

void enableStdAllocInIl2Cpp() {
    Il2CppMemoryCallbacks callbacks;
    callbacks.malloc_func = stdMalloc;
    callbacks.aligned_malloc_func = stdAlignedMalloc;
    callbacks.free_func = stdFree;
    callbacks.aligned_free_func = stdAlignedFree;
    callbacks.calloc_func = stdCalloc;
    callbacks.realloc_func = stdRealloc;
    callbacks.aligned_realloc_func = stdAlignedRealloc;
    il2cpp_set_memory_callbacks(&callbacks);
}
} // namespace

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

#ifdef TRACY_ENABLE
    enableStdAllocInIl2Cpp();
#endif
}
} // namespace API
} // namespace Il2Cpp

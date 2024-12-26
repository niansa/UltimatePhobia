#include "il2cpp.hpp"
#include "global_state.hpp"

#include <string_view>
#include <map>



namespace il2cpp {
namespace {
struct Il2CppDomain;
struct Il2CppImage;

struct Il2CppAssemblyName {
    const char* name;
    const char* culture;
    const uint8_t* public_key;
    uint32_t hash_alg;
    int32_t hash_len;
    uint32_t flags;
    int32_t major;
    int32_t minor;
    int32_t build;
    int32_t revision;
    uint8_t public_key_token[8];
};
struct Il2CppAssembly {
    Il2CppImage* image;
    uint32_t token;
    int32_t referencedAssemblyStart;
    int32_t referencedAssemblyCount;
    Il2CppAssemblyName aname;
};

HMODULE &gameAssemblyDll = *reinterpret_cast<HMODULE *>(&g.base);
std::map<std::string_view, const Il2CppAssembly *> modules;

Il2CppDomain *il2cpp_domain_get() {
    return reinterpret_cast<decltype(il2cpp_domain_get)*>(GetProcAddress(gameAssemblyDll, "il2cpp_domain_get"))();
}
const Il2CppAssembly **il2cpp_domain_get_assemblies(const Il2CppDomain *domain, size_t *size) {
    return reinterpret_cast<decltype(il2cpp_domain_get_assemblies)*>(GetProcAddress(gameAssemblyDll, "il2cpp_domain_get_assemblies"))(domain, size);
}
const char *il2cpp_image_get_name(const Il2CppImage *image) {
    return reinterpret_cast<decltype(il2cpp_image_get_name)*>(GetProcAddress(gameAssemblyDll, "il2cpp_image_get_name"))(image);
}
Il2CppClass *il2cpp_class_from_name(const Il2CppImage *image, const char *namespaze, const char *name) {
    return reinterpret_cast<decltype(il2cpp_class_from_name)*>(GetProcAddress(gameAssemblyDll, "il2cpp_class_from_name"))(image, namespaze, name);
}
Il2CppObject *il2cpp_object_new(const Il2CppClass *klass) {
    return reinterpret_cast<decltype(il2cpp_object_new)*>(GetProcAddress(gameAssemblyDll, "il2cpp_object_new"))(klass);
}
}


void init() {
    g.logger->info("Initializing il2cpp...");

    const Il2CppDomain* domain = il2cpp_domain_get();

    const Il2CppAssembly **assemblies;
    size_t size;

    assemblies = il2cpp_domain_get_assemblies(domain, &size);

    for (size_t i = 0; i < size; ++i) {
        const Il2CppAssembly *assembly = assemblies[i];

        if (!assembly)
            continue;

        std::string_view assemblyName = il2cpp_image_get_name(assembly->image);
        modules[assemblyName] = assembly;
    }
}

Il2CppClass *getClass(std::string_view module, const char *namespaze, const char *name) {
    auto module_res = modules.find(module);
    if (module_res == modules.end())
        return nullptr;

    return il2cpp_class_from_name(module_res->second->image, namespaze, name);
}

Il2CppObject *createObject(const Il2CppClass *klass) {
    return il2cpp_object_new(klass);
}
}

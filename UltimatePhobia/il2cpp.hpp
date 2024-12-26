#pragma once

#include <string_view>
#include <windows.h>


struct Il2CppClass;


namespace il2cpp {
void init();
Il2CppClass *getClass(std::string_view module, const char *namespaze, const char *name);
Il2CppObject *createObject(const Il2CppClass *);
};

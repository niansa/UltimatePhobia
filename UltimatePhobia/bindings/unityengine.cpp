#include "unityengine.hpp"
#include "il2cpp_cppinterop.hpp"
#include "global_state.hpp"
#include "generated/il2cpp.hpp"

using namespace Il2Cpp;



namespace {
System_Type_o *GetType(std::string_view name) {
    const auto fres = System::Type::GetType(CppInterop::ToCsString(name));
    if (!fres)
        g.logger->error("Failed to get type '{}'!", name);
    return fres;
}

System_Type_o *GetType(std::string_view name, std::string_view assemblyName) {
    return GetType(fmt::format("{}, {}, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null", name, assemblyName));
}

void *CreateInstance(System_Type_o *type) {
    return System::Activator::CreateInstance(type);
}
}


namespace Il2Cpp::UnityEngine {
UnityEngine_GameObject_o *GameObject::New(std::string_view name) {
    auto fres = reinterpret_cast<UnityEngine_GameObject_o *>(CreateInstance(GetType("UnityEngine.GameObject", "UnityEngine.CoreModule")));
    _ctor(fres, CppInterop::ToCsString(name));
    return fres;
}

void GameObject::RecursiveDestroy(UnityEngine_GameObject_o *__this) {
    auto transform = get_transform(__this);
    while (Transform::get_childCount(transform) > 0)
        RecursiveDestroy(Component::get_gameObject(reinterpret_cast<UnityEngine_Component_o *>((Transform::GetChild(transform, 0)))));
    Object::Destroy(reinterpret_cast<UnityEngine_Object_o *>(__this));
}

UnityEngine_Component_o *GameObject::AddComponent(UnityEngine_GameObject_o *__this, std::string_view name, std::string_view assemblyName) {
    return AddComponent(__this, GetType(name, assemblyName));
}

UnityEngine_GameObject_o *GameObject::get_parent(UnityEngine_GameObject_o *__this) {
    return Component::get_gameObject(reinterpret_cast<UnityEngine_Component_o *>((Transform::get_parent(get_transform(__this)))));
}
}

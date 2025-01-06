#include "unityengine.hpp"
#include "gamedata.hpp"
#include "game_types.hpp"
#include "global_state.hpp"



namespace {
System_Type_o *GetType(std::string_view name) {
    const auto fres = GameData::getMethod("System_Type_o* System_Type__GetType (System_String_o* typeName, const MethodInfo* method);")
        .getFunction<System_Type_o *(System_String_o *, const MethodInfo *)>()
        (GameTypes::createCsString(name), nullptr);
    if (!fres)
        g.logger->error("Failed to get type '{}'!", name);
    return fres;
}

System_Type_o *GetType(std::string_view name, std::string_view assemblyName) {
    return GetType(fmt::format("{}, {}, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null", name, assemblyName));
}

void *CreateInstance(System_Type_o *type) {
    return GameData::getMethod("Il2CppObject* System_Activator__CreateInstance (System_Type_o* type, const MethodInfo* method);")
        .getFunction<void *(System_Type_o *, const MethodInfo *)>()
        (type, nullptr);
}
}


namespace UnityEngine {
UnityEngine_GameObject_o *GameObject::New(std::string_view name) {
    auto fres = reinterpret_cast<UnityEngine_GameObject_o *>(CreateInstance(GetType("UnityEngine.GameObject", "UnityEngine.CoreModule")));
    GameData::getMethod("void UnityEngine_GameObject___ctor (UnityEngine_GameObject_o* __this, System_String_o* name, const MethodInfo* method);")
        .getFunction<void (UnityEngine_GameObject_o *, System_String_o *, const MethodInfo *)>()
        (fres, GameTypes::createCsString(name), nullptr);
    return fres;
}

UnityEngine_GameObject_o *GameObject::Find(std::string_view name) {
    return GameData::getMethod("UnityEngine.GameObject$$Find")
        .getFunction<UnityEngine_GameObject_o *(System_String_o *, const MethodInfo *)>()
        (GameTypes::createCsString(name), nullptr);
}

void GameObject::Destroy(UnityEngine_GameObject_o *__this) {
    GameData::getMethod("void UnityEngine_Object__Destroy (UnityEngine_Object_o* obj, const MethodInfo* method);")
        .getFunction<void (UnityEngine_GameObject_o *, const MethodInfo *)>()
        (__this, nullptr);
}

void GameObject::RecursiveDestroy(UnityEngine_GameObject_o *__this) {
    auto transform = get_transform(__this);
    while (Transform::get_childCount(transform) > 0)
        RecursiveDestroy(Component::get_gameObject(Component::From(Transform::GetChild(transform))));
    Destroy(__this);
}

void GameObject::SetActive(UnityEngine_GameObject_o *__this, bool value) {
    GameData::getMethod("UnityEngine.GameObject$$SetActive")
        .getFunction<void (UnityEngine_GameObject_o *, bool, const MethodInfo *)>()
        (__this, value, nullptr);
}

UnityEngine_Component_o *GameObject::AddComponent(UnityEngine_GameObject_o *__this, std::string_view name, std::string_view assemblyName) {
    return GameData::getMethod("UnityEngine.GameObject$$AddComponent")
        .getFunction<UnityEngine_Component_o *(UnityEngine_GameObject_o *, System_Type_o *, const MethodInfo *)>()
        (__this, GetType(name, assemblyName), nullptr);
}

UnityEngine_Transform_o *GameObject::get_transform(UnityEngine_GameObject_o *__this) {
    return GameData::getMethod("UnityEngine.GameObject$$get_transform")
        .getFunction<UnityEngine_Transform_o *(UnityEngine_GameObject_o *, const MethodInfo *)>()
        (__this, nullptr);
}

UnityEngine_GameObject_o *GameObject::get_parent(UnityEngine_GameObject_o *__this) {
    return Component::get_gameObject(Component::From(Transform::get_parent(get_transform(__this))));
}


UnityEngine_Transform_o *Transform::GetChild(UnityEngine_Transform_o *__this, int32_t index) {
    return GameData::getMethod("UnityEngine.Transform$$GetChild")
        .getFunction<UnityEngine_Transform_o *(UnityEngine_Transform_o *, int32_t, const MethodInfo *)>()
        (__this, index, nullptr);
}

void Transform::SetParent(UnityEngine_Transform_o *__this, UnityEngine_Transform_o *parent, bool worldPositionStays) {
    GameData::getMethod("void UnityEngine_Transform__SetParent (UnityEngine_Transform_o* __this, UnityEngine_Transform_o* parent, bool worldPositionStays, const MethodInfo* method);")
        .getFunction<UnityEngine_Transform_o *(UnityEngine_Transform_o *, UnityEngine_Transform_o *, bool, const MethodInfo *)>()
        (__this, parent, worldPositionStays, nullptr);
}

int32_t Transform::get_childCount(UnityEngine_Transform_o *__this) {
    return GameData::getMethod("UnityEngine.Transform$$get_childCount")
        .getFunction<int32_t (UnityEngine_Transform_o *, const MethodInfo *)>()
        (__this, nullptr);
}

UnityEngine_Vector3_o Transform::get_position(UnityEngine_Transform_o *__this) {
    return GameData::getMethod("UnityEngine.Transform$$get_position")
        .getFunction<UnityEngine_Vector3_o (UnityEngine_Transform_o *, const MethodInfo *)>()
        (__this, nullptr);
}

void Transform::set_position(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value) {
    GameData::getMethod("UnityEngine.Transform$$set_position")
        .getFunction<void (UnityEngine_Transform_o *, UnityEngine_Vector3_o, const MethodInfo *)>()
        (__this, value, nullptr);
}

UnityEngine_Transform_o *Transform::get_parent(UnityEngine_Transform_o *__this) {
    return GameData::getMethod("UnityEngine.Transform$$get_parent")
        .getFunction<UnityEngine_Transform_o *(UnityEngine_Transform_o *, const MethodInfo *)>()
        (__this, nullptr);
}


UnityEngine_GameObject_o *Component::get_gameObject(UnityEngine_Component_o *__this) {
    return GameData::getMethod("UnityEngine.Component$$get_gameObject")
        .getFunction<UnityEngine_GameObject_o *(UnityEngine_Component_o *, const MethodInfo *)>()
        (__this, nullptr);
}


UnityEngine_Vector3_o Camera::WorldToScreenPoint(UnityEngine_Camera_o *__this, UnityEngine_Vector3_o position) {
    return GameData::getMethod("UnityEngine_Vector3_o UnityEngine_Camera__WorldToScreenPoint (UnityEngine_Camera_o* __this, UnityEngine_Vector3_o position, const MethodInfo* method);")
        .getFunction<UnityEngine_Vector3_o (UnityEngine_Camera_o *, UnityEngine_Vector3_o, const MethodInfo *)>()
        (__this, position, nullptr);
}

UnityEngine_Camera_o *Camera::get_main() {
    return GameData::getMethod("UnityEngine.Camera$$get_main")
        .getFunction<UnityEngine_Camera_o *(const MethodInfo *)>()
        (nullptr);
}


void TextMesh::set_text(UnityEngine_TextMesh_o *__this, std::string_view value) {
    GameData::getMethod("UnityEngine.TextMesh$$set_text")
        .getFunction<void (UnityEngine_TextMesh_o *, System_String_o *, const MethodInfo *)>()
        (__this, GameTypes::createCsString(value), nullptr);
}

void TextMesh::set_characterSize(UnityEngine_TextMesh_o *__this, float value) {
    GameData::getMethod("UnityEngine.TextMesh$$set_characterSize")
        .getFunction<void (UnityEngine_TextMesh_o *, float, const MethodInfo *)>()
        (__this, value, nullptr);
}

void TextMesh::set_alignment(UnityEngine_TextMesh_o *__this, TextAlignment value) {
    GameData::getMethod("UnityEngine.TextMesh$$set_alignment")
        .getFunction<void (UnityEngine_TextMesh_o *, int32_t, const MethodInfo *)>()
        (__this, static_cast<int32_t>(value), nullptr);
}

void TextMesh::set_anchor(UnityEngine_TextMesh_o *__this, TextAnchor value) {
    GameData::getMethod("UnityEngine.TextMesh$$set_anchor")
        .getFunction<void (UnityEngine_TextMesh_o *, int32_t, const MethodInfo *)>()
        (__this, static_cast<int32_t>(value), nullptr);
}


void SceneManagement::SceneManager::LoadScene(std::string_view sceneName) {
    GameData::getMethod("void UnityEngine_SceneManagement_SceneManager__LoadScene (System_String_o* sceneName, const MethodInfo* method);")
        .getFunction<void (System_String_o *, const MethodInfo *)>()
        (GameTypes::createCsString(sceneName), nullptr);
}
}

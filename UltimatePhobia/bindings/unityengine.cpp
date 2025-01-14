#include "unityengine.hpp"
#include "game_types.hpp"
#include "global_state.hpp"
#include "generated/il2cpp.hpp"

using namespace Il2Cpp::Methods;



namespace {
System_Type_o *GetType(std::string_view name) {
    const auto fres = System_Type__GetType(GameTypes::createCsString(name));
    if (!fres)
        g.logger->error("Failed to get type '{}'!", name);
    return fres;
}

System_Type_o *GetType(std::string_view name, std::string_view assemblyName) {
    return GetType(fmt::format("{}, {}, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null", name, assemblyName));
}

void *CreateInstance(System_Type_o *type) {
    return System_Activator__CreateInstance(type);
}
}


namespace UnityEngine {
UnityEngine_GameObject_o *GameObject::New(std::string_view name) {
    auto fres = reinterpret_cast<UnityEngine_GameObject_o *>(CreateInstance(GetType("UnityEngine.GameObject", "UnityEngine.CoreModule")));
    UnityEngine_GameObject___ctor(fres, GameTypes::createCsString(name));
    return fres;
}

UnityEngine_GameObject_o *GameObject::Find(std::string_view name) {
    return UnityEngine_GameObject__Find(GameTypes::createCsString(name));
}

void GameObject::Destroy(UnityEngine_GameObject_o *__this) {
    UnityEngine_Object__Destroy(reinterpret_cast<UnityEngine_Object_o *>(__this));
}

void GameObject::RecursiveDestroy(UnityEngine_GameObject_o *__this) {
    auto transform = get_transform(__this);
    while (Transform::get_childCount(transform) > 0)
        RecursiveDestroy(Component::get_gameObject(Component::From(Transform::GetChild(transform))));
    Destroy(__this);
}

void GameObject::SetActive(UnityEngine_GameObject_o *__this, bool value) {
    UnityEngine_GameObject__SetActive(__this, value);
}

UnityEngine_Component_o *GameObject::AddComponent(UnityEngine_GameObject_o *__this, std::string_view name, std::string_view assemblyName) {
    return UnityEngine_GameObject__AddComponent(__this, GetType(name, assemblyName));
}

UnityEngine_Transform_o *GameObject::get_transform(UnityEngine_GameObject_o *__this) {
    return UnityEngine_GameObject__get_transform(__this);
}

UnityEngine_GameObject_o *GameObject::get_parent(UnityEngine_GameObject_o *__this) {
    return Component::get_gameObject(Component::From(Transform::get_parent(get_transform(__this))));
}


UnityEngine_Transform_o *Transform::GetChild(UnityEngine_Transform_o *__this, int32_t index) {
    return UnityEngine_Transform__GetChild(__this, index);
}

void Transform::SetParent(UnityEngine_Transform_o *__this, UnityEngine_Transform_o *parent, bool worldPositionStays) {
    UnityEngine_Transform__SetParent(__this, parent);
}

void Transform::LookAt(UnityEngine_Transform_o *__this, UnityEngine_Transform_o *target) {
    UnityEngine_Transform__LookAt(__this, target);
}

void Transform::Rotate(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o eulers) {
    UnityEngine_Transform__Rotate(__this, eulers);
}

int32_t Transform::get_childCount(UnityEngine_Transform_o *__this) {
    return UnityEngine_Transform__get_childCount(__this);
}

UnityEngine_Vector3_o Transform::get_position(UnityEngine_Transform_o *__this) {
    return UnityEngine_Transform__get_position(__this);
}

void Transform::set_position(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value) {
    UnityEngine_Transform__set_position(__this, value);
}

UnityEngine_Vector3_o Transform::get_localPosition(UnityEngine_Transform_o *__this) {
    return UnityEngine_Transform__get_localPosition(__this);
}

void Transform::set_localPosition(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value) {
    UnityEngine_Transform__set_localPosition(__this, value);
}

UnityEngine_Transform_o *Transform::get_parent(UnityEngine_Transform_o *__this) {
    return UnityEngine_Transform__get_parent(__this);
}


UnityEngine_GameObject_o *Component::get_gameObject(UnityEngine_Component_o *__this) {
    return UnityEngine_Component__get_gameObject(__this);
}


UnityEngine_Vector3_o Camera::WorldToScreenPoint(UnityEngine_Camera_o *__this, UnityEngine_Vector3_o position) {
    return UnityEngine_Camera__WorldToScreenPoint(__this, position);
}

UnityEngine_Camera_o *Camera::get_main() {
    return UnityEngine_Camera__get_main();
}


void TextMesh::set_text(UnityEngine_TextMesh_o *__this, std::string_view value) {
    UnityEngine_TextMesh__set_text(__this, GameTypes::createCsString(value));
}

void TextMesh::set_characterSize(UnityEngine_TextMesh_o *__this, float value) {
    UnityEngine_TextMesh__set_characterSize(__this, value);
}

void TextMesh::set_fontSize(UnityEngine_TextMesh_o *__this, int32_t value) {
    UnityEngine_TextMesh__set_fontSize(__this, value);
}

void TextMesh::set_alignment(UnityEngine_TextMesh_o *__this, TextAlignment value) {
    UnityEngine_TextMesh__set_alignment(__this, static_cast<int32_t>(value));
}

void TextMesh::set_anchor(UnityEngine_TextMesh_o *__this, TextAnchor value) {
    UnityEngine_TextMesh__set_anchor(__this, static_cast<int32_t>(value));
}


void SceneManagement::SceneManager::LoadScene(std::string_view sceneName) {
    UnityEngine_SceneManagement_SceneManager__LoadScene(GameTypes::createCsString(sceneName));
}


float Vector3::Distance(UnityEngine_Vector3_o a, UnityEngine_Vector3_o b) {
    return UnityEngine_Vector3__Distance(a, b);
}

float Vector2::Distance(UnityEngine_Vector2_o a, UnityEngine_Vector2_o b) {
    return UnityEngine_Vector2__Distance(a, b);
}
}

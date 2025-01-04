#include "unityengine.hpp"
#include "gamedata.hpp"



namespace UnityEngine {
void GameObject::Destroy(UnityEngine_GameObject_o *__this) {
    GameData::getMethod("void UnityEngine_Object__Destroy (UnityEngine_Object_o* obj, const MethodInfo* method);")
        .getFunction<void (UnityEngine_GameObject_o *, const MethodInfo *)>()
        (__this, nullptr);
}

void GameObject::RecursiveDestroy(UnityEngine_GameObject_o *__this) {
    auto transform = get_transform(__this);
    while (Transform::get_childCount(transform) > 0)
        RecursiveDestroy(Transform::get_gameObject(Transform::GetChild(transform)));
    Destroy(__this);
}

UnityEngine_Transform_o *GameObject::get_transform(UnityEngine_GameObject_o *__this) {
    return GameData::getMethod("UnityEngine.GameObject$$get_transform")
        .getFunction<UnityEngine_Transform_o *(UnityEngine_GameObject_o *, const MethodInfo *)>()
        (__this, nullptr);
}

UnityEngine_GameObject_o *GameObject::get_parent(UnityEngine_GameObject_o *__this) {
    return Transform::get_gameObject(Transform::get_parent(get_transform(__this)));
}


UnityEngine_Transform_o *Transform::GetChild(UnityEngine_Transform_o *__this, int32_t index) {
    return GameData::getMethod("UnityEngine.Transform$$GetChild")\
        .getFunction<UnityEngine_Transform_o *(UnityEngine_Transform_o *, int32_t, const MethodInfo *)>()
        (__this, index, nullptr);
}

int32_t Transform::get_childCount(UnityEngine_Transform_o *__this) {
    return GameData::getMethod("UnityEngine.Transform$$get_childCount")\
        .getFunction<int32_t (UnityEngine_Transform_o *, const MethodInfo *)>()
        (__this, nullptr);
}

void Transform::set_position(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value) {
    GameData::getMethod("UnityEngine.Transform$$set_position")\
        .getFunction<void (UnityEngine_Transform_o *, UnityEngine_Vector3_o, const MethodInfo *)>()
        (__this, value, nullptr);
}

UnityEngine_Transform_o *Transform::get_parent(UnityEngine_Transform_o *__this) {
    return GameData::getMethod("UnityEngine.Transform$$get_parent")\
        .getFunction<UnityEngine_Transform_o *(UnityEngine_Transform_o *, const MethodInfo *)>()
        (__this, nullptr);
}

UnityEngine_GameObject_o *Transform::get_gameObject(UnityEngine_Transform_o *__this) {
    return GameData::getMethod("UnityEngine.Component$$get_gameObject")\
        .getFunction<UnityEngine_GameObject_o *(UnityEngine_Transform_o *, const MethodInfo *)>()
        (__this, nullptr);
}
}

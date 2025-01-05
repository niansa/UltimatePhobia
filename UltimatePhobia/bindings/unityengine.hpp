#pragma once

#include <string_view>


struct UnityEngine_GameObject_o;
struct UnityEngine_Transform_o;
struct UnityEngine_Component_o;


namespace UnityEngine {
namespace GameObject {
UnityEngine_GameObject_o *Find(std::string_view name);
void Destroy(UnityEngine_GameObject_o *__this);
void RecursiveDestroy(UnityEngine_GameObject_o *__this);
void SetActive(UnityEngine_GameObject_o *__this, bool value);
UnityEngine_Transform_o *get_transform(UnityEngine_GameObject_o *__this);
UnityEngine_GameObject_o *get_parent(UnityEngine_GameObject_o *__this);
}

namespace Transform {
UnityEngine_Transform_o *GetChild(UnityEngine_Transform_o *__this, int32_t index = 0);
int32_t get_childCount(UnityEngine_Transform_o *__this);
void set_position(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value);
UnityEngine_Transform_o *get_parent(UnityEngine_Transform_o *__this);
}

namespace Component {
template<typename T>
UnityEngine_Component_o *From(T *component) {
    return reinterpret_cast<UnityEngine_Component_o *>(component);
}
UnityEngine_GameObject_o *get_gameObject(UnityEngine_Component_o *__this);
}

namespace SceneManagement::SceneManager {
void LoadScene(std::string_view sceneName);
}
}

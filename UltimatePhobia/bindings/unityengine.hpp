#pragma once

#include <string_view>


struct UnityEngine_GameObject_o;
struct UnityEngine_Transform_o;
struct UnityEngine_Component_o;
struct UnityEngine_Camera_o;


namespace UnityEngine {
enum class TextAlignment {
    Left = 0,
    Center = 1,
    Right = 2
};

enum class TextAnchor {
    UpperLeft = 0,
    UpperCenter = 1,
    UpperRight = 2,
    MiddleLeft = 3,
    MiddleCenter = 4,
    MiddleRight = 5,
    LowerLeft = 6,
    LowerCenter = 7,
    LowerRight = 8
};

namespace GameObject {
UnityEngine_GameObject_o *New(std::string_view name);
UnityEngine_GameObject_o *Find(std::string_view name);
void Destroy(UnityEngine_GameObject_o *__this);
void RecursiveDestroy(UnityEngine_GameObject_o *__this);
void SetActive(UnityEngine_GameObject_o *__this, bool value);
UnityEngine_Component_o *AddComponent(UnityEngine_GameObject_o *__this, std::string_view name, std::string_view assemblyName);
UnityEngine_Transform_o *get_transform(UnityEngine_GameObject_o *__this);
UnityEngine_GameObject_o *get_parent(UnityEngine_GameObject_o *__this);
}

namespace Transform {
UnityEngine_Transform_o *GetChild(UnityEngine_Transform_o *__this, int32_t index = 0);
void SetParent(UnityEngine_Transform_o *__this, UnityEngine_Transform_o* parent, bool worldPositionStays = true);
void LookAt(UnityEngine_Transform_o *__this, UnityEngine_Transform_o* target);
void Rotate(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o eulers);
int32_t get_childCount(UnityEngine_Transform_o *__this);
UnityEngine_Vector3_o get_position(UnityEngine_Transform_o *__this);
void set_position(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value);
void set_localPosition(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value);
UnityEngine_Transform_o *get_parent(UnityEngine_Transform_o *__this);
}

namespace Component {
template<typename T>
UnityEngine_Component_o *From(T *component) {
    return reinterpret_cast<UnityEngine_Component_o *>(component);
}
UnityEngine_GameObject_o *get_gameObject(UnityEngine_Component_o *__this);
}

namespace Camera {
UnityEngine_Vector3_o WorldToScreenPoint(UnityEngine_Camera_o *__this, UnityEngine_Vector3_o position);
UnityEngine_Camera_o *get_main();
}

namespace TextMesh {
void set_text(UnityEngine_TextMesh_o *__this, std::string_view value);
void set_characterSize(UnityEngine_TextMesh_o *__this, float value);
void set_alignment(UnityEngine_TextMesh_o *__this, TextAlignment value);
void set_anchor(UnityEngine_TextMesh_o *__this, TextAnchor value);
}

namespace SceneManagement::SceneManager {
void LoadScene(std::string_view sceneName);
}
}

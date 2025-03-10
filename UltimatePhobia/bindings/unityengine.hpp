#pragma once

#include <string_view>

struct UnityEngine_GameObject_o;
struct UnityEngine_Transform_o;
struct UnityEngine_Component_o;
struct UnityEngine_Camera_o;
struct UnityEngine_SceneManagement_Scene_o;
struct UnityEngine_Vector3_o;

namespace Il2Cpp::UnityEngine {
namespace TextAlignment {
constexpr uint32_t Left = 0, Center = 1, Right = 2;
};

namespace TextAnchor {
constexpr uint32_t UpperLeft = 0, UpperCenter = 1, UpperRight = 2, MiddleLeft = 3, MiddleCenter = 4, MiddleRight = 5, LowerLeft = 6, LowerCenter = 7,
                   LowerRight = 8;
};

namespace GameObject {
UnityEngine_GameObject_o *New(std::string_view name);
void RecursiveDestroy(UnityEngine_GameObject_o *__this);
UnityEngine_Component_o *AddComponent(UnityEngine_GameObject_o *__this, std::string_view name, std::string_view assemblyName);
UnityEngine_GameObject_o *get_parent(UnityEngine_GameObject_o *__this);
} // namespace GameObject

namespace Vector3 {
constexpr UnityEngine_Vector3_o Zero = {{0.0f, 0.0f, 0.0f}};
inline static UnityEngine_Vector3_o From(UnityEngine_Vector2_o vec) { return {vec.fields.x, 0.0f, vec.fields.y}; }
} // namespace Vector3

namespace Vector2 {
constexpr UnityEngine_Vector2_o Zero = {{0.0f, 0.0f}};
inline static UnityEngine_Vector2_o From(UnityEngine_Vector3_o vec) { return {vec.fields.x, vec.fields.z}; }
} // namespace Vector2
} // namespace Il2Cpp::UnityEngine

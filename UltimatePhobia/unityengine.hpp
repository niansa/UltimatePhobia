#pragma once


namespace UnityEngine {
namespace GameObject {
void Destroy(UnityEngine_GameObject_o *__this);
void RecursiveDestroy(UnityEngine_GameObject_o *__this);
UnityEngine_Transform_o *get_transform(UnityEngine_GameObject_o *__this);
UnityEngine_GameObject_o *get_parent(UnityEngine_GameObject_o *__this);
}

namespace Transform {
UnityEngine_Transform_o *GetChild(UnityEngine_Transform_o *__this, int32_t index = 0);
int32_t get_childCount(UnityEngine_Transform_o *__this);
void set_position(UnityEngine_Transform_o *__this, UnityEngine_Vector3_o value);
UnityEngine_Transform_o *get_parent(UnityEngine_Transform_o *__this);
UnityEngine_GameObject_o *get_gameObject(UnityEngine_Transform_o *__this);
}
}

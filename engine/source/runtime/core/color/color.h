#pragma once

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/math/vector3.h"

namespace Sammi
{
    // 声明Color类型支持反射
    REFLECTION_TYPE(Color)

    // 使用反射系统注册Color类，Fields表示将注册类的成员变量
    CLASS(Color, Fields)
    {
        REFLECTION_BODY(Color);  // 反射体声明（应在类内部）

    public:
        float r;  // 红色分量 [0.0, 1.0]
        float g;  // 绿色分量 [0.0, 1.0]
        float b;  // 蓝色分量 [0.0, 1.0]

        // 转换为三维向量
        Vector3 toVector3() const { return Vector3(r, g, b); }
    };
}
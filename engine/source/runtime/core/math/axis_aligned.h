#pragma once

#include "runtime/core/math/vector3.h"                // 三维向量实现
#include "runtime/core/meta/reflection/reflection.h"  // 反射系统支持
#include <limits>                                     // 数值极限

namespace Sammi
{
    // 声明AxisAlignedBox类型参与反射
    REFLECTION_TYPE(AxisAlignedBox)

    // 注册为反射类（Fields选项表示自动注册字段）
    CLASS(AxisAlignedBox, Fields)
    {
        REFLECTION_BODY(AxisAlignedBox)  // 反射体声明
    public:
        // 默认构造函数
        // 初始化：中心点为原点，半尺寸为0
        //       最小角为最大浮点数，最大角为最小浮点数（表示空包围盒）
        AxisAlignedBox() {}

        // 参数化构造函数
        // 参数：
        //   center      - 包围盒中心点
        //   half_extent - 从中心到各面的距离（XYZ方向半边长）
        AxisAlignedBox(const Vector3& center, const Vector3& half_extent);

        // 扩展包围盒以包含新点
        // 参数：
        //   new_point - 要包含的新3D点
        void merge(const Vector3& new_point);

        // 直接更新包围盒参数
        // 参数：
        //   center      - 新的中心点
        //   half_extent - 新的半边长
        void update(const Vector3& center, const Vector3& half_extent);

        // 访问器方法（均为const引用返回，确保封装）
        const Vector3& getCenter() const { return m_center; }           // 获取中心点
        const Vector3& getHalfExtent() const { return m_half_extent; }  // 获取半边长
        const Vector3& getMinCorner() const { return m_min_corner; }    // 获取最小角点
        const Vector3& getMaxCorner() const { return m_max_corner; }    // 获取最大角点

    private:
        Vector3 m_center {Vector3::ZERO};       // 中心点（默认(0,0,0)）
        Vector3 m_half_extent {Vector3::ZERO};  // XYZ方向半边长（默认0）

        // 最小角点初始化：使用最大浮点数（表示空包围盒）
        Vector3 m_min_corner
        {
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        };

        // 最大角点初始化：使用最小浮点数（表示空包围盒）
        Vector3 m_max_corner
        {
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max()
        };
    };
}
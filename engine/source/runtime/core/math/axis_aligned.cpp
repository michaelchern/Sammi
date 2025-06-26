
#include "runtime/core/math/axis_aligned.h"  // 轴对齐包围盒的头文件

namespace Sammi
{
    // 参数化构造函数
    // 参数：
    //   center      - 包围盒中心的3D坐标
    //   half_extent - 从中心到各面的最大距离（半边长）
    AxisAlignedBox::AxisAlignedBox(const Vector3& center, const Vector3& half_extent)
    {
        // 调用update方法初始化包围盒
        update(center, half_extent);
    }

    // 扩展包围盒以包含新点
    // 参数：
    //   new_point - 要包含的新3D点
    void AxisAlignedBox::merge(const Vector3& new_point)
    {
        // 更新最小角坐标：确保是所有点的最小值
        // 对每个坐标分量取最小值：min(new_point, m_min_corner)
        m_min_corner.makeFloor(new_point);

        // 更新最大角坐标：确保是所有点的最大值
        // 对每个坐标分量取最大值：max(new_point, m_max_corner)
        m_max_corner.makeCeil(new_point);

        // 重新计算中心点：(min + max)/2
        m_center      = 0.5f * (m_min_corner + m_max_corner);

        // 重新计算半边长：从中心到最小角的距离（即(max-min)/2）
        m_half_extent = m_center - m_min_corner;
    }

    // 直接更新包围盒参数
    // 参数：
    //   center      - 新的中心点
    //   half_extent - 新的半边长
    void AxisAlignedBox::update(const Vector3& center, const Vector3& half_extent)
    {
        m_center      = center;                // 设置中心点
        m_half_extent = half_extent;           // 设置半边长

        // 计算最小角点：center - half_extent
        m_min_corner  = center - half_extent;  

        // 计算最大角点：center + half_extent
        m_max_corner  = center + half_extent;  
    }
}
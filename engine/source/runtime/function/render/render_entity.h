#pragma once

#include "runtime/core/math/axis_aligned.h"
#include "runtime/core/math/matrix4.h"

// 包含标准库容器和类型
#include <cstdint>  // 精确宽度整数类型（如uint32_t）
#include <vector>   // 动态数组容器

namespace Sammi
{
    /**
     * @brief 渲染场景中的基础实体类，描述一个可渲染对象的核心属性和状态
     *
     * 该类封装了渲染实体所需的关键信息，包括模型变换、关联的网格/材质资源、
     * 渲染特性（如混合、双面渲染）以及PBR（基于物理的渲染）相关参数。
     */
    class RenderEntity
    {
    public:
        uint32_t  m_instance_id {0};                     // 实体唯一实例ID（用于标识和管理不同渲染对象）
        Matrix4x4 m_model_matrix {Matrix4x4::IDENTITY};  // 模型矩阵（局部空间 -> 世界空间的变换矩阵）

        // 网格相关属性
        size_t                 m_mesh_asset_id {0};               // 关联的网格资源ID（指向加载的网格数据资产）
        bool                   m_enable_vertex_blending {false};  // 是否启用顶点混合（骨骼动画开关）
        std::vector<Matrix4x4> m_joint_matrices;                  // 关节变换矩阵数组（骨骼动画中各关节的局部变换）
        AxisAlignedBox         m_bounding_box;                    // 轴对齐包围盒（世界空间中的碰撞/裁剪边界）

        // 材质相关属性
        size_t  m_material_asset_id {0};  // 关联的材质资源ID（指向加载的材质参数资产）
        bool    m_blend {false};          // 是否启用颜色混合（透明度渲染开关）
        bool    m_double_sided {false};   // 是否双面渲染（正反面均可见开关）

        // PBR（基于物理的渲染）材质参数
        Vector4 m_base_color_factor {1.0f, 1.0f, 1.0f, 1.0f};  // 基础颜色因子（漫反射/反照率颜色，RGBA）
        float   m_metallic_factor {1.0f};                      // 金属度因子（0=绝缘体，1=金属）
        float   m_roughness_factor {1.0f};                     // 粗糙度因子（0=光滑，1=粗糙）
        float   m_normal_scale {1.0f};                         // 法线贴图缩放因子（调整法线细节强度）
        float   m_occlusion_strength {1.0f};                   // 环境光遮蔽强度（调整AO对光照的影响）
        Vector3 m_emissive_factor {0.0f, 0.0f, 0.0f};          // 自发光因子（RGB颜色，强度由亮度决定）
    };
}

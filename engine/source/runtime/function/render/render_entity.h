#pragma once

#include "runtime/core/math/axis_aligned.h"  // 轴对齐边界框(AABB)相关数学
#include "runtime/core/math/matrix4.h"       // 4x4矩阵数学支持

#include <cstdint>  // 标准整数类型
#include <vector>   // 向量容器

namespace Sammi
{
    // 渲染实体类
    // 表示场景中的一个可渲染物体，包含网格和材质信息
    class RenderEntity
    {
    public:
        uint32_t  m_instance_id {0};                     // 实体实例的唯一标识符
        Matrix4x4 m_model_matrix {Matrix4x4::IDENTITY};  // 模型变换矩阵（默认为单位矩阵）

        // ================== 网格相关属性 ==================
        size_t                 m_mesh_asset_id {0};               // 网格资源的唯一标识符
        bool                   m_enable_vertex_blending {false};  // 是否启用顶点混合（用于蒙皮动画）
        std::vector<Matrix4x4> m_joint_matrices;                  // 关节变换矩阵数组（用于顶点混合）
        AxisAlignedBox         m_bounding_box;                    // 物体的包围盒（用于视锥剔除）

        // ================== 材质相关属性 ==================
        size_t  m_material_asset_id {0};                       // 材质资源的唯一标识符
        bool    m_blend {false};                               // 是否启用透明混合（alpha blending）
        bool    m_double_sided {false};                        // 是否双面渲染（禁用背面剔除）
        Vector4 m_base_color_factor {1.0f, 1.0f, 1.0f, 1.0f};  // 基础颜色调节因子（RGBA）
        float   m_metallic_factor {1.0f};                      // 金属度调节因子
        float   m_roughness_factor {1.0f};                     // 粗糙度调节因子
        float   m_normal_scale {1.0f};                         // 法线贴图强度
        float   m_occlusion_strength {1.0f};                   // 环境光遮蔽强度
        Vector3 m_emissive_factor {0.0f, 0.0f, 0.0f};          // 自发光调节因子（RGB）
    };
}

#pragma once

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"
#include "runtime/function/render/render_type.h"
#include "interface/rhi.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Sammi
{
    // ---------------------- 全局常量定义 ----------------------
    // 点光源阴影贴图分辨率（2048x2048）
    static const uint32_t s_point_light_shadow_map_dimension = 2048;
    // 平行光阴影贴图分辨率（4096x4096，更高分辨率用于更清晰的阴影）
    static const uint32_t s_directional_light_shadow_map_dimension = 4096;

    // 每帧绘制调用最大实例数（限制实例化渲染的批量大小，平衡性能与灵活性）
    static uint32_t const s_mesh_per_drawcall_max_instance_count = 64;
    // 顶点混合最大关节数（限制骨骼动画的复杂度，避免矩阵计算过载）
    static uint32_t const s_mesh_vertex_blending_max_joint_count = 1024;
    // 场景中最大点光源数量（受限于GPU缓冲区大小和着色器处理能力）
    static uint32_t const s_max_point_light_count = 15;
    // 注意：需与"shader_include/constants.h"中的宏同步（确保CPU/GPU常量一致）

    // ---------------------- 场景光照结构体 ----------------------
    // Vulkan场景用平行光（方向光）数据结构（适配GPU缓冲区布局）
    struct VulkanSceneDirectionalLight
    {
        Vector3 direction;              // 光照方向（单位向量，通常指向光源相反方向，如太阳光方向）
        float   _padding_direction; // 填充字段（确保后续成员内存对齐）
        Vector3 color;// 光的颜色（RGB分量，通常已归一化或包含亮度信息）
        float   _padding_color;// 填充字段（内存对齐）
    };

    // Vulkan场景用点光源数据结构（适配GPU缓冲区布局）
    struct VulkanScenePointLight
    {
        Vector3 position;// 光源在世界空间中的位置（三维坐标）
        float   radius;// 光照有效半径（超出此半径的光被剔除）
        Vector3 intensity;// 辐射强度（单位：W/sr，各向同性光源各方向强度相同）
        float   _padding_intensity;// 填充字段（内存对齐）
    };

    // ---------------------- 每帧全局数据（SSBO） ----------------------
    // 每帧更新的网格渲染全局存储缓冲区对象（供着色器访问）
    struct MeshPerframeStorageBufferObject
    {
        Matrix4x4                   proj_view_matrix;
        Vector3                     camera_position;
        float                       _padding_camera_position;
        Vector3                     ambient_light;
        float                       _padding_ambient_light;
        uint32_t                    point_light_num;
        uint32_t                    _padding_point_light_num_1;
        uint32_t                    _padding_point_light_num_2;
        uint32_t                    _padding_point_light_num_3;
        VulkanScenePointLight       scene_point_lights[s_max_point_light_count];
        VulkanSceneDirectionalLight scene_directional_light;
        Matrix4x4                   directional_light_proj_view;
    };

    struct VulkanMeshInstance
    {
        float     enable_vertex_blending;
        float     _padding_enable_vertex_blending_1;
        float     _padding_enable_vertex_blending_2;
        float     _padding_enable_vertex_blending_3;
        Matrix4x4 model_matrix;
    };

    struct MeshPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPerMaterialUniformBufferObject
    {
        Vector4 baseColorFactor {0.0f, 0.0f, 0.0f, 0.0f};

        float metallicFactor    = 0.0f;
        float roughnessFactor   = 0.0f;
        float normalScale       = 0.0f;
        float occlusionStrength = 0.0f;

        Vector3  emissiveFactor  = {0.0f, 0.0f, 0.0f};
        uint32_t is_blend        = 0;
        uint32_t is_double_sided = 0;
    };

    struct MeshPointLightShadowPerframeStorageBufferObject
    {
        uint32_t point_light_num;
        uint32_t _padding_point_light_num_1;
        uint32_t _padding_point_light_num_2;
        uint32_t _padding_point_light_num_3;
        Vector4  point_lights_position_and_radius[s_max_point_light_count];
    };

    struct MeshPointLightShadowPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshPointLightShadowPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshDirectionalLightShadowPerframeStorageBufferObject
    {
        Matrix4x4 light_proj_view;
    };

    struct MeshDirectionalLightShadowPerdrawcallStorageBufferObject
    {
        VulkanMeshInstance mesh_instances[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    struct AxisStorageBufferObject
    {
        Matrix4x4 model_matrix  = Matrix4x4::IDENTITY;
        uint32_t  selected_axis = 3;
    };

    struct ParticleBillboardPerframeStorageBufferObject
    {
        Matrix4x4 proj_view_matrix;
        Vector3   right_direction;
        float     _padding_right_position;
        Vector3   up_direction;
        float     _padding_up_direction;
        Vector3   foward_direction;
        float     _padding_forward_position;
    };

    struct ParticleCollisionPerframeStorageBufferObject
    {
        Matrix4x4 view_matrix;
        Matrix4x4 proj_view_matrix;
        Matrix4x4 proj_inv_matrix;
    };

    // TODO: 4096 may not be the best
    static constexpr int s_particle_billboard_buffer_size = 4096;
    struct ParticleBillboardPerdrawcallStorageBufferObject
    {
        Vector4 positions[s_particle_billboard_buffer_size];
        Vector4 sizes[s_particle_billboard_buffer_size];
        Vector4 colors[s_particle_billboard_buffer_size];
    };

    struct MeshInefficientPickPerframeStorageBufferObject
    {
        Matrix4x4 proj_view_matrix;
        uint32_t  rt_width;
        uint32_t  rt_height;
    };

    struct MeshInefficientPickPerdrawcallStorageBufferObject
    {
        Matrix4x4 model_matrices[s_mesh_per_drawcall_max_instance_count];
        uint32_t  node_ids[s_mesh_per_drawcall_max_instance_count];
        float     enable_vertex_blendings[s_mesh_per_drawcall_max_instance_count];
    };

    struct MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject
    {
        Matrix4x4 joint_matrices[s_mesh_vertex_blending_max_joint_count * s_mesh_per_drawcall_max_instance_count];
    };

    // mesh
    struct VulkanMesh
    {
        bool enable_vertex_blending;

        uint32_t mesh_vertex_count;

        RHIBuffer*    mesh_vertex_position_buffer;
        VmaAllocation mesh_vertex_position_buffer_allocation;

        RHIBuffer*    mesh_vertex_varying_enable_blending_buffer;
        VmaAllocation mesh_vertex_varying_enable_blending_buffer_allocation;

        RHIBuffer*    mesh_vertex_joint_binding_buffer;
        VmaAllocation mesh_vertex_joint_binding_buffer_allocation;

        RHIDescriptorSet* mesh_vertex_blending_descriptor_set;

        RHIBuffer*    mesh_vertex_varying_buffer;
        VmaAllocation mesh_vertex_varying_buffer_allocation;

        uint32_t mesh_index_count;

        RHIBuffer*    mesh_index_buffer;
        VmaAllocation mesh_index_buffer_allocation;
    };

    // material
    struct VulkanPBRMaterial
    {
        RHIImage*       base_color_texture_image;
        RHIImageView*   base_color_image_view;
        VmaAllocation   base_color_image_allocation;

        RHIImage*       metallic_roughness_texture_image;
        RHIImageView*   metallic_roughness_image_view;
        VmaAllocation   metallic_roughness_image_allocation;

        RHIImage*       normal_texture_image;
        RHIImageView*   normal_image_view;
        VmaAllocation   normal_image_allocation;

        RHIImage*       occlusion_texture_image;
        RHIImageView*   occlusion_image_view;
        VmaAllocation   occlusion_image_allocation;

        RHIImage*       emissive_texture_image;
        RHIImageView*   emissive_image_view;
        VmaAllocation   emissive_image_allocation;

        RHIBuffer*      material_uniform_buffer;
        VmaAllocation   material_uniform_buffer_allocation;

        RHIDescriptorSet* material_descriptor_set;
    };

    // nodes
    struct RenderMeshNode
    {
        const Matrix4x4*   model_matrix {nullptr};
        const Matrix4x4*   joint_matrices {nullptr};
        uint32_t           joint_count {0};
        VulkanMesh*        ref_mesh {nullptr};
        VulkanPBRMaterial* ref_material {nullptr};
        uint32_t           node_id;
        bool               enable_vertex_blending {false};
    };

    struct RenderAxisNode
    {
        Matrix4x4   model_matrix {Matrix4x4::IDENTITY};
        VulkanMesh* ref_mesh {nullptr};
        uint32_t    node_id;
        bool        enable_vertex_blending {false};
    };

    struct TextureDataToUpdate
    {
        void*              base_color_image_pixels;
        uint32_t           base_color_image_width;
        uint32_t           base_color_image_height;
        RHIFormat base_color_image_format;
        void*              metallic_roughness_image_pixels;
        uint32_t           metallic_roughness_image_width;
        uint32_t           metallic_roughness_image_height;
        RHIFormat metallic_roughness_image_format;
        void*              normal_roughness_image_pixels;
        uint32_t           normal_roughness_image_width;
        uint32_t           normal_roughness_image_height;
        RHIFormat normal_roughness_image_format;
        void*              occlusion_image_pixels;
        uint32_t           occlusion_image_width;
        uint32_t           occlusion_image_height;
        RHIFormat occlusion_image_format;
        void*              emissive_image_pixels;
        uint32_t           emissive_image_width;
        uint32_t           emissive_image_height;
        RHIFormat emissive_image_format;
        VulkanPBRMaterial* now_material;
    };
}

#pragma once

#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_type.h"
#include "runtime/function/render/interface/rhi.h"

#include "runtime/function/render/render_common.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <map>
#include <vector>
#include <cmath>

namespace Sammi
{
    class RHI;
    class RenderPassBase;
    class RenderCamera;

    // -------------------------- IBL（基于图像的光照）资源结构体 --------------------------
    /// 存储IBL（基于图像的光照）所需的GPU资源
    struct IBLResource
    {
        RHIImage*     _brdfLUT_texture_image;                // BRDF LUT（双向反射分布函数查找表）图像对象
        RHIImageView* _brdfLUT_texture_image_view;           // BRDF LUT图像视图（用于访问图像数据）
        RHISampler*   _brdfLUT_texture_sampler;              // BRDF LUT采样器（控制纹理采样方式）
        VmaAllocation _brdfLUT_texture_image_allocation;     // BRDF LUT图像的Vulkan内存分配句柄

        RHIImage*     _irradiance_texture_image;             // 辐照度贴图（环境光漫反射部分）图像对象
        RHIImageView* _irradiance_texture_image_view;        // 辐照度贴图图像视图
        RHISampler*   _irradiance_texture_sampler;           // 辐照度贴图采样器
        VmaAllocation _irradiance_texture_image_allocation;  // 辐照度贴图内存分配

        RHIImage*     _specular_texture_image;               // 镜面反射贴图（环境光镜面反射部分）图像对象
        RHIImageView* _specular_texture_image_view;          // 镜面反射贴图图像视图
        RHISampler*   _specular_texture_sampler;             // 镜面反射贴图采样器
        VmaAllocation _specular_texture_image_allocation;    // 镜面反射贴图内存分配
    };

    /// 存储IBL资源的原始像素数据（用于从CPU上传到GPU）
    struct IBLResourceData
    {
        void*                _brdfLUT_texture_image_pixels;     // BRDF LUT图像像素数据指针
        uint32_t             _brdfLUT_texture_image_width;      // BRDF LUT图像宽度
        uint32_t             _brdfLUT_texture_image_height;     // BRDF LUT图像高度
        RHIFormat            _brdfLUT_texture_image_format;     // BRDF LUT图像格式（如RGBA8）

        std::array<void*, 6> _irradiance_texture_image_pixels;  // 辐照度贴图的6个面（立方体贴图）像素数据
        uint32_t             _irradiance_texture_image_width;   // 辐照度贴图宽度
        uint32_t             _irradiance_texture_image_height;  // 辐照度贴图高度
        RHIFormat            _irradiance_texture_image_format;  // 辐照度贴图格式

        std::array<void*, 6> _specular_texture_image_pixels;    // 镜面反射贴图的6个面像素数据
        uint32_t             _specular_texture_image_width;     // 镜面反射贴图宽度
        uint32_t             _specular_texture_image_height;    // 镜面反射贴图高度
        RHIFormat            _specular_texture_image_format;    // 镜面反射贴图格式
    };

    // -------------------------- 颜色分级资源结构体 --------------------------
    /// 存储颜色分级（Color Grading）所需的GPU资源
    struct ColorGradingResource
    {
        RHIImage*     _color_grading_LUT_texture_image;             // 颜色分级LUT图像对象（用于调整色调/饱和度等）
        RHIImageView* _color_grading_LUT_texture_image_view;        // 颜色分级LUT图像视图
        VmaAllocation _color_grading_LUT_texture_image_allocation;  // 内存分配句柄
    };

    /// 存储颜色分级资源的原始像素数据
    struct ColorGradingResourceData
    {
        void*     _color_grading_LUT_texture_image_pixels;  // 颜色分级LUT像素数据指针
        uint32_t  _color_grading_LUT_texture_image_width;   // 图像宽度
        uint32_t  _color_grading_LUT_texture_image_height;  // 图像高度
        RHIFormat _color_grading_LUT_texture_image_format;  // 图像格式
    };

    // -------------------------- 存储缓冲区结构体 --------------------------
    /// 存储各类缓冲区资源及其硬件限制
    struct StorageBuffer
    {
        // 硬件限制（来自物理设备的属性）
        uint32_t _min_uniform_buffer_offset_alignment{ 256 };  // 最小统一缓冲区偏移对齐
        uint32_t _min_storage_buffer_offset_alignment{ 256 };  // 最小存储缓冲区偏移对齐
        uint32_t _max_storage_buffer_range{ 1 << 27 };         // 最大存储缓冲区范围（128MB）
        uint32_t _non_coherent_atom_size{ 256 };               // 非一致内存原子操作大小

        // 全局上传环缓冲区（用于高效上传临时数据到GPU）
        RHIBuffer* _global_upload_ringbuffer;                  // 环形缓冲区对象
        RHIDeviceMemory* _global_upload_ringbuffer_memory;     // 缓冲区关联的设备内存
        void* _global_upload_ringbuffer_memory_pointer;        // 内存指针（用于CPU直接写入）

        // 环形缓冲区分段管理（跟踪各帧数据的起始/结束位置和大小）
        std::vector<uint32_t> _global_upload_ringbuffers_begin;
        std::vector<uint32_t> _global_upload_ringbuffers_end;
        std::vector<uint32_t> _global_upload_ringbuffers_size;

        // 空描述符存储缓冲区（用于需要非空绑定的着色器资源）
        RHIBuffer* _global_null_descriptor_storage_buffer;               // 空存储缓冲区对象
        RHIDeviceMemory* _global_null_descriptor_storage_buffer_memory;  // 关联内存

        // 轴存储缓冲区（存储世界坐标系轴向量等常量数据）
        RHIBuffer* _axis_inefficient_storage_buffer;               // 轴存储缓冲区对象
        RHIDeviceMemory* _axis_inefficient_storage_buffer_memory;  // 关联内存
        void* _axis_inefficient_storage_buffer_memory_pointer;     // 内存指针
    };

    // -------------------------- 全局渲染资源结构体 --------------------------
    /// 整合全局共享的渲染资源（IBL、颜色分级、存储缓冲区）
    struct GlobalRenderResource
    {
        IBLResource          _ibl_resource;            // IBL光照资源
        ColorGradingResource _color_grading_resource;  // 颜色分级资源
        StorageBuffer        _storage_buffer;          // 存储缓冲区管理
    };

    // -------------------------- 渲染资源管理类 --------------------------
    /// 负责管理渲染资源（上传、更新、缓存）
    class RenderResource : public RenderResourceBase
    {
    public:
        /// 清理所有渲染资源（释放GPU内存、销毁对象等）
        void clear() override final;

        /**
         * @brief 上传全局渲染资源到GPU
         * @param rhi 渲染硬件接口实例
         * @param level_resource_desc 关卡资源描述（包含IBL、颜色分级等数据）
         */
        virtual void uploadGlobalRenderResource(std::shared_ptr<RHI> rhi, LevelResourceDesc level_resource_desc) override final;

        /**
         * @brief 上传游戏对象的网格资源（重载版本）
         * @param rhi 渲染硬件接口实例
         * @param render_entity 游戏实体
         * @param mesh_data 网格数据（顶点、索引等）
         * @param material_data 材质数据（着色器参数、纹理等）
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data, RenderMaterialData material_data) override final;

        /**
         * @brief 上传游戏对象的网格资源（重载版本）
         * @param rhi 渲染硬件接口实例
         * @param render_entity 游戏实体
         * @param mesh_data 网格数据（顶点、索引等）
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data) override final;

        /**
         * @brief 上传游戏对象的材质资源（重载版本）
         * @param rhi 渲染硬件接口实例
         * @param render_entity 游戏实体
         * @param material_data 材质数据（着色器参数、纹理等）
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMaterialData material_data) override final;

        /**
         * @brief 更新每帧更新的缓冲区数据（如相机矩阵、光照信息）
         * @param render_scene 当前渲染场景
         * @param camera 当前相机
         */
        virtual void updatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) override final;

        /// 获取实体的Vulkan网格对象（缓存或加载）
        VulkanMesh& getEntityMesh(RenderEntity entity);

        /// 获取实体的Vulkan PBR材质对象（缓存或加载）
        VulkanPBRMaterial& getEntityMaterial(RenderEntity entity);

        /// 重置指定帧索引的环形缓冲区偏移量（用于下一帧数据上传）
        void resetRingBufferOffset(uint8_t current_frame_index);

        // 全局渲染资源（贯穿整个渲染流程的核心资源）
        GlobalRenderResource m_global_render_resource;

        // 各类每帧更新的存储缓冲区对象（按功能分类）
        MeshPerframeStorageBufferObject m_mesh_perframe_storage_buffer_object;                                                 // 网格每帧数据
        MeshPointLightShadowPerframeStorageBufferObject m_mesh_point_light_shadow_perframe_storage_buffer_object;              // 点光源阴影每帧数据
        MeshDirectionalLightShadowPerframeStorageBufferObject m_mesh_directional_light_shadow_perframe_storage_buffer_object;  // 方向光阴影每帧数据
        AxisStorageBufferObject m_axis_storage_buffer_object;                                                                  // 轴数据
        MeshInefficientPickPerframeStorageBufferObject m_mesh_inefficient_pick_perframe_storage_buffer_object;                 // 低效拾取每帧数据
        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;                       // 粒子公告牌每帧数据
        ParticleCollisionPerframeStorageBufferObject m_particle_collision_perframe_storage_buffer_object;                      // 粒子碰撞每帧数据

        // 缓存的网格和材质（避免重复创建）
        std::map<size_t, VulkanMesh>        m_vulkan_meshes;         // 键：实体哈希，值：Vulkan网格对象
        std::map<size_t, VulkanPBRMaterial> m_vulkan_pbr_materials;  // 键：实体哈希，值：Vulkan PBR材质对象

        // 描述符集布局指针（用于资源上传时的布局绑定）
        RHIDescriptorSetLayout* const* m_mesh_descriptor_set_layout {nullptr};      // 网格描述符集布局
        RHIDescriptorSetLayout* const* m_material_descriptor_set_layout {nullptr};  // 材质描述符集布局

    private:
        /**
         * @brief 创建并映射存储缓冲区到CPU内存（用于高效数据上传）
         * @param rhi 渲染硬件接口实例
         */
        void createAndMapStorageBuffer(std::shared_ptr<RHI> rhi);

        /**
         * @brief 创建IBL资源的采样器（控制纹理过滤/寻址模式）
         * @param rhi 渲染硬件接口实例
         */
        void createIBLSamplers(std::shared_ptr<RHI> rhi);

        /**
         * @brief 创建IBL纹理并上传数据到GPU
         * @param rhi 渲染硬件接口实例
         * @param irradiance_maps 辐照度贴图的6个面数据（立方体贴图）
         * @param specular_maps 镜面反射贴图的6个面数据
         */
        void createIBLTextures(std::shared_ptr<RHI> rhi, std::array<std::shared_ptr<TextureData>, 6> irradiance_maps, std::array<std::shared_ptr<TextureData>, 6> specular_maps);

        /**
         * @brief 获取或创建Vulkan网格对象（缓存机制）
         * @param rhi 渲染硬件接口实例
         * @param entity 游戏实体
         * @param mesh_data 网格数据
         * @return VulkanMesh& 已存在的或新创建的网格对象
         */
        VulkanMesh& getOrCreateVulkanMesh(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMeshData mesh_data);

        /**
         * @brief 获取或创建Vulkan PBR材质对象（缓存机制）
         * @param rhi 渲染硬件接口实例
         * @param entity 游戏实体
         * @param material_data 材质数据
         * @return VulkanPBRMaterial& 已存在的或新创建的材质对象
         */
        VulkanPBRMaterial& getOrCreateVulkanMaterial(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMaterialData material_data);

        /**
         * @brief 更新网格数据到GPU缓冲区（顶点、索引等）
         * @param rhi 渲染硬件接口实例
         * @param enable_vertex_blending 是否启用顶点混合（骨骼动画）
         * @param index_buffer_size 索引缓冲区大小（字节）
         * @param index_buffer_data 索引数据指针
         * @param vertex_buffer_size 顶点缓冲区大小（字节）
         * @param vertex_buffer_data 顶点数据指针
         * @param joint_binding_buffer_size 关节绑定缓冲区大小
         * @param joint_binding_buffer_data 关节绑定数据指针
         * @param now_mesh 目标网格对象
         */
        void updateMeshData(std::shared_ptr<RHI>                          rhi,
                            bool                                          enable_vertex_blending,
                            uint32_t                                      index_buffer_size,
                            void*                                         index_buffer_data,
                            uint32_t                                      vertex_buffer_size,
                            struct MeshVertexDataDefinition const*        vertex_buffer_data,
                            uint32_t                                      joint_binding_buffer_size,
                            struct MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                            VulkanMesh&                                   now_mesh);

        /**
         * @brief 更新顶点缓冲区数据
         * @param rhi 渲染硬件接口实例
         * @param enable_vertex_blending 是否启用顶点混合
         * @param vertex_buffer_size 顶点缓冲区大小
         * @param vertex_buffer_data 顶点数据指针
         * @param joint_binding_buffer_size 关节绑定缓冲区大小
         * @param joint_binding_buffer_data 关节绑定数据指针
         * @param index_buffer_size 索引缓冲区大小
         * @param index_buffer_data 索引数据指针（uint16_t类型）
         * @param now_mesh 目标网格对象
         */
        void updateVertexBuffer(std::shared_ptr<RHI>                          rhi,
                                bool                                          enable_vertex_blending,
                                uint32_t                                      vertex_buffer_size,
                                struct MeshVertexDataDefinition const*        vertex_buffer_data,
                                uint32_t                                      joint_binding_buffer_size,
                                struct MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                uint32_t                                      index_buffer_size,
                                uint16_t*                                     index_buffer_data,
                                VulkanMesh&                                   now_mesh);

        /**
         * @brief 更新索引缓冲区数据
         * @param rhi 渲染硬件接口实例
         * @param index_buffer_size 索引缓冲区大小（字节）
         * @param index_buffer_data 索引数据指针（uint16_t类型）
         * @param now_mesh 目标网格对象
         */
        void updateIndexBuffer(std::shared_ptr<RHI> rhi,
                               uint32_t             index_buffer_size,
                               void*                index_buffer_data,
                               VulkanMesh&          now_mesh);

        /**
         * @brief 更新纹理图像数据（从CPU上传到GPU）
         * @param rhi 渲染硬件接口实例
         * @param texture_data 包含纹理像素数据和元信息的结构体
         */
        void updateTextureImageData(std::shared_ptr<RHI> rhi, const TextureDataToUpdate& texture_data);
    };
}

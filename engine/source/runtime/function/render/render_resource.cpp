#include "runtime/function/render/render_resource.h"

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_helper.h"

#include "runtime/function/render/render_mesh.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/interface/vulkan/vulkan_util.h"

#include "runtime/function/render/passes/main_camera_pass.h"

#include "runtime/core/base/macro.h"

#include <stdexcept>

namespace Sammi
{
    void RenderResource::clear()
    {
    }

    void RenderResource::uploadGlobalRenderResource(std::shared_ptr<RHI> rhi, LevelResourceDesc level_resource_desc)
    {
        // 创建并映射全局存储缓冲区（用于存储渲染所需的统一资源数据，如材质参数、光照配置等）
        createAndMapStorageBuffer(rhi);

        // -------------------------- 天空盒辐照度（IBL辐照度）资源加载 --------------------------
        // 从关卡资源描述中提取天空盒辐照度资源的配置信息
        SkyBoxIrradianceMap skybox_irradiance_map = level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map;
        // 加载六个方向的HDR辐照度纹理（立方体贴图的六个面）
        // 正负X/Y/Z方向对应天空盒的六个面，用于模拟环境光的漫反射贡献
        std::shared_ptr<TextureData> irradiace_pos_x_map = loadTextureHDR(skybox_irradiance_map.m_positive_x_map);  // 右（+X）
        std::shared_ptr<TextureData> irradiace_neg_x_map = loadTextureHDR(skybox_irradiance_map.m_negative_x_map);  // 左（-X）
        std::shared_ptr<TextureData> irradiace_pos_y_map = loadTextureHDR(skybox_irradiance_map.m_positive_y_map);  // 上（+Y）
        std::shared_ptr<TextureData> irradiace_neg_y_map = loadTextureHDR(skybox_irradiance_map.m_negative_y_map);  // 下（-Y）
        std::shared_ptr<TextureData> irradiace_pos_z_map = loadTextureHDR(skybox_irradiance_map.m_positive_z_map);  // 前（+Z）
        std::shared_ptr<TextureData> irradiace_neg_z_map = loadTextureHDR(skybox_irradiance_map.m_negative_z_map);  // 后（-Z）

        // -------------------------- 天空盒高光（IBL高光）资源加载 --------------------------
        // 从关卡资源描述中提取天空盒高光资源的配置信息（用于镜面反射计算）
        SkyBoxSpecularMap skybox_specular_map = level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map;
        // 加载六个方向的HDR高光纹理（立方体贴图的六个面）
        // 高光纹理通常存储预计算的球谐系数或环境光遮蔽信息，用于快速计算镜面反射
        std::shared_ptr<TextureData> specular_pos_x_map  = loadTextureHDR(skybox_specular_map.m_positive_x_map);
        std::shared_ptr<TextureData> specular_neg_x_map  = loadTextureHDR(skybox_specular_map.m_negative_x_map);
        std::shared_ptr<TextureData> specular_pos_y_map  = loadTextureHDR(skybox_specular_map.m_positive_y_map);
        std::shared_ptr<TextureData> specular_neg_y_map  = loadTextureHDR(skybox_specular_map.m_negative_y_map);
        std::shared_ptr<TextureData> specular_pos_z_map  = loadTextureHDR(skybox_specular_map.m_positive_z_map);
        std::shared_ptr<TextureData> specular_neg_z_map  = loadTextureHDR(skybox_specular_map.m_negative_z_map);

        // -------------------------- BRDF查找表（LUT）资源加载 --------------------------
        // 加载BRDF预计算查找表（2D纹理），用于快速查询不同粗糙度和入射角下的BRDF值
        // 避免在着色器中实时计算复杂的积分，提升渲染性能
        std::shared_ptr<TextureData> brdf_map = loadTextureHDR(level_resource_desc.m_ibl_resource_desc.m_brdf_map);

        // -------------------------- 创建IBL采样器 --------------------------
        // 创建IBL相关纹理的采样器（如过滤方式、寻址模式等），确保纹理采样行为符合需求
        createIBLSamplers(rhi);

        // -------------------------- 创建IBL纹理（立方体贴图） --------------------------
        // 注意：立方体贴图的六个面需要按特定顺序排列（取决于底层API的要求，如Vulkan的VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT）
        // 辐照度纹理按 [右, 左, 前, 后, 上, 下] 顺序排列（可能适配某些API的立方体贴图面顺序要求）
        std::array<std::shared_ptr<TextureData>, 6> irradiance_maps =
        {
            irradiace_pos_x_map,
            irradiace_neg_x_map,
            irradiace_pos_z_map,
            irradiace_neg_z_map,
            irradiace_pos_y_map,
            irradiace_neg_y_map
        };
        // 高光纹理按 [右, 左, 前, 后, 上, 下] 顺序排列（与辐照度保持一致的面顺序）
        std::array<std::shared_ptr<TextureData>, 6> specular_maps =
        {
            specular_pos_x_map,
            specular_neg_x_map,
            specular_pos_z_map,
            specular_neg_z_map,
            specular_pos_y_map,
            specular_neg_y_map
        };

        // 创建IBL纹理对象（包含图像、图像视图和内存分配），将加载的HDR纹理上传至GPU
        // 这些纹理将作为全局资源供后续着色器采样使用
        createIBLTextures(rhi, irradiance_maps, specular_maps);

        // -------------------------- 创建BRDF LUT纹理 --------------------------
        // 在GPU上创建BRDF查找表的全局图像资源（包含图像、视图和内存分配）
        // 参数说明：图像对象、图像视图、内存分配句柄、宽度、高度、像素数据、纹理格式
        rhi->createGlobalImage(
            m_global_render_resource._ibl_resource._brdfLUT_texture_image,
            m_global_render_resource._ibl_resource._brdfLUT_texture_image_view,
            m_global_render_resource._ibl_resource._brdfLUT_texture_image_allocation,
            brdf_map->m_width,
            brdf_map->m_height,
            brdf_map->m_pixels,
            brdf_map->m_format);

        // -------------------------- 颜色分级（Color Grading）资源加载 --------------------------
        // 加载颜色分级LUT纹理（用于调整画面的色调、对比度、饱和度等）
        std::shared_ptr<TextureData> color_grading_map = loadTexture(level_resource_desc.m_color_grading_resource_desc.m_color_grading_map);

        // -------------------------- 创建颜色分级LUT纹理 --------------------------
        // 在GPU上创建颜色分级查找表的全局图像资源（包含图像、视图和内存分配）
        // 该纹理将用于后期处理阶段，对最终画面进行颜色校正
        rhi->createGlobalImage(
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image,
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_view,
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_allocation,
            color_grading_map->m_width,
            color_grading_map->m_height,
            color_grading_map->m_pixels,
            color_grading_map->m_format);
    }

    void RenderResource::uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data, RenderMaterialData material_data)
    {
        getOrCreateVulkanMesh(rhi, render_entity, mesh_data);
        getOrCreateVulkanMaterial(rhi, render_entity, material_data);
    }

    void RenderResource::uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data)
    {
        getOrCreateVulkanMesh(rhi, render_entity, mesh_data);
    }

    void RenderResource::uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMaterialData material_data)
    {
        getOrCreateVulkanMaterial(rhi, render_entity, material_data);
    }

    void RenderResource::updatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera)
    {
        // -------------------------- 相机矩阵计算 --------------------------
        Matrix4x4 view_matrix = camera->getViewMatrix();         // 相机视图矩阵（世界空间→相机空间）
        Matrix4x4 proj_matrix = camera->getPersProjMatrix();     // 相机透视投影矩阵（相机空间→裁剪空间）
        Vector3   camera_position = camera->position();          // 相机在世界空间中的位置
        Matrix4x4 proj_view_matrix = proj_matrix * view_matrix;  // 投影-视图复合矩阵（世界空间→裁剪空间）

        // -------------------------- 光照数据提取 --------------------------
        Vector3  ambient_light = render_scene->m_ambient_light.m_irradiance;                                 // 环境光辐照度（全局环境光照）
        uint32_t point_light_num = static_cast<uint32_t>(render_scene->m_point_light_list.m_lights.size());  // 点光源数量

        // -------------------------- 填充粒子碰撞通道UBO --------------------------
        m_particle_collision_perframe_storage_buffer_object.view_matrix      = view_matrix;            // 视图矩阵
        m_particle_collision_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;       // 投影-视图矩阵
        m_particle_collision_perframe_storage_buffer_object.proj_inv_matrix  = proj_matrix.inverse();  // 投影矩阵逆（用于深度解包等计算）

        // -------------------------- 填充网格渲染通道UBO --------------------------
        m_mesh_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;  // 投影-视图矩阵（供顶点着色器使用）
        m_mesh_perframe_storage_buffer_object.camera_position = camera_position;    // 相机位置（供光照计算使用）
        m_mesh_perframe_storage_buffer_object.ambient_light = ambient_light;        // 环境光辐照度（供PBR计算使用）
        m_mesh_perframe_storage_buffer_object.point_light_num = point_light_num;    // 点光源数量（用于循环上限）

        // -------------------------- 填充点光源阴影通道UBO --------------------------
        m_mesh_point_light_shadow_perframe_storage_buffer_object.point_light_num = point_light_num;  // 点光源数量

        // -------------------------- 遍历点光源数据 --------------------------
        for (uint32_t i = 0; i < point_light_num; i++)
        {
            // 提取点光源位置和强度（强度通过光通量/4π归一化，符合PBR能量守恒）
            Vector3 point_light_position  = render_scene->m_point_light_list.m_lights[i].m_position;
            Vector3 point_light_intensity = render_scene->m_point_light_list.m_lights[i].m_flux / (4.0f * Math_PI);

            // 计算点光源半径（根据光强和衰减参数动态调整，确保光照范围合理）
            float radius = render_scene->m_point_light_list.m_lights[i].calculateRadius();

            // 填充网格渲染通道的点光源数组（供片段着色器采样）
            m_mesh_perframe_storage_buffer_object.scene_point_lights[i].position  = point_light_position;
            m_mesh_perframe_storage_buffer_object.scene_point_lights[i].radius    = radius;
            m_mesh_perframe_storage_buffer_object.scene_point_lights[i].intensity = point_light_intensity;

            // 填充点光源阴影通道的位置-半径数据（用于阴影映射）
            m_mesh_point_light_shadow_perframe_storage_buffer_object.point_lights_position_and_radius[i] = Vector4(point_light_position, radius);
        }

        // -------------------------- 填充方向光数据 --------------------------
        // 方向光方向（单位化）和颜色（供阴影计算和直接光照使用）
        m_mesh_perframe_storage_buffer_object.scene_directional_light.direction = render_scene->m_directional_light.m_direction.normalisedCopy();
        m_mesh_perframe_storage_buffer_object.scene_directional_light.color     = render_scene->m_directional_light.m_color;

        // -------------------------- 填充拾取通道UBO --------------------------
        // 拾取通道使用与主渲染相同的投影-视图矩阵（用于将屏幕坐标转换为世界坐标）
        m_mesh_inefficient_pick_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;

        // -------------------------- 填充粒子广告牌通道UBO --------------------------
        // 广告牌需要相机方向信息以始终面向相机（右/前/上方向用于调整粒子朝向）
        m_particlebillboard_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;
        m_particlebillboard_perframe_storage_buffer_object.right_direction  = camera->right();
        m_particlebillboard_perframe_storage_buffer_object.foward_direction = camera->forward();
        m_particlebillboard_perframe_storage_buffer_object.up_direction     = camera->up();
    }

    void RenderResource::createIBLSamplers(std::shared_ptr<RHI> rhi)
    {
        // 将共享指针转换为Vulkan RHI的具体实现指针（假设RHI的多态实现中，VulkanRHI是具体子类）
        VulkanRHI* raw_rhi = static_cast<VulkanRHI*>(rhi.get());

        // -------------------------- 获取物理设备属性 --------------------------
        RHIPhysicalDeviceProperties physical_device_properties{};
        rhi->getPhysicalDeviceProperties(&physical_device_properties);
        // 用于获取GPU支持的最大各向异性级别等参数（影响采样器配置）

        // -------------------------- 初始化采样器创建信息 --------------------------
        RHISamplerCreateInfo samplerInfo{};
        samplerInfo.sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // 纹理过滤方式：放大/缩小均使用线性插值（平滑过渡，适合环境贴图）
        samplerInfo.magFilter = RHI_FILTER_LINEAR;
        samplerInfo.minFilter = RHI_FILTER_LINEAR;

        // 纹理坐标超出[0,1]范围时的处理方式：边缘钳制（避免边缘黑边或重复）
        samplerInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        // 启用各向异性过滤（提升纹理在视角倾斜时的采样质量）
        samplerInfo.anisotropyEnable = RHI_TRUE;
        // 最大各向异性级别（取自GPU支持的物理设备属性，通常为2的幂次，如4/8/16）
        samplerInfo.maxAnisotropy = physical_device_properties.limits.maxSamplerAnisotropy;

        // 边界颜色：当纹理坐标钳制到边缘时，使用不透明黑色（常见于环境贴图）
        samplerInfo.borderColor = RHI_BORDER_COLOR_INT_OPAQUE_BLACK;
        // 是否使用非标准化坐标（此处使用标准化坐标[0,1]）
        samplerInfo.unnormalizedCoordinates = RHI_FALSE;
        // 禁用深度比较（非阴影采样器，普通纹理采样不需要比较操作）
        samplerInfo.compareEnable = RHI_FALSE;
        samplerInfo.compareOp = RHI_COMPARE_OP_ALWAYS;
        // Mipmap过滤方式：线性插值（平滑过渡不同Mip层级）
        samplerInfo.mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.maxLod = 0.0f;

        // -------------------------- 创建/更新BRDF LUT采样器 --------------------------
        // BRDF LUT是2D纹理，通常不需要Mipmap（或仅单级），初始最大Lod设为0
        if (m_global_render_resource._ibl_resource._brdfLUT_texture_sampler != RHI_NULL_HANDLE)
        {
            rhi->destroySampler(m_global_render_resource._ibl_resource._brdfLUT_texture_sampler);
        }

        // 创建新的BRDF LUT采样器（使用上述通用配置）
        if (rhi->createSampler(&samplerInfo, m_global_render_resource._ibl_resource._brdfLUT_texture_sampler) != RHI_SUCCESS)
        {
            throw std::runtime_error("vk create sampler!");
        }

        // -------------------------- 更新辐照度贴图采样器 --------------------------
        // 辐照度贴图是立方体贴图，通常包含多级Mipmap（用于不同距离的采样）
        samplerInfo.minLod = 0.0f;// 最小Mipmap层级（从0级开始）
        samplerInfo.maxLod = 8.0f; // 最大Mipmap层级（假设辐照度贴图有8级，TODO需根据实际调整）
        samplerInfo.mipLodBias = 0.0f;// Mipmap层级偏移（无额外偏移）  

        if (m_global_render_resource._ibl_resource._irradiance_texture_sampler != RHI_NULL_HANDLE)
        {
            rhi->destroySampler(m_global_render_resource._ibl_resource._irradiance_texture_sampler);
        }

        if (rhi->createSampler(&samplerInfo, m_global_render_resource._ibl_resource._irradiance_texture_sampler) != RHI_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }

        // -------------------------- 更新高光贴图采样器 --------------------------
        // 高光贴图（预计算的环境光遮蔽或球谐系数）通常与辐照度贴图使用相同采样配置
        if (m_global_render_resource._ibl_resource._specular_texture_sampler != RHI_NULL_HANDLE)
        {
            rhi->destroySampler(m_global_render_resource._ibl_resource._specular_texture_sampler);
        }

        if (rhi->createSampler(&samplerInfo, m_global_render_resource._ibl_resource._specular_texture_sampler) != RHI_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }
    }

    void RenderResource::createIBLTextures(std::shared_ptr<RHI> rhi, std::array<std::shared_ptr<TextureData>, 6> irradiance_maps, std::array<std::shared_ptr<TextureData>, 6> specular_maps)
    {
        // -------------------------- 计算辐照度贴图的Mipmap层级数 --------------------------
        // 辐照度贴图是立方体贴图，其Mipmap层级由宽高的最大值决定（log2(max(w,h))向下取整 +1）
        uint32_t irradiance_cubemap_miplevels = static_cast<uint32_t>(std::floor(log2(std::max(irradiance_maps[0]->m_width, irradiance_maps[0]->m_height)))) + 1;

        // 调用Vulkan RHI接口创建立方体贴图：
        // - 图像对象、视图对象、内存分配句柄（输出参数）
        // - 宽高（立方体贴图每个面为正方形）
        // - 六个面的像素数据（按顺序传入）
        // - 格式（如R16G16B16A16_SFLOAT）
        // - Mipmap层级数（用于生成所有层级的Mipmap）
        rhi->createCubeMap(
            m_global_render_resource._ibl_resource._irradiance_texture_image,
            m_global_render_resource._ibl_resource._irradiance_texture_image_view,
            m_global_render_resource._ibl_resource._irradiance_texture_image_allocation,
            irradiance_maps[0]->m_width,
            irradiance_maps[0]->m_height,
            { irradiance_maps[0]->m_pixels,   // +X面像素数据
             irradiance_maps[1]->m_pixels,    // -X面像素数据
             irradiance_maps[2]->m_pixels,    // +Z面像素数据
             irradiance_maps[3]->m_pixels,    // -Z面像素数据
             irradiance_maps[4]->m_pixels,    // +Y面像素数据
             irradiance_maps[5]->m_pixels },  // -Y面像素数据
            irradiance_maps[0]->m_format,     // 纹理格式
            irradiance_cubemap_miplevels);    // Mipmap层级数

        // -------------------------- 计算高光贴图的Mipmap层级数 --------------------------
        uint32_t specular_cubemap_miplevels = static_cast<uint32_t>(std::floor(log2(std::max(specular_maps[0]->m_width, specular_maps[0]->m_height)))) + 1;

        // -------------------------- 创建高光贴图立方体贴图 --------------------------
        rhi->createCubeMap(
            m_global_render_resource._ibl_resource._specular_texture_image,
            m_global_render_resource._ibl_resource._specular_texture_image_view,
            m_global_render_resource._ibl_resource._specular_texture_image_allocation,
            specular_maps[0]->m_width,
            specular_maps[0]->m_height,
            { specular_maps[0]->m_pixels,
             specular_maps[1]->m_pixels,
             specular_maps[2]->m_pixels,
             specular_maps[3]->m_pixels,
             specular_maps[4]->m_pixels,
             specular_maps[5]->m_pixels },
            specular_maps[0]->m_format,
            specular_cubemap_miplevels);
    }

    VulkanMesh& RenderResource::getOrCreateVulkanMesh(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMeshData mesh_data)
    {
        size_t assetid = entity.m_mesh_asset_id;  // 网格资源的唯一标识（资产ID）

        // -------------------------- 检查资源是否已存在 --------------------------
        auto it = m_vulkan_meshes.find(assetid);  // 在已管理的Vulkan网格映射中查找
        if (it != m_vulkan_meshes.end())  // 若找到已存在的资源
        {
            return it->second;  // 直接返回现有资源
        }
        else
        {
            VulkanMesh temp; // 临时VulkanMesh对象（用于插入映射）
            auto       res = m_vulkan_meshes.insert(std::make_pair(assetid, std::move(temp)));// 插入新资源到映射
            assert(res.second);// 断言插入成功（避免重复插入）

            // -------------------------- 提取顶点/索引缓冲区数据 --------------------------
            uint32_t index_buffer_size = static_cast<uint32_t>(mesh_data.m_static_mesh_data.m_index_buffer->m_size);  // 索引缓冲区大小（字节）
            void* index_buffer_data = mesh_data.m_static_mesh_data.m_index_buffer->m_data;// 索引缓冲区数据指针

            uint32_t vertex_buffer_size = static_cast<uint32_t>(mesh_data.m_static_mesh_data.m_vertex_buffer->m_size);// 顶点缓冲区大小（字节）
            MeshVertexDataDefinition* vertex_buffer_data = reinterpret_cast<MeshVertexDataDefinition*>(mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);// 顶点缓冲区数据指针（转换为自定义结构体）

            // -------------------------- 获取当前新创建的网格资源引用 --------------------------
            VulkanMesh& now_mesh = res.first->second;

            // -------------------------- 处理骨骼绑定数据（若有） --------------------------
            if (mesh_data.m_skeleton_binding_buffer)  // 若存在骨骼绑定缓冲区（如角色模型的骨骼权重/索引）
            {
                uint32_t joint_binding_buffer_size = (uint32_t)mesh_data.m_skeleton_binding_buffer->m_size;  // 骨骼绑定缓冲区大小
                MeshVertexBindingDataDefinition* joint_binding_buffer_data = reinterpret_cast<MeshVertexBindingDataDefinition*>(mesh_data.m_skeleton_binding_buffer->m_data);

                // 调用更新函数，上传顶点/索引/骨骼数据到GPU（使用骨骼绑定数据）
                updateMeshData(rhi,
                               true,
                               index_buffer_size,
                               index_buffer_data,
                               vertex_buffer_size,
                               vertex_buffer_data,
                               joint_binding_buffer_size,
                               joint_binding_buffer_data,
                               now_mesh);
            }
            else
            {
                // 调用更新函数，仅上传顶点/索引数据（无骨骼绑定）
                updateMeshData(rhi,
                               false,
                               index_buffer_size,
                               index_buffer_data,
                               vertex_buffer_size,
                               vertex_buffer_data,
                               0,
                               NULL,
                               now_mesh);
            }

            return now_mesh;  // 返回新创建的网格资源
        }
    }

    VulkanPBRMaterial& RenderResource::getOrCreateVulkanMaterial(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMaterialData material_data)
    {
        VulkanRHI* vulkan_context = static_cast<VulkanRHI*>(rhi.get());

        // 从材质实体中提取资产 ID（材质资源的唯一标识，用于缓存查找）
        size_t assetid = entity.m_material_asset_id;

        // -------------------- 步骤 1：检查材质是否已缓存 --------------------
        // 在已管理的 Vulkan PBR 材质容器中查找是否存在该资产的材质
        auto it = m_vulkan_pbr_materials.find(assetid);
        if (it != m_vulkan_pbr_materials.end())
        {
            return it->second;
        }
        else
        {
            // -------------------- 步骤 2：插入临时材质对象到容器 --------------------
            VulkanPBRMaterial temp;
            // 插入新材质到容器（使用 emplace 或 insert 避免拷贝）
            auto res = m_vulkan_pbr_materials.insert(std::make_pair(assetid, std::move(temp)));
            assert(res.second);

            // -------------------- 步骤 3：初始化纹理相关参数（默认值 + 实际纹理数据） --------------------
            // 以下为各类纹理（基础颜色、金属粗糙度等）的像素数据、尺寸和格式的初始化
            // 默认使用 1x1 的灰色像素（RGBA 各通道 0.5f），格式为 SRGB（适合 sRGB 纹理空间）

            float empty_image[] = { 0.5f, 0.5f, 0.5f, 0.5f };

            // 基础颜色纹理参数
            void*     base_color_image_pixels = empty_image;  // 像素数据指针（默认空图像）
            uint32_t  base_color_image_width  = 1;            // 纹理宽度（默认 1）
            uint32_t  base_color_image_height = 1;            // 纹理高度（默认 1）
            RHIFormat base_color_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB;  // 格式（默认 SRGB）
            if (material_data.m_base_color_texture)           // 若存在基础颜色纹理
            {
                // 使用实际纹理数据覆盖默认值
                base_color_image_pixels = material_data.m_base_color_texture->m_pixels;
                base_color_image_width  = static_cast<uint32_t>(material_data.m_base_color_texture->m_width);
                base_color_image_height = static_cast<uint32_t>(material_data.m_base_color_texture->m_height);
                base_color_image_format = material_data.m_base_color_texture->m_format;
            }

            // 金属-粗糙度纹理参数（类似基础颜色纹理的处理逻辑）
            void*     metallic_roughness_image_pixels = empty_image;
            uint32_t  metallic_roughness_width        = 1;
            uint32_t  metallic_roughness_height       = 1;
            RHIFormat metallic_roughness_format       = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_metallic_roughness_texture)
            {
                metallic_roughness_image_pixels = material_data.m_metallic_roughness_texture->m_pixels;
                metallic_roughness_width        = static_cast<uint32_t>(material_data.m_metallic_roughness_texture->m_width);
                metallic_roughness_height       = static_cast<uint32_t>(material_data.m_metallic_roughness_texture->m_height);
                metallic_roughness_format       = material_data.m_metallic_roughness_texture->m_format;
            }

            // 法线-粗糙度纹理参数（注意：此处可能为笔误，通常法线纹理格式为 RGB，但按代码逻辑处理）
            void* normal_roughness_image_pixels = empty_image;
            uint32_t           normal_roughness_width = 1;
            uint32_t           normal_roughness_height = 1;
            RHIFormat normal_roughness_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_normal_texture)
            {
                normal_roughness_image_pixels = material_data.m_normal_texture->m_pixels;
                normal_roughness_width = static_cast<uint32_t>(material_data.m_normal_texture->m_width);
                normal_roughness_height = static_cast<uint32_t>(material_data.m_normal_texture->m_height);
                normal_roughness_format = material_data.m_normal_texture->m_format;
            }

            // 遮挡纹理参数
            void* occlusion_image_pixels = empty_image;
            uint32_t           occlusion_image_width = 1;
            uint32_t           occlusion_image_height = 1;
            RHIFormat occlusion_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_occlusion_texture)
            {
                occlusion_image_pixels = material_data.m_occlusion_texture->m_pixels;
                occlusion_image_width = static_cast<uint32_t>(material_data.m_occlusion_texture->m_width);
                occlusion_image_height = static_cast<uint32_t>(material_data.m_occlusion_texture->m_height);
                occlusion_image_format = material_data.m_occlusion_texture->m_format;
            }

            // 自发光纹理参数
            void* emissive_image_pixels = empty_image;
            uint32_t           emissive_image_width = 1;
            uint32_t           emissive_image_height = 1;
            RHIFormat emissive_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_emissive_texture)
            {
                emissive_image_pixels = material_data.m_emissive_texture->m_pixels;
                emissive_image_width  = static_cast<uint32_t>(material_data.m_emissive_texture->m_width);
                emissive_image_height = static_cast<uint32_t>(material_data.m_emissive_texture->m_height);
                emissive_image_format = material_data.m_emissive_texture->m_format;
            }

            VulkanPBRMaterial& now_material = res.first->second;

            // -------------------- 步骤 4：初始化材质统一缓冲区（Uniform Buffer） --------------------
            // 统一缓冲区用于存储材质参数（如混合标志、双面渲染、基色因子等），供着色器访问
            {
                // 统一缓冲区大小（等于 MeshPerMaterialUniformBufferObject 结构体大小）
                RHIDeviceSize buffer_size = sizeof(MeshPerMaterialUniformBufferObject);

                // 创建临时暂存缓冲区（用于将数据从主机内存复制到 GPU 内存）
                // 使用 HOST_VISIBLE 和 HOST_COHERENT 属性，允许主机直接读写且无需手动刷新缓存
                RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
                RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
                rhi->createBuffer(
                    buffer_size,
                    RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    inefficient_staging_buffer,
                    inefficient_staging_buffer_memory);

                // 映射暂存缓冲区内存到主机地址空间（获取指针以写入数据）
                void* staging_buffer_data = nullptr;
                rhi->mapMemory(
                    inefficient_staging_buffer_memory,
                    0,
                    buffer_size,
                    0,
                    &staging_buffer_data);
                
                // 填充统一缓冲区数据（基于材质实体的属性）
                MeshPerMaterialUniformBufferObject& material_uniform_buffer_info = (*static_cast<MeshPerMaterialUniformBufferObject*>(staging_buffer_data));
                material_uniform_buffer_info.is_blend = entity.m_blend;// 是否启用混合
                material_uniform_buffer_info.is_double_sided = entity.m_double_sided;// 是否双面渲染
                material_uniform_buffer_info.baseColorFactor = entity.m_base_color_factor;// 基色因子（RGBA）
                material_uniform_buffer_info.metallicFactor = entity.m_metallic_factor;// 金属度因子
                material_uniform_buffer_info.roughnessFactor = entity.m_roughness_factor;// 粗糙度因子
                material_uniform_buffer_info.normalScale = entity.m_normal_scale;// 法线贴图缩放
                material_uniform_buffer_info.occlusionStrength = entity.m_occlusion_strength;// 遮挡强度
                material_uniform_buffer_info.emissiveFactor = entity.m_emissive_factor;// 自发光因子

                // 解除内存映射（释放主机端指针，数据已同步到 GPU）
                rhi->unmapMemory(inefficient_staging_buffer_memory);

                // 创建 GPU 专用统一缓冲区（使用 VMA 库管理内存分配）
                // 缓冲区用途：UNIFORM_BUFFER（着色器统一缓冲区） + TRANSFER_DST（接收传输数据）
                RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                bufferInfo.size = buffer_size;
                bufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

                // VMA 内存分配策略：仅 GPU 可见（避免 CPU 访问开销）
                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

                // 分配缓冲区并绑定内存（使用对齐要求，满足 Vulkan 对齐约束）
                rhi->createBufferWithAlignmentVMA(
                    vulkan_context->m_assets_allocator,// VMA 分配器实例
                    &bufferInfo,// 缓冲区创建信息
                    &allocInfo,// 内存分配策略
                    m_global_render_resource._storage_buffer._min_uniform_buffer_offset_alignment,// 最小对齐要求
                    now_material.material_uniform_buffer,
                    &now_material.material_uniform_buffer_allocation,
                    NULL);

                // use the data from staging buffer
                rhi->copyBuffer(inefficient_staging_buffer, now_material.material_uniform_buffer, 0, 0, buffer_size);

                // release staging buffer
                rhi->destroyBuffer(inefficient_staging_buffer);
                rhi->freeMemory(inefficient_staging_buffer_memory);
            }

            TextureDataToUpdate update_texture_data;
            update_texture_data.base_color_image_pixels         = base_color_image_pixels;
            update_texture_data.base_color_image_width          = base_color_image_width;
            update_texture_data.base_color_image_height         = base_color_image_height;
            update_texture_data.base_color_image_format         = base_color_image_format;
            update_texture_data.metallic_roughness_image_pixels = metallic_roughness_image_pixels;
            update_texture_data.metallic_roughness_image_width  = metallic_roughness_width;
            update_texture_data.metallic_roughness_image_height = metallic_roughness_height;
            update_texture_data.metallic_roughness_image_format = metallic_roughness_format;
            update_texture_data.normal_roughness_image_pixels   = normal_roughness_image_pixels;
            update_texture_data.normal_roughness_image_width    = normal_roughness_width;
            update_texture_data.normal_roughness_image_height   = normal_roughness_height;
            update_texture_data.normal_roughness_image_format   = normal_roughness_format;
            update_texture_data.occlusion_image_pixels          = occlusion_image_pixels;
            update_texture_data.occlusion_image_width           = occlusion_image_width;
            update_texture_data.occlusion_image_height          = occlusion_image_height;
            update_texture_data.occlusion_image_format          = occlusion_image_format;
            update_texture_data.emissive_image_pixels           = emissive_image_pixels;
            update_texture_data.emissive_image_width            = emissive_image_width;
            update_texture_data.emissive_image_height           = emissive_image_height;
            update_texture_data.emissive_image_format           = emissive_image_format;
            update_texture_data.now_material                    = &now_material;

            updateTextureImageData(rhi, update_texture_data);

            RHIDescriptorSetAllocateInfo material_descriptor_set_alloc_info;
            material_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            material_descriptor_set_alloc_info.pNext = NULL;
            material_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
            material_descriptor_set_alloc_info.descriptorSetCount = 1;
            material_descriptor_set_alloc_info.pSetLayouts        = m_material_descriptor_set_layout;

            if (RHI_SUCCESS != rhi->allocateDescriptorSets(
                &material_descriptor_set_alloc_info,
                now_material.material_descriptor_set))
            {
                throw std::runtime_error("allocate material descriptor set");
            }

            RHIDescriptorBufferInfo material_uniform_buffer_info = {};
            material_uniform_buffer_info.offset = 0;
            material_uniform_buffer_info.range = sizeof(MeshPerMaterialUniformBufferObject);
            material_uniform_buffer_info.buffer = now_material.material_uniform_buffer;

            RHIDescriptorImageInfo base_color_image_info = {};
            base_color_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            base_color_image_info.imageView = now_material.base_color_image_view;
            base_color_image_info.sampler = rhi->getOrCreateMipmapSampler(base_color_image_width,
                                                                          base_color_image_height);

            RHIDescriptorImageInfo metallic_roughness_image_info = {};
            metallic_roughness_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            metallic_roughness_image_info.imageView = now_material.metallic_roughness_image_view;
            metallic_roughness_image_info.sampler = rhi->getOrCreateMipmapSampler(metallic_roughness_width,
                                                                                  metallic_roughness_height);

            RHIDescriptorImageInfo normal_roughness_image_info = {};
            normal_roughness_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normal_roughness_image_info.imageView = now_material.normal_image_view;
            normal_roughness_image_info.sampler = rhi->getOrCreateMipmapSampler(normal_roughness_width,
                                                                                normal_roughness_height);

            RHIDescriptorImageInfo occlusion_image_info = {};
            occlusion_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            occlusion_image_info.imageView = now_material.occlusion_image_view;
            occlusion_image_info.sampler = rhi->getOrCreateMipmapSampler(occlusion_image_width,occlusion_image_height);

            RHIDescriptorImageInfo emissive_image_info = {};
            emissive_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            emissive_image_info.imageView = now_material.emissive_image_view;
            emissive_image_info.sampler = rhi->getOrCreateMipmapSampler(emissive_image_width, emissive_image_height);

            RHIWriteDescriptorSet mesh_descriptor_writes_info[6];

            mesh_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_descriptor_writes_info[0].pNext = NULL;
            mesh_descriptor_writes_info[0].dstSet = now_material.material_descriptor_set;
            mesh_descriptor_writes_info[0].dstBinding = 0;
            mesh_descriptor_writes_info[0].dstArrayElement = 0;
            mesh_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            mesh_descriptor_writes_info[0].descriptorCount = 1;
            mesh_descriptor_writes_info[0].pBufferInfo = &material_uniform_buffer_info;

            mesh_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_descriptor_writes_info[1].pNext = NULL;
            mesh_descriptor_writes_info[1].dstSet = now_material.material_descriptor_set;
            mesh_descriptor_writes_info[1].dstBinding = 1;
            mesh_descriptor_writes_info[1].dstArrayElement = 0;
            mesh_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            mesh_descriptor_writes_info[1].descriptorCount = 1;
            mesh_descriptor_writes_info[1].pImageInfo = &base_color_image_info;

            mesh_descriptor_writes_info[2] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[2].dstBinding = 2;
            mesh_descriptor_writes_info[2].pImageInfo = &metallic_roughness_image_info;

            mesh_descriptor_writes_info[3] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[3].dstBinding = 3;
            mesh_descriptor_writes_info[3].pImageInfo = &normal_roughness_image_info;

            mesh_descriptor_writes_info[4] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[4].dstBinding = 4;
            mesh_descriptor_writes_info[4].pImageInfo = &occlusion_image_info;

            mesh_descriptor_writes_info[5] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[5].dstBinding = 5;
            mesh_descriptor_writes_info[5].pImageInfo = &emissive_image_info;

            rhi->updateDescriptorSets(6, mesh_descriptor_writes_info, 0, nullptr);

            return now_material;
        }
    }

    void RenderResource::updateMeshData(std::shared_ptr<RHI>                   rhi,
                                        bool                                   enable_vertex_blending,
                                        uint32_t                               index_buffer_size,
                                        void*                                  index_buffer_data,
                                        uint32_t                               vertex_buffer_size,
                                        MeshVertexDataDefinition const*        vertex_buffer_data,
                                        uint32_t                               joint_binding_buffer_size,
                                        MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                        VulkanMesh&                            now_mesh)
    {
        now_mesh.enable_vertex_blending = enable_vertex_blending;
        assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
        now_mesh.mesh_vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
        updateVertexBuffer(rhi,
                           enable_vertex_blending,
                           vertex_buffer_size,
                           vertex_buffer_data,
                           joint_binding_buffer_size,
                           joint_binding_buffer_data,
                           index_buffer_size,
                           reinterpret_cast<uint16_t*>(index_buffer_data),
                           now_mesh);
        assert(0 == (index_buffer_size % sizeof(uint16_t)));
        now_mesh.mesh_index_count = index_buffer_size / sizeof(uint16_t);
        updateIndexBuffer(rhi, index_buffer_size, index_buffer_data, now_mesh);
    }

    void RenderResource::updateVertexBuffer(std::shared_ptr<RHI>                   rhi,
                                            bool                                   enable_vertex_blending,
                                            uint32_t                               vertex_buffer_size,
                                            MeshVertexDataDefinition const*        vertex_buffer_data,
                                            uint32_t                               joint_binding_buffer_size,
                                            MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                            uint32_t                               index_buffer_size,
                                            uint16_t*                              index_buffer_data,
                                            VulkanMesh&                            now_mesh)
    {
        VulkanRHI* vulkan_context = static_cast<VulkanRHI*>(rhi.get());

        if (enable_vertex_blending)
        {
            assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
            uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
            assert(0 == (index_buffer_size % sizeof(uint16_t)));
            uint32_t index_count = index_buffer_size / sizeof(uint16_t);

            RHIDeviceSize vertex_position_buffer_size = sizeof(MeshVertex::VulkanMeshVertexPostition) * vertex_count;
            RHIDeviceSize vertex_varying_enable_blending_buffer_size =
                sizeof(MeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
            RHIDeviceSize vertex_varying_buffer_size = sizeof(MeshVertex::VulkanMeshVertexVarying) * vertex_count;
            RHIDeviceSize vertex_joint_binding_buffer_size =
                sizeof(MeshVertex::VulkanMeshVertexJointBinding) * index_count;

            RHIDeviceSize vertex_position_buffer_offset = 0;
            RHIDeviceSize vertex_varying_enable_blending_buffer_offset =
                vertex_position_buffer_offset + vertex_position_buffer_size;
            RHIDeviceSize vertex_varying_buffer_offset =
                vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;
            RHIDeviceSize vertex_joint_binding_buffer_offset = vertex_varying_buffer_offset + vertex_varying_buffer_size;

            // temporary staging buffer
            RHIDeviceSize inefficient_staging_buffer_size =
                vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size +
                vertex_joint_binding_buffer_size;
            RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
            RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
            rhi->createBuffer(inefficient_staging_buffer_size,
                              RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

            void* inefficient_staging_buffer_data;
            rhi->mapMemory(inefficient_staging_buffer_memory,
                           0,
                           RHI_WHOLE_SIZE,
                           0,
                           &inefficient_staging_buffer_data);

            MeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
                reinterpret_cast<MeshVertex::VulkanMeshVertexPostition*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
            MeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
                    vertex_varying_enable_blending_buffer_offset);
            MeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVarying*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);
            MeshVertex::VulkanMeshVertexJointBinding* mesh_vertex_joint_binding =
                reinterpret_cast<MeshVertex::VulkanMeshVertexJointBinding*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_joint_binding_buffer_offset);

            for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
            {
                Vector3 normal = Vector3(vertex_buffer_data[vertex_index].nx,
                    vertex_buffer_data[vertex_index].ny,
                    vertex_buffer_data[vertex_index].nz);
                Vector3 tangent = Vector3(vertex_buffer_data[vertex_index].tx,
                    vertex_buffer_data[vertex_index].ty,
                    vertex_buffer_data[vertex_index].tz);

                mesh_vertex_positions[vertex_index].position = Vector3(vertex_buffer_data[vertex_index].x,
                    vertex_buffer_data[vertex_index].y,
                    vertex_buffer_data[vertex_index].z);

                mesh_vertex_blending_varyings[vertex_index].normal = normal;
                mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

                mesh_vertex_varyings[vertex_index].texcoord =
                    Vector2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
            }

            for (uint32_t index_index = 0; index_index < index_count; ++index_index)
            {
                uint32_t vertex_buffer_index = index_buffer_data[index_index];

                // TODO: move to assets loading process

                mesh_vertex_joint_binding[index_index].indices[0] = joint_binding_buffer_data[vertex_buffer_index].m_index0;
                mesh_vertex_joint_binding[index_index].indices[1] = joint_binding_buffer_data[vertex_buffer_index].m_index1;
                mesh_vertex_joint_binding[index_index].indices[2] = joint_binding_buffer_data[vertex_buffer_index].m_index2;
                mesh_vertex_joint_binding[index_index].indices[3] = joint_binding_buffer_data[vertex_buffer_index].m_index3;

                float inv_total_weight = joint_binding_buffer_data[vertex_buffer_index].m_weight0 +
                                         joint_binding_buffer_data[vertex_buffer_index].m_weight1 +
                                         joint_binding_buffer_data[vertex_buffer_index].m_weight2 +
                                         joint_binding_buffer_data[vertex_buffer_index].m_weight3;

                inv_total_weight = (inv_total_weight != 0.0) ? 1 / inv_total_weight : 1.0;

                mesh_vertex_joint_binding[index_index].weights =
                    Vector4(joint_binding_buffer_data[vertex_buffer_index].m_weight0 * inv_total_weight,
                        joint_binding_buffer_data[vertex_buffer_index].m_weight1 * inv_total_weight,
                        joint_binding_buffer_data[vertex_buffer_index].m_weight2 * inv_total_weight,
                        joint_binding_buffer_data[vertex_buffer_index].m_weight3 * inv_total_weight);
            }

            rhi->unmapMemory(inefficient_staging_buffer_memory);

            // use the vmaAllocator to allocate asset vertex buffer
            RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.size = vertex_position_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_position_buffer,
                                 &now_mesh.mesh_vertex_position_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_enable_blending_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_enable_blending_buffer,
                                 &now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_buffer,
                                 &now_mesh.mesh_vertex_varying_buffer_allocation,
                                 NULL);

            bufferInfo.usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.size = vertex_joint_binding_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_joint_binding_buffer,
                                 &now_mesh.mesh_vertex_joint_binding_buffer_allocation,
                                 NULL);

            // use the data from staging buffer
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_position_buffer,
                            vertex_position_buffer_offset,
                            0,
                            vertex_position_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_enable_blending_buffer,
                            vertex_varying_enable_blending_buffer_offset,
                            0,
                            vertex_varying_enable_blending_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_buffer,
                            vertex_varying_buffer_offset,
                            0,
                            vertex_varying_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_joint_binding_buffer,
                            vertex_joint_binding_buffer_offset,
                            0,
                            vertex_joint_binding_buffer_size);

            // release staging buffer
            rhi->destroyBuffer(inefficient_staging_buffer);
            rhi->freeMemory(inefficient_staging_buffer_memory);

            // update descriptor set
            RHIDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
                RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts        = m_mesh_descriptor_set_layout;

            if (RHI_SUCCESS != rhi->allocateDescriptorSets(
                &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                now_mesh.mesh_vertex_blending_descriptor_set))
            {
                throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
            }

            RHIDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
            mesh_vertex_Joint_binding_storage_buffer_info.offset = 0;
            mesh_vertex_Joint_binding_storage_buffer_info.range = vertex_joint_binding_buffer_size;
            mesh_vertex_Joint_binding_storage_buffer_info.buffer = now_mesh.mesh_vertex_joint_binding_buffer;
            assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
                m_global_render_resource._storage_buffer._max_storage_buffer_range);

            RHIDescriptorSet* descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

            RHIWriteDescriptorSet descriptor_writes[1];

            RHIWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
                descriptor_writes[0];
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
                RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = NULL;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet = descriptor_set_to_write;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
                RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
                &mesh_vertex_Joint_binding_storage_buffer_info;

            rhi->updateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                                      descriptor_writes,
                                      0,
                                      NULL);
        }
        else
        {
            assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
            uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);

            RHIDeviceSize vertex_position_buffer_size = sizeof(MeshVertex::VulkanMeshVertexPostition) * vertex_count;
            RHIDeviceSize vertex_varying_enable_blending_buffer_size =
                sizeof(MeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
            RHIDeviceSize vertex_varying_buffer_size = sizeof(MeshVertex::VulkanMeshVertexVarying) * vertex_count;

            RHIDeviceSize vertex_position_buffer_offset = 0;
            RHIDeviceSize vertex_varying_enable_blending_buffer_offset =
                vertex_position_buffer_offset + vertex_position_buffer_size;
            RHIDeviceSize vertex_varying_buffer_offset =
                vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;

            // temporary staging buffer
            RHIDeviceSize inefficient_staging_buffer_size =
                vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size;
            RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
            RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
            rhi->createBuffer(inefficient_staging_buffer_size,
                              RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

            void* inefficient_staging_buffer_data;
            rhi->mapMemory(inefficient_staging_buffer_memory,
                           0,
                           RHI_WHOLE_SIZE,
                           0,
                           &inefficient_staging_buffer_data);

            MeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
                reinterpret_cast<MeshVertex::VulkanMeshVertexPostition*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
            MeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
                    vertex_varying_enable_blending_buffer_offset);
            MeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVarying*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);

            for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
            {
                Vector3 normal = Vector3(vertex_buffer_data[vertex_index].nx,
                    vertex_buffer_data[vertex_index].ny,
                    vertex_buffer_data[vertex_index].nz);
                Vector3 tangent = Vector3(vertex_buffer_data[vertex_index].tx,
                    vertex_buffer_data[vertex_index].ty,
                    vertex_buffer_data[vertex_index].tz);

                mesh_vertex_positions[vertex_index].position = Vector3(vertex_buffer_data[vertex_index].x,
                    vertex_buffer_data[vertex_index].y,
                    vertex_buffer_data[vertex_index].z);

                mesh_vertex_blending_varyings[vertex_index].normal = normal;
                mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

                mesh_vertex_varyings[vertex_index].texcoord =
                    Vector2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
            }

            rhi->unmapMemory(inefficient_staging_buffer_memory);

            // use the vmaAllocator to allocate asset vertex buffer
            RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            bufferInfo.size = vertex_position_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_position_buffer,
                                 &now_mesh.mesh_vertex_position_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_enable_blending_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_enable_blending_buffer,
                                 &now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_buffer,
                                 &now_mesh.mesh_vertex_varying_buffer_allocation,
                                 NULL);

            // use the data from staging buffer
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_position_buffer,
                            vertex_position_buffer_offset,
                            0,
                            vertex_position_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_enable_blending_buffer,
                            vertex_varying_enable_blending_buffer_offset,
                            0,
                            vertex_varying_enable_blending_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_buffer,
                            vertex_varying_buffer_offset,
                            0,
                            vertex_varying_buffer_size);

            // release staging buffer
            rhi->destroyBuffer(inefficient_staging_buffer);
            rhi->freeMemory(inefficient_staging_buffer_memory);

            // update descriptor set
            RHIDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
                RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts        = m_mesh_descriptor_set_layout;

            if (RHI_SUCCESS != rhi->allocateDescriptorSets(
                &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                now_mesh.mesh_vertex_blending_descriptor_set))
            {
                throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
            }

            RHIDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
            mesh_vertex_Joint_binding_storage_buffer_info.offset = 0;
            mesh_vertex_Joint_binding_storage_buffer_info.range = 1;
            mesh_vertex_Joint_binding_storage_buffer_info.buffer =
                m_global_render_resource._storage_buffer._global_null_descriptor_storage_buffer;
            assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
                m_global_render_resource._storage_buffer._max_storage_buffer_range);

            RHIDescriptorSet* descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

            RHIWriteDescriptorSet descriptor_writes[1];

            RHIWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
                descriptor_writes[0];
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
                RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = NULL;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet = descriptor_set_to_write;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
                RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
                &mesh_vertex_Joint_binding_storage_buffer_info;

            rhi->updateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                                      descriptor_writes,
                                      0,
                                      NULL);
        }
    }

    void RenderResource::updateIndexBuffer(std::shared_ptr<RHI> rhi,
                                           uint32_t             index_buffer_size,
                                           void*                index_buffer_data,
                                           VulkanMesh&          now_mesh)
    {
        VulkanRHI* vulkan_context = static_cast<VulkanRHI*>(rhi.get());

        // temp staging buffer
        RHIDeviceSize buffer_size = index_buffer_size;

        RHIBuffer* inefficient_staging_buffer;
        RHIDeviceMemory* inefficient_staging_buffer_memory;
        rhi->createBuffer(buffer_size,
                          RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          inefficient_staging_buffer,
                          inefficient_staging_buffer_memory);

        void* staging_buffer_data;
        rhi->mapMemory(inefficient_staging_buffer_memory, 0, buffer_size, 0, &staging_buffer_data);
        memcpy(staging_buffer_data, index_buffer_data, (size_t)buffer_size);
        rhi->unmapMemory(inefficient_staging_buffer_memory);

        // use the vmaAllocator to allocate asset index buffer
        RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = buffer_size;
        bufferInfo.usage = RHI_BUFFER_USAGE_INDEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                             &bufferInfo,
                             &allocInfo,
                             now_mesh.mesh_index_buffer,
                             &now_mesh.mesh_index_buffer_allocation,
                             NULL);

        // use the data from staging buffer
        rhi->copyBuffer( inefficient_staging_buffer, now_mesh.mesh_index_buffer, 0, 0, buffer_size);

        // release temp staging buffer
        rhi->destroyBuffer(inefficient_staging_buffer);
        rhi->freeMemory(inefficient_staging_buffer_memory);
    }

    void RenderResource::updateTextureImageData(std::shared_ptr<RHI> rhi, const TextureDataToUpdate& texture_data)
    {
        rhi->createGlobalImage(
            texture_data.now_material->base_color_texture_image,
            texture_data.now_material->base_color_image_view,
            texture_data.now_material->base_color_image_allocation,
            texture_data.base_color_image_width,
            texture_data.base_color_image_height,
            texture_data.base_color_image_pixels,
            texture_data.base_color_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->metallic_roughness_texture_image,
            texture_data.now_material->metallic_roughness_image_view,
            texture_data.now_material->metallic_roughness_image_allocation,
            texture_data.metallic_roughness_image_width,
            texture_data.metallic_roughness_image_height,
            texture_data.metallic_roughness_image_pixels,
            texture_data.metallic_roughness_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->normal_texture_image,
            texture_data.now_material->normal_image_view,
            texture_data.now_material->normal_image_allocation,
            texture_data.normal_roughness_image_width,
            texture_data.normal_roughness_image_height,
            texture_data.normal_roughness_image_pixels,
            texture_data.normal_roughness_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->occlusion_texture_image,
            texture_data.now_material->occlusion_image_view,
            texture_data.now_material->occlusion_image_allocation,
            texture_data.occlusion_image_width,
            texture_data.occlusion_image_height,
            texture_data.occlusion_image_pixels,
            texture_data.occlusion_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->emissive_texture_image,
            texture_data.now_material->emissive_image_view,
            texture_data.now_material->emissive_image_allocation,
            texture_data.emissive_image_width,
            texture_data.emissive_image_height,
            texture_data.emissive_image_pixels,
            texture_data.emissive_image_format);
    }

    VulkanMesh& RenderResource::getEntityMesh(RenderEntity entity)
    {
        size_t assetid = entity.m_mesh_asset_id;

        auto it = m_vulkan_meshes.find(assetid);
        if (it != m_vulkan_meshes.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("failed to get entity mesh");
        }
    }

    VulkanPBRMaterial& RenderResource::getEntityMaterial(RenderEntity entity)
    {
        size_t assetid = entity.m_material_asset_id;

        auto it = m_vulkan_pbr_materials.find(assetid);
        if (it != m_vulkan_pbr_materials.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("failed to get entity material");
        }
    }

    void RenderResource::resetRingBufferOffset(uint8_t current_frame_index)
    {
        m_global_render_resource._storage_buffer._global_upload_ringbuffers_end[current_frame_index] =
            m_global_render_resource._storage_buffer._global_upload_ringbuffers_begin[current_frame_index];
    }

    void RenderResource::createAndMapStorageBuffer(std::shared_ptr<RHI> rhi)
    {
        VulkanRHI* raw_rhi = static_cast<VulkanRHI*>(rhi.get());
        StorageBuffer& _storage_buffer = m_global_render_resource._storage_buffer;
        uint32_t       frames_in_flight = raw_rhi->k_max_frames_in_flight;

        RHIPhysicalDeviceProperties properties;
        rhi->getPhysicalDeviceProperties(&properties);

        _storage_buffer._min_uniform_buffer_offset_alignment =
            static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
        _storage_buffer._min_storage_buffer_offset_alignment =
            static_cast<uint32_t>(properties.limits.minStorageBufferOffsetAlignment);
        _storage_buffer._max_storage_buffer_range = properties.limits.maxStorageBufferRange;
        _storage_buffer._non_coherent_atom_size = properties.limits.nonCoherentAtomSize;

        // In Vulkan, the storage buffer should be pre-allocated.
        // The size is 128MB in NVIDIA D3D11
        // driver(https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0).
        uint32_t global_storage_buffer_size = 1024 * 1024 * 128;
        rhi->createBuffer(global_storage_buffer_size,
                          RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          _storage_buffer._global_upload_ringbuffer,
                          _storage_buffer._global_upload_ringbuffer_memory);

        _storage_buffer._global_upload_ringbuffers_begin.resize(frames_in_flight);
        _storage_buffer._global_upload_ringbuffers_end.resize(frames_in_flight);
        _storage_buffer._global_upload_ringbuffers_size.resize(frames_in_flight);
        for (uint32_t i = 0; i < frames_in_flight; ++i)
        {
            _storage_buffer._global_upload_ringbuffers_begin[i] = (global_storage_buffer_size * i) / frames_in_flight;
            _storage_buffer._global_upload_ringbuffers_size[i] =
                (global_storage_buffer_size * (i + 1)) / frames_in_flight -
                (global_storage_buffer_size * i) / frames_in_flight;
        }

        // axis
        rhi->createBuffer(sizeof(AxisStorageBufferObject),
                          RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          _storage_buffer._axis_inefficient_storage_buffer,
                          _storage_buffer._axis_inefficient_storage_buffer_memory);

        // null descriptor
        rhi->createBuffer(64,
                          RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          0,
                          _storage_buffer._global_null_descriptor_storage_buffer,
                          _storage_buffer._global_null_descriptor_storage_buffer_memory);

        // TODO: Unmap when program terminates
        rhi->mapMemory(_storage_buffer._global_upload_ringbuffer_memory,
                       0,
                       RHI_WHOLE_SIZE,
                       0,
                       &_storage_buffer._global_upload_ringbuffer_memory_pointer);

        rhi->mapMemory(_storage_buffer._axis_inefficient_storage_buffer_memory,
                       0,
                       RHI_WHOLE_SIZE,
                       0,
                       &_storage_buffer._axis_inefficient_storage_buffer_memory_pointer);

        static_assert(64 >= sizeof(MeshVertex::VulkanMeshVertexJointBinding), "");
    }
}

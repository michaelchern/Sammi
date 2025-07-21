#include "runtime/function/render/render_system.h"

#include "runtime/core/base/macro.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_pass.h"
#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_scene.h"
#include "runtime/function/render/window_system.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"

#include "runtime/function/render/passes/main_camera_pass.h"
#include "runtime/function/render/passes/particle_pass.h"

#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

namespace Sammi
{
    RenderSystem::~RenderSystem()
    {
        clear();
    }

    void RenderSystem::initialize(RenderSystemInitInfo init_info)
    {
        // 获取全局资源管理器（配置管理器和资产加载器）
        std::shared_ptr<ConfigManager> config_manager = g_runtime_global_context.m_config_manager;
        ASSERT(config_manager);
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // -------------------- 步骤1：初始化RHI（渲染硬件接口） --------------------
        RHIInitInfo rhi_init_info;
        rhi_init_info.window_system = init_info.window_system;  // 设置窗口系统（用于创建交换链/窗口）
        m_rhi = std::make_shared<VulkanRHI>();                  // 创建Vulkan实现
        m_rhi->initialize(rhi_init_info);                       // 初始化Vulkan（创建实例/设备/队列等）

        // -------------------- 步骤2：加载全局渲染资源 --------------------
        // 加载全局资源到临时结构体
        GlobalRenderingRes global_rendering_res;
        // 从配置获取全局渲染资源路径（如IBL贴图、颜色分级表等）
        const std::string& global_rendering_res_url = config_manager->getGlobalRenderingResUrl();
        asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

        // 上传全局渲染资源到GPU（如IBL环境贴图、BRDF积分图、颜色分级LUT）
        LevelResourceDesc level_resource_desc;
        // 天空盒辐照度贴图（用于环境光照计算）
        level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
        // 天空盒预过滤贴图（用于环境反射计算）
        level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map = global_rendering_res.m_skybox_specular_map;
        // BRDF积分图（用于PBR材质计算）
        level_resource_desc.m_ibl_resource_desc.m_brdf_map = global_rendering_res.m_brdf_map;
        // 颜色分级贴图（用于后期处理颜色调整）
        level_resource_desc.m_color_grading_resource_desc.m_color_grading_map = global_rendering_res.m_color_grading_map;
        
        // 创建渲染资源管理器并上传全局资源到GPU
        m_render_resource = std::make_shared<RenderResource>();
        m_render_resource->uploadGlobalRenderResource(m_rhi, level_resource_desc);

        // -------------------- 步骤3：初始化渲染相机 --------------------
        const CameraPose& camera_pose = global_rendering_res.m_camera_config.m_pose;              // 获取相机位姿（位置、朝向、上向量）
        m_render_camera               = std::make_shared<RenderCamera>();                         // 创建渲染相机实例
        m_render_camera->lookAt(camera_pose.m_position, camera_pose.m_target, camera_pose.m_up);  // 设置相机朝向
        m_render_camera->m_zfar  = global_rendering_res.m_camera_config.m_z_far;                  // 设置相机远裁剪面
        m_render_camera->m_znear = global_rendering_res.m_camera_config.m_z_near;                 // 设置相机近裁剪面
        // 设置相机宽高比（根据配置的宽高比）
        m_render_camera->setAspect(global_rendering_res.m_camera_config.m_aspect.x /
                                   global_rendering_res.m_camera_config.m_aspect.y);

        // -------------------- 步骤4：初始化渲染场景 --------------------
        // 创建渲染场景实例
        m_render_scene = std::make_shared<RenderScene>();
        // 设置环境光颜色（从全局配置获取）
        m_render_scene->m_ambient_light = { global_rendering_res.m_ambient_light.toVector3() };
        // 设置平行光方向（归一化向量）
        m_render_scene->m_directional_light.m_direction = global_rendering_res.m_directional_light.m_direction.normalisedCopy();
        // 设置平行光颜色（从全局配置获取）
        m_render_scene->m_directional_light.m_color = global_rendering_res.m_directional_light.m_color.toVector3();
        // 初始化可见对象引用（用于后续可见性计算）
        m_render_scene->setVisibleNodesReference();

        // -------------------- 步骤5：初始化渲染管线 --------------------
        RenderPipelineInitInfo pipeline_init_info;
        pipeline_init_info.enable_fxaa     = global_rendering_res.m_enable_fxaa;  // 启用FXAA抗锯齿
        pipeline_init_info.render_resource = m_render_resource;                   // 关联渲染资源

        m_render_pipeline        = std::make_shared<RenderPipeline>();            // 创建渲染管线实例
        m_render_pipeline->m_rhi = m_rhi;                                         // 关联RHI实例
        m_render_pipeline->initialize(pipeline_init_info);                        // 初始化渲染管线（创建渲染通道、描述符集等）

        // 设置主相机通道的描述符集布局（用于后续资源绑定）
        // 网格（Mesh）的描述符集布局（用于绑定顶点/索引缓冲区、统一缓冲区等）
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_mesh_descriptor_set_layout     = &static_cast<RenderPass*>(m_render_pipeline->m_main_camera_pass.get())->m_descriptor_infos[MainCameraPass::LayoutType::_per_mesh].layout;
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_material_descriptor_set_layout =&static_cast<RenderPass*>(m_render_pipeline->m_main_camera_pass.get())->m_descriptor_infos[MainCameraPass::LayoutType::_mesh_per_material].layout;
    }

    void RenderSystem::tick(float delta_time)
    {
        // 处理逻辑与渲染上下文的交换数据（如加载新资源、删除旧对象）
        processSwapData();

        // 准备渲染命令上下文（开始命令缓冲区录制）
        m_rhi->prepareContext();

        // 更新每帧缓冲区（如相机的视图投影矩阵、光照参数等）
        m_render_resource->updatePerFrameBuffer(m_render_scene, m_render_camera);

        // 更新当前帧可见的对象（基于相机视锥体裁剪）
        m_render_scene->updateVisibleObjects(std::static_pointer_cast<RenderResource>(m_render_resource), m_render_camera);

        // 准备渲染管线的各通道数据（如设置渲染目标、绑定描述符集等）
        m_render_pipeline->preparePassData(m_render_resource);

        // 更新调试绘制管理器（绘制辅助线、坐标轴等）
        g_runtime_global_context.m_debugdraw_manager->tick(delta_time);

        // 根据当前渲染管线类型执行渲染（前向渲染或延迟渲染）
        if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE)
        {
            // 前向渲染流程
            m_render_pipeline->forwardRender(m_rhi, m_render_resource);
        }
        else if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
        {
            // 延迟渲染流程
            m_render_pipeline->deferredRender(m_rhi, m_render_resource);
        }
        else
        {
            // 不支持的管线类型报错
            LOG_ERROR(__FUNCTION__, "unsupported render pipeline type");
        }
    }

    void RenderSystem::clear()
    {
        if (m_rhi)
        {
            m_rhi->clear();
        }
        m_rhi.reset();

        if (m_render_scene)
        {
            m_render_scene->clear();
        }
        m_render_scene.reset();

        if (m_render_resource)
        {
            m_render_resource->clear();
        }
        m_render_resource.reset();
        
        if (m_render_pipeline)
        {
            m_render_pipeline->clear();
        }
        m_render_pipeline.reset();
    }

    void RenderSystem::swapLogicRenderData() { m_swap_context.swapLogicRenderData(); }

    RenderSwapContext& RenderSystem::getSwapContext() { return m_swap_context; }

    std::shared_ptr<RenderCamera> RenderSystem::getRenderCamera() const { return m_render_camera; }

    std::shared_ptr<RHI> RenderSystem::getRHI() const { return m_rhi; }

    void RenderSystem::updateEngineContentViewport(float offset_x, float offset_y, float width, float height)
    {
        // 更新Vulkan RHI的视口参数
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x        = offset_x;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y        = offset_y;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width    = width;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height   = height;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.minDepth = 0.0f;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.maxDepth = 1.0f;

        // 更新相机宽高比（影响投影矩阵）
        m_render_camera->setAspect(width / height);
    }

    // 获取当前引擎内容视口参数
    EngineContentViewport RenderSystem::getEngineContentViewport() const
    {
        float x      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x;
        float y      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y;
        float width  = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width;
        float height = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height;
        return { x, y, width, height };
    }

    // 获取被选中网格的GUID（全局唯一标识符）
    uint32_t RenderSystem::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        return m_render_pipeline->getGuidOfPickedMesh(picked_uv);
    }

    // 根据网格ID获取对应的游戏对象ID
    GObjectID RenderSystem::getGObjectIDByMeshID(uint32_t mesh_id) const
    {
        return m_render_scene->getGObjectIDByMeshID(mesh_id);
    }

    // 创建坐标轴辅助对象（X/Y/Z轴可视化）
    void RenderSystem::createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas)
    {
        for (int i = 0; i < axis_entities.size(); i++)
        {
            // 为每个轴实体上传渲染资源（网格、材质等）到GPU
            m_render_resource->uploadGameObjectRenderResource(m_rhi, axis_entities[i], mesh_datas[i]);
        }
    }

    // 设置坐标轴的可见状态（显示/隐藏）
    void RenderSystem::setVisibleAxis(std::optional<RenderEntity> axis)
    {
        // 更新场景中的坐标轴实体
        m_render_scene->m_render_axis = axis;

        // 通过渲染管线设置坐标轴的可见状态
        if (axis.has_value())
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(true);
        }
        else
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(false);
        }
    }

    // 设置当前选中的坐标轴（0:X, 1:Y, 2:Z）
    void RenderSystem::setSelectedAxis(size_t selected_axis)
    {
        std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setSelectedAxis(selected_axis);
    }

    // 获取游戏对象实例ID分配器（用于管理动态生成的实例ID）
    GuidAllocator<GameObjectPartId>& RenderSystem::getGOInstanceIdAllocator()
    {
        // 返回场景的实例ID分配器
        return m_render_scene->getInstanceIdAllocator();
    }

    // 获取网格资源ID分配器（用于管理网格资产的唯一标识）
    GuidAllocator<MeshSourceDesc>& RenderSystem::getMeshAssetIdAllocator()
    {
        return m_render_scene->getMeshAssetIdAllocator();
    }

    // 清理关卡重新加载所需的数据（移除当前关卡所有对象）
    void RenderSystem::clearForLevelReloading()
    {
        m_render_scene->clearForLevelReloading();
    }

    // 设置当前渲染管线类型（前向/延迟）
    void RenderSystem::setRenderPipelineType(RENDER_PIPELINE_TYPE pipeline_type)
    {
        m_render_pipeline_type = pipeline_type;
    }

    // 初始化UI渲染后端（关联UI窗口）
    void RenderSystem::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_render_pipeline->initializeUIRenderBackend(window_ui);
    }

    // 处理逻辑与渲染上下文的交换数据（关键数据同步函数）
    void RenderSystem::processSwapData()
    {
        RenderSwapData& swap_data = m_swap_context.getRenderSwapData();

        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // -------------------- 步骤1：更新全局资源（如关卡IBL贴图） --------------------
        if (swap_data.m_level_resource_desc.has_value())
        {
            // 上传新的全局资源到GPU
            m_render_resource->uploadGlobalRenderResource(m_rhi, *swap_data.m_level_resource_desc);

            // 重置交换数据状态（标记为已处理）
            m_swap_context.resetLevelRsourceSwapData();
        }

        // -------------------- 步骤2：处理游戏对象资源（加载/更新网格、材质） --------------------
        if (swap_data.m_game_object_resource_desc.has_value())
        {
            // 循环处理所有待处理的游戏对象
            while (!swap_data.m_game_object_resource_desc->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_resource_desc->getNextProcessObject();

                // 遍历对象的每个部件（如模型不同部分）
                for (size_t part_index = 0; part_index < gobject.getObjectParts().size(); part_index++)
                {
                    // 当前部件描述
                    const auto& game_object_part = gobject.getObjectParts()[part_index];
                    // 部件唯一ID（对象ID+部件索引）
                    GameObjectPartId part_id = { gobject.getId(), part_index };

                    // 检查该部件是否已在场景中存在（通过实例ID分配器判断）
                    bool is_entity_in_scene = m_render_scene->getInstanceIdAllocator().hasElement(part_id);

                    // 渲染实体（存储渲染所需数据）
                    RenderEntity render_entity;
                    // 分配实例ID（若不存在则新建，若存在则复用）
                    render_entity.m_instance_id = static_cast<uint32_t>(m_render_scene->getInstanceIdAllocator().allocGuid(part_id));
                    // 设置模型的世界变换矩阵（从部件描述获取）
                    render_entity.m_model_matrix = game_object_part.m_transform_desc.m_transform_matrix;

                    // 记录实例ID到游戏对象ID的映射（用于后续查找）
                    m_render_scene->addInstanceIdToMap(render_entity.m_instance_id, gobject.getId());

                    // -------------------- 处理网格资源 --------------------
                    // 网格资源描述（文件路径）
                    MeshSourceDesc mesh_source = { game_object_part.m_mesh_desc.m_mesh_file };
                    // 检查网格是否已加载（通过场景的网格资产分配器）
                    bool is_mesh_loaded = m_render_scene->getMeshAssetIdAllocator().hasElement(mesh_source);

                    // 网格渲染数据（顶点/索引缓冲区等）
                    RenderMeshData mesh_data;
                    if (!is_mesh_loaded)
                    {
                        // 加载新网格（从文件读取并上传到GPU）
                        mesh_data = m_render_resource->loadMeshData(mesh_source, render_entity.m_bounding_box);
                    }
                    else
                    {
                        // 已加载则获取缓存的包围盒（用于碰撞检测、视锥体裁剪）
                        render_entity.m_bounding_box = m_render_resource->getCachedBoudingBox(mesh_source);
                    }

                    // 分配网格资源ID（若未加载则新建，否则复用）
                    render_entity.m_mesh_asset_id = m_render_scene->getMeshAssetIdAllocator().allocGuid(mesh_source);

                    // 设置顶点混合属性（若部件有关联的骨骼动画）
                    render_entity.m_enable_vertex_blending = game_object_part.m_skeleton_animation_result.m_transforms.size() > 1;
                    // 存储骨骼动画的变换矩阵（每帧更新）
                    render_entity.m_joint_matrices.resize(game_object_part.m_skeleton_animation_result.m_transforms.size());
                    for (size_t i = 0; i < game_object_part.m_skeleton_animation_result.m_transforms.size(); ++i)
                    {
                        render_entity.m_joint_matrices[i] = game_object_part.m_skeleton_animation_result.m_transforms[i].m_matrix;
                    }

                    // -------------------- 处理材质资源 --------------------
                    // 材质资源描述（纹理文件路径）
                    MaterialSourceDesc material_source;
                    if (game_object_part.m_material_desc.m_with_texture)
                    {
                        // 自定义材质：使用配置的纹理路径
                        material_source = { game_object_part.m_material_desc.m_base_color_texture_file,
                                           game_object_part.m_material_desc.m_metallic_roughness_texture_file,
                                           game_object_part.m_material_desc.m_normal_texture_file,
                                           game_object_part.m_material_desc.m_occlusion_texture_file,
                                           game_object_part.m_material_desc.m_emissive_texture_file };
                    }
                    else
                    {
                        // 默认材质：使用引擎内置的默认纹理（铝箔、法线等）
                        material_source = {
                            asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                            "",// 无AO纹理
                            ""};// 无自发光纹理
                    }

                    // 检查材质是否已加载（通过场景的材质资产分配器）
                    bool is_material_loaded = m_render_scene->getMaterialAssetdAllocator().hasElement(material_source);

                    // 材质渲染数据（着色器参数、采样器等）
                    RenderMaterialData material_data;
                    if (!is_material_loaded)
                    {
                        // 加载新材质（编译着色器、上传纹理到GPU）
                        material_data = m_render_resource->loadMaterialData(material_source);
                    }

                    // 分配材质资源ID（若未加载则新建，否则复用）
                    render_entity.m_material_asset_id = m_render_scene->getMaterialAssetdAllocator().allocGuid(material_source);

                    // -------------------- 上传渲染实体到GPU --------------------
                    // 若网格未加载：上传网格数据到GPU（创建缓冲区）
                    if (!is_mesh_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, mesh_data);
                    }

                    // 若材质未加载：上传材质数据到GPU（创建描述符集）
                    if (!is_material_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, material_data);
                    }

                    // -------------------- 更新场景中的渲染实体列表 --------------------
                    // 若对象不在场景中：添加新实体到场景列表
                    if (!is_entity_in_scene)
                    {
                        m_render_scene->m_render_entities.push_back(render_entity);
                    }
                    else
                    {
                        // 若对象已存在：更新现有实体的数据（如变换矩阵、材质等）
                        for (auto& entity : m_render_scene->m_render_entities)
                        {
                            if (entity.m_instance_id == render_entity.m_instance_id)
                            {
                                entity = render_entity;
                                break;
                            }
                        }
                    }
                }
                // 处理完当前游戏对象的所有部件后，从交换数据中移除该对象
                swap_data.m_game_object_resource_desc->pop();
            }

            // 重置游戏对象交换数据状态（标记为无待处理对象）
            m_swap_context.resetGameObjectResourceSwapData();
        }

        // -------------------- 步骤3：删除不再需要的游戏对象 --------------------
        if (swap_data.m_game_object_to_delete.has_value())
        {
            // 循环处理所有待删除的对象
            while (!swap_data.m_game_object_to_delete->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_to_delete->getNextProcessObject();
                // 从场景中删除该对象（移除所有相关实体）
                m_render_scene->deleteEntityByGObjectID(gobject.getId());
                swap_data.m_game_object_to_delete->pop();
            }
            // 重置删除交换数据状态
            m_swap_context.resetGameObjectToDelete();
        }

        // -------------------- 步骤4：处理相机数据交换（更新相机参数） --------------------
        if (swap_data.m_camera_swap_data.has_value())
        {
            // 更新相机的FOV（水平视角）
            if (swap_data.m_camera_swap_data->m_fov_x.has_value())
            {
                m_render_camera->setFOVx(*swap_data.m_camera_swap_data->m_fov_x);
            }

            // 更新相机的视图矩阵（世界->视图变换）
            if (swap_data.m_camera_swap_data->m_view_matrix.has_value())
            {
                m_render_camera->setMainViewMatrix(*swap_data.m_camera_swap_data->m_view_matrix);
            }

            // 更新相机类型（如透视投影、正交投影）
            if (swap_data.m_camera_swap_data->m_camera_type.has_value())
            {
                m_render_camera->setCurrentCameraType(*swap_data.m_camera_swap_data->m_camera_type);
            }

            // 处理完相机数据后，重置交换数据状态
            m_swap_context.resetCameraSwapData();
        }

        // -------------------- 步骤5：处理粒子发射请求 --------------------
        if (swap_data.m_particle_submit_request.has_value())
        {
            // 获取粒子通道（负责粒子系统渲染的子系统）
            std::shared_ptr<ParticlePass> particle_pass = std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass);

            // 获取发射器数量
            int emitter_count = swap_data.m_particle_submit_request->getEmitterCount();
            // 设置发射器数量
            particle_pass->setEmitterCount(emitter_count);

            // 遍历所有发射器，创建并初始化
            for (int index = 0; index < emitter_count; ++index)
            {
                const ParticleEmitterDesc& desc = swap_data.m_particle_submit_request->getEmitterDesc(index);
                particle_pass->createEmitter(index, desc);
            }

            particle_pass->initializeEmitters();

            m_swap_context.resetPartilceBatchSwapData();
        }

        // -------------------- 步骤6：处理粒子发射器更新请求 --------------------
        if (swap_data.m_emitter_tick_request.has_value())
        {
            // 设置需要更新的发射器索引（每帧更新指定发射器的状态）
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTickIndices(swap_data.m_emitter_tick_request->m_emitter_indices);
            m_swap_context.resetEmitterTickSwapData();
        }

        // -------------------- 步骤7：处理粒子发射器变换请求 --------------------
        if (swap_data.m_emitter_transform_request.has_value())
        {
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTransformIndices(swap_data.m_emitter_transform_request->m_transform_descs);
            m_swap_context.resetEmitterTransformSwapData();
        }
    }
}

#include "runtime/function/render/render_system.h"

// 包含系统所需的头文件
#include "runtime/core/base/macro.h"                         // 基础宏定义
#include "runtime/resource/asset_manager/asset_manager.h"    // 资源管理器
#include "runtime/resource/config_manager/config_manager.h"  // 配置管理器
#include "runtime/function/render/render_camera.h"           // 渲染摄像机
// ... 其他渲染相关组件的头文件 ...
#include "runtime/function/render/render_pass.h"
#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_scene.h"
#include "runtime/function/render/window_system.h"

#include "runtime/function/global/global_context.h"                // 全局上下文
#include "runtime/function/render/debugdraw/debug_draw_manager.h"  // 调试绘制

// ... 渲染过程相关的头文件 ...
#include "runtime/function/render/passes/main_camera_pass.h"
#include "runtime/function/render/passes/particle_pass.h"

// Vulkan具体实现
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

namespace Sammi
{
    RenderSystem::~RenderSystem()
    {
        clear();
    }

    // 初始化渲染系统
    void RenderSystem::initialize(RenderSystemInitInfo init_info)
    {
        // 获取全局管理器
        std::shared_ptr<ConfigManager> config_manager = g_runtime_global_context.m_config_manager;
        ASSERT(config_manager);  // 运行时检查
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // 初始化渲染硬件接口 (Vulkan)
        RHIInitInfo rhi_init_info;
        rhi_init_info.window_system = init_info.window_system;  // 传递窗口系统

        m_rhi = std::make_shared<VulkanRHI>();  // 使用Vulkan实现
        m_rhi->initialize(rhi_init_info);

        // 加载全局渲染资源配置
        GlobalRenderingRes global_rendering_res;
        const std::string& global_rendering_res_url = config_manager->getGlobalRenderingResUrl();
        asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

        // 设置基于图像光照(IBL)和颜色分级资源
        LevelResourceDesc level_resource_desc;
        level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
        level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map   = global_rendering_res.m_skybox_specular_map;
        level_resource_desc.m_ibl_resource_desc.m_brdf_map              = global_rendering_res.m_brdf_map;
        level_resource_desc.m_color_grading_resource_desc.m_color_grading_map =
            global_rendering_res.m_color_grading_map;

        // 创建并上传全局渲染资源
        m_render_resource = std::make_shared<RenderResource>();
        m_render_resource->uploadGlobalRenderResource(m_rhi, level_resource_desc);

        // 设置主渲染相机
        const CameraPose& camera_pose = global_rendering_res.m_camera_config.m_pose;
        m_render_camera               = std::make_shared<RenderCamera>();
        m_render_camera->lookAt(camera_pose.m_position, camera_pose.m_target, camera_pose.m_up);  // 相机位置和朝向
        m_render_camera->m_zfar  = global_rendering_res.m_camera_config.m_z_far;  // 远平面
        m_render_camera->m_znear = global_rendering_res.m_camera_config.m_z_near;  // 近平面
        m_render_camera->setAspect(global_rendering_res.m_camera_config.m_aspect.x /
                                   global_rendering_res.m_camera_config.m_aspect.y);  // 宽高比

        // 初始化渲染场景
        m_render_scene                  = std::make_shared<RenderScene>();
        m_render_scene->m_ambient_light = {global_rendering_res.m_ambient_light.toVector3()};  // 环境光
        m_render_scene->m_directional_light.m_direction =  // 方向光
            global_rendering_res.m_directional_light.m_direction.normalisedCopy();
        m_render_scene->m_directional_light.m_color = global_rendering_res.m_directional_light.m_color.toVector3();
        m_render_scene->setVisibleNodesReference();  // 设置可见节点引用

        // 创建渲染管线
        RenderPipelineInitInfo pipeline_init_info;
        pipeline_init_info.enable_fxaa     = global_rendering_res.m_enable_fxaa;  // FXAA抗锯齿开关
        pipeline_init_info.render_resource = m_render_resource;  // 关联资源管理器

        m_render_pipeline        = std::make_shared<RenderPipeline>();
        m_render_pipeline->m_rhi = m_rhi;  // 关联渲染硬件
        m_render_pipeline->initialize(pipeline_init_info);

        // 设置主相机通道中的描述符集布局
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_mesh_descriptor_set_layout =
            &static_cast<RenderPass*>(m_render_pipeline->m_main_camera_pass.get())
                 ->m_descriptor_infos[MainCameraPass::LayoutType::_per_mesh]
                 .layout;
        // 设置材质描述符集布局
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_material_descriptor_set_layout =
            &static_cast<RenderPass*>(m_render_pipeline->m_main_camera_pass.get())
                 ->m_descriptor_infos[MainCameraPass::LayoutType::_mesh_per_material]
                 .layout;
    }

    // 每帧更新函数
    void RenderSystem::tick(float delta_time)
    {
        // 步骤1: 处理逻辑线程和渲染线程交换的数据
        processSwapData();

        // 步骤2: 准备渲染命令上下文
        m_rhi->prepareContext();

        // 步骤3: 更新每帧常量缓冲区数据
        m_render_resource->updatePerFrameBuffer(m_render_scene, m_render_camera);

        // 步骤4: 更新可见物体列表（视锥体裁剪）
        m_render_scene->updateVisibleObjects(std::static_pointer_cast<RenderResource>(m_render_resource),
                                             m_render_camera);

        // 步骤5: 准备渲染通道所需数据
        m_render_pipeline->preparePassData(m_render_resource);

        // 步骤6: 更新调试绘制系统
        g_runtime_global_context.m_debugdraw_manager->tick(delta_time);

        // 步骤7: 根据管线类型选择渲染方式
        if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE)
        {
            m_render_pipeline->forwardRender(m_rhi, m_render_resource);
        }
        else if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
        {
            m_render_pipeline->deferredRender(m_rhi, m_render_resource);
        }
        else
        {
            LOG_ERROR(__FUNCTION__, "unsupported render pipeline type");
        }
    }

    // 清理渲染系统资源
    void RenderSystem::clear()
    {
        // 使用逆向初始化顺序释放资源
        if (m_rhi)
        {
            m_rhi->clear();              // 清理渲染硬件资源
        }
        m_rhi.reset();                   // 释放智能指针

        if (m_render_scene)
        {
            m_render_scene->clear();     // 清理场景数据
        }
        m_render_scene.reset();

        if (m_render_resource)
        {
            m_render_resource->clear();  // 清理渲染资源
        }
        m_render_resource.reset();
        
        if (m_render_pipeline)
        {
            m_render_pipeline->clear();  // 清理渲染管线
        }
        m_render_pipeline.reset();
    }

    // 交换逻辑线程和渲染线程的数据缓冲区
    void RenderSystem::swapLogicRenderData()
    {
        m_swap_context.swapLogicRenderData();
    }

    // 获取数据交换上下文引用
    RenderSwapContext& RenderSystem::getSwapContext()
    {
        return m_swap_context;
    }

    // 获取渲染相机
    std::shared_ptr<RenderCamera> RenderSystem::getRenderCamera() const
    {
        return m_render_camera;
    }

    // 获取渲染硬件接口
    std::shared_ptr<RHI> RenderSystem::getRHI() const
    {
        return m_rhi;
    }

    // 更新引擎内容视口（用于编辑器视窗调整）
    void RenderSystem::updateEngineContentViewport(float offset_x, float offset_y, float width, float height)
    {
        // 更新Vulkan视口参数
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x        = offset_x;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y        = offset_y;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width    = width;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height   = height;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.minDepth = 0.0f;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.maxDepth = 1.0f;

        // 更新相机宽高比
        m_render_camera->setAspect(width / height);
    }

    // 获取当前引擎视口设置
    EngineContentViewport RenderSystem::getEngineContentViewport() const
    {
        float x      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x;
        float y      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y;
        float width  = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width;
        float height = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height;
        return {x, y, width, height};
    }

    // 通过UV坐标获取拾取到的网格GUID
    uint32_t RenderSystem::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        return m_render_pipeline->getGuidOfPickedMesh(picked_uv);
    }

    // 通过网格ID查找对应的游戏对象ID
    GObjectID RenderSystem::getGObjectIDByMeshID(uint32_t mesh_id) const
    {
        return m_render_scene->getGObjectIDByMeshID(mesh_id);
    }

    // 创建坐标轴渲染对象（编辑器用）
    void RenderSystem::createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas)
    {
        for (int i = 0; i < axis_entities.size(); i++)
        {
            m_render_resource->uploadGameObjectRenderResource(m_rhi, axis_entities[i], mesh_datas[i]);
        }
    }

    // 设置坐标轴可见性
    void RenderSystem::setVisibleAxis(std::optional<RenderEntity> axis)
    {
        m_render_scene->m_render_axis = axis;  // 场景中保存坐标轴状态

        // 更新管线中的显示状态
        if (axis.has_value())
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(true);
        }
        else
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(false);
        }
    }

    // 设置选中的坐标轴（X/Y/Z）
    void RenderSystem::setSelectedAxis(size_t selected_axis)
    {
        std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setSelectedAxis(selected_axis);
    }

    // 获取游戏对象部件ID分配器
    GuidAllocator<GameObjectPartId>& RenderSystem::getGOInstanceIdAllocator()
    {
        return m_render_scene->getInstanceIdAllocator();
    }

    // 获取网格资产ID分配器
    GuidAllocator<MeshSourceDesc>& RenderSystem::getMeshAssetIdAllocator()
    {
        return m_render_scene->getMeshAssetIdAllocator();
    }

    // 为关卡重载准备的清理
    void RenderSystem::clearForLevelReloading()
    {
        m_render_scene->clearForLevelReloading();
    }

    // 设置当前渲染管线类型
    void RenderSystem::setRenderPipelineType(RENDER_PIPELINE_TYPE pipeline_type)
    {
        m_render_pipeline_type = pipeline_type;
    }

    // 初始化UI渲染后端
    void RenderSystem::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_render_pipeline->initializeUIRenderBackend(window_ui);
    }

    // 核心：处理从逻辑线程交换过来的数据
    void RenderSystem::processSwapData()
    {
        RenderSwapData& swap_data = m_swap_context.getRenderSwapData();
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        /*********************** 处理全局资源更新 ***********************/
        if (swap_data.m_level_resource_desc.has_value())
        {
            // 上传新关卡的全局资源（如天空盒）
            m_render_resource->uploadGlobalRenderResource(m_rhi, *swap_data.m_level_resource_desc);
            m_swap_context.resetLevelRsourceSwapData();  // 重置交换数据
        }

        /*********************** 处理游戏对象更新 ***********************/
        if (swap_data.m_game_object_resource_desc.has_value())
        {
            while (!swap_data.m_game_object_resource_desc->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_resource_desc->getNextProcessObject();

                // 处理对象的每个部件
                for (size_t part_index = 0; part_index < gobject.getObjectParts().size(); part_index++)
                {
                    const auto&      game_object_part = gobject.getObjectParts()[part_index];
                    GameObjectPartId part_id          = {gobject.getId(), part_index};

                    // 检查对象是否已在场景中
                    bool is_entity_in_scene = m_render_scene->getInstanceIdAllocator().hasElement(part_id);

                    // 准备渲染实体数据
                    RenderEntity render_entity;
                    render_entity.m_instance_id =
                        static_cast<uint32_t>(m_render_scene->getInstanceIdAllocator().allocGuid(part_id));
                    render_entity.m_model_matrix = game_object_part.m_transform_desc.m_transform_matrix;

                    // 维护实例ID到游戏对象ID的映射
                    m_render_scene->addInstanceIdToMap(render_entity.m_instance_id, gobject.getId());

                    /** 处理网格资源 **/
                    MeshSourceDesc mesh_source    = {game_object_part.m_mesh_desc.m_mesh_file};
                    bool           is_mesh_loaded = m_render_scene->getMeshAssetIdAllocator().hasElement(mesh_source);

                    RenderMeshData mesh_data;
                    if (!is_mesh_loaded)
                    {
                        // 首次加载网格：计算包围盒
                        mesh_data = m_render_resource->loadMeshData(mesh_source, render_entity.m_bounding_box);
                    }
                    else
                    {
                        // 使用缓存的包围盒
                        render_entity.m_bounding_box = m_render_resource->getCachedBoudingBox(mesh_source);
                    }

                    // 分配网格资产ID
                    render_entity.m_mesh_asset_id = m_render_scene->getMeshAssetIdAllocator().allocGuid(mesh_source);

                    /** 处理骨骼动画 **/
                    render_entity.m_enable_vertex_blending =
                        game_object_part.m_skeleton_animation_result.m_transforms.size() > 1; // take care
                    render_entity.m_joint_matrices.resize(
                        game_object_part.m_skeleton_animation_result.m_transforms.size());
                    for (size_t i = 0; i < game_object_part.m_skeleton_animation_result.m_transforms.size(); ++i)
                    {
                        render_entity.m_joint_matrices[i] =
                            game_object_part.m_skeleton_animation_result.m_transforms[i].m_matrix;
                    }

                    /** 处理材质资源 **/
                    MaterialSourceDesc material_source;
                    if (game_object_part.m_material_desc.m_with_texture)
                    {
                        // 使用对象指定的材质纹理
                        material_source = {game_object_part.m_material_desc.m_base_color_texture_file,
                                           game_object_part.m_material_desc.m_metallic_roughness_texture_file,
                                           game_object_part.m_material_desc.m_normal_texture_file,
                                           game_object_part.m_material_desc.m_occlusion_texture_file,
                                           game_object_part.m_material_desc.m_emissive_texture_file};
                    }
                    else
                    {
                        // 使用默认材质纹理
                        material_source = {
                            asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                            "",
                            ""};
                    }

                    bool is_material_loaded = m_render_scene->getMaterialAssetdAllocator().hasElement(material_source);
                    RenderMaterialData material_data;
                    if (!is_material_loaded)
                    {
                        material_data = m_render_resource->loadMaterialData(material_source);
                    }

                    render_entity.m_material_asset_id =
                        m_render_scene->getMaterialAssetdAllocator().allocGuid(material_source);

                    /** 上传资源到GPU **/
                    if (!is_mesh_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, mesh_data);
                    }

                    if (!is_material_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, material_data);
                    }

                    /** 添加到渲染场景 **/
                    if (!is_entity_in_scene)
                    {
                        m_render_scene->m_render_entities.push_back(render_entity);
                    }
                    else
                    {
                        // 更新现有对象
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

                // 处理完当前对象后弹出队列
                swap_data.m_game_object_resource_desc->pop();
            }

            // 重置交换数据
            m_swap_context.resetGameObjectResourceSwapData();
        }

        /*********************** 处理对象删除 ***********************/
        if (swap_data.m_game_object_to_delete.has_value())
        {
            while (!swap_data.m_game_object_to_delete->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_to_delete->getNextProcessObject();
                m_render_scene->deleteEntityByGObjectID(gobject.getId());  // 从场景删除
                swap_data.m_game_object_to_delete->pop();
            }

            m_swap_context.resetGameObjectToDelete();  // 重置交换数据
        }

        /*********************** 处理相机更新 ***********************/
        if (swap_data.m_camera_swap_data.has_value())
        {
            if (swap_data.m_camera_swap_data->m_fov_x.has_value())
            {
                m_render_camera->setFOVx(*swap_data.m_camera_swap_data->m_fov_x);
            }

            if (swap_data.m_camera_swap_data->m_view_matrix.has_value())
            {
                m_render_camera->setMainViewMatrix(*swap_data.m_camera_swap_data->m_view_matrix);
            }

            if (swap_data.m_camera_swap_data->m_camera_type.has_value())
            {
                m_render_camera->setCurrentCameraType(*swap_data.m_camera_swap_data->m_camera_type);
            }

            m_swap_context.resetCameraSwapData();  // 重置交换数据
        }

        /*********************** 处理粒子系统 ***********************/
        if (swap_data.m_particle_submit_request.has_value())
        {
            std::shared_ptr<ParticlePass> particle_pass =
                std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass);

            // 创建粒子发射器
            int emitter_count = swap_data.m_particle_submit_request->getEmitterCount();
            particle_pass->setEmitterCount(emitter_count);

            for (int index = 0; index < emitter_count; ++index)
            {
                const ParticleEmitterDesc& desc = swap_data.m_particle_submit_request->getEmitterDesc(index);
                particle_pass->createEmitter(index, desc);
            }

            particle_pass->initializeEmitters();

            m_swap_context.resetPartilceBatchSwapData();  // 重置交换数据
        }

        // 粒子系统每帧更新请求
        if (swap_data.m_emitter_tick_request.has_value())
        {
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTickIndices(swap_data.m_emitter_tick_request->m_emitter_indices);
            m_swap_context.resetEmitterTickSwapData();
        }

        // 粒子发射器变换更新
        if (swap_data.m_emitter_transform_request.has_value())
        {
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTransformIndices(swap_data.m_emitter_transform_request->m_transform_descs);
            m_swap_context.resetEmitterTransformSwapData();
        }
    }
}
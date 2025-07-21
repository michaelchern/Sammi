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
        // ��ȡȫ����Դ�����������ù��������ʲ���������
        std::shared_ptr<ConfigManager> config_manager = g_runtime_global_context.m_config_manager;
        ASSERT(config_manager);
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // -------------------- ����1����ʼ��RHI����ȾӲ���ӿڣ� --------------------
        RHIInitInfo rhi_init_info;
        rhi_init_info.window_system = init_info.window_system;  // ���ô���ϵͳ�����ڴ���������/���ڣ�
        m_rhi = std::make_shared<VulkanRHI>();                  // ����Vulkanʵ��
        m_rhi->initialize(rhi_init_info);                       // ��ʼ��Vulkan������ʵ��/�豸/���еȣ�

        // -------------------- ����2������ȫ����Ⱦ��Դ --------------------
        // ����ȫ����Դ����ʱ�ṹ��
        GlobalRenderingRes global_rendering_res;
        // �����û�ȡȫ����Ⱦ��Դ·������IBL��ͼ����ɫ�ּ���ȣ�
        const std::string& global_rendering_res_url = config_manager->getGlobalRenderingResUrl();
        asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

        // �ϴ�ȫ����Ⱦ��Դ��GPU����IBL������ͼ��BRDF����ͼ����ɫ�ּ�LUT��
        LevelResourceDesc level_resource_desc;
        // ��պз��ն���ͼ�����ڻ������ռ��㣩
        level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
        // ��պ�Ԥ������ͼ�����ڻ���������㣩
        level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map = global_rendering_res.m_skybox_specular_map;
        // BRDF����ͼ������PBR���ʼ��㣩
        level_resource_desc.m_ibl_resource_desc.m_brdf_map = global_rendering_res.m_brdf_map;
        // ��ɫ�ּ���ͼ�����ں��ڴ�����ɫ������
        level_resource_desc.m_color_grading_resource_desc.m_color_grading_map = global_rendering_res.m_color_grading_map;
        
        // ������Ⱦ��Դ���������ϴ�ȫ����Դ��GPU
        m_render_resource = std::make_shared<RenderResource>();
        m_render_resource->uploadGlobalRenderResource(m_rhi, level_resource_desc);

        // -------------------- ����3����ʼ����Ⱦ��� --------------------
        const CameraPose& camera_pose = global_rendering_res.m_camera_config.m_pose;              // ��ȡ���λ�ˣ�λ�á�������������
        m_render_camera               = std::make_shared<RenderCamera>();                         // ������Ⱦ���ʵ��
        m_render_camera->lookAt(camera_pose.m_position, camera_pose.m_target, camera_pose.m_up);  // �����������
        m_render_camera->m_zfar  = global_rendering_res.m_camera_config.m_z_far;                  // �������Զ�ü���
        m_render_camera->m_znear = global_rendering_res.m_camera_config.m_z_near;                 // ����������ü���
        // ���������߱ȣ��������õĿ�߱ȣ�
        m_render_camera->setAspect(global_rendering_res.m_camera_config.m_aspect.x /
                                   global_rendering_res.m_camera_config.m_aspect.y);

        // -------------------- ����4����ʼ����Ⱦ���� --------------------
        // ������Ⱦ����ʵ��
        m_render_scene = std::make_shared<RenderScene>();
        // ���û�������ɫ����ȫ�����û�ȡ��
        m_render_scene->m_ambient_light = { global_rendering_res.m_ambient_light.toVector3() };
        // ����ƽ�йⷽ�򣨹�һ��������
        m_render_scene->m_directional_light.m_direction = global_rendering_res.m_directional_light.m_direction.normalisedCopy();
        // ����ƽ�й���ɫ����ȫ�����û�ȡ��
        m_render_scene->m_directional_light.m_color = global_rendering_res.m_directional_light.m_color.toVector3();
        // ��ʼ���ɼ��������ã����ں����ɼ��Լ��㣩
        m_render_scene->setVisibleNodesReference();

        // -------------------- ����5����ʼ����Ⱦ���� --------------------
        RenderPipelineInitInfo pipeline_init_info;
        pipeline_init_info.enable_fxaa     = global_rendering_res.m_enable_fxaa;  // ����FXAA�����
        pipeline_init_info.render_resource = m_render_resource;                   // ������Ⱦ��Դ

        m_render_pipeline        = std::make_shared<RenderPipeline>();            // ������Ⱦ����ʵ��
        m_render_pipeline->m_rhi = m_rhi;                                         // ����RHIʵ��
        m_render_pipeline->initialize(pipeline_init_info);                        // ��ʼ����Ⱦ���ߣ�������Ⱦͨ�������������ȣ�

        // ���������ͨ���������������֣����ں�����Դ�󶨣�
        // ����Mesh���������������֣����ڰ󶨶���/������������ͳһ�������ȣ�
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_mesh_descriptor_set_layout     = &static_cast<RenderPass*>(m_render_pipeline->m_main_camera_pass.get())->m_descriptor_infos[MainCameraPass::LayoutType::_per_mesh].layout;
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_material_descriptor_set_layout =&static_cast<RenderPass*>(m_render_pipeline->m_main_camera_pass.get())->m_descriptor_infos[MainCameraPass::LayoutType::_mesh_per_material].layout;
    }

    void RenderSystem::tick(float delta_time)
    {
        // �����߼�����Ⱦ�����ĵĽ������ݣ����������Դ��ɾ���ɶ���
        processSwapData();

        // ׼����Ⱦ���������ģ���ʼ�������¼�ƣ�
        m_rhi->prepareContext();

        // ����ÿ֡�����������������ͼͶӰ���󡢹��ղ����ȣ�
        m_render_resource->updatePerFrameBuffer(m_render_scene, m_render_camera);

        // ���µ�ǰ֡�ɼ��Ķ��󣨻��������׶��ü���
        m_render_scene->updateVisibleObjects(std::static_pointer_cast<RenderResource>(m_render_resource), m_render_camera);

        // ׼����Ⱦ���ߵĸ�ͨ�����ݣ���������ȾĿ�ꡢ�����������ȣ�
        m_render_pipeline->preparePassData(m_render_resource);

        // ���µ��Ի��ƹ����������Ƹ����ߡ�������ȣ�
        g_runtime_global_context.m_debugdraw_manager->tick(delta_time);

        // ���ݵ�ǰ��Ⱦ��������ִ����Ⱦ��ǰ����Ⱦ���ӳ���Ⱦ��
        if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE)
        {
            // ǰ����Ⱦ����
            m_render_pipeline->forwardRender(m_rhi, m_render_resource);
        }
        else if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
        {
            // �ӳ���Ⱦ����
            m_render_pipeline->deferredRender(m_rhi, m_render_resource);
        }
        else
        {
            // ��֧�ֵĹ������ͱ���
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
        // ����Vulkan RHI���ӿڲ���
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x        = offset_x;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y        = offset_y;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width    = width;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height   = height;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.minDepth = 0.0f;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.maxDepth = 1.0f;

        // ���������߱ȣ�Ӱ��ͶӰ����
        m_render_camera->setAspect(width / height);
    }

    // ��ȡ��ǰ���������ӿڲ���
    EngineContentViewport RenderSystem::getEngineContentViewport() const
    {
        float x      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x;
        float y      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y;
        float width  = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width;
        float height = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height;
        return { x, y, width, height };
    }

    // ��ȡ��ѡ�������GUID��ȫ��Ψһ��ʶ����
    uint32_t RenderSystem::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        return m_render_pipeline->getGuidOfPickedMesh(picked_uv);
    }

    // ��������ID��ȡ��Ӧ����Ϸ����ID
    GObjectID RenderSystem::getGObjectIDByMeshID(uint32_t mesh_id) const
    {
        return m_render_scene->getGObjectIDByMeshID(mesh_id);
    }

    // ���������Ḩ������X/Y/Z����ӻ���
    void RenderSystem::createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas)
    {
        for (int i = 0; i < axis_entities.size(); i++)
        {
            // Ϊÿ����ʵ���ϴ���Ⱦ��Դ�����񡢲��ʵȣ���GPU
            m_render_resource->uploadGameObjectRenderResource(m_rhi, axis_entities[i], mesh_datas[i]);
        }
    }

    // ����������Ŀɼ�״̬����ʾ/���أ�
    void RenderSystem::setVisibleAxis(std::optional<RenderEntity> axis)
    {
        // ���³����е�������ʵ��
        m_render_scene->m_render_axis = axis;

        // ͨ����Ⱦ��������������Ŀɼ�״̬
        if (axis.has_value())
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(true);
        }
        else
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(false);
        }
    }

    // ���õ�ǰѡ�е������ᣨ0:X, 1:Y, 2:Z��
    void RenderSystem::setSelectedAxis(size_t selected_axis)
    {
        std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setSelectedAxis(selected_axis);
    }

    // ��ȡ��Ϸ����ʵ��ID�����������ڹ���̬���ɵ�ʵ��ID��
    GuidAllocator<GameObjectPartId>& RenderSystem::getGOInstanceIdAllocator()
    {
        // ���س�����ʵ��ID������
        return m_render_scene->getInstanceIdAllocator();
    }

    // ��ȡ������ԴID�����������ڹ��������ʲ���Ψһ��ʶ��
    GuidAllocator<MeshSourceDesc>& RenderSystem::getMeshAssetIdAllocator()
    {
        return m_render_scene->getMeshAssetIdAllocator();
    }

    // ����ؿ����¼�����������ݣ��Ƴ���ǰ�ؿ����ж���
    void RenderSystem::clearForLevelReloading()
    {
        m_render_scene->clearForLevelReloading();
    }

    // ���õ�ǰ��Ⱦ�������ͣ�ǰ��/�ӳ٣�
    void RenderSystem::setRenderPipelineType(RENDER_PIPELINE_TYPE pipeline_type)
    {
        m_render_pipeline_type = pipeline_type;
    }

    // ��ʼ��UI��Ⱦ��ˣ�����UI���ڣ�
    void RenderSystem::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_render_pipeline->initializeUIRenderBackend(window_ui);
    }

    // �����߼�����Ⱦ�����ĵĽ������ݣ��ؼ�����ͬ��������
    void RenderSystem::processSwapData()
    {
        RenderSwapData& swap_data = m_swap_context.getRenderSwapData();

        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // -------------------- ����1������ȫ����Դ����ؿ�IBL��ͼ�� --------------------
        if (swap_data.m_level_resource_desc.has_value())
        {
            // �ϴ��µ�ȫ����Դ��GPU
            m_render_resource->uploadGlobalRenderResource(m_rhi, *swap_data.m_level_resource_desc);

            // ���ý�������״̬�����Ϊ�Ѵ���
            m_swap_context.resetLevelRsourceSwapData();
        }

        // -------------------- ����2��������Ϸ������Դ������/�������񡢲��ʣ� --------------------
        if (swap_data.m_game_object_resource_desc.has_value())
        {
            // ѭ���������д��������Ϸ����
            while (!swap_data.m_game_object_resource_desc->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_resource_desc->getNextProcessObject();

                // ���������ÿ����������ģ�Ͳ�ͬ���֣�
                for (size_t part_index = 0; part_index < gobject.getObjectParts().size(); part_index++)
                {
                    // ��ǰ��������
                    const auto& game_object_part = gobject.getObjectParts()[part_index];
                    // ����ΨһID������ID+����������
                    GameObjectPartId part_id = { gobject.getId(), part_index };

                    // ���ò����Ƿ����ڳ����д��ڣ�ͨ��ʵ��ID�������жϣ�
                    bool is_entity_in_scene = m_render_scene->getInstanceIdAllocator().hasElement(part_id);

                    // ��Ⱦʵ�壨�洢��Ⱦ�������ݣ�
                    RenderEntity render_entity;
                    // ����ʵ��ID�������������½������������ã�
                    render_entity.m_instance_id = static_cast<uint32_t>(m_render_scene->getInstanceIdAllocator().allocGuid(part_id));
                    // ����ģ�͵�����任���󣨴Ӳ���������ȡ��
                    render_entity.m_model_matrix = game_object_part.m_transform_desc.m_transform_matrix;

                    // ��¼ʵ��ID����Ϸ����ID��ӳ�䣨���ں������ң�
                    m_render_scene->addInstanceIdToMap(render_entity.m_instance_id, gobject.getId());

                    // -------------------- ����������Դ --------------------
                    // ������Դ�������ļ�·����
                    MeshSourceDesc mesh_source = { game_object_part.m_mesh_desc.m_mesh_file };
                    // ��������Ƿ��Ѽ��أ�ͨ�������������ʲ���������
                    bool is_mesh_loaded = m_render_scene->getMeshAssetIdAllocator().hasElement(mesh_source);

                    // ������Ⱦ���ݣ�����/�����������ȣ�
                    RenderMeshData mesh_data;
                    if (!is_mesh_loaded)
                    {
                        // ���������񣨴��ļ���ȡ���ϴ���GPU��
                        mesh_data = m_render_resource->loadMeshData(mesh_source, render_entity.m_bounding_box);
                    }
                    else
                    {
                        // �Ѽ������ȡ����İ�Χ�У�������ײ��⡢��׶��ü���
                        render_entity.m_bounding_box = m_render_resource->getCachedBoudingBox(mesh_source);
                    }

                    // ����������ԴID����δ�������½��������ã�
                    render_entity.m_mesh_asset_id = m_render_scene->getMeshAssetIdAllocator().allocGuid(mesh_source);

                    // ���ö��������ԣ��������й����Ĺ���������
                    render_entity.m_enable_vertex_blending = game_object_part.m_skeleton_animation_result.m_transforms.size() > 1;
                    // �洢���������ı任����ÿ֡���£�
                    render_entity.m_joint_matrices.resize(game_object_part.m_skeleton_animation_result.m_transforms.size());
                    for (size_t i = 0; i < game_object_part.m_skeleton_animation_result.m_transforms.size(); ++i)
                    {
                        render_entity.m_joint_matrices[i] = game_object_part.m_skeleton_animation_result.m_transforms[i].m_matrix;
                    }

                    // -------------------- ���������Դ --------------------
                    // ������Դ�����������ļ�·����
                    MaterialSourceDesc material_source;
                    if (game_object_part.m_material_desc.m_with_texture)
                    {
                        // �Զ�����ʣ�ʹ�����õ�����·��
                        material_source = { game_object_part.m_material_desc.m_base_color_texture_file,
                                           game_object_part.m_material_desc.m_metallic_roughness_texture_file,
                                           game_object_part.m_material_desc.m_normal_texture_file,
                                           game_object_part.m_material_desc.m_occlusion_texture_file,
                                           game_object_part.m_material_desc.m_emissive_texture_file };
                    }
                    else
                    {
                        // Ĭ�ϲ��ʣ�ʹ���������õ�Ĭ���������������ߵȣ�
                        material_source = {
                            asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                            "",// ��AO����
                            ""};// ���Է�������
                    }

                    // �������Ƿ��Ѽ��أ�ͨ�������Ĳ����ʲ���������
                    bool is_material_loaded = m_render_scene->getMaterialAssetdAllocator().hasElement(material_source);

                    // ������Ⱦ���ݣ���ɫ���������������ȣ�
                    RenderMaterialData material_data;
                    if (!is_material_loaded)
                    {
                        // �����²��ʣ�������ɫ�����ϴ�����GPU��
                        material_data = m_render_resource->loadMaterialData(material_source);
                    }

                    // ���������ԴID����δ�������½��������ã�
                    render_entity.m_material_asset_id = m_render_scene->getMaterialAssetdAllocator().allocGuid(material_source);

                    // -------------------- �ϴ���Ⱦʵ�嵽GPU --------------------
                    // ������δ���أ��ϴ��������ݵ�GPU��������������
                    if (!is_mesh_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, mesh_data);
                    }

                    // ������δ���أ��ϴ��������ݵ�GPU����������������
                    if (!is_material_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, material_data);
                    }

                    // -------------------- ���³����е���Ⱦʵ���б� --------------------
                    // �������ڳ����У������ʵ�嵽�����б�
                    if (!is_entity_in_scene)
                    {
                        m_render_scene->m_render_entities.push_back(render_entity);
                    }
                    else
                    {
                        // �������Ѵ��ڣ���������ʵ������ݣ���任���󡢲��ʵȣ�
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
                // �����굱ǰ��Ϸ��������в����󣬴ӽ����������Ƴ��ö���
                swap_data.m_game_object_resource_desc->pop();
            }

            // ������Ϸ���󽻻�����״̬�����Ϊ�޴��������
            m_swap_context.resetGameObjectResourceSwapData();
        }

        // -------------------- ����3��ɾ��������Ҫ����Ϸ���� --------------------
        if (swap_data.m_game_object_to_delete.has_value())
        {
            // ѭ���������д�ɾ���Ķ���
            while (!swap_data.m_game_object_to_delete->isEmpty())
            {
                GameObjectDesc gobject = swap_data.m_game_object_to_delete->getNextProcessObject();
                // �ӳ�����ɾ���ö����Ƴ��������ʵ�壩
                m_render_scene->deleteEntityByGObjectID(gobject.getId());
                swap_data.m_game_object_to_delete->pop();
            }
            // ����ɾ����������״̬
            m_swap_context.resetGameObjectToDelete();
        }

        // -------------------- ����4������������ݽ������������������ --------------------
        if (swap_data.m_camera_swap_data.has_value())
        {
            // ���������FOV��ˮƽ�ӽǣ�
            if (swap_data.m_camera_swap_data->m_fov_x.has_value())
            {
                m_render_camera->setFOVx(*swap_data.m_camera_swap_data->m_fov_x);
            }

            // �����������ͼ��������->��ͼ�任��
            if (swap_data.m_camera_swap_data->m_view_matrix.has_value())
            {
                m_render_camera->setMainViewMatrix(*swap_data.m_camera_swap_data->m_view_matrix);
            }

            // ����������ͣ���͸��ͶӰ������ͶӰ��
            if (swap_data.m_camera_swap_data->m_camera_type.has_value())
            {
                m_render_camera->setCurrentCameraType(*swap_data.m_camera_swap_data->m_camera_type);
            }

            // ������������ݺ����ý�������״̬
            m_swap_context.resetCameraSwapData();
        }

        // -------------------- ����5���������ӷ������� --------------------
        if (swap_data.m_particle_submit_request.has_value())
        {
            // ��ȡ����ͨ������������ϵͳ��Ⱦ����ϵͳ��
            std::shared_ptr<ParticlePass> particle_pass = std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass);

            // ��ȡ����������
            int emitter_count = swap_data.m_particle_submit_request->getEmitterCount();
            // ���÷���������
            particle_pass->setEmitterCount(emitter_count);

            // �������з���������������ʼ��
            for (int index = 0; index < emitter_count; ++index)
            {
                const ParticleEmitterDesc& desc = swap_data.m_particle_submit_request->getEmitterDesc(index);
                particle_pass->createEmitter(index, desc);
            }

            particle_pass->initializeEmitters();

            m_swap_context.resetPartilceBatchSwapData();
        }

        // -------------------- ����6���������ӷ������������� --------------------
        if (swap_data.m_emitter_tick_request.has_value())
        {
            // ������Ҫ���µķ�����������ÿ֡����ָ����������״̬��
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTickIndices(swap_data.m_emitter_tick_request->m_emitter_indices);
            m_swap_context.resetEmitterTickSwapData();
        }

        // -------------------- ����7���������ӷ������任���� --------------------
        if (swap_data.m_emitter_transform_request.has_value())
        {
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
                ->setTransformIndices(swap_data.m_emitter_transform_request->m_transform_descs);
            m_swap_context.resetEmitterTransformSwapData();
        }
    }
}

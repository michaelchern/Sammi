#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/passes/color_grading_pass.h"
#include "runtime/function/render/passes/combine_ui_pass.h"
#include "runtime/function/render/passes/directional_light_pass.h"
#include "runtime/function/render/passes/main_camera_pass.h"
#include "runtime/function/render/passes/pick_pass.h"
#include "runtime/function/render/passes/point_light_pass.h"
#include "runtime/function/render/passes/tone_mapping_pass.h"
#include "runtime/function/render/passes/ui_pass.h"
#include "runtime/function/render/passes/particle_pass.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"
#include "runtime/core/base/macro.h"

namespace Sammi
{
    void RenderPipeline::initialize(RenderPipelineInitInfo init_info)
    {
        // 创建各渲染通道实例（具体执行渲染逻辑的对象）
        m_point_light_shadow_pass = std::make_shared<PointLightShadowPass>();        // 点光源阴影通道
        m_directional_light_pass  = std::make_shared<DirectionalLightShadowPass>();  // 方向光阴影通道
        m_main_camera_pass        = std::make_shared<MainCameraPass>();              // 主相机渲染通道
        m_tone_mapping_pass       = std::make_shared<ToneMappingPass>();             // 色调映射通道
        m_color_grading_pass      = std::make_shared<ColorGradingPass>();            // 色彩分级通道
        m_ui_pass                 = std::make_shared<UIPass>();                      // UI渲染通道
        m_combine_ui_pass         = std::make_shared<CombineUIPass>();               // UI合成通道
        m_pick_pass               = std::make_shared<PickPass>();                    // 拾取通道（鼠标点击检测）
        m_fxaa_pass               = std::make_shared<FXAAPass>();                    // FXAA抗锯齿通道
        m_particle_pass           = std::make_shared<ParticlePass>();                // 粒子系统通道

        // 定义渲染通道公共信息（所有通道共享的基础配置）
        RenderPassCommonInfo pass_common_info;
        pass_common_info.rhi = m_rhi;                                  // 渲染硬件接口（访问底层API）
        pass_common_info.render_resource = init_info.render_resource;  // 渲染资源管理器（纹理/缓冲区）

        // 为所有渲染通道设置公共信息
        m_point_light_shadow_pass->setCommonInfo(pass_common_info);
        m_directional_light_pass->setCommonInfo(pass_common_info);
        m_main_camera_pass->setCommonInfo(pass_common_info);
        m_tone_mapping_pass->setCommonInfo(pass_common_info);
        m_color_grading_pass->setCommonInfo(pass_common_info);
        m_ui_pass->setCommonInfo(pass_common_info);
        m_combine_ui_pass->setCommonInfo(pass_common_info);
        m_pick_pass->setCommonInfo(pass_common_info);
        m_fxaa_pass->setCommonInfo(pass_common_info);
        m_particle_pass->setCommonInfo(pass_common_info);

        // 初始化阴影渲染通道（无额外参数）
        m_point_light_shadow_pass->initialize(nullptr);
        m_directional_light_pass->initialize(nullptr);

        // 类型转换（获取主相机通道的具体派生类指针）
        std::shared_ptr<MainCameraPass> main_camera_pass = std::static_pointer_cast<MainCameraPass>(m_main_camera_pass);
        // 类型转换（获取通用渲染通道指针，用于获取描述符布局）
        std::shared_ptr<RenderPass> _main_camera_pass = std::static_pointer_cast<RenderPass>(m_main_camera_pass);
        // 类型转换（获取粒子通道的具体派生类指针）
        std::shared_ptr<ParticlePass> particle_pass = std::static_pointer_cast<ParticlePass>(m_particle_pass);

        // 初始化粒子通道（传递全局粒子管理器）
        ParticlePassInitInfo particle_init_info{};
        particle_init_info.m_particle_manager = g_runtime_global_context.m_particle_manager;
        m_particle_pass->initialize(&particle_init_info);

        // 主相机通道需要粒子通道的引用（用于渲染粒子）
        main_camera_pass->m_point_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_point_light_shadow_pass)->getFramebufferImageViews()[0];// 获取方向光阴影帧缓冲的颜色视图
        main_camera_pass->m_directional_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_directional_light_pass)->m_framebuffer.attachments[0].view;// 获取方向光阴影帧缓冲的第一个附件视图

        // 初始化主相机通道（传递FXAA开关配置）
        MainCameraPassInitInfo main_camera_init_info;
        main_camera_init_info.enble_fxaa = init_info.enable_fxaa;
        // 设置粒子通道引用（用于主相机渲染粒子）
        main_camera_pass->setParticlePass(particle_pass);
        // 执行主相机通道初始化（获取帧缓冲、渲染通道等资源）
        m_main_camera_pass->initialize(&main_camera_init_info);

        // 粒子通道设置（绑定主相机通道的渲染命令缓冲）
        std::static_pointer_cast<ParticlePass>(m_particle_pass)->setupParticlePass();

        // 获取主相机通道的描述符集布局（用于其他通道绑定相同布局的资源）
        std::vector<RHIDescriptorSetLayout*> descriptor_layouts = _main_camera_pass->getDescriptorSetLayouts();
        std::static_pointer_cast<PointLightShadowPass>(m_point_light_shadow_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);
        std::static_pointer_cast<DirectionalLightShadowPass>(m_directional_light_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);

        m_point_light_shadow_pass->postInitialize();
        m_directional_light_pass->postInitialize();

        ToneMappingPassInitInfo tone_mapping_init_info;
        tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
        tone_mapping_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        m_tone_mapping_pass->initialize(&tone_mapping_init_info);

        ColorGradingPassInitInfo color_grading_init_info;
        color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
        color_grading_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_color_grading_pass->initialize(&color_grading_init_info);

        UIPassInitInfo ui_init_info;
        ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        m_ui_pass->initialize(&ui_init_info);

        CombineUIPassInitInfo combine_ui_init_info;
        combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        combine_ui_init_info.scene_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        combine_ui_init_info.ui_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_combine_ui_pass->initialize(&combine_ui_init_info);

        PickPassInitInfo pick_init_info;
        pick_init_info.per_mesh_layout = descriptor_layouts[MainCameraPass::LayoutType::_per_mesh];
        m_pick_pass->initialize(&pick_init_info);

        FXAAPassInitInfo fxaa_init_info;
        fxaa_init_info.render_pass = _main_camera_pass->getRenderPass();
        fxaa_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd];
        m_fxaa_pass->initialize(&fxaa_init_info);

    }

    void RenderPipeline::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        vulkan_rhi->waitForFences();

        vulkan_rhi->resetCommandPool();

        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }

        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();

        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());

        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->drawForward(color_grading_pass,
                          fxaa_pass,
                          tone_mapping_pass,
                          ui_pass,
                          combine_ui_pass,
                          particle_pass,
                          vulkan_rhi->m_current_swapchain_image_index);

        
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
    }

    void RenderPipeline::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        vulkan_rhi->waitForFences();

        vulkan_rhi->resetCommandPool();

        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }

        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();

        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());

        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->draw(color_grading_pass,
                   fxaa_pass,
                   tone_mapping_pass,
                   ui_pass,
                   combine_ui_pass,
                   particle_pass,
                   vulkan_rhi->m_current_swapchain_image_index);
                   
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
    }

    void RenderPipeline::passUpdateAfterRecreateSwapchain()
    {
        MainCameraPass&   main_camera_pass   = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        PickPass&         pick_pass          = *(static_cast<PickPass*>(m_pick_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        main_camera_pass.updateAfterFramebufferRecreate();
        tone_mapping_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);
        color_grading_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        fxaa_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd]);
        combine_ui_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd],
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        pick_pass.recreateFramebuffer();
        particle_pass.updateAfterFramebufferRecreate();
        g_runtime_global_context.m_debugdraw_manager->updateAfterRecreateSwapchain();
    }
    uint32_t RenderPipeline::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        PickPass& pick_pass = *(static_cast<PickPass*>(m_pick_pass.get()));
        return pick_pass.pick(picked_uv);
    }

    void RenderPipeline::setAxisVisibleState(bool state)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_is_show_axis  = state;
    }

    void RenderPipeline::setSelectedAxis(size_t selected_axis)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_selected_axis = selected_axis;
    }
}

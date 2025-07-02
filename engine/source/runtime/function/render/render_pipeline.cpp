#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

// 包含各种渲染通道的头文件
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
    // 初始化渲染管线
    void RenderPipeline::initialize(RenderPipelineInitInfo init_info)
    {
        m_point_light_shadow_pass = std::make_shared<PointLightShadowPass>();        // 点光源阴影通道
        m_directional_light_pass  = std::make_shared<DirectionalLightShadowPass>();  // 方向光阴影通道
        m_main_camera_pass        = std::make_shared<MainCameraPass>();              // 主相机通道
        m_tone_mapping_pass       = std::make_shared<ToneMappingPass>();             // 色调映射通道
        m_color_grading_pass      = std::make_shared<ColorGradingPass>();            // 颜色分级通道
        m_ui_pass                 = std::make_shared<UIPass>();                      // UI渲染通道
        m_combine_ui_pass         = std::make_shared<CombineUIPass>();               // UI合成通道
        m_pick_pass               = std::make_shared<PickPass>();                    // 拾取通道
        m_fxaa_pass               = std::make_shared<FXAAPass>();                    // FXAA抗锯齿通道
        m_particle_pass           = std::make_shared<ParticlePass>();                // 粒子系统通道

        // 2. 设置所有通道的公共信息
        RenderPassCommonInfo pass_common_info;
        pass_common_info.rhi             = m_rhi;  // 渲染硬件接口
        pass_common_info.render_resource = init_info.render_resource;  // 渲染资源管理器

        // 应用公共信息到所有通道
        m_point_light_shadow_pass->setCommonInfo(pass_common_info);
        // ... (其他通道设置相同)
        m_directional_light_pass->setCommonInfo(pass_common_info);
        m_main_camera_pass->setCommonInfo(pass_common_info);
        m_tone_mapping_pass->setCommonInfo(pass_common_info);
        m_color_grading_pass->setCommonInfo(pass_common_info);
        m_ui_pass->setCommonInfo(pass_common_info);
        m_combine_ui_pass->setCommonInfo(pass_common_info);
        m_pick_pass->setCommonInfo(pass_common_info);
        m_fxaa_pass->setCommonInfo(pass_common_info);
        m_particle_pass->setCommonInfo(pass_common_info);

        // 3. 初始化光照通道
        m_point_light_shadow_pass->initialize(nullptr);
        m_directional_light_pass->initialize(nullptr);

        std::shared_ptr<MainCameraPass> main_camera_pass = std::static_pointer_cast<MainCameraPass>(m_main_camera_pass);
        std::shared_ptr<RenderPass>     _main_camera_pass = std::static_pointer_cast<RenderPass>(m_main_camera_pass);
        std::shared_ptr<ParticlePass> particle_pass = std::static_pointer_cast<ParticlePass>(m_particle_pass);

        // 4. 特殊通道初始化
        // 4.1 粒子通道初始化（需要粒子管理器）
        ParticlePassInitInfo particle_init_info{};
        particle_init_info.m_particle_manager = g_runtime_global_context.m_particle_manager;
        m_particle_pass->initialize(&particle_init_info);

        // 4.2 主相机通道初始化
        // 传递阴影贴图视图引用
        main_camera_pass->m_point_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_point_light_shadow_pass)->getFramebufferImageViews()[0];
        main_camera_pass->m_directional_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_directional_light_pass)->m_framebuffer.attachments[0].view;

        // 设置主相机通道参数
        MainCameraPassInitInfo main_camera_init_info;
        main_camera_init_info.enble_fxaa = init_info.enable_fxaa;  // FXAA开关
        main_camera_pass->setParticlePass(particle_pass);          // 关联粒子通道
        m_main_camera_pass->initialize(&main_camera_init_info);   // 初始化主相机通道

        std::static_pointer_cast<ParticlePass>(m_particle_pass)->setupParticlePass();

        // 4.3 设置光照通道的网格布局
        std::vector<RHIDescriptorSetLayout*> descriptor_layouts = _main_camera_pass->getDescriptorSetLayouts();
        std::static_pointer_cast<PointLightShadowPass>(m_point_light_shadow_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);
        std::static_pointer_cast<DirectionalLightShadowPass>(m_directional_light_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);

        // ... (方向光设置相同)
        m_point_light_shadow_pass->postInitialize();
        m_directional_light_pass->postInitialize();

        // 5. 后处理通道初始化
        // 5.1 色调映射通道
        ToneMappingPassInitInfo tone_mapping_init_info;
        tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
        tone_mapping_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        m_tone_mapping_pass->initialize(&tone_mapping_init_info);

        // 5.2 颜色分级通道（类似设置）...
        ColorGradingPassInitInfo color_grading_init_info;
        color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
        color_grading_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_color_grading_pass->initialize(&color_grading_init_info);

        // 5.3 UI通道（类似设置）...
        UIPassInitInfo ui_init_info;
        ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        m_ui_pass->initialize(&ui_init_info);

        // 5.4 UI合成通道（类似设置）...
        CombineUIPassInitInfo combine_ui_init_info;
        combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        combine_ui_init_info.scene_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        combine_ui_init_info.ui_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_combine_ui_pass->initialize(&combine_ui_init_info);

        // 6. 拾取通道初始化
        PickPassInitInfo pick_init_info;
        pick_init_info.per_mesh_layout = descriptor_layouts[MainCameraPass::LayoutType::_per_mesh];
        m_pick_pass->initialize(&pick_init_info);

        // 7. FXAA抗锯齿通道（类似设置）...
        FXAAPassInitInfo fxaa_init_info;
        fxaa_init_info.render_pass = _main_camera_pass->getRenderPass();
        fxaa_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd];
        m_fxaa_pass->initialize(&fxaa_init_info);

    }

    // 前向渲染执行
    void RenderPipeline::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        // 转换接口类型
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        // 1. 重置环形缓冲区偏移量
        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        // 2. 等待上一帧完成
        vulkan_rhi->waitForFences();

        // 3. 重置命令池
        vulkan_rhi->resetCommandPool();

        // 4. 准备渲染前工作（处理交换链重建）
        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)  // 需要重建交换链则跳过本次渲染
        {
            return;
        }

        // 5. 执行阴影渲染通道
        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();
        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        // 6. 转换各通道引用
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        // 7. 设置粒子通道的命令缓冲区
        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());

        // 8. 执行主相机通道（前向渲染路径）
        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->drawForward(color_grading_pass,  // 前向渲染特定调用
                          fxaa_pass,
                          tone_mapping_pass,
                          ui_pass,
                          combine_ui_pass,
                          particle_pass,
                          vulkan_rhi->m_current_swapchain_image_index);

        // 9. 执行调试绘制
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        // 10. 提交渲染命令
        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));

        // 11. 粒子系统后处理
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();  // 粒子模拟计算
    }

    // 延迟渲染执行（与前向渲染大部分逻辑相同）
    void RenderPipeline::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        // ... (同前向渲染步骤1-6)
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

        // 主要区别在步骤8 - 执行主相机通道（延迟渲染路径）
        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->draw(color_grading_pass,  // 延迟渲染调用
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

    // 交换链重建后更新所有通道
    void RenderPipeline::passUpdateAfterRecreateSwapchain()
    {
        // 获取所有通道的引用
        MainCameraPass&   main_camera_pass   = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        // ... (其他通道引用)
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        PickPass&         pick_pass          = *(static_cast<PickPass*>(m_pick_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        // 更新各通道的资源
        main_camera_pass.updateAfterFramebufferRecreate();   // 更新主相机通道帧缓冲
        tone_mapping_pass.updateAfterFramebufferRecreate(    // 更新色调映射通道输入
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);
        // ... (其他通道更新)
        color_grading_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        fxaa_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd]);
        combine_ui_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd],
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        pick_pass.recreateFramebuffer();
        particle_pass.updateAfterFramebufferRecreate();  // 更新粒子通道
        g_runtime_global_context.m_debugdraw_manager->updateAfterRecreateSwapchain();  // 调试绘制系统更新
    }

    // 获取被拾取网格的GUID
    uint32_t RenderPipeline::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        PickPass& pick_pass = *(static_cast<PickPass*>(m_pick_pass.get()));
        return pick_pass.pick(picked_uv);   // 执行拾取操作
    }

    // 设置坐标轴可见性
    void RenderPipeline::setAxisVisibleState(bool state)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_is_show_axis  = state;  // 控制主相机通道中坐标轴显示状态
    }

    // 设置选中的坐标轴
    void RenderPipeline::setSelectedAxis(size_t selected_axis)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_selected_axis = selected_axis;  // 设置当前选中的坐标轴
    }
}
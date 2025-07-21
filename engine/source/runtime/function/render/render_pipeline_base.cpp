#include "runtime/function/render/render_pipeline_base.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"
#include "runtime/core/base/macro.h"
#include "runtime/function/global/global_context.h"

namespace Sammi
{
    void RenderPipelineBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        // 准备主相机渲染通道的数据（如相机视锥体、投影矩阵、可见物体列表等）
        m_main_camera_pass->preparePassData(render_resource);

        // 准备拾取渲染通道的数据（如生成用于鼠标拾取的深度/ID纹理）
        m_pick_pass->preparePassData(render_resource);

        // 准备方向光渲染通道的数据（如阴影贴图生成所需的灯光矩阵、级联参数等）
        m_directional_light_pass->preparePassData(render_resource);

        // 准备点光源阴影渲染通道的数据（如点光源阴影贴图的立方体贴图生成参数）
        m_point_light_shadow_pass->preparePassData(render_resource);

        // 准备粒子系统渲染通道的数据（如粒子位置、生命周期、材质参数等）
        m_particle_pass->preparePassData(render_resource);

        // 触发全局调试绘制管理器的数据准备（如调试网格框、坐标轴的绘制参数）
        g_runtime_global_context.m_debugdraw_manager->preparePassData(render_resource);
    }

    void RenderPipelineBase::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
    }

    void RenderPipelineBase::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
    }

    void RenderPipelineBase::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_ui_pass->initializeUIRenderBackend(window_ui);
    }
}

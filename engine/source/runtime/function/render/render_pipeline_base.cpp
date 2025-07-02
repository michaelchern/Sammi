#include "runtime/function/render/render_pipeline_base.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"  // 调试绘制管理器
#include "runtime/core/base/macro.h"                               // 宏定义
#include "runtime/function/global/global_context.h"                // 全局上下文

namespace Sammi
{
    // 准备所有渲染通道所需的数据
    void RenderPipelineBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        // 按合理的顺序准备各通道的数据
        m_main_camera_pass->preparePassData(render_resource);                            // 主相机通道（场景几何渲染）
        m_pick_pass->preparePassData(render_resource);                                   // 拾取通道（对象选择）
        m_directional_light_pass->preparePassData(render_resource);                      // 方向光阴影通道
        m_point_light_shadow_pass->preparePassData(render_resource);                     // 点光源阴影通道
        m_particle_pass->preparePassData(render_resource);                               // 粒子系统通道
        g_runtime_global_context.m_debugdraw_manager->preparePassData(render_resource);  // 调试绘制通道
    }

    // 前向渲染实现（基类为空实现）
    void RenderPipelineBase::forwardRender(std::shared_ptr<RHI>                rhi,
                                           std::shared_ptr<RenderResourceBase> render_resource)
    {
        // 基类不实现具体渲染逻辑
        // 具体实现应由派生类提供
        // 通常包括：
        //   1. 设置渲染状态
        //   2. 绑定全局资源
        //   3. 执行各渲染通道
        //   4. 后处理效果链
        //   5. UI渲染和合成
    }

    // 延迟渲染实现（基类为空实现）
    void RenderPipelineBase::deferredRender(std::shared_ptr<RHI>                rhi,
                                            std::shared_ptr<RenderResourceBase> render_resource)
    {
        // 基类不实现具体渲染逻辑
        // 具体实现应由派生类提供
        // 通常包括：
        //   1. G-buffer生成阶段
        //   2. 光照计算阶段
        //   3. 后处理效果链
        //   4. UI渲染和合成
        //   5. 与前向渲染不同的资源管理和光照处理
    }

    // 初始化UI渲染后端
    void RenderPipelineBase::initializeUIRenderBackend(WindowUI* window_ui)
    {
        // 将UI系统的初始化请求转发给UI渲染通道
        m_ui_pass->initializeUIRenderBackend(window_ui);

        // 这个调用通常完成以下工作：
        //   1. 创建UI专用的渲染资源（字体纹理等）
        //   2. 设置UI渲染所需的描述符集
        //   3. 注册UI绘制回调接口
        //   4. 准备UI顶点/索引缓冲区
    }
}
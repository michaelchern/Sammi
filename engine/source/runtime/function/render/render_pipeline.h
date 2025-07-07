#pragma once

// 包含渲染管线基类头文件
#include "runtime/function/render/render_pipeline_base.h"

namespace Sammi
{
    // 具体渲染管线实现类（继承自RenderPipelineBase）
    class RenderPipeline : public RenderPipelineBase
    {
    public:
        // 初始化渲染管线（重写基类纯虚函数）
        virtual void initialize(RenderPipelineInitInfo init_info) override final;

        // 前向渲染实现（重写基类函数）
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource) override final;

        // 延迟渲染实现（重写基类函数）
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource) override final;

        // 在交换链重建后更新通道资源（窗口大小/分辨率改变时调用）
        void passUpdateAfterRecreateSwapchain();

        // 获取被拾取网格的GUID（实现基类纯虚函数）
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) override final;

        // 设置坐标轴可见性（编辑器功能）
        void setAxisVisibleState(bool state);

        // 设置选中的坐标轴（X/Y/Z，编辑器功能）
        void setSelectedAxis(size_t selected_axis);
    };
}
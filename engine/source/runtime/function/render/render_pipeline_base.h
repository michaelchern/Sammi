#pragma once

// 包含基础数学向量结构
#include "runtime/core/math/vector2.h"
// 包含渲染通道基类
#include "runtime/function/render/render_pass_base.h"

#include <memory>
#include <vector>

namespace Sammi
{
    // 前置声明
    class RHI;                      // 渲染硬件接口层
    class RenderResourceBase;       // 渲染资源基类
    class WindowUI;                 // UI窗口系统接口

    // 渲染管线初始化参数结构
    struct RenderPipelineInitInfo
    {
        bool enable_fxaa{ false };  // 是否启用FXAA抗锯齿
        std::shared_ptr<RenderResourceBase> render_resource;  // 渲染资源管理器指针
    };

    // 渲染管线基类
    class RenderPipelineBase
    {
        // 允许渲染系统直接访问
        friend class RenderSystem;

    public:
        virtual ~RenderPipelineBase() {}  // 虚析构函数

        virtual void clear() {};

        // 初始化函数（纯虚函数，必须由派生类实现）
        virtual void initialize(RenderPipelineInitInfo init_info) = 0;
        // 准备渲染通道数据（默认实现）
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);
        // 前向渲染实现（默认实现）
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);
        // 延迟渲染实现（默认实现）
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

        // 初始化UI渲染后端
        void initializeUIRenderBackend(WindowUI* window_ui);
        // 获取被拾取网格的GUID（纯虚函数，必须由派生类实现）
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) = 0;

    protected:
        std::shared_ptr<RHI> m_rhi;  // 渲染硬件接口引用

        // 渲染通道模块 --------------------------
        // 这些成员描述了现代渲染管线的典型阶段
        // -------------------------------------
        std::shared_ptr<RenderPassBase> m_directional_light_pass;   // 方向光阴影处理通道
        std::shared_ptr<RenderPassBase> m_point_light_shadow_pass;  // 点光源阴影处理通道
        std::shared_ptr<RenderPassBase> m_main_camera_pass;         // 主摄像机渲染通道（核心几何通道）
        std::shared_ptr<RenderPassBase> m_color_grading_pass;       // 颜色分级/校正处理通道
        std::shared_ptr<RenderPassBase> m_fxaa_pass;                // FXAA抗锯齿处理通道
        std::shared_ptr<RenderPassBase> m_tone_mapping_pass;        // 色调映射处理通道 (HDR->LDR)
        std::shared_ptr<RenderPassBase> m_ui_pass;                  // UI元素渲染通道
        std::shared_ptr<RenderPassBase> m_combine_ui_pass;          // 场景与UI合成通道
        std::shared_ptr<RenderPassBase> m_pick_pass;                // 对象拾取专用通道
        std::shared_ptr<RenderPassBase> m_particle_pass;            // 粒子系统渲染通道
    };
}
#pragma once

#include "runtime/core/math/vector2.h"
#include "runtime/function/render/render_pass_base.h"

#include <memory>
#include <vector>

namespace Sammi
{
    // 前置声明依赖的渲染系统核心类
    class RHI;// 渲染硬件接口（抽象底层API如Vulkan/DirectX）
    class RenderResourceBase;// 渲染资源基类（管理纹理、缓冲区等资源）
    class WindowUI;// UI渲染窗口类（处理UI绘制）

    /**
     * @brief 渲染管线初始化信息结构体
     * 用于传递渲染管线初始化所需的配置参数和资源
     */
    struct RenderPipelineInitInfo
    {
        bool                                enable_fxaa {false};// 是否启用FXAA抗锯齿（快速近似抗锯齿）
        std::shared_ptr<RenderResourceBase> render_resource;// 渲染资源管理器（提供纹理/缓冲区等资源访问）
    };

    /**
     * @brief 渲染管线基类（抽象接口）
     * 定义所有具体渲染管线（如前向渲染、延迟渲染）必须实现的接口，
     * 负责协调多个渲染通道（RenderPass）完成完整的渲染流程。
     * 作为渲染系统的核心模块，负责管理渲染状态、资源调度和各阶段渲染任务的执行。
     */
    class RenderPipelineBase
    {
        // 声明RenderSystem为友元类（允许其访问本类的私有/保护成员）
        friend class RenderSystem;

    public:
        /**
         * @brief 虚析构函数
         * 确保派生类对象通过基类指针销毁时正确调用析构函数
         */
        virtual ~RenderPipelineBase() {}

        /**
         * @brief 清理渲染管线资源
         * 释放所有渲染通道、RHI资源等，用于管线重置或程序退出
         */
        virtual void clear() {};

        /**
         * @brief 初始化渲染管线（纯虚函数）
         * @param init_info 渲染管线初始化配置（包含抗锯齿开关、渲染资源等）
         * 子类必须实现此函数以完成具体的初始化逻辑（如创建渲染通道、分配资源）
         */
        virtual void initialize(RenderPipelineInitInfo init_info) = 0;

        /**
         * @brief 准备渲染通道数据
         * @param render_resource 渲染资源管理器（提供纹理/缓冲区等资源）
         * 在渲染前更新各渲染通道需要的临时数据（如光照参数、相机矩阵等）
         */
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);

        /**
         * @brief 执行前向渲染流程（纯虚函数）
         * @param rhi 渲染硬件接口（访问底层API）
         * @param render_resource 渲染资源管理器
         * 前向渲染：按物体顺序逐个渲染（适合透明物体、少量光源场景）
         * 子类必须实现此函数以完成具体的前向渲染逻辑
         */
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

        /**
         * @brief 执行延迟渲染流程（纯虚函数）
         * @param rhi 渲染硬件接口
         * @param render_resource 渲染资源管理器
         * 延迟渲染：分两步（几何阶段存储GBuffer，光照阶段计算光照）（适合复杂光照场景）
         * 子类必须实现此函数以完成具体的延迟渲染逻辑
         */
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

        /**
         * @brief 初始化UI渲染后端
         * @param window_ui UI窗口对象（提供UI绘制上下文）
         * 将UI渲染集成到主渲染流程中（如设置UI渲染目标、同步分辨率等）
         */
        void initializeUIRenderBackend(WindowUI* window_ui);

        /**
         * @brief 获取被拾取网格的GUID（纯虚函数）
         * @param picked_uv 屏幕空间UV坐标（鼠标点击位置的归一化坐标）
         * 用于交互场景（如点击选择物体）：根据屏幕坐标反推被选中的网格对象
         * @return 被拾取网格的唯一标识GUID（若未命中返回无效值）
         */
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) = 0;

    protected:
        std::shared_ptr<RHI> m_rhi;  // 渲染硬件接口实例（访问底层API的核心句柄）

        // 渲染通道集合（各阶段渲染任务的执行者）
        std::shared_ptr<RenderPassBase> m_directional_light_pass;   // 方向光渲染通道（处理方向光阴影/光照）
        std::shared_ptr<RenderPassBase> m_point_light_shadow_pass;  // 点光源阴影渲染通道（生成点光源阴影贴图）
        std::shared_ptr<RenderPassBase> m_main_camera_pass;         // 主相机渲染通道（渲染场景主视角物体）
        std::shared_ptr<RenderPassBase> m_color_grading_pass;       // 色彩分级通道（调整画面色调/对比度）
        std::shared_ptr<RenderPassBase> m_fxaa_pass;                // FXAA抗锯齿通道（优化边缘锯齿）
        std::shared_ptr<RenderPassBase> m_tone_mapping_pass;        // 色调映射通道（HDR转LDR）
        std::shared_ptr<RenderPassBase> m_ui_pass;                  // UI渲染通道（绘制UI元素）
        std::shared_ptr<RenderPassBase> m_combine_ui_pass;          // UI合成通道（合并UI与游戏画面）
        std::shared_ptr<RenderPassBase> m_pick_pass;                // 拾取渲染通道（生成用于拾取的深度/ID纹理）
        std::shared_ptr<RenderPassBase> m_particle_pass;            // 粒子系统渲染通道（绘制粒子效果）
    };
}

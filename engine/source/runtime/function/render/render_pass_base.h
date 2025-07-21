#pragma once

#include "runtime/function/render/interface/rhi.h"

namespace Sammi
{
    class RHI;
    class RenderResourceBase;
    class WindowUI;

    // ============================== 渲染通道初始化信息（空结构体） ==============================
    // 用于传递渲染通道初始化时的参数（当前版本无具体字段，预留扩展）
    struct RenderPassInitInfo {};

    // ============================== 渲染通道公共信息 ==============================
    // 存储渲染通道运行所需的公共资源与上下文信息
    struct RenderPassCommonInfo
    {
        std::shared_ptr<RHI>                rhi;
        std::shared_ptr<RenderResourceBase> render_resource;
    };

    // ============================== 渲染通道基类（抽象接口） ==============================
    // 所有具体渲染通道（如前向渲染、延迟渲染）的基类，定义渲染流程的核心接口
    class RenderPassBase
    {
    public:
        // 纯虚函数：初始化渲染通道（子类必须实现）
        // 参数：init_info - 初始化参数（当前为空，可扩展传递配置数据）
        virtual void initialize(const RenderPassInitInfo* init_info) = 0;

        // 虚函数：初始化后的后续操作（如资源绑定、管线状态最终设置）
        // （默认空实现，子类可重写）
        virtual void postInitialize();

        // 虚函数：设置渲染通道的公共信息（RHI实例与渲染资源）
        // 参数：common_info - 包含RHI和渲染资源的公共信息结构体
        virtual void setCommonInfo(RenderPassCommonInfo common_info);

        // 虚函数：准备渲染通道所需的数据（如更新动态缓冲区、设置纹理采样状态等）
        // 参数：render_resource - 具体渲染资源对象（可能用于数据上传或绑定）
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);

        // 虚函数：初始化UI渲染后端（将UI渲染集成到当前渲染通道）
        // 参数：window_ui - 窗口与UI管理类实例（提供UI渲染所需的上下文）
        virtual void initializeUIRenderBackend(WindowUI* window_ui);

    protected:
        std::shared_ptr<RHI>                m_rhi;
        std::shared_ptr<RenderResourceBase> m_render_resource;
    };
}

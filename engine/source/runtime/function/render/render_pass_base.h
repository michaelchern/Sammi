#pragma once

#include "runtime/function/render/interface/rhi.h"  // 包含渲染硬件接口声明

namespace Sammi
{
    // 前置声明
    class RHI;                 // 渲染硬件接口层
    class RenderResourceBase;  // 渲染资源基类
    class WindowUI;            // UI系统接口

    // 渲染通道初始化信息结构（基类，派生通道可扩展）
    struct RenderPassInitInfo
    {};

    // 渲染通道通用信息（所有渲染通道共享）
    struct RenderPassCommonInfo
    {
        std::shared_ptr<RHI>                rhi;              // 渲染硬件接口
        std::shared_ptr<RenderResourceBase> render_resource;  // 渲染资源管理器
    };

    // 渲染通道基类（所有具体通道的父类）
    class RenderPassBase
    {
    public:
        // 初始化渲染通道（纯虚函数，必须由具体通道实现）
        virtual void initialize(const RenderPassInitInfo* init_info) = 0;

        // 后初始化处理（可选重写，默认空实现）
        virtual void postInitialize();

        // 设置通道通用信息
        virtual void setCommonInfo(RenderPassCommonInfo common_info);

        // 准备每帧渲染数据（可选重写，默认空实现）
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);

        // 初始化UI渲染后端（可选重写，默认空实现）
        virtual void initializeUIRenderBackend(WindowUI* window_ui);

    protected:
        std::shared_ptr<RHI>                m_rhi;              // 渲染硬件接口引用
        std::shared_ptr<RenderResourceBase> m_render_resource;  // 渲染资源管理器引用
    };
}

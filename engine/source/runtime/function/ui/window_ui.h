#pragma once

#include <memory>

namespace Sammi
{
    // 前置声明（避免包含实际头文件）
    class WindowSystem;
    class RenderSystem;

    // UI初始化信息结构体
    struct WindowUIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;  // 窗口系统的共享指针
        std::shared_ptr<RenderSystem> render_system;  // 渲染系统的共享指针
    };

    // 窗口UI抽象基类
    class WindowUI
    {
    public:
        virtual ~WindowUI() = default;

        // 初始化UI组件
        // @param init_info 包含窗口和渲染系统依赖的初始化信息
        virtual void initialize(WindowUIInitInfo init_info) = 0;

        // 渲染前的准备工作
        // 每帧渲染前调用（通常用于处理UI逻辑/状态更新）
        virtual void preRender() = 0;
    };
}
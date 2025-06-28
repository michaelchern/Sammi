#pragma once

namespace Sammi
{
    /// 编辑器全局上下文初始化信息（用于编辑器启动）
    struct EditorGlobalContextInitInfo
    {
        class WindowSystem*   window_system;   // 窗口系统指针
        class RenderSystem*   render_system;   // 渲染系统指针
        class PiccoloEngine*  engine_runtime;  // 引擎运行时指针
    };

    /// 编辑器全局上下文（集中管理所有核心子系统）
    class EditorGlobalContext
    {
    public:
        // 核心模块指针
        class EditorSceneManager* m_scene_manager {nullptr};   // 场景管理模块
        class EditorInputManager* m_input_manager {nullptr};   // 输入管理模块
        class RenderSystem*       m_render_system {nullptr};   // 渲染系统
        class WindowSystem*       m_window_system {nullptr};   // 窗口系统
        class PiccoloEngine*      m_engine_runtime {nullptr};  // 引擎运行时

    public:
        // 使用初始化信息配置上下文
        void initialize(const EditorGlobalContextInitInfo& init_info);

        // 清除所有指针引用（安全关闭）
        void clear();
    };

    // 声明全局唯一编辑器上下文实例（在cpp文件中定义）
    extern EditorGlobalContext g_editor_global_context;
}
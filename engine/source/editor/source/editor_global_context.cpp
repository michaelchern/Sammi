// 包含编辑器全局上下文头文件
#include "editor/include/editor_global_context.h"

// 包含编辑器子系统头文件
#include "editor/include/editor_input_manager.h"
#include "editor/include/editor_scene_manager.h"

// 包含运行时系统头文件
#include "runtime/function/render/render_system.h"
#include "runtime/function/render/window_system.h"

namespace Sammi
{
    // 声明全局编辑器上下文实例（在.cpp文件中定义）
    EditorGlobalContext g_editor_global_context;

    /// 初始化编辑器全局上下文
    /// @param init_info 初始化所需的信息结构体
    void EditorGlobalContext::initialize(const EditorGlobalContextInitInfo& init_info)
    {
        // 1. 设置核心系统引用 (非拥有)
        g_editor_global_context.m_window_system  = init_info.window_system;   // 窗口系统
        g_editor_global_context.m_render_system  = init_info.render_system;   // 渲染系统
        g_editor_global_context.m_engine_runtime = init_info.engine_runtime;  // 引擎运行时

        // 2. 创建并初始化核心子系统
        m_scene_manager = new EditorSceneManager();  // 创建场景管理器
        m_input_manager = new EditorInputManager();  // 创建输入管理器

        // 初始化子系统
        m_scene_manager->initialize();  // 初始化场景系统
        m_input_manager->initialize();  // 初始化输入系统
    }

    /// 清理编辑器全局上下文（释放所有资源）
    void EditorGlobalContext::clear()
    {
        // 1. 删除并重置场景管理器
        delete (m_scene_manager);

        // 2. 删除并重置输入管理器
        delete (m_input_manager);
    }
}
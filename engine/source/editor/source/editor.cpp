#include "editor//include/editor.h"  // 编辑器主头文件

// 包含引擎核心系统
#include "runtime/engine.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_system.h"

// 包含编辑器子系统
#include "editor/include/editor_global_context.h"
#include "editor/include/editor_input_manager.h"
#include "editor/include/editor_scene_manager.h"
#include "editor/include/editor_ui.h"

namespace Sammi
{
    /// 注册需要在编辑器模式下刷新的组件类型
    void registerEdtorTickComponent(std::string component_type_name)
    {
        g_editor_tick_component_types.insert(component_type_name);
    }

    /// 编辑器构造函数
    SammiEditor::SammiEditor()
    {
        // 注册需要每帧更新的组件类型
        registerEdtorTickComponent("TransformComponent");  // 变换组件
        registerEdtorTickComponent("MeshComponent");       // 网格组件
    }

    /// 编辑器析构函数
    SammiEditor::~SammiEditor() {}

    /// 初始化编辑器系统
    void SammiEditor::initialize(SammiEngine* engine_runtime)
    {
        assert(engine_runtime);  // 确保引擎运行时有效

        g_is_editor_mode = true;  // 设置全局编辑器模式标志
        m_engine_runtime = engine_runtime;  // 保存引擎运行时指针

        // 初始化编辑器全局上下文
        EditorGlobalContextInitInfo init_info = {g_runtime_global_context.m_window_system.get(),  // 窗口系统
                                                 g_runtime_global_context.m_render_system.get(),  // 渲染系统
                                                 engine_runtime};                                 // 引擎运行时
        g_editor_global_context.initialize(init_info);

        // 设置编辑器相机
        g_editor_global_context.m_scene_manager->setEditorCamera(g_runtime_global_context.m_render_system->getRenderCamera());

        // 上传坐标轴资源到GPU
        g_editor_global_context.m_scene_manager->uploadAxisResource();

        // 初始化编辑器UI
        m_editor_ui                   = std::make_shared<EditorUI>();
        WindowUIInitInfo ui_init_info = {g_runtime_global_context.m_window_system,   // 窗口系统
                                         g_runtime_global_context.m_render_system};  // 渲染系统
        m_editor_ui->initialize(ui_init_info);
    }

    /// 清理编辑器资源
    void SammiEditor::clear()
    {
        g_editor_global_context.clear();  // 清理全局上下文
    }

    /// 运行编辑器主循环
    void SammiEditor::run()
    {
        assert(m_engine_runtime);  // 确保引擎运行时已初始化
        assert(m_editor_ui);       // 确保编辑器UI已初始化

        float delta_time;          // 帧时间差
        while (true)
        {
            // 计算帧时间差
            delta_time = m_engine_runtime->calculateDeltaTime();

            // 更新场景管理器
            g_editor_global_context.m_scene_manager->tick(delta_time);

            // 更新输入管理器
            g_editor_global_context.m_input_manager->tick(delta_time);

            // 执行引擎单帧更新（包含渲染）
            if (!m_engine_runtime->tickOneFrame(delta_time))
                return;  // 如果引擎返回false，退出循环
        }
    }
}
#include "editor/include/editor_input_manager.h"  // 输入管理器头文件

// 包含编辑器相关头文件
#include "editor/include/editor.h"                 // 编辑器主类
#include "editor/include/editor_global_context.h"  // 全局上下文
#include "editor/include/editor_scene_manager.h"   // 场景管理器

// 包含引擎运行时头文件
#include "runtime/engine.h"                                  // 引擎核心
#include "runtime/function/framework/level/level.h"          // 关卡管理
#include "runtime/function/framework/world/world_manager.h"  // 世界管理
#include "runtime/function/global/global_context.h"          // 全局上下文
#include "runtime/function/input/input_system.h"             // 输入系统

// 包含渲染相关头文件
#include "runtime/function/render/render_camera.h"  // 渲染相机
#include "runtime/function/render/render_system.h"  // 渲染系统
#include "runtime/function/render/window_system.h"  // 窗口系统

namespace Sammi
{
    /// 初始化输入管理器
    void EditorInputManager::initialize()
    {
        registerInput();  // 注册输入回调
    }

    void EditorInputManager::tick(float delta_time)
    {
        processEditorCommand();  // 处理编辑器命令
    }

    /// 注册所有输入回调函数
    void EditorInputManager::registerInput()
    {
        // 注册窗口系统回调
        g_editor_global_context.m_window_system->registerOnResetFunc(
            std::bind(&EditorInputManager::onReset, this));

        g_editor_global_context.m_window_system->registerOnCursorPosFunc(
            std::bind(&EditorInputManager::onCursorPos, this, std::placeholders::_1, std::placeholders::_2));

        g_editor_global_context.m_window_system->registerOnCursorEnterFunc(
            std::bind(&EditorInputManager::onCursorEnter, this, std::placeholders::_1));

        g_editor_global_context.m_window_system->registerOnScrollFunc(
            std::bind(&EditorInputManager::onScroll, this, std::placeholders::_1, std::placeholders::_2));

        g_editor_global_context.m_window_system->registerOnMouseButtonFunc(
            std::bind(&EditorInputManager::onMouseButtonClicked, this, std::placeholders::_1, std::placeholders::_2));

        g_editor_global_context.m_window_system->registerOnWindowCloseFunc(
            std::bind(&EditorInputManager::onWindowClosed, this));

        g_editor_global_context.m_window_system->registerOnKeyFunc(
            std::bind(&EditorInputManager::onKey, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }

    /// 更新光标在坐标轴上的状态
    void EditorInputManager::updateCursorOnAxis(Vector2 cursor_uv)
    {
        if (g_editor_global_context.m_scene_manager->getEditorCamera())
        {
            Vector2 window_size(m_engine_window_size.x, m_engine_window_size.y);
            // 查询光标当前在哪个坐标轴上
            m_cursor_on_axis = g_editor_global_context.m_scene_manager->updateCursorOnAxis(cursor_uv, window_size);
        }
    }

    /// 处理编辑器命令（相机移动、对象删除等）
    void EditorInputManager::processEditorCommand()
    {
        float           camera_speed  = m_camera_speed;
        std::shared_ptr editor_camera = g_editor_global_context.m_scene_manager->getEditorCamera();
        Quaternion      camera_rotate = editor_camera->rotation().inverse();
        Vector3         camera_relative_pos(0, 0, 0);  // 相机相对移动量

        // 处理相机移动命令（使用位标志判断）
        if ((unsigned int)EditorCommand::camera_foward & m_editor_command)
        {
            camera_relative_pos += camera_rotate * Vector3 {0, camera_speed, 0};
        }
        if ((unsigned int)EditorCommand::camera_back & m_editor_command)
        {
            camera_relative_pos += camera_rotate * Vector3 {0, -camera_speed, 0};
        }
        if ((unsigned int)EditorCommand::camera_left & m_editor_command)
        {
            camera_relative_pos += camera_rotate * Vector3 {-camera_speed, 0, 0};
        }
        if ((unsigned int)EditorCommand::camera_right & m_editor_command)
        {
            camera_relative_pos += camera_rotate * Vector3 {camera_speed, 0, 0};
        }
        if ((unsigned int)EditorCommand::camera_up & m_editor_command)
        {
            camera_relative_pos += Vector3 {0, 0, camera_speed};
        }
        if ((unsigned int)EditorCommand::camera_down & m_editor_command)
        {
            camera_relative_pos += Vector3 {0, 0, -camera_speed};
        }

        // 处理删除对象命令
        if ((unsigned int)EditorCommand::delete_object & m_editor_command)
        {
            g_editor_global_context.m_scene_manager->onDeleteSelectedGObject();
        }

        // 应用相机移动
        editor_camera->move(camera_relative_pos);
    }

    /// 编辑器模式下的按键处理
    void EditorInputManager::onKeyInEditorMode(int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)  // 按键按下
        {
            switch (key)
            {
                case GLFW_KEY_A:
                    m_editor_command |= (unsigned int)EditorCommand::camera_left;
                    break;
                case GLFW_KEY_S:
                    m_editor_command |= (unsigned int)EditorCommand::camera_back;
                    break;
                case GLFW_KEY_W:
                    m_editor_command |= (unsigned int)EditorCommand::camera_foward;
                    break;
                case GLFW_KEY_D:
                    m_editor_command |= (unsigned int)EditorCommand::camera_right;
                    break;
                case GLFW_KEY_Q:
                    m_editor_command |= (unsigned int)EditorCommand::camera_up;
                    break;
                case GLFW_KEY_E:
                    m_editor_command |= (unsigned int)EditorCommand::camera_down;
                    break;
                case GLFW_KEY_T:
                    m_editor_command |= (unsigned int)EditorCommand::translation_mode;
                    break;
                case GLFW_KEY_R:
                    m_editor_command |= (unsigned int)EditorCommand::rotation_mode;
                    break;
                case GLFW_KEY_C:
                    m_editor_command |= (unsigned int)EditorCommand::scale_mode;
                    break;
                case GLFW_KEY_DELETE:
                    m_editor_command |= (unsigned int)EditorCommand::delete_object;
                    break;
                default:
                    break;
            }
        }
        else if (action == GLFW_RELEASE)  // 按键释放
        {
            switch (key)
            {
                case GLFW_KEY_ESCAPE:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::exit);
                    break;
                case GLFW_KEY_A:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_left);
                    break;
                case GLFW_KEY_S:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_back);
                    break;
                case GLFW_KEY_W:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_foward);
                    break;
                case GLFW_KEY_D:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_right);
                    break;
                case GLFW_KEY_Q:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_up);
                    break;
                case GLFW_KEY_E:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::camera_down);
                    break;
                case GLFW_KEY_T:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::translation_mode);
                    break;
                case GLFW_KEY_R:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::rotation_mode);
                    break;
                case GLFW_KEY_C:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::scale_mode);
                    break;
                case GLFW_KEY_DELETE:
                    m_editor_command &= (k_complement_control_command ^ (unsigned int)EditorCommand::delete_object);
                    break;
                default:
                    break;
            }
        }
    }

    /// 通用按键处理（根据模式分发）
    void EditorInputManager::onKey(int key, int scancode, int action, int mods)
    {
        if (g_is_editor_mode)  // 仅在编辑器模式下处理
        {
            onKeyInEditorMode(key, scancode, action, mods);
        }
    }

    /// 重置输入状态（待实现）
    void EditorInputManager::onReset()
    {
        // TODO: 实现输入状态重置逻辑
    }

    /// 鼠标移动处理
    void EditorInputManager::onCursorPos(double xpos, double ypos)
    {
        if (!g_is_editor_mode)  // 非编辑器模式不处理
            return;

        // 计算角度速度（基于窗口大小）
        float angularVelocity = 180.0f / Math::max(m_engine_window_size.x, m_engine_window_size.y);

        if (m_mouse_x >= 0.0f && m_mouse_y >= 0.0f)  // 有效鼠标位置
        {
            // 右键按下：旋转相机
            if (g_editor_global_context.m_window_system->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                glfwSetInputMode(g_editor_global_context.m_window_system->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                g_editor_global_context.m_scene_manager->getEditorCamera()->rotate(Vector2(ypos - m_mouse_y, xpos - m_mouse_x) * angularVelocity);
            }
            // 左键按下：移动物体
            else if (g_editor_global_context.m_window_system->isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
            {
                g_editor_global_context.m_scene_manager->moveEntity(
                    xpos,
                    ypos,
                    m_mouse_x,
                    m_mouse_y,
                    m_engine_window_pos,
                    m_engine_window_size,
                    m_cursor_on_axis,
                    g_editor_global_context.m_scene_manager->getSelectedObjectMatrix());
                glfwSetInputMode(g_editor_global_context.m_window_system->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            // 无按键：更新光标在坐标轴上的状态
            else
            {
                glfwSetInputMode(g_editor_global_context.m_window_system->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                // 检查光标是否在引擎视口内
                if (isCursorInRect(m_engine_window_pos, m_engine_window_size))
                {
                    // 计算UV坐标
                    Vector2 cursor_uv = Vector2((m_mouse_x - m_engine_window_pos.x) / m_engine_window_size.x,
                                                (m_mouse_y - m_engine_window_pos.y) / m_engine_window_size.y);
                    updateCursorOnAxis(cursor_uv);
                }
            }
        }

        // 更新鼠标位置
        m_mouse_x = xpos;
        m_mouse_y = ypos;
    }

    /// 光标进入/离开窗口处理
    void EditorInputManager::onCursorEnter(int entered)
    {
        if (!entered)  // 失去焦点
        {
            m_mouse_x = m_mouse_y = -1.0f;  // 重置鼠标位置
        }
    }

    /// 鼠标滚轮处理
    void EditorInputManager::onScroll(double xoffset, double yoffset)
    {
        if (!g_is_editor_mode)  // 非编辑器模式不处理
        {
            return;
        }

        // 检查光标是否在引擎视口内
        if (isCursorInRect(m_engine_window_pos, m_engine_window_size))
        {
            // 右键按下：调整相机速度
            if (g_editor_global_context.m_window_system->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                if (yoffset > 0)
                {
                    m_camera_speed *= 1.2f;
                }
                else
                {
                    m_camera_speed *= 0.8f;
                }
            }
            else  // 其他情况：缩放相机
            {
                g_editor_global_context.m_scene_manager->getEditorCamera()->zoom((float)yoffset * 2.0f);
            }
        }
    }

    /// 鼠标按键点击处理
    void EditorInputManager::onMouseButtonClicked(int key, int action)
    {
        if (!g_is_editor_mode)  // 非编辑器模式不处理
            return;
        if (m_cursor_on_axis != 3)  // 光标在坐标轴上时不处理选择
            return;

        // 获取当前活动关卡
        std::shared_ptr<Level> current_active_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
        if (current_active_level == nullptr)
            return;

        // 检查光标是否在引擎视口内
        if (isCursorInRect(m_engine_window_pos, m_engine_window_size))
        {
            // 左键点击：选择对象
            if (key == GLFW_MOUSE_BUTTON_LEFT)
            {
                // 计算点击位置的UV坐标
                Vector2 picked_uv((m_mouse_x - m_engine_window_pos.x) / m_engine_window_size.x,
                                  (m_mouse_y - m_engine_window_pos.y) / m_engine_window_size.y);

                // 获取点击的网格ID
                size_t  select_mesh_id = g_editor_global_context.m_scene_manager->getGuidOfPickedMesh(picked_uv);

                // 通过网格ID获取游戏对象ID
                size_t gobject_id = g_editor_global_context.m_render_system->getGObjectIDByMeshID(select_mesh_id);

                // 选择游戏对象
                g_editor_global_context.m_scene_manager->onGObjectSelected(gobject_id);
            }
        }
    }

    /// 窗口关闭处理
    void EditorInputManager::onWindowClosed()
    {
        g_editor_global_context.m_engine_runtime->shutdownEngine();  // 关闭引擎
    }

    /// 检查光标是否在指定矩形区域内
    bool EditorInputManager::isCursorInRect(Vector2 pos, Vector2 size) const
    {
        return pos.x <= m_mouse_x && m_mouse_x <= pos.x + size.x && pos.y <= m_mouse_y && m_mouse_y <= pos.y + size.y;
    }
}
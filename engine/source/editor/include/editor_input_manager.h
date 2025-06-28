#pragma once

// 包含数学库和相机系统
#include "runtime/core/math/vector2.h"
#include "runtime/function/render/render_camera.h"

#include <vector>  // 标准向量容器

namespace Sammi
{
    class SammiEditor;  // 前向声明编辑器类

    // 编辑器命令枚举（使用位标志组合）
    enum class EditorCommand : unsigned int
    {
        camera_left      = 1 << 0,  // A键      - 相机左移
        camera_back      = 1 << 1,  // S键      - 相机后移
        camera_foward    = 1 << 2,  // W键      - 相机前移
        camera_right     = 1 << 3,  // D键      - 相机右移
        camera_up        = 1 << 4,  // Q键      - 相机上移
        camera_down      = 1 << 5,  // E键      - 相机下移
        translation_mode = 1 << 6,  // T键      - 切换到平移模式
        rotation_mode    = 1 << 7,  // R键      - 切换到旋转模式
        scale_mode       = 1 << 8,  // C键      - 切换到缩放模式
        exit             = 1 << 9,  // Esc键    - 退出编辑器
        delete_object    = 1 << 10, // Delete键 - 删除选中物体
    };

    /// 编辑器输入管理器（处理所有输入事件）
    class EditorInputManager
    {
    public:
        // 初始化输入系统
        void initialize();
        // 每帧更新（参数：帧时间差）
        void tick(float delta_time);

    public:
        // 注册输入回调到窗口系统
        void registerInput();
        // 更新光标在坐标轴上的状态（参数：光标UV坐标）
        void updateCursorOnAxis(Vector2 cursor_uv);
        // 处理当前激活的编辑器命令
        void processEditorCommand();
        // 编辑器模式下的按键处理
        void onKeyInEditorMode(int key, int scancode, int action, int mods);

        // 通用按键处理
        void onKey(int key, int scancode, int action, int mods);
        // 重置输入状态
        void onReset();
        // 鼠标移动事件
        void onCursorPos(double xpos, double ypos);
        // 鼠标进出窗口事件
        void onCursorEnter(int entered);
        // 鼠标滚轮事件
        void onScroll(double xoffset, double yoffset);
        // 鼠标按键事件
        void onMouseButtonClicked(int key, int action);
        // 窗口关闭事件
        void onWindowClosed();

        // 判断光标是否在指定矩形区域
        bool isCursorInRect(Vector2 pos, Vector2 size) const;

    public:
        // 获取引擎窗口位置
        Vector2 getEngineWindowPos() const { return m_engine_window_pos; };
        // 获取引擎窗口尺寸
        Vector2 getEngineWindowSize() const { return m_engine_window_size; };
        // 获取相机移动速度
        float   getCameraSpeed() const { return m_camera_speed; };

        // 设置引擎窗口位置
        void setEngineWindowPos(Vector2 new_window_pos) { m_engine_window_pos = new_window_pos; };
        // 设置引擎窗口尺寸
        void setEngineWindowSize(Vector2 new_window_size) { m_engine_window_size = new_window_size; };
        // 重置编辑器命令状态
        void resetEditorCommand() { m_editor_command = 0; }

    private:
        Vector2 m_engine_window_pos {0.0f, 0.0f};        // 引擎视口位置（像素）
        Vector2 m_engine_window_size {1280.0f, 768.0f};  // 引擎视口尺寸（像素）
        float   m_mouse_x {0.0f};                        // 当前鼠标X坐标
        float   m_mouse_y {0.0f};                        // 当前鼠标Y坐标
        float   m_camera_speed {0.05f};                  // 相机移动速度系数

        size_t       m_cursor_on_axis {3};               // 光标所在坐标轴（0=X,1=Y,2=Z,3=无）
        unsigned int m_editor_command {0};               // 当前激活的命令位标志组合
    };
}
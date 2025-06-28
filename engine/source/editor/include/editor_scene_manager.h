#pragma once

// 包含坐标轴控件定义
#include "editor/include/axis.h"

// 包含游戏对象和渲染对象
#include "runtime/function/framework/object/object.h"
#include "runtime/function/render/render_object.h"

#include <memory>  // 智能指针支持

namespace Sammi
{
    // 前向声明类
    class SammiEditor;
    class RenderCamera;
    class RenderEntity;

    /// 编辑器变换模式枚举
    enum class EditorAxisMode : int
    {
        TranslateMode = 0,  // 平移模式
        RotateMode = 1,     // 旋转模式
        ScaleMode = 2,      // 缩放模式
        Default = 3         // 无操作模式
    };

    /// 编辑器场景管理器（管理场景中的对象选择、变换操作）
    class EditorSceneManager
    {
    public:
        // 初始化场景管理器
        void initialize();
        // 每帧更新逻辑
        void tick(float delta_time);

    public:
        // 更新光标在坐标轴上的状态（返回坐标轴索引）
        // 参数：光标UV坐标，引擎窗口尺寸
        size_t updateCursorOnAxis(Vector2 cursor_uv, Vector2 game_engine_window_size);

        // 绘制选中实体的坐标轴（根据当前模式）
        void drawSelectedEntityAxis();
        // 获取选中的游戏对象（弱指针避免所有权问题）
        std::weak_ptr<GObject> getSelectedGObject() const;
        // 获取当前模式对应的坐标轴网格
        RenderEntity* getAxisMeshByType(EditorAxisMode axis_mode);
        // 当选中某个游戏对象时调用
        void onGObjectSelected(GObjectID selected_gobject_id);
        // 删除当前选中的游戏对象
        void onDeleteSelectedGObject();
        // 移动实体（核心变换逻辑）
        void moveEntity(float     new_mouse_pos_x,     // 新鼠标X坐标
                        float     new_mouse_pos_y,     // 新鼠标Y坐标
                        float     last_mouse_pos_x,    // 上一帧鼠标X
                        float     last_mouse_pos_y,    // 上一帧鼠标Y
                        Vector2   engine_window_pos,   // 引擎视口位置
                        Vector2   engine_window_size,  // 引擎视口尺寸
                        size_t    cursor_on_axis,      // 操作的坐标轴
                        Matrix4x4 model_matrix);       // 当前模型矩阵

        // 设置编辑器相机
        void setEditorCamera(std::shared_ptr<RenderCamera> camera) { m_camera = camera; }
        // 创建坐标轴网格资源
        void uploadAxisResource();
        // 通过拾取的UV坐标获取网格GUID（用于对象拾取）
        size_t getGuidOfPickedMesh(const Vector2& picked_uv) const;

    public:
        // 获取编辑器相机
        std::shared_ptr<RenderCamera> getEditorCamera() { return m_camera; };
        // 获取选中对象的ID
        GObjectID getSelectedObjectID() { return m_selected_gobject_id; };
        // 获取选中对象的变换矩阵
        Matrix4x4 getSelectedObjectMatrix() { return m_selected_object_matrix; }
        // 获取当前变换模式
        EditorAxisMode getEditorAxisMode() { return m_axis_mode; }

        // 设置选中对象的ID
        void setSelectedObjectID(GObjectID selected_gobject_id) { m_selected_gobject_id = selected_gobject_id; };
        // 设置选中对象的变换矩阵
        void setSelectedObjectMatrix(Matrix4x4 new_object_matrix) { m_selected_object_matrix = new_object_matrix; }
        // 设置变换模式
        void setEditorAxisMode(EditorAxisMode new_axis_mode) { m_axis_mode = new_axis_mode; }

    private:
        // 坐标轴控件实例
        EditorTranslationAxis m_translation_axis;  // 平移坐标轴
        EditorRotationAxis    m_rotation_axis;     // 旋转坐标轴
        EditorScaleAxis       m_scale_aixs;        // 缩放坐标轴

        // 选中对象状态
        GObjectID m_selected_gobject_id{ k_invalid_gobject_id };    // 无效ID表示无选中
        Matrix4x4 m_selected_object_matrix{ Matrix4x4::IDENTITY };  // 初始为单位矩阵

        // 编辑器状态
        EditorAxisMode m_axis_mode{ EditorAxisMode::TranslateMode };  // 默认平移模式
        std::shared_ptr<RenderCamera> m_camera;                       // 编辑器使用的相机

        // 操作状态
        size_t m_selected_axis{ 3 };   // 当前操作的坐标轴索引（0=X,1=Y,2=Z,3=无）
        bool   m_is_show_axis = true;  // 是否显示坐标轴控件
    };
}
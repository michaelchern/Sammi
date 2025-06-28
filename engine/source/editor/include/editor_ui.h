#pragma once

// 包含坐标轴控件定义
#include "editor/include/axis.h"
// 基础数学库
#include "runtime/core/math/vector2.h"
// 游戏对象系统
#include "runtime/function/framework/object/object.h"
// UI基础系统
#include "runtime/function/ui/window_ui.h"
// 编辑器文件服务
#include "editor/include/editor_file_service.h"

// 标准库包含
#include <chrono>  // 时间处理
#include <map>     // 映射容器
#include <vector>  // 动态数组

namespace Sammi
{
    // 前向声明类
    class SammiEditor;
    class WindowSystem;
    class RenderSystem;

    /// 编辑器主界面类（继承自基础UI窗口系统）
    class EditorUI : public WindowUI
    {
    public:
        EditorUI();  // 构造函数

    private:
        // 内部回调函数
        void        onFileContentItemClicked(EditorFileNode* node);                                // 文件项点击处理
        void        buildEditorFileAssetsUITree(EditorFileNode* node);                             // 构建文件树UI
        void        drawAxisToggleButton(const char* string_id, bool check_state, int axis_mode);  // 绘制变换模式按钮
        void        createClassUI(Reflection::ReflectionInstance& instance);                       // 创建反射对象的类UI
        void        createLeafNodeUI(Reflection::ReflectionInstance& instance);                    // 创建反射对象的叶节点UI
        std::string getLeafUINodeParentLabel();                                                    // 获取叶子节点UI的父标签

        // UI窗口渲染函数
        void showEditorUI();                              // 主UI渲染入口
        void showEditorMenu(bool* p_open);                // 菜单栏
        void showEditorWorldObjectsWindow(bool* p_open);  // 场景对象窗口
        void showEditorFileContentWindow(bool* p_open);   // 文件内容窗口
        void showEditorGameWindow(bool* p_open);          // 游戏视图窗口
        void showEditorDetailWindow(bool* p_open);        // 属性详情窗口

        void setUIColorStyle();                           // 设置UI颜色主题

    public:
        // 继承自基类的接口
        virtual void initialize(WindowUIInitInfo init_info) override final;  // 初始化UI系统
        virtual void preRender() override final;                             // 每帧渲染前的预处理

    private:
        // UI创建器映射表（类型标识符到创建函数的映射）
        std::unordered_map<std::string, std::function<void(std::string, void*)>> m_editor_ui_creator;
        // 新对象索引映射表
        std::unordered_map<std::string, unsigned int>                            m_new_object_index_map;
        EditorFileService                                                        m_editor_file_service;    // 文件服务实例
        std::chrono::time_point<std::chrono::steady_clock>                       m_last_file_tree_update;  // 文件树最后更新时间

        // 窗口可见性控制
        bool m_editor_menu_window_open       = true;  // 主菜单窗口可见
        bool m_asset_window_open             = true;  // 资源窗口可见
        bool m_game_engine_window_open       = true;  // 游戏视图窗口可见
        bool m_file_content_window_open      = true;  // 文件内容窗口可见
        bool m_detail_window_open            = true;  // 属性详情窗口可见
        bool m_scene_lights_window_open      = true;  // 场景光源窗口可见
        bool m_scene_lights_data_window_open = true;  // 光源数据窗口可见
    };
}
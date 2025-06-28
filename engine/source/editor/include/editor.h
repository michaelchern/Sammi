#pragma once

#include "runtime/core/math/vector2.h"  // 二维向量数学库
#include <memory>                       // 智能指针支持

namespace Sammi
{
    // 前置声明类（避免头文件依赖）
    class EditorUI;     // 编辑器UI系统
    class SammiEngine;  // 引擎核心系统

    /// 游戏编辑器主类
    class SammiEditor 
    {
        friend class EditorUI;     // 允许EditorUI访问私有成员

    public:
        SammiEditor();             // 构造函数
        virtual ~PiccoloEditor();  // 虚析构函数（支持继承）

        // 使用引擎实例初始化编辑器
        void initialize(PiccoloEngine* engine_runtime);

        // 清理编辑器资源
        void clear();

        // 启动编辑器主循环
        void run();

    protected:
        std::shared_ptr<EditorUI> m_editor_ui;     // 编辑器UI系统（智能指针管理）
        SammiEngine* m_engine_runtime{ nullptr };  // 指向引擎核心的指针（不拥有所有权）
    };
}
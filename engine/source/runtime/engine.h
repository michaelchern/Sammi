#pragma once

#include <atomic>         // 原子操作支持
#include <chrono>         // 时间处理库
#include <filesystem>     // 文件系统操作
#include <string>         // 字符串处理
#include <unordered_set>  // 哈希集合容器

namespace Sammi
{
    extern bool                            g_is_editor_mode;               // 标识当前是否处于编辑器模式
    extern std::unordered_set<std::string> g_editor_tick_component_types;  // 编辑器模式下需要刷新的组件类型集合

    class SammiEngine
    {
        friend class SammiEditor;                               // 声明友元类，允许SammiEditor访问私有成员

        static const float s_fps_alpha;                         // FPS计算平滑因子（常量）

    public:
        // 核心生命周期控制
        void startEngine(const std::string& config_file_path);  // 通过配置文件路径启动引擎
        void shutdownEngine();                                  // 安全关闭引擎

        // 初始化与资源清理
        void initialize();                                      // 初始化引擎子系统
        void clear();                                           // 清理引擎资源

        // 状态查询
        bool isQuit() const { return m_is_quit; }               // 检查引擎是否收到退出信号

        // 主循环控制
        void run();                                             // 启动主循环
        bool tickOneFrame(float delta_time);                    // 执行单帧逻辑（含逻辑/渲染tick）

        // 性能指标
        int getFPS() const { return m_fps; }                    // 获取当前平均FPS

    protected:
        // 帧处理逻辑
        void logicalTick(float delta_time);                     // 执行游戏逻辑更新
        bool rendererTick(float delta_time);                    // 执行渲染管线（返回是否成功）

        // FPS计算
        void calculateFPS(float delta_time);                    // 根据帧时长更新平均FPS

        /**
        * 每帧限调用一次的时间计算
        * @return 当前帧与上一帧的时间间隔（秒）
        */
        float calculateDeltaTime();

    protected:
        // 运行时状态
        bool m_is_quit {false};                                 // 引擎退出标志

        // 时间管理
        std::chrono::steady_clock::time_point m_last_tick_time_point {std::chrono::steady_clock::now()};  // 上一帧时间戳

        // FPS统计相关
        float m_average_duration {0.f};                         // 平均帧时长（平滑处理）
        int   m_frame_count {0};                                // 累计帧数（用于统计）
        int   m_fps {0};                                        // 当前平均帧率
    };
}
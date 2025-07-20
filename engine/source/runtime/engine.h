#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace Sammi
{
    // 全局编辑器模式标志，用于控制编辑器相关功能的启用/禁用状态
    // 当引擎运行在编辑器环境（如Sammi编辑器）中时，此标志会被设置为true
    extern bool g_is_editor_mode;

    // 编辑器模式下需要每帧更新的组件类型集合
    // 存储组件类型的字符串标识（如"TransformComponent", "RenderComponent"）
    // 编辑器通过此集合筛选需要在编辑器tick中处理的组件，避免影响运行时逻辑
    extern std::unordered_set<std::string> g_editor_tick_component_types;

    /**
     * @brief 引擎核心管理类，负责引擎的生命周期管理、主循环运行及核心功能协调
     *
     * 该类封装了引擎的启动、初始化、主循环运行、帧更新、资源管理等核心功能，
     * 是引擎系统的核心入口点。支持运行时状态查询（如FPS、退出状态）和编辑器模式集成。
     */
    class SammiEngine
    {
        friend class SammiEditor;        // 允许编辑器类直接访问引擎内部成员（如私有状态）

        static const float s_fps_alpha;  // FPS平滑计算的指数移动平均系数（0<alpha<1）
                                         // 用于缓解帧率波动，使显示的FPS值更稳定（类外定义）

    public:
        /**
         * @brief 启动引擎并加载配置
         * @param config_file_path 引擎配置文件路径（如"config/engine.json"）
         * 功能：初始化底层系统（如日志、资源管理器）、加载配置参数、准备运行环境
         * 注意：需在调用initialize()前调用，或在内部自动触发初始化流程（具体实现依赖设计）
         */
        void startEngine(const std::string& config_file_path);

        /**
         * @brief 关闭引擎并释放所有资源
         * 功能：停止所有运行中的子系统（如渲染、物理）、清理内存资源、保存必要数据
         * 注意：应在程序退出前调用，确保资源正确释放，避免内存泄漏
         */
        void shutdownEngine();

        /**
         * @brief 初始化引擎核心子系统
         * 功能：初始化逻辑模块（如组件系统、事件系统）、渲染器、物理引擎等核心组件
         * 注意：通常在startEngine()后调用，或作为startEngine()的内部步骤执行
         */
        void initialize();

        /**
         * @brief 清理引擎当前状态（保留配置和资源）
         * 功能：重置逻辑状态（如销毁临时对象、停止动画）、清空渲染命令队列
         * 注意：可用于重置引擎到初始可运行状态，不释放底层资源（区别于shutdownEngine）
         */
        void clear();

        /**
         * @brief 查询引擎是否应退出主循环
         * @return true 引擎标记为退出；false 继续运行
         * 用途：主循环中判断是否终止引擎运行（如用户点击关闭按钮或调用quit()）
         */
        bool isQuit() const { return m_is_quit; }

        /**
         * @brief 运行引擎主循环
         * 流程：持续调用tickOneFrame()处理单帧更新，直到isQuit()返回true
         * 注意：主循环会阻塞当前线程，通常应在独立线程中运行（如主线程）
         */
        void run();

        /**
         * @brief 处理单帧更新（逻辑+渲染）
         * @param delta_time 当前帧与上一帧的时间间隔（秒），由calculateDeltaTime()提供
         * @return true 单帧更新成功；false 更新失败（如渲染错误）
         * 流程：先调用logicalTick()处理逻辑，再调用rendererTick()处理渲染
         */
        bool tickOneFrame(float delta_time);

        /**
         * @brief 获取当前引擎帧率（FPS）
         * @return 当前平滑后的帧率值（整数，如60、30）
         * 说明：基于最近多帧的时间间隔计算，通过s_fps_alpha平滑避免数值剧烈波动
         */
        int getFPS() const { return m_fps; }

    protected:
        /**
         * @brief 处理单帧逻辑更新
         * @param delta_time 当前帧时间间隔（秒）
         * 功能：更新游戏逻辑（如角色移动、AI决策）、处理事件、更新组件状态
         * 注意：每帧仅调用一次，由tickOneFrame()触发
         */
        void logicalTick(float delta_time);

        /**
         * @brief 处理单帧渲染更新
         * @param delta_time 当前帧时间间隔（秒）
         * @return true 渲染成功；false 渲染失败（如资源丢失）
         * 功能：提交渲染命令、更新UI、绘制调试信息
         * 注意：渲染结果可能在下一帧或更晚显示，具体依赖渲染API（如OpenGL/Vulkan）
         */
        bool rendererTick(float delta_time);

        /**
         * @brief 基于当前帧时间间隔更新FPS计算
         * @param delta_time 当前帧时间间隔（秒）
         * 算法：使用指数移动平均平滑FPS值
         *      m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha
         *      m_fps = static_cast<int>(1.0f / m_average_duration)
         */
        void calculateFPS(float delta_time);

        /**
         * @brief 计算当前帧与上一帧的时间间隔（delta time）
         * @return 当前帧时间间隔（秒，精度取决于steady_clock）
         * 实现：使用std::chrono::steady_clock获取高精度时间戳，计算时间差
         * 注意：每帧必须且只能调用一次，否则会导致时间计算错误
         */
        float calculateDeltaTime();

    protected:
        bool m_is_quit {false};  // 引擎退出标志，触发主循环终止

        // 上一帧tick的时间戳（用于计算delta_time）
        std::chrono::steady_clock::time_point m_last_tick_time_point {std::chrono::steady_clock::now()};

        float m_average_duration {0.f};  // 平滑后的平均帧时间（用于FPS计算）
        int   m_frame_count {0};         // 帧计数器（用于调试或辅助计算）
        int   m_fps {0};                 // 当前平滑后的帧率值
    };

}

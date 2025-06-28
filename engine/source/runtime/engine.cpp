#include "runtime/engine.h"

// 包含必要的功能模块头文件
#include "runtime/core/base/macro.h"                               // 基础宏定义
#include "runtime/core/meta/reflection/reflection_register.h"      // 反射注册系统

#include "runtime/function/framework/world/world_manager.h"        // 场景世界管理
#include "runtime/function/global/global_context.h"                // 全局上下文
#include "runtime/function/input/input_system.h"                   // 输入系统
#include "runtime/function/particle/particle_manager.h"            // 粒子系统管理
#include "runtime/function/physics/physics_manager.h"              // 物理系统管理
#include "runtime/function/render/render_system.h"                 // 渲染系统
#include "runtime/function/render/window_system.h"                 // 窗口系统
#include "runtime/function/render/debugdraw/debug_draw_manager.h"  // 调试绘制

namespace Sammi
{
    // 初始化全局变量
    bool                            g_is_editor_mode {false};          // 默认非编辑器模式
    std::unordered_set<std::string> g_editor_tick_component_types {};  // 空的组件类型集合

    // 启动引擎核心系统
    void SammiEngine::startEngine(const std::string& config_file_path)
    {
        Reflection::TypeMetaRegister::metaRegister();                  // 注册反射元数据

        g_runtime_global_context.startSystems(config_file_path);       // 初始化所有子系统

        LOG_INFO("engine start");                                      // 日志记录引擎启动
    }

    // 关闭引擎
    void SammiEngine::shutdownEngine()
    {
        LOG_INFO("engine shutdown");                                   // 日志记录引擎关闭

        g_runtime_global_context.shutdownSystems();                    // 安全关闭所有子系统

        Reflection::TypeMetaRegister::metaUnregister();                // 注销反射元数据
    }

    // 空初始化函数（作为扩展点）
    void SammiEngine::initialize() {}

    // 清理引擎资源（当前无实际操作）
    void SammiEngine::clear() {}

    // 引擎主循环
    void SammiEngine::run()
    {
        std::shared_ptr<WindowSystem> window_system = g_runtime_global_context.m_window_system;
        ASSERT(window_system);  // 确保窗口系统已初始化

        // 主循环：当窗口未关闭时持续运行
        while (!window_system->shouldClose())
        {
            const float delta_time = calculateDeltaTime();            // 计算帧间时间差
            tickOneFrame(delta_time);                                 // 执行单帧更新
        }
    }

    // 计算帧时间间隔
    float SammiEngine::calculateDeltaTime()
    {
        float delta_time;
        {
            using namespace std::chrono;

            steady_clock::time_point tick_time_point = steady_clock::now();  // 获取当前时间点

            // 计算与前一次的时间差
            duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);
            delta_time                = time_span.count();  // 转换为秒

            m_last_tick_time_point = tick_time_point;  // 更新上次执行时间点
        }
        return delta_time;
    }

    // 单帧处理逻辑
    bool SammiEngine::tickOneFrame(float delta_time)
    {
        logicalTick(delta_time);  // 执行游戏逻辑更新
        calculateFPS(delta_time);  // 更新帧率统计

        // 单线程模式：交换逻辑和渲染数据
        g_runtime_global_context.m_render_system->swapLogicRenderData();

        rendererTick(delta_time);  // 执行渲染更新

// 物理调试渲染的可选功能
#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        g_runtime_global_context.m_physics_manager->renderPhysicsWorld(delta_time);
#endif

        g_runtime_global_context.m_window_system->pollEvents();  // 处理窗口事件

        // 更新窗口标题显示当前FPS
        g_runtime_global_context.m_window_system->setTitle(
            std::string("Piccolo - " + std::to_string(getFPS()) + " FPS").c_str());

        // 返回引擎是否应继续运行
        const bool should_window_close = g_runtime_global_context.m_window_system->shouldClose();
        return !should_window_close;
    }

    // 逻辑帧更新
    void SammiEngine::logicalTick(float delta_time)
    {
        g_runtime_global_context.m_world_manager->tick(delta_time); // 更新场景世界
        g_runtime_global_context.m_input_system->tick();// 处理输入事件
    }

    // 渲染帧更新
    bool SammiEngine::rendererTick(float delta_time)
    {
        g_runtime_global_context.m_render_system->tick(delta_time);// 执行渲染管道
        return true; // 始终返回渲染成功
    }

    // FPS计算相关常量和方法
    const float SammiEngine::s_fps_alpha = 1.f / 100;        // FPS平滑因子（100帧移动平均）
    void        SammiEngine::calculateFPS(float delta_time)
    {
        m_frame_count++;   // 帧计数器递增

        if (m_frame_count == 1)// 第一帧初始化
        {
            m_average_duration = delta_time;
        }
        else// 后续帧使用指数平滑
        {
            // 帧时长指数移动平均公式
            m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);// 计算当前FPS
    }
}
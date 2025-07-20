#include "runtime/engine.h"

// 包含核心功能头文件
#include "runtime/core/base/macro.h"                               // 宏定义（如ASSERT、LOG_INFO等）
#include "runtime/core/meta/reflection/reflection_register.h"      // 反射系统注册（用于类型元信息管理）

// 包含各功能模块头文件（引擎核心子系统）
#include "runtime/function/framework/world/world_manager.h"        // 世界管理器（管理游戏对象/场景）
#include "runtime/function/global/global_context.h"                // 全局上下文（单例，管理所有子系统实例）
#include "runtime/function/input/input_system.h"                   // 输入系统（处理用户输入）
#include "runtime/function/particle/particle_manager.h"            // 粒子系统（管理粒子效果）
#include "runtime/function/physics/physics_manager.h"              // 物理系统（处理物理模拟）
#include "runtime/function/render/render_system.h"                 // 渲染系统（处理3D/2D渲染）
#include "runtime/function/render/window_system.h"                 // 窗口系统（管理窗口创建/事件/渲染目标）
#include "runtime/function/render/debugdraw/debug_draw_manager.h"  // 调试绘制（用于物理/场景调试）

namespace Sammi
{
    bool                            g_is_editor_mode {false};
    std::unordered_set<std::string> g_editor_tick_component_types {};

    void SammiEngine::startEngine(const std::string& config_file_path)
    {
        // 步骤1：注册反射类型元信息
        // 反射系统用于在运行时获取类型信息（如类成员、函数），常用于序列化、脚本绑定、编辑器反射等场景
        Reflection::TypeMetaRegister::metaRegister();

        // 步骤2：启动全局上下文中的所有系统
        // g_runtime_global_context是全局单例，管理引擎所有子系统（如窗口、输入、渲染等）
        // startSystems会根据配置文件（config_file_path）初始化各子系统
        g_runtime_global_context.startSystems(config_file_path);

        LOG_INFO("SammiEngine start!");
    }

    void SammiEngine::shutdownEngine()
    {
        LOG_INFO("SammiEngine shutdown!");

        // 步骤1：关闭全局上下文中的所有系统
        // 按相反顺序释放子系统资源（如先停止渲染，再停止物理等）
        g_runtime_global_context.shutdownSystems();

        // 步骤2：取消反射类型元信息注册
        // 避免内存泄漏（反射系统可能缓存类型信息，需显式释放）
        Reflection::TypeMetaRegister::metaUnregister();
    }

    // 初始化引擎（当前为空实现，可能由子类重写或在其他阶段完成）
    void SammiEngine::initialize() {}

    // 清理引擎资源（当前为空实现，可能释放自定义资源）
    void SammiEngine::clear() {}

    // 引擎主循环：驱动每帧逻辑和渲染更新
    void SammiEngine::run()
    {
        // 获取窗口系统实例（全局上下文管理的核心子系统之一）
        std::shared_ptr<WindowSystem> window_system = g_runtime_global_context.m_window_system;
        ASSERT(window_system);  // 断言确保窗口系统已初始化（否则程序终止）

        // 主循环：持续运行直到窗口请求关闭（如用户点击关闭按钮）
        while (!window_system->shouldClose())
        {
            // 计算当前帧与上一帧的时间差（delta_time，单位：秒）
            const float delta_time = calculateDeltaTime();

            // 处理单帧逻辑（包含逻辑更新和渲染准备）
            tickOneFrame(delta_time);
        }
    }

    float SammiEngine::calculateDeltaTime()
    {
        float delta_time;
        {
            // 使用C++11的steady_clock（单调时钟，不受系统时间调整影响）
            using namespace std::chrono;

            // 获取当前时间点
            steady_clock::time_point tick_time_point = steady_clock::now();

            // 计算时间跨度（转换为秒为单位）
            duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);

            // 提取时间差（秒）
            delta_time = time_span.count();

            // 更新上一帧时间点为当前时间点（供下一帧计算）
            m_last_tick_time_point = tick_time_point;
        }
        return delta_time;
    }

    bool SammiEngine::tickOneFrame(float delta_time)
    {
        // 1. 逻辑层更新（游戏对象行为、输入响应等）
        logicalTick(delta_time);

        // 2. 计算并更新FPS（帧率显示）
        calculateFPS(delta_time);

        // 3. 交换逻辑与渲染上下文数据
        // 渲染系统可能需要访问逻辑层更新后的数据（如物体位置、材质状态）
        // 此函数负责将逻辑层数据复制/同步到渲染专用数据结构中
        g_runtime_global_context.m_render_system->swapLogicRenderData();

        // 4. 渲染层更新（准备绘制指令、更新渲染资源等）
        rendererTick(delta_time);

        // 5. 物理调试绘制（可选功能，编译时通过ENABLE_PHYSICS_DEBUG_RENDERER启用）
#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        g_runtime_global_context.m_physics_manager->renderPhysicsWorld(delta_time);
#endif

        // 6. 处理窗口事件（输入事件、窗口大小调整等）
        g_runtime_global_context.m_window_system->pollEvents();

        // 7. 更新窗口标题（显示当前FPS）
        g_runtime_global_context.m_window_system->setTitle(std::string("Sammi - " + std::to_string(getFPS()) + " FPS").c_str());

        // 8. 检查窗口是否请求关闭（返回true表示窗口未关闭，继续循环）
        const bool should_window_close = g_runtime_global_context.m_window_system->shouldClose();
        return !should_window_close;
    }

    // 逻辑层更新（处理游戏核心逻辑）
    void SammiEngine::logicalTick(float delta_time)
    {
        // 1. 更新世界管理器（处理游戏对象的创建、销毁、行为更新）
        g_runtime_global_context.m_world_manager->tick(delta_time);

        // 2. 更新输入系统（处理键盘、鼠标等输入设备的状态）
        g_runtime_global_context.m_input_system->tick();
    }

    // 渲染层更新（准备渲染所需数据）
    bool SammiEngine::rendererTick(float delta_time)
    {
        // 调用渲染系统的每帧更新（如更新相机矩阵、提交绘制命令等）
        g_runtime_global_context.m_render_system->tick(delta_time);
        return true;  // 返回true表示渲染成功（可用于错误处理）
    }

    // FPS计算相关成员变量
    const float SammiEngine::s_fps_alpha = 1.f / 100;  // 平滑系数（指数移动平均）

    // 计算并平滑FPS（避免数值剧烈波动）
    void SammiEngine::calculateFPS(float delta_time)
    {
        m_frame_count++;  // 帧计数器递增

        if (m_frame_count == 1)
        {
            // 第一帧直接使用当前delta_time作为平均时长（初始值）
            m_average_duration = delta_time;
        }
        else
        {
            // 后续帧使用指数平滑算法：新值 = 旧值*(1-α) + 当前值*α
            // α=1/100表示保留99%的历史值，1%的当前值，使FPS变化更平滑
            m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
        }

        // FPS = 1 / 平均每帧时长（避免除以0，理论上average_duration>0）
        m_fps = static_cast<int>(1.f / m_average_duration);
    }
}

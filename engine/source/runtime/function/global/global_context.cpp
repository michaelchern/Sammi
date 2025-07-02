#include "runtime/function/global/global_context.h"                // 全局上下文实现

// 包含所有子系统头文件
#include "core/log/log_system.h"                                   // 日志系统
#include "runtime/engine.h"                                        // 引擎主循环

#include "runtime/platform/file_service/file_service.h"            // 文件系统服务

#include "runtime/resource/asset_manager/asset_manager.h"          // 资产管理
#include "runtime/resource/config_manager/config_manager.h"        // 配置管理

// 各功能子系统
#include "runtime/function/framework/world/world_manager.h"        // 场景世界管理
#include "runtime/function/input/input_system.h"                   // 输入系统
#include "runtime/function/particle/particle_manager.h"            // 粒子系统
//#include "runtime/function/physics/physics_manager.h"              // 物理系统
#include "runtime/function/render/debugdraw/debug_draw_manager.h"  // 调试绘制
#include "runtime/function/render/render_debug_config.h"           // 渲染调试配置
#include "runtime/function/render/render_system.h"                 // 渲染系统
#include "runtime/function/render/window_system.h"                 // 窗口系统

namespace Sammi
{
    // 定义全局运行时上下文实例（在全局作用域）
    RuntimeGlobalContext g_runtime_global_context;

    // 创建并初始化所有全局子系统
    void RuntimeGlobalContext::startSystems(const std::string& config_file_path)
    {
        // 配置管理 (1) - 首先加载配置文件
        m_config_manager = std::make_shared<ConfigManager>();
        m_config_manager->initialize(config_file_path);  // 使用指定配置文件路径初始化

        // 文件系统 (2) - 提供基础文件访问服务
        m_file_system = std::make_shared<FileSystem>();

        // 日志系统 (3) - 现在可以记录日志
        m_logger_system = std::make_shared<LogSystem>();

        // 资产管理 (4) - 加载纹理、模型等资源
        m_asset_manager = std::make_shared<AssetManager>();

        // 物理系统 (5) - 初始化物理世界
        //m_physics_manager = std::make_shared<PhysicsManager>();
        //m_physics_manager->initialize();

        // 世界管理 (6) - 创建和管理游戏场景和实体
        m_world_manager = std::make_shared<WorldManager>();
        m_world_manager->initialize();

        // 窗口系统 (7) - 创建应用程序窗口
        m_window_system = std::make_shared<WindowSystem>();
        WindowCreateInfo window_create_info;  // 默认窗口参数
        m_window_system->initialize(window_create_info);

        // 输入系统 (8) - 处理键盘鼠标等输入设备
        m_input_system = std::make_shared<InputSystem>();
        m_input_system->initialize();

        // 粒子系统 (9) - 管理粒子效果
        m_particle_manager = std::make_shared<ParticleManager>();
        m_particle_manager->initialize();

        // 渲染系统 (10) - 核心渲染管线（注意初始化顺序依赖窗口）
        m_render_system = std::make_shared<RenderSystem>();
        RenderSystemInitInfo render_init_info;
        render_init_info.window_system = m_window_system;  // 传递窗口系统引用
        m_render_system->initialize(render_init_info);

        // 调试绘制系统 (11) - 用于渲染调试信息
        m_debugdraw_manager = std::make_shared<DebugDrawManager>();
        m_debugdraw_manager->initialize();

        // 渲染调试配置 (12) - 管理渲染调试选项
        m_render_debug_config = std::make_shared<RenderDebugConfig>();
    }

    // 销毁所有子系统（按逆初始化顺序）
    void RuntimeGlobalContext::shutdownSystems()
    {
        // 销毁顺序与初始化相反（后初始化的先销毁）
        m_render_debug_config.reset();  // (12) 先销毁渲染调试配置

        m_debugdraw_manager.reset();    // (11) 然后销毁调试绘制

        // 渲染系统 (10)
        m_render_system->clear();       // 执行自定义清理逻辑
        m_render_system.reset();        // 释放资源

        m_window_system.reset();        // (7) 关闭窗口系统（释放前会销毁窗口）

        // 世界管理 (6)
        m_world_manager->clear();       // 清除所有场景实体
        m_world_manager.reset();

        // 物理系统 (5)
        //m_physics_manager->clear();     // 清理物理世界
        //m_physics_manager.reset();

        // 输入系统 (8)
        m_input_system->clear();        // 断开输入设备
        m_input_system.reset();

        m_asset_manager.reset();        // (4) 释放所有加载的资源

        m_logger_system.reset();        // (3) 最后关闭日志系统（确保能记录所有日志）

        m_file_system.reset();          // (2) 关闭文件服务

        m_config_manager.reset();       // (1) 清除配置数据

        m_particle_manager.reset();     // (9) 关闭粒子系统
    }
}
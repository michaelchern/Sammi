#pragma once

#include <memory>  // 智能指针支持
#include <string>  // 字符串处理

namespace Sammi
{
    // 前置声明所有子系统类（避免头文件包含依赖）
    class LogSystem;
    class InputSystem;
    class PhysicsManager;
    class FileSystem;
    class AssetManager;
    class ConfigManager;
    class WorldManager;
    class RenderSystem;
    class WindowSystem;
    class ParticleManager;
    class DebugDrawManager;
    class RenderDebugConfig;

    struct EngineInitParams;  // 引擎初始化参数结构体（前置声明）

    /// 管理所有全局系统的生命周期和创建/销毁顺序
    class RuntimeGlobalContext
    {
    public:
        // 创建所有全局系统并初始化（传入配置文件路径）
        void startSystems(const std::string& config_file_path);

        // 销毁所有全局系统（按正确顺序）
        void shutdownSystems();

    public:
        // 各子系统智能指针（按初始化顺序排列）
        std::shared_ptr<LogSystem>         m_logger_system;        // 日志系统（应最先初始化）
        std::shared_ptr<InputSystem>       m_input_system;         // 输入系统
        std::shared_ptr<FileSystem>        m_file_system;          // 文件系统（基础资源访问）
        std::shared_ptr<AssetManager>      m_asset_manager;        // 资产管理（模型/纹理等）
        std::shared_ptr<ConfigManager>     m_config_manager;       // 配置管理（读取运行时配置）
        std::shared_ptr<WorldManager>      m_world_manager;        // 场景世界管理（实体/组件）
        std::shared_ptr<PhysicsManager>    m_physics_manager;      // 物理系统管理
        std::shared_ptr<WindowSystem>      m_window_system;        // 窗口系统（创建和管理窗口）
        std::shared_ptr<RenderSystem>      m_render_system;        // 渲染系统（核心渲染管线）
        std::shared_ptr<ParticleManager>   m_particle_manager;     // 粒子系统管理（特效）
        std::shared_ptr<DebugDrawManager>  m_debugdraw_manager;    // 调试绘制管理器
        std::shared_ptr<RenderDebugConfig> m_render_debug_config;  // 渲染调试配置
    };

    // 声明全局运行时上下文实例（在cpp文件中定义）
    extern RuntimeGlobalContext g_runtime_global_context;
}
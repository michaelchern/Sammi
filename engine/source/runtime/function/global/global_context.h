#pragma once

#include <memory>
#include <string>

namespace Sammi
{
    // 前置声明所有子系统类（仅声明类名，无需完整定义）
    // 这些类代表引擎的核心功能模块，具体实现在其他源文件中

    class LogSystem;          // 日志系统（全局日志记录与管理）
    class InputSystem;        // 输入系统（处理键盘/鼠标等输入设备）
    class PhysicsManager;     // 物理管理器（模拟物理世界，如刚体、碰撞）
    class FileSystem;         // 文件系统（管理文件读写、资源路径）
    class AssetManager;       // 资产管理器（缓存/加载模型、纹理等资源）
    class ConfigManager;      // 配置管理器（解析配置文件，如窗口尺寸、物理参数）
    class WorldManager;       // 世界管理器（管理游戏对象/场景，维护场景图）
    class RenderSystem;       // 渲染系统（处理3D/2D图形渲染）
    class WindowSystem;       // 窗口系统（创建/管理游戏窗口，处理窗口事件）
    class ParticleManager;    // 粒子管理器（管理粒子效果，如火焰、烟雾）
    class DebugDrawManager;   // 调试绘制管理器（显示碰撞体、物理边界等调试信息）
    class RenderDebugConfig;  // 渲染调试配置（控制调试功能开关，如显示网格）

    // 引擎初始化参数结构体（当前代码未展开，可能包含额外初始化选项）
    // 例如：是否启用调试模式、自定义资源路径等（预留扩展）
    struct EngineInitParams;

    /// 管理所有全局子系统的生命周期（创建、初始化、销毁）
    /// 作为引擎的"中枢"，提供统一入口访问所有核心功能模块
    class RuntimeGlobalContext
    {
    public:
        // 启动所有全局子系统（按依赖顺序初始化）
        // 参数：config_file_path - 配置文件路径（如"Config/Engine.ini"）
        void startSystems(const std::string& config_file_path);

        // 关闭所有全局子系统（按逆依赖顺序释放资源）
        void shutdownSystems();

    public:
        std::shared_ptr<LogSystem>         m_logger_system;
        std::shared_ptr<InputSystem>       m_input_system;
        std::shared_ptr<FileSystem>        m_file_system;
        std::shared_ptr<AssetManager>      m_asset_manager;
        std::shared_ptr<ConfigManager>     m_config_manager;
        std::shared_ptr<WorldManager>      m_world_manager;
        std::shared_ptr<PhysicsManager>    m_physics_manager;
        std::shared_ptr<WindowSystem>      m_window_system;
        std::shared_ptr<RenderSystem>      m_render_system;
        std::shared_ptr<ParticleManager>   m_particle_manager;
        std::shared_ptr<DebugDrawManager>  m_debugdraw_manager;
        std::shared_ptr<RenderDebugConfig> m_render_debug_config;
    };

    // 全局上下文实例（单例模式，全局唯一访问入口）
    // 其他模块通过此实例访问引擎核心功能（如获取窗口系统、输入系统等）
    extern RuntimeGlobalContext g_runtime_global_context;
}

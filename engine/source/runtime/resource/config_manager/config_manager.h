#pragma once

#include <filesystem>

namespace Sammi
{
    // 前置声明引擎初始化参数结构体（具体定义可能在其他文件中）
    // 用于引擎启动时传递额外初始化选项（如自定义配置路径、调试模式等）
    struct EngineInitParams;

    /// 管理引擎核心配置的加载与访问
    /// 负责解析配置文件（如INI/JSON），存储并提供全局配置参数（路径、资源URL等）
    class ConfigManager
    {
    public:
        /// 初始化配置管理器（从指定配置文件加载配置）
        /// @param config_file_path 配置文件路径（如"Config/SammiEditor.ini"）
        void initialize(const std::filesystem::path& config_file_path);

        // ------------------------- 路径获取接口 -------------------------
        /// 获取引擎根文件夹路径（所有其他路径的基准目录）
        /// @return 根文件夹路径（如"/Project/Root/"）
        const std::filesystem::path& getRootFolder() const;

        /// 获取资源文件夹路径（存放模型、纹理、材质等资源）
        /// @return 资源文件夹路径（如"/Project/Root/Assets/"）
        const std::filesystem::path& getAssetFolder() const;

        /// 获取模式文件夹路径（存放XML Schema等模式定义文件）
        /// @return 模式文件夹路径（如"/Project/Root/Schemas/"）
        const std::filesystem::path& getSchemaFolder() const;

        // 编辑器专用路径（仅在编辑器模式下使用）
        /// 获取编辑器大图标路径（用于窗口标题栏/任务栏）
        /// @return 大图标路径（如"/Editor/Resources/Icons/EditorLarge.ico"）
        const std::filesystem::path& getEditorBigIconPath() const;

        /// 获取编辑器小图标路径（用于文件资源管理器等小尺寸显示场景）
        /// @return 小图标路径（如"/Editor/Resources/Icons/EditorSmall.ico"）
        const std::filesystem::path& getEditorSmallIconPath() const;

        /// 获取编辑器字体路径（用于界面文本渲染）
        /// @return 字体路径（如"/Editor/Resources/Fonts/Roboto-Regular.ttf"）
        const std::filesystem::path& getEditorFontPath() const;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        // 物理调试渲染器专用路径（仅在启用该功能时有效）
        /// 获取Jolt物理引擎资源文件夹路径（存放碰撞形状、调试绘制资源等）
        /// @return Jolt物理资源路径（如"/Engine/Physics/Jolt/Assets/"）
        const std::filesystem::path& getJoltPhysicsAssetFolder() const;
#endif

        // ------------------------- 资源URL获取接口 -------------------------
        /// 获取默认世界URL（引擎启动时自动加载的场景文件路径）
        /// @return 默认世界URL（如"Scenes/MainLevel.sammi"）
        const std::string& getDefaultWorldUrl() const;

        /// 获取全局渲染资源URL（存放着色器、光照贴图等渲染相关资源）
        /// @return 全局渲染资源URL（如"Resources/Rendering/Global.pak"）
        const std::string& getGlobalRenderingResUrl() const;

        /// 获取全局粒子资源URL（存放粒子效果模板、发射器配置等）
        /// @return 全局粒子资源URL（如"Resources/Particles/Global.pak"）
        const std::string& getGlobalParticleResUrl() const;

    private:
        // ------------------------- 内部存储的配置值 -------------------------
        std::filesystem::path m_root_folder;             // 引擎根文件夹路径
        std::filesystem::path m_asset_folder;            // 资源文件夹路径
        std::filesystem::path m_schema_folder;           // 模式文件夹路径
        std::filesystem::path m_editor_big_icon_path;    // 编辑器大图标路径
        std::filesystem::path m_editor_small_icon_path;  // 编辑器小图标路径
        std::filesystem::path m_editor_font_path;        // 编辑器字体路径

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        std::filesystem::path m_jolt_physics_asset_folder;  // Jolt物理资源路径（条件编译）
#endif

        std::string m_default_world_url;         // 默认世界URL
        std::string m_global_rendering_res_url;  // 全局渲染资源URL
        std::string m_global_particle_res_url;   // 全局粒子资源URL
    };
}

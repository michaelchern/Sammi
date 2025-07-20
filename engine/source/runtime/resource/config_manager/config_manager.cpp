#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/engine.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace Sammi
{
    /// 从配置文件加载并解析引擎配置参数
    /// @param config_file_path 配置文件路径（如"Config/SammiEditor.ini"）
    void ConfigManager::initialize(const std::filesystem::path& config_file_path)
    {
        // 1. 打开配置文件（使用C++标准文件流）
        std::ifstream config_file(config_file_path);

        // 检查文件是否成功打开（关键错误处理，避免后续操作无效）
        if (!config_file.is_open())
        {
            LOG_ERROR("Failed to open config file: {}", config_file_path.string());
            return;
        }

        std::string config_line;  // 存储当前读取的配置行

        // 2. 逐行读取配置文件内容
        while (std::getline(config_file, config_line))
        {
            // 跳过空行或注释行（假设以'#'开头的行是注释）
            if (config_line.empty() || config_line.starts_with('#'))
                continue;

            // 3. 解析键值对（格式："键=值"）
            size_t seperate_pos = config_line.find_first_of('=');  // 查找'='分隔符位置

            // 检查分隔符有效性（必须在非首尾位置）
            if (seperate_pos > 0 && seperate_pos < (config_line.length() - 1))
            {
                // 提取键（分隔符前的子串）和值（分隔符后的子串，去除首尾空格）
                std::string name  = config_line.substr(0, seperate_pos);
                std::string value = config_line.substr(seperate_pos + 1, config_line.length() - seperate_pos - 1);
                if (name == "BinaryRootFolder")
                {
                    m_root_folder = config_file_path.parent_path() / value;
                }
                else if (name == "AssetFolder")
                {
                    m_asset_folder = m_root_folder / value;
                }
                else if (name == "SchemaFolder")
                {
                    m_schema_folder = m_root_folder / value;
                }
                else if (name == "DefaultWorld")
                {
                    m_default_world_url = value;
                }
                else if (name == "BigIconFile")
                {
                    m_editor_big_icon_path = m_root_folder / value;
                }
                else if (name == "SmallIconFile")
                {
                    m_editor_small_icon_path = m_root_folder / value;
                }
                else if (name == "FontFile")
                {
                    m_editor_font_path = m_root_folder / value;
                }
                else if (name == "GlobalRenderingRes")
                {
                    m_global_rendering_res_url = value;
                }
                else if (name == "GlobalParticleRes")
                {
                    m_global_particle_res_url = value;
                }
#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
                else if (name == "JoltAssetFolder")
                {
                    m_jolt_physics_asset_folder = m_root_folder / value;
                }
#endif
            }
        }
    }

    const std::filesystem::path& ConfigManager::getRootFolder() const { return m_root_folder; }

    const std::filesystem::path& ConfigManager::getAssetFolder() const { return m_asset_folder; }

    const std::filesystem::path& ConfigManager::getSchemaFolder() const { return m_schema_folder; }

    const std::filesystem::path& ConfigManager::getEditorBigIconPath() const { return m_editor_big_icon_path; }

    const std::filesystem::path& ConfigManager::getEditorSmallIconPath() const { return m_editor_small_icon_path; }

    const std::filesystem::path& ConfigManager::getEditorFontPath() const { return m_editor_font_path; }

    const std::string& ConfigManager::getDefaultWorldUrl() const { return m_default_world_url; }

    const std::string& ConfigManager::getGlobalRenderingResUrl() const { return m_global_rendering_res_url; }

    const std::string& ConfigManager::getGlobalParticleResUrl() const { return m_global_particle_res_url; }

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
    const std::filesystem::path& ConfigManager::getJoltPhysicsAssetFolder() const { return m_jolt_physics_asset_folder; }
#endif

}

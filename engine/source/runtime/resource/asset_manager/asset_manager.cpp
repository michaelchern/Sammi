#include "runtime/resource/asset_manager/asset_manager.h"    // 包含资产管理器头文件
#include "runtime/resource/config_manager/config_manager.h"  // 包含配置管理器
#include "runtime/function/global/global_context.h"          // 包含全局上下文
#include <filesystem>                                        // 文件系统库

namespace Sammi
{
    // 实现获取完整文件系统路径的方法
    // 参数：relative_path - 资源的相对路径（如"textures/character.png"）
    // 返回值：资源的完整绝对路径
    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        // 1. 从全局配置管理器获取根目录路径
        // 2. 将相对路径附加到根目录后
        // 3. 使用filesystem::absolute确保路径是绝对路径
        return std::filesystem::absolute(g_runtime_global_context.m_config_manager->getRootFolder() / relative_path);
    }
}
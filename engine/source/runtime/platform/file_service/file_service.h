#pragma once

#include <filesystem>  // 包含C++17标准库中的文件系统操作功能
#include <vector>  // 包含向量容器功能

namespace Sammi
{
    // 文件系统工具类
    class FileSystem 
    {
    public:
        // 获取指定目录中的所有文件路径
        // 参数: directory - 要扫描的目标目录路径
        // 返回值: 包含该目录下所有文件路径的向量
        std::vector<std::filesystem::path> getFiles(const std::filesystem::path& directory);
    };
}
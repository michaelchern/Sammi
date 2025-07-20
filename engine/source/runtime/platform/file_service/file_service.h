#pragma once

#include <filesystem>
#include <vector>

namespace Sammi
{
    // 封装文件系统操作的类，提供与文件/目录交互的功能接口
    class FileSystem 
    {
    public:
        /**
         * @brief 获取指定目录下的所有文件路径（不包含子目录中的文件）
         *
         * @param directory 目标目录的路径对象（std::filesystem::path类型）
         * @return std::vector<std::filesystem::path> 包含目标目录下所有文件路径的vector容器
         *
         * 功能说明：
         *  - 输入一个目录路径（如 "D:/data" 或 "/home/user/docs"）
         *  - 遍历该目录下的直接子文件（不递归子目录）
         *  - 将每个文件的路径以std::filesystem::path类型存入vector并返回
         * 注意：
         *  - 若目录不存在或无权限访问，可能返回空vector（具体行为依赖实现）
         *  - 路径格式支持跨平台（Windows/Linux/macOS）
         */
        std::vector<std::filesystem::path> getFiles(const std::filesystem::path& directory);
    };
}

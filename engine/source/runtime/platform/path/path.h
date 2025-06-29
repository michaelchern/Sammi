#pragma once

#include <filesystem>  // C++17 文件系统库
#include <string>      // 字符串操作
#include <tuple>       // 元组容器
#include <vector>      // 向量容器

namespace Sammi
{
    // 路径处理工具类
    // 提供静态方法处理文件系统路径相关操作
    class Path
    {
    public:
        // 获取文件相对于目录的相对路径
        // 参数:
        //   directory - 基准目录路径
        //   file_path - 目标文件路径
        // 返回值: 相对路径对象
        static const std::filesystem::path getRelativePath(const std::filesystem::path& directory,
                                                           const std::filesystem::path& file_path);

        // 将文件路径分割为路径片段列表
        // 示例: "project/assets/textures/wall.png" -> ["project", "assets", "textures", "wall.png"]
        // 参数:
        //   file_path - 目标文件路径
        // 返回值: 包含路径片段的字符串向量
        static const std::vector<std::string> getPathSegments(const std::filesystem::path& file_path);

        // 获取文件扩展名的三个组成部分
        // 参数:
        //   file_path - 目标文件路径
        // 返回值: 包含三个部分的元组:
        //   std::tuple<主扩展名, 副扩展名, 完整扩展名>
        // 示例: 
        //   "config.json" -> ("json", "", ".json")
        //   "archive.tar.gz" -> ("gz", "tar", ".tar.gz")
        //   "script.shader" -> ("shader", "", ".shader")
        static const std::tuple<std::string, std::string, std::string>
        getFileExtensions(const std::filesystem::path& file_path);

        // 获取不带扩展名的文件纯名称
        // 示例: "character.png" -> "character"
        //        "level.scene.json" -> "level"
        // 参数:
        //   full_name - 完整文件名（可包含路径或仅文件名）
        // 返回值: 纯文件名字符串
        static const std::string getFilePureName(const std::string);
    };
}
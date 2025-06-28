#pragma once

#include <filesystem>  // 文件系统库（C++17）
#include <vector>      // 向量容器

/// 命名空间路径类型定义
/// 使用字符串向量表示嵌套命名空间结构
/// 示例: {"Sammi", "Editor"} 表示命名空间 Sammi::Editor
typedef std::vector<std::string> Namespace;
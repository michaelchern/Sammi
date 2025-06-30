#pragma once

#include "common/namespace.h"  // 命名空间支持
#include "cursor/cursor.h"     // Clang AST游标支持

// 工具函数命名空间
// 提供元数据处理、字符串操作和文件操作等实用功能
namespace Utils
{
    // 转换CXString到std::string（自动释放资源）
    void toString(const CXString& str, std::string& output);

    // 获取类型的全限定名（包含命名空间）
    std::string getQualifiedName(const CursorType& type);

    // 使用显示名和命名空间构造全限定名
    std::string getQualifiedName(const std::string& display_name, const Namespace& current_namespace);

    // 重载：使用游标获取全限定名
    std::string getQualifiedName(const Cursor& cursor, const Namespace& current_namespace);

    // 格式化限定名（替换特殊字符）
    std::string formatQualifiedName(std::string& source_string);

    // 计算相对路径（处理"./"和"../"）
    fs::path makeRelativePath(const fs::path& from, const fs::path& to);

    // 报告致命错误并退出程序
    void fatalError(const std::string& error);

    // 模板：检查两个范围是否相等（前向声明）
    template<typename A, typename B>
    bool rangeEqual(A startA, A endA, B startB, B endB);

    // 使用分隔符分割字符串
    std::vector<std::string> split(std::string input, std::string pat);

    // 从完整路径提取文件名
    std::string getFileName(std::string path);

    // 移除成员变量前缀"m_"
    std::string getNameWithoutFirstM(std::string& name);

    // 获取不包含命名空间的类型名
    std::string getTypeNameWithoutNamespace(const CursorType& type);

    // 从容器类型提取模板参数类型
    std::string getNameWithoutContainer(std::string name);

    // 移除字符串两端的引号
    std::string getStringWithoutQuot(std::string input);

    // 字符串替换（子字符串版本）
    std::string replace(std::string& source_string, std::string sub_string, const std::string new_string);

    // 字符串替换（字符版本）
    std::string replace(std::string& source_string, char taget_char, const char new_char);

    // 转换字符串为大写
    std::string toUpper(std::string& source_string);

    // 使用分隔符拼接字符串列表
    std::string join(std::vector<std::string> context_list, std::string separator);

    // 修剪字符串两端指定字符
    std::string trim(std::string& source_string, const std::string trim_chars);

    // 加载文件内容到字符串
    std::string loadFile(std::string path);

    // 保存字符串到文件（自动创建目录）
    void saveFile(const std::string& outpu_string, const std::string& output_file);

    // 全局替换所有匹配的子字符串
    void replaceAll(std::string& resource_str, std::string sub_str, std::string new_str);

    // 规范化路径字符串
    unsigned long formatPathString(const std::string& path_string, std::string& out_string);

    // 转换为大驼峰命名法
    std::string convertNameToUpperCamelCase(const std::string& name, std::string pat);
}

// 包含模板函数和内联函数的实现
#include "meta_utils.hpp"
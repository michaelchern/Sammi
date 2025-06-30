#pragma once

#include "common/precompiled.h"    // 预编译头文件（包含常用标准库）
#include "common/namespace.h"      // 命名空间工具类
#include "common/schema_module.h"  // 模式模块定义

#include "cursor/cursor.h"         // Clang AST游标封装

#include "generator/generator.h"                // 代码生成器接口
#include "template_manager/template_manager.h"  // 模板管理器

class Class;  // 前置声明

// 元数据解析器类（核心组件）
// 负责：
//   - 解析C++源代码文件
//   - 构建类/字段/方法等元数据模型
//   - 管理代码生成过程
class MetaParser
{
public:
    // 静态初始化方法（在程序启动时调用）
    static void prepare(void);

    // 构造函数（初始化解析环境）
    // project_input_file: 主输入文件路径（项目文件或单个源文件）
    // include_file_path: 头文件搜索路径
    // include_path: 额外的包含路径
    // include_sys: 系统包含路径
    // module_name: 输出模块名称
    // is_show_errors: 是否显示Clang解析错误
    MetaParser(const std::string project_input_file,
               const std::string include_file_path,
               const std::string include_path,
               const std::string include_sys,
               const std::string module_name,
               bool              is_show_errors);

    // 析构函数（清理资源）
    ~MetaParser(void);

    // 清理工作（关闭资源）
    void finish(void);

    // 执行解析过程
    // return: 0成功，非零错误码
    int parse(void);

    // 调用所有注册的生成器输出文件
    void generateFiles(void);

private:
    // 输入文件路径
    std::string m_project_input_file;

    // 工作路径集合
    std::vector<std::string> m_work_paths;

    // 输出模块名称
    std::string m_module_name;

    // 系统包含路径
    std::string m_sys_include;

    // 主包含文件名
    std::string m_source_include_file_name;

    // Clang索引（管理全局状态）
    CXIndex m_index;

    // Clang翻译单元（解析后的AST）
    CXTranslationUnit m_translation_unit;

    // 类型映射表（类型名 → 类型定义）
    std::unordered_map<std::string, std::string>  m_type_table;

    // 模式模块映射（模块名 → 模块对象）
    std::unordered_map<std::string, SchemaModule> m_schema_modules;

    std::vector<const char*> arguments =
    {
        {   "-x", "c++",                // 语言类型
            "-std=c++11",               // C++标准
            "-D__REFLECTION_PARSER__",  // 自定义宏
            "-DNDEBUG",                 // 禁用调试
            "-D__clang__",              // 标记为clang
            "-w",                       // 忽略所有警告
            "-MG",                      // 生成依赖关系时忽略丢失的头文件
            "-M",                       // 输出依赖关系规则
            "-ferror-limit=0",          // 显示全部错误
            "-o clangLog.txt"           // 输出日志文件
        }
    };

    // 代码生成器列表（负责输出不同类型文件）
    std::vector<Generator::GeneratorInterface*> m_generators;

    // 是否显示错误信息（控制错误输出）
    bool m_is_show_errors;

private:
    // 解析项目文件（构建AST）
    bool parseProject(void);

    // 递归构建类AST（处理命名空间）
    // cursor: 当前AST节点
    // current_namespace: 当前命名空间栈
    void buildClassAST(const Cursor& cursor, Namespace& current_namespace);

    // 获取包含文件路径（搜索工作路径）
    // name: 文件名
    std::string getIncludeFile(std::string name);
};
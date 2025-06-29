#pragma once

#include "generator/generator.h"  // 包含基类定义

namespace Generator
{
    // 反射代码生成器（继承自GeneratorInterface）
    class ReflectionGenerator : public GeneratorInterface
    {
    public:
        // 禁止默认构造函数
        ReflectionGenerator() = delete;

        // 带参数构造函数
        // source_directory: 项目源代码根目录
        // get_include_function: 包含文件路径获取函数
        ReflectionGenerator(std::string source_directory, std::function<std::string(std::string)> get_include_function);

        // 实现基类接口：生成反射代码的主方法
        virtual int  generate(std::string path, SchemaModule schema) override;

        // 完成生成后的清理工作
        virtual void finish() override;

        // 析构函数
        virtual ~ReflectionGenerator() override;

    protected:
        // 重写基类方法：准备生成环境（创建输出目录等）
        virtual void        prepareStatus(std::string path) override;

        // 重写基类方法：处理输出文件名策略
        virtual std::string processFileName(std::string path) override;

    private:
        std::vector<std::string> m_head_file_list;   // 收集生成的头部文件(.h/.hpp)
        std::vector<std::string> m_sourcefile_list;  // 收集生成的源文件(.cpp)
    };
}
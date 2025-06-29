#pragma once

#include "generator/generator.h"  // 包含基类定义

namespace Generator
{
    // 序列化代码生成器（继承自GeneratorInterface）
    class SerializerGenerator : public GeneratorInterface
    {
    public:
        // 禁止默认构造函数
        SerializerGenerator() = delete;

        // 带参数构造函数
        // source_directory: 项目源代码根目录
        // get_include_function: 包含文件路径获取函数
        SerializerGenerator(std::string source_directory, std::function<std::string(std::string)> get_include_function);

        // 实现基类接口：生成序列化代码的主方法
        virtual int generate(std::string path, SchemaModule schema) override;

        // 完成生成后的清理和汇总工作
        virtual void finish() override;

        // 析构函数
        virtual ~SerializerGenerator() override;

    protected:
        // 重写基类方法：准备输出目录
        virtual void prepareStatus(std::string path) override;

        // 重写基类方法：处理输出文件名策略
        virtual std::string processFileName(std::string path) override;

    private:
        Mustache::data m_class_defines {Mustache::data::type::list};      // 收集所有类的序列化定义数据
        Mustache::data m_include_headfiles {Mustache::data::type::list};  // 收集所有需要的头文件包含
    };
}
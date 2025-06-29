#pragma once

#include "common/schema_module.h"  // 包含模块元数据结构定义

#include <functional>  // 用于std::function
#include <string>  // 字符串处理

namespace Generator
{
    // 抽象生成器接口类
    class GeneratorInterface
    {
    public:
        // 构造函数：初始化生成器基本参数
        GeneratorInterface(std::string out_path,  // 生成代码的输出目录
                           std::string root_path,  // 项目根目录（用于路径计算）
                           std::function<std::string(std::string)> get_include_func)  // 包含文件路径获取函数
            : m_out_path(out_path),  // 初始化输出路径
              m_root_path(root_path),  // 初始化项目根路径
              m_get_include_func(get_include_func)   // 初始化包含文件获取函数
        {}

        // 纯虚函数：生成代码的主入口，必须由子类实现
        // path: 输出文件路径
        // schema: 模块元数据
        virtual int  generate(std::string path, SchemaModule schema) = 0;

        // 虚函数：生成结束时的清理工作（默认空实现）
        virtual void finish() {};

        // 虚析构函数（确保正确销毁子类）
        virtual ~GeneratorInterface() {};

    protected:
        // 以下成员提供代码生成的具体步骤，有默认实现也可被子类重写

        // 准备输出状态（创建目录等）
        virtual void prepareStatus(std::string path);

        // 生成类级别的渲染数据
        virtual void genClassRenderData(std::shared_ptr<Class> class_temp, Mustache::data& class_def);

        // 生成类字段的渲染数据
        virtual void genClassFieldRenderData(std::shared_ptr<Class> class_temp, Mustache::data& feild_defs);

        // 生成类方法的渲染数据
        virtual void genClassMethodRenderData(std::shared_ptr<Class> class_temp, Mustache::data& method_defs);

        // 纯虚函数：处理输出文件名（必须由子类实现）
        virtual std::string processFileName(std::string path) = 0;

        // 生成器配置数据

        std::string                             m_out_path {"gen_src"};  // 输出目录（如"gen_src"）
        std::string                             m_root_path;             // 项目根路径（用于计算相对路径）

        // 函数对象：根据类型名获取头文件包含路径
        // 例如：输入"std::vector" 返回 "<vector>"
        std::function<std::string(std::string)> m_get_include_func;
    };
}
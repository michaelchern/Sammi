#include "generator/serializer_generator.h"
#include "common/precompiled.h"
#include "language_types/class.h"

namespace Generator
{
    // 构造函数：初始化序列化生成器
    SerializerGenerator::SerializerGenerator(std::string                             source_directory,
                                             std::function<std::string(std::string)> get_include_function) :
        // 调用基类构造函数，设置输出目录为source_directory/_generated/serializer
        GeneratorInterface(source_directory + "/_generated/serializer", source_directory, get_include_function)
    {
        // 准备生成状态（创建输出目录、加载模板等）
        prepareStatus(m_out_path);
    }

    // 准备生成器状态
    void SerializerGenerator::prepareStatus(std::string path)
    {
        // 调用基类方法准备状态
        GeneratorInterface::prepareStatus(path);
        // 加载必要的Mustache模板文件
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allSerializer.h");
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allSerializer.ipp");
        TemplateManager::getInstance()->loadTemplates(m_root_path, "commonSerializerGenFile");
        return;
    }

    // 处理文件名：将源文件名转换为序列化文件名
    // 例如：path/to/class.cpp -> _generated/serializer/class.serializer.gen.h
    std::string SerializerGenerator::processFileName(std::string path)
    {
        auto relativeDir = fs::path(path).filename().replace_extension("serializer.gen.h").string();
        return m_out_path + "/" + relativeDir;
    }

    // 生成序列化代码的核心方法
    int SerializerGenerator::generate(std::string path, SchemaModule schema)
    {
        // 获取处理后的输出文件路径
        std::string file_path = processFileName(path);

        // 准备Mustache渲染数据
        Mustache::data muatache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);  // 包含的头文件列表
        Mustache::data class_defines(Mustache::data::type::list);  // 类定义列表

        // 添加当前处理的源文件作为包含的头文件（使用相对路径）
        include_headfiles.push_back(Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, path).string()));

        // 遍历模式中的所有类
        for (auto class_temp : schema.classes)
        {
            // 跳过不需要序列化的类
            if (!class_temp->shouldCompileFields())
                continue;

            Mustache::data class_def;
            // 生成类的渲染数据（填充class_def）
            genClassRenderData(class_temp, class_def);

            // 处理基类：确保基类的序列化头文件被包含
            for (int index = 0; index < class_temp->m_base_classes.size(); ++index)
            {
                auto include_file = m_get_include_func(class_temp->m_base_classes[index]->name);
                if (!include_file.empty())
                {
                    // 获取基类序列化文件的路径
                    auto include_file_base = processFileName(include_file);
                    // 避免重复包含（当前文件生成的序列化文件不包含自身）
                    if (file_path != include_file_base)
                    {
                        // 将基类的序列化文件添加到包含列表
                        include_headfiles.push_back(Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, include_file_base).string()));
                    }
                }
            }

            // 处理字段依赖
            for (auto field : class_temp->m_fields)
            {
                if (!field->shouldCompile())
                    continue;

                // 处理vector类型字段
                if (field->m_type.find("std::vector") == 0)
                {
                    // 获取vector元素类型对应的头文件
                    auto include_file = m_get_include_func(field->m_name);  // 注：这里可能应该是field->m_type
                    if (!include_file.empty())
                    {
                        auto include_file_base = processFileName(include_file);
                        if (file_path != include_file_base)
                        {
                            include_headfiles.push_back(Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, include_file_base).string()));
                        }
                    }
                }
                // 其他普通字段的处理（此处没有具体实现）
            }

            // 添加类定义到列表
            class_defines.push_back(class_def);
            // 同时添加到成员变量，用于最后的汇总文件
            m_class_defines.push_back(class_def);
        }

        // 设置最终的渲染数据
        muatache_data.set("class_defines", class_defines);
        muatache_data.set("include_headfiles", include_headfiles);

        // 使用模板渲染单个序列化文件
        std::string render_string = TemplateManager::getInstance()->renderByTemplate("commonSerializerGenFile", muatache_data);
        Utils::saveFile(render_string, file_path);

        // 将生成的序列化文件添加到全局包含列表
        m_include_headfiles.push_back(Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, file_path).string()));
        return 0;
    }

    // 在所有文件处理完成后调用，生成汇总文件
    void SerializerGenerator::finish()
    {
        Mustache::data mustache_data;
        // 设置全局类定义和头文件包含
        mustache_data.set("class_defines", m_class_defines);
        mustache_data.set("include_headfiles", m_include_headfiles);

        // 生成汇总头文件
        std::string render_string = TemplateManager::getInstance()->renderByTemplate("allSerializer.h", mustache_data);
        Utils::saveFile(render_string, m_out_path + "/all_serializer.h");

        // 生成汇总实现文件
        render_string = TemplateManager::getInstance()->renderByTemplate("allSerializer.ipp", mustache_data);
        Utils::saveFile(render_string, m_out_path + "/all_serializer.ipp");
    }

    // 析构函数
    SerializerGenerator::~SerializerGenerator() {}
}
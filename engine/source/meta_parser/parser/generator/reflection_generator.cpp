// 包含必要的头文件
#include "common/precompiled.h"
#include "generator/reflection_generator.h"
#include "language_types/class.h"
#include "template_manager/template_manager.h"

#include <map>
#include <set>

namespace Generator
{
    // 构造函数：初始化反射生成器
    ReflectionGenerator::ReflectionGenerator(std::string                             source_directory,
                                             std::function<std::string(std::string)> get_include_function) :
        // 调用基类构造函数，设置输出目录为source_directory/_generated/reflection
        GeneratorInterface(source_directory + "/_generated/reflection", source_directory, get_include_function)
    {
        // 准备生成状态（创建输出目录、加载模板等）
        prepareStatus(m_out_path);
    }

    // 准备生成器状态（创建输出目录、加载模板）
    void ReflectionGenerator::prepareStatus(std::string path)
    {
        // 调用基类方法准备状态（如创建输出目录）
        GeneratorInterface::prepareStatus(path);
        // 加载必要的Mustache模板文件
        TemplateManager::getInstance()->loadTemplates(m_root_path, "commonReflectionFile");  // 单个反射文件模板
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allReflectionFile");     // 汇总反射文件模板
        return;
    }

    // 处理文件名：将源文件名转换为反射文件名
    // 例如：path/to/class.cpp -> _generated/reflection/class.reflection.gen.h
    std::string ReflectionGenerator::processFileName(std::string path)
    {
        // 获取文件名（不含路径）并替换扩展名
        auto relativeDir = fs::path(path).filename().replace_extension("reflection.gen.h").string();
        return m_out_path + "/" + relativeDir;
    }

    // 生成反射代码的核心方法
    int ReflectionGenerator::generate(std::string path, SchemaModule schema)
    {
        static const std::string vector_prefix = "std::vector<";

        // 获取处理后的输出文件路径
        std::string file_path = processFileName(path);

        // 准备Mustache渲染数据
        Mustache::data mustache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);  // 包含的头文件列表
        Mustache::data class_defines(Mustache::data::type::list);      // 类定义列表

        // 添加当前处理的源文件作为包含的头文件（使用相对路径）
        include_headfiles.push_back(Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, path).string()));

        // 记录需要生成的类名
        std::map<std::string, bool> class_names;

        // 遍历模式中的所有类
        for (auto class_temp : schema.classes)
        {
            // 跳过不需要编译的类
            if (!class_temp->shouldCompile())
                continue;

            class_names.insert_or_assign(class_temp->getClassName(), false);
            class_names[class_temp->getClassName()] = true;  // 标记该类需要生成

            // 存储字段信息
            std::vector<std::string> field_names;
            // 存储向量类型信息（类型名 -> (格式化的类型名, 元素类型)）
            std::map<std::string, std::pair<std::string, std::string>> vector_map;

            Mustache::data class_def;
            Mustache::data vector_defines(Mustache::data::type::list);

            // 生成类的渲染数据（填充class_def）
            genClassRenderData(class_temp, class_def);

            // 遍历类的所有字段
            for (auto field : class_temp->m_fields)
            {
                // 跳过不需要编译的字段
                if (!field->shouldCompile())
                    continue;

                field_names.emplace_back(field->m_name);  // 记录字段名

                // 检查是否是vector类型
                bool is_array = field->m_type.find(vector_prefix) == 0;
                if (is_array)
                {
                    // 处理vector类型信息
                    std::string array_useful_name = field->m_type;
                    Utils::formatQualifiedName(array_useful_name);  // 格式化类型名
                    std::string item_type = field->m_type;

                    // 提取vector中的元素类型
                    item_type = Utils::getNameWithoutContainer(item_type);

                    // 存储处理后的类型信息
                    vector_map[field->m_type] = std::make_pair(array_useful_name, item_type);
                }
            }

            // 如果存在vector字段，添加到渲染数据
            if (vector_map.size() > 0)
            {
                if (nullptr == class_def.get("vector_exist"))
                {
                    class_def.set("vector_exist", true);  // 设置标记
                }

                // 遍历所有vector类型
                for (auto vector_item : vector_map)
                {
                    std::string array_useful_name = vector_item.second.first;
                    std::string item_type = vector_item.second.second;
                    Mustache::data vector_define;
                    vector_define.set("vector_useful_name", array_useful_name);
                    vector_define.set("vector_type_name", vector_item.first);
                    vector_define.set("vector_element_type_name", item_type);
                    vector_defines.push_back(vector_define);
                }
            }
            class_def.set("vector_defines", vector_defines);  // 添加vector定义数据
            class_defines.push_back(class_def);               // 添加类定义
        }

        // 设置最终的渲染数据
        mustache_data.set("class_defines", class_defines);          // 所有类定义
        mustache_data.set("include_headfiles", include_headfiles);  // 包含的头文件列表

        // 生成大写驼峰格式的文件标识符（用于宏定义）
        std::string tmp = Utils::convertNameToUpperCamelCase(fs::path(path).stem().string(), "_");
        mustache_data.set("sourefile_name_upper_camel_case", tmp);

        // 使用模板渲染结果
        std::string render_string = TemplateManager::getInstance()->renderByTemplate("commonReflectionFile", mustache_data);

        // 保存渲染结果到文件
        Utils::saveFile(render_string, file_path);

        // 记录已处理的文件信息
        m_sourcefile_list.emplace_back(tmp);  // 存储文件标识符
        m_head_file_list.emplace_back(Utils::makeRelativePath(m_root_path, file_path).string());  // 存储头文件相对路径

        return 0;
    }

    // 在所有文件处理完成后调用，生成汇总文件
    void ReflectionGenerator::finish()
    {
        Mustache::data mustache_data;
        Mustache::data include_headfiles = Mustache::data::type::list;  // 包含的头文件列表
        Mustache::data sourefile_names   = Mustache::data::type::list;  // 源文件名列表

        // 遍历所有生成的头文件
        for (auto& head_file : m_head_file_list)
        {
            include_headfiles.push_back(Mustache::data("headfile_name", head_file));
        }

        // 遍历所有源文件标识符
        for (auto& sourefile_name_upper_camel_case : m_sourcefile_list)
        {
            sourefile_names.push_back(Mustache::data("sourefile_name_upper_camel_case", sourefile_name_upper_camel_case));
        }

        // 设置汇总文件的渲染数据
        mustache_data.set("include_headfiles", include_headfiles);
        mustache_data.set("sourefile_names", sourefile_names);

        // 渲染汇总文件模板
        std::string render_string = TemplateManager::getInstance()->renderByTemplate("allReflectionFile", mustache_data);

        // 保存汇总文件（所有反射信息的入口）
        Utils::saveFile(render_string, m_out_path + "/all_reflection.h");
    }

    // 析构函数
    ReflectionGenerator::~ReflectionGenerator() {}
}
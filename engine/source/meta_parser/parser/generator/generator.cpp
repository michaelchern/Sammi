#include "common/precompiled.h"    // 预编译头文件
#include "generator/generator.h"   // 生成器基类声明
#include "language_types/class.h"  // 类元数据结构定义

namespace Generator
{
    // 准备生成目录：确保输出路径存在
    void GeneratorInterface::prepareStatus(std::string path)
    {
        if (!fs::exists(path))  // 检查路径是否存在
        {
            fs::create_directories(path);  // 递归创建整个目录树
        }
    }

    // 生成类级别的渲染数据（核心函数）
    void GeneratorInterface::genClassRenderData(std::shared_ptr<Class> class_temp,  // 输入的类元数据
                                                Mustache::data& class_def)          // 输出的Mustache模板数据
    {
        // 设置基础类信息
        class_def.set("class_name", class_temp->getClassName());  // 类名
        class_def.set("class_base_class_size", std::to_string(class_temp->m_base_classes.size()));  // 基类数量
        class_def.set("class_need_register", true);  // 标记需要注册（可能用于序列化/反射）

        // 处理基类信息
        if (class_temp->m_base_classes.size() > 0)
        {
            Mustache::data class_base_class_defines(Mustache::data::type::list);  // 基类列表
            class_def.set("class_has_base", true);  // 标记存在基类

            // 遍历所有基类
            for (int index = 0; index < class_temp->m_base_classes.size(); ++index)
            {
                Mustache::data class_base_class_def;
                class_base_class_def.set("class_base_class_name", class_temp->m_base_classes[index]->name);  // 基类名称
                class_base_class_def.set("class_base_class_index", std::to_string(index));  // 基类索引
                class_base_class_defines.push_back(class_base_class_def);  // 添加到列表
            }
            class_def.set("class_base_class_defines", class_base_class_defines);  // 设置基类列表数据
        }

        // 生成字段数据
        Mustache::data class_field_defines = Mustache::data::type::list;  // 字段列表
        genClassFieldRenderData(class_temp, class_field_defines);  // 填充字段数据
        class_def.set("class_field_defines", class_field_defines);  // 设置到主数据
   
        // 生成方法数据
        Mustache::data class_method_defines = Mustache::data::type::list;  // 方法列表
        genClassMethodRenderData(class_temp, class_method_defines);  // 填充方法数据
        class_def.set("class_method_defines", class_method_defines);  // 设置到主数据
    }

    // 生成类字段的渲染数据
    void GeneratorInterface::genClassFieldRenderData(std::shared_ptr<Class> class_temp,  // 输入的类元数据
                                                     Mustache::data& feild_defs)  // 输出的字段模板数据
    {
        static const std::string vector_prefix = "std::vector<";  // 向量类型前缀

        // 遍历所有字段
        for (auto& field : class_temp->m_fields)
        {
            if (!field->shouldCompile())  // 跳过不需要编译的字段
                continue;
            Mustache::data filed_define;  // 单个字段的数据容器

            // 设置字段基本属性
            filed_define.set("class_field_name", field->m_name);  // 字段名
            filed_define.set("class_field_type", field->m_type);  // 字段类型（字符串形式）
            filed_define.set("class_field_display_name", field->m_display_name);  // 显示名（可能带命名空间）

            // 检测是否是std::vector类型
            bool is_vector = field->m_type.find(vector_prefix) == 0;
            filed_define.set("class_field_is_vector", is_vector);  // 设置向量类型标记

            // 添加到字段列表
            feild_defs.push_back(filed_define);
        }
    }

    // 生成类方法的渲染数据
    void GeneratorInterface::genClassMethodRenderData(std::shared_ptr<Class> class_temp,  // 输入的类元数据
                                                      Mustache::data& method_defs)  // 输出的方法模板数据
    {
       for (auto& method : class_temp->m_methods)  // 遍历所有方法
        {
            if (!method->shouldCompile())  // 跳过不需要编译的方法
                continue;

            Mustache::data method_define;  // 单个方法的数据容器
            method_define.set("class_method_name", method->m_name);  // 设置方法名
            method_defs.push_back(method_define);  // 添加到方法列表
        }
    }
}
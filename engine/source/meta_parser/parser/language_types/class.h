#pragma once

#include "type_info.h"  // 包含类型信息基类
#include "field.h"      // 包含字段信息类
#include "method.h"     // 包含方法信息类

// 表示类的基类信息
struct BaseClass
{
    // 构造函数：从Clang的Cursor对象解析基类信息
    BaseClass(const Cursor& cursor);

    std::string name;  // 基类的名称
};

// 表示类信息的类，继承自TypeInfo
class Class : public TypeInfo
{
    // 友元声明：允许Field、Method和MetaParser访问私有成员
    friend class Field;
    friend class Method;
    friend class MetaParser;

public:
    // 构造函数：从Clang的Cursor对象和当前命名空间解析类信息
    Class(const Cursor& cursor, const Namespace& current_namespace);

    // 判断此类是否需要编译（生成元数据代码）
    virtual bool shouldCompile(void) const;

    // 判断此类的字段是否需要编译
    bool shouldCompileFields(void) const;
    // 判断此类的方法是否需要编译
    bool shouldCompileMethods(void) const;

    // 模板别名：定义共享指针的向量类型
    template<typename T>
    using SharedPtrVector = std::vector<std::shared_ptr<T>>;

    // 获取类的名称
    std::string getClassName(void);

    // 基类列表（使用共享指针的向量）
    SharedPtrVector<BaseClass> m_base_classes;

public:
    // 类名（不含命名空间）
    std::string m_name;

    // 完全限定名（包含命名空间）
    std::string m_qualified_name;

    // 字段（成员变量）列表
    SharedPtrVector<Field>  m_fields;
    // 方法（成员函数）列表
    SharedPtrVector<Method> m_methods;

    // 显示名称（用于代码生成或界面显示）
    std::string m_display_name;

    // 判断此类是否可访问（public访问级别）
    bool isAccessible(void) const;
};
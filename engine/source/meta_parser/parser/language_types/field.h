#pragma once

#include "type_info.h"  // 基础类型信息基类

class Class;  // 前置声明Class类，避免循环依赖

// 表示一个类/结构体的成员字段（成员变量）
class Field : public TypeInfo  // 继承自类型信息基类
{

public:
    // 构造函数
    // cursor: 解析器游标，用于获取源码信息
    // current_namespace: 当前所在命名空间
    // parent: 指向所属Class对象的指针（可选）
    Field(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);

    virtual ~Field(void) {}  // 虚析构函数（空实现）

    // 检查该字段是否需要生成代码
    bool shouldCompile(void) const;

public:  // 成员变量（公开访问，实际项目中可能需要封装）
    bool m_is_const;             // 标记字段是否为const类型

    Class* m_parent;             // 指向该字段所属的父类Class对象

    std::string m_name;          // 字段的原始名称（源码中的标识符）
    std::string m_display_name;  // 字段的显示名称（可能用于UI或文档）
    std::string m_type;          // 字段类型的字符串表示（如"int", "std::string"）

    std::string m_default;       // 字段的默认值文本（如果有初始化值）

    // 检查字段是否可从外部访问（如public权限）
    bool isAccessible(void) const;
};
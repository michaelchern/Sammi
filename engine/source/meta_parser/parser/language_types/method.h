#pragma once

#include "type_info.h"  // 基础类型信息基类

class Class;  // 前置声明Class类，避免循环依赖

// 表示一个类/结构体的成员方法
class Method : public TypeInfo  // 继承自类型信息基类
{

public:
    // 构造函数
    // cursor: 解析器游标，用于获取源码信息
    // current_namespace: 当前所在命名空间
    // parent: 指向所属Class对象的指针（可选）
    Method(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);

    virtual ~Method(void) {}  // 虚析构函数（空实现）

    // 检查该方法是否需要参与代码生成
    bool shouldCompile(void) const;

public:

    Class* m_parent;     // 指向该方法所属的父类Class对象

    std::string m_name;  // 方法的名称（函数名）

    // 检查方法是否可从外部访问（public/protected/private等权限判断）
    bool isAccessible(void) const;
};
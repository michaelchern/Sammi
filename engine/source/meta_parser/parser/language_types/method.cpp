#include "common/precompiled.h"  // 预编译头文件
#include "class.h"               // 类定义头文件
#include "method.h"              // 方法定义头文件

// 方法构造函数：从AST节点解析方法信息
Method::Method(const Cursor& cursor, const Namespace& current_namespace, Class* parent)
    : TypeInfo(cursor, current_namespace),  // 初始化基类
      m_parent(parent),                     // 指向所属父类
      m_name(cursor.getSpelling())          // 原始方法名
{
    // 注意：此处仅初始化基础信息
    // 实际实现可能需要解析：
    // - 返回类型 (m_return_type)
    // - 参数列表 (m_parameters)
    // - 虚函数/重写等特性标记
}

// 判断是否应参与代码生成
bool Method::shouldCompile(void) const
{
    return isAccessible();  // 直接委托给可访问性检查
}

// 判断方法是否可访问/是否需要生成
bool Method::isAccessible(void) const
{
    // 逻辑分解：
    // 情况1：父类启用方法生成 && 方法未被禁用
    bool case1 = (m_parent->m_meta_data.getFlag(NativeProperty::Methods) ||  // 父类标记生成所有方法
                  m_parent->m_meta_data.getFlag(NativeProperty::All)) &&     // 或父类标记生成所有内容
                 !m_meta_data.getFlag(NativeProperty::Disable);              // 且方法未标记禁用

    // 情况2：父类启用白名单模式 && 方法被显式启用
    bool case2 = m_parent->m_meta_data.getFlag(NativeProperty::WhiteListMethods) &&  // 父类标记白名单模式
                 m_meta_data.getFlag(NativeProperty::Enable);                        // 且方法标记启用

    return case1 || case2;  // 满足任一条件即可访问
}
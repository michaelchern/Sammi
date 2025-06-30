#include "common/precompiled.h"  // 预编译头文件
#include "type_info.h"           // 类型信息基类头文件

// TypeInfo构造函数：初始化类型基础信息
TypeInfo::TypeInfo(const Cursor& cursor, const Namespace& current_namespace)
    : m_meta_data(cursor),                                     // 从游标解析元数据（注解属性）
      m_enabled(m_meta_data.getFlag(NativeProperty::Enable)),  // 是否启用（根据@Enable注解）
      m_root_cursor(cursor),                                   // 保存原始AST节点游标
      m_namespace(current_namespace)                           // 记录所属命名空间
{
    // 此处可添加额外初始化逻辑：
    // - 解析中文别名（如@Alias("位置")）
    // - 设置其他元数据相关属性

}

// 获取类型关联的元数据集合（只读）
const MetaInfo& TypeInfo::getMetaData(void) const
{
    return m_meta_data;
}

// 获取声明该类型的源文件路径
std::string TypeInfo::getSourceFile(void) const
{
    return m_root_cursor.getSourceFile();
}

// 获取类型所属的命名空间
Namespace TypeInfo::getCurrentNamespace() const
{
    return m_namespace;
}

// 获取底层AST游标（可修改版本）
Cursor& TypeInfo::getCurosr()
{
    return m_root_cursor;
}
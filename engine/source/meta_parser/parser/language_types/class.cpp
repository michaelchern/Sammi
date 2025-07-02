
#include "common/precompiled.h"  // 预编译头文件（包含常用头文件）
#include "class.h"               // 类声明头文件

// BaseClass 构造函数：从基类游标中提取类型名
BaseClass::BaseClass(const Cursor& cursor) : name(Utils::getTypeNameWithoutNamespace(cursor.getType())) {}

// Class 构造函数：解析类定义
Class::Class(const Cursor& cursor, const Namespace& current_namespace)
    : TypeInfo(cursor, current_namespace),                                     // 初始化基类
      m_name(cursor.getDisplayName()),                                         // 原始显示名（含空格）
      m_qualified_name(Utils::getTypeNameWithoutNamespace(cursor.getType())),  // 无命名空间的限定名
      m_display_name(Utils::getNameWithoutFirstM(m_qualified_name))            // 最终显示名（移除特殊前缀）
{
    // 清理类名格式
    Utils::replaceAll(m_name, " ", "");          // 移除所有空格
    Utils::replaceAll(m_name, "Sammi::", "");    // 移除引擎命名空间前缀

    // 遍历类成员（AST子节点）
    for (auto& child : cursor.getChildren())
    {
        switch (child.getKind())  // 根据AST节点类型分类处理
        {
            // 处理基类继承
            case CXCursor_CXXBaseSpecifier:
            {
                auto base_class = new BaseClass(child);   // 创建基类对象
                m_base_classes.emplace_back(base_class);  // 添加到基类列表
            }
            break;

            // 处理字段声明
            case CXCursor_FieldDecl:
                m_fields.emplace_back(new Field(child, current_namespace, this));  // 创建字段对象
                break;

            // 处理方法声明
            case CXCursor_CXXMethod:
                m_methods.emplace_back(new Method(child, current_namespace, this));  // 创建方法对象

            default:
                break;  // 忽略其他类型节点
        }
    }
}

// 判断是否需要为此类生成代码
bool Class::shouldCompile(void) const
{
    return shouldCompileFields() || shouldCompileMethods();  // 需要生成字段或方法代码
}

// 判断是否需要生成字段代码
bool Class::shouldCompileFields(void) const
{
    // 根据元数据标记判断：
    // - NativeProperty::All:             生成所有内容
    // - NativeProperty::Fields:          生成所有字段
    // - NativeProperty::WhiteListFields: 仅生成白名单字段
    return m_meta_data.getFlag(NativeProperty::All) ||
           m_meta_data.getFlag(NativeProperty::Fields) ||
           m_meta_data.getFlag(NativeProperty::WhiteListFields);
}

// 判断是否需要生成方法代码
bool Class::shouldCompileMethods(void) const
{
    // 逻辑同字段，对应方法相关标记
    return m_meta_data.getFlag(NativeProperty::All) ||
           m_meta_data.getFlag(NativeProperty::Methods) ||
           m_meta_data.getFlag(NativeProperty::WhiteListMethods);
}

// 获取处理后的类名（清理格式后）
std::string Class::getClassName(void)
{
    return m_name;
}

// 检查类是否可访问（根据m_enabled标志）
bool Class::isAccessible(void) const
{
    return m_enabled;
}
#include "common/precompiled.h"  // 预编译头文件
#include "class.h"               // 类定义头文件
#include "field.h"               // 字段定义头文件

// 字段构造函数：从AST节点解析字段信息
Field::Field(const Cursor& cursor, const Namespace& current_namespace, Class* parent)
    : TypeInfo(cursor, current_namespace),                          // 初始化基类
      m_is_const(cursor.getType().IsConst()),                       // 检测const修饰符
      m_parent(parent),                                             // 指向所属父类
      m_name(cursor.getSpelling()),                                 // 原始字段名
      m_display_name(Utils::getNameWithoutFirstM(m_name)),          // 显示名（移除前缀）
      m_type(Utils::getTypeNameWithoutNamespace(cursor.getType()))  // 原始类型名
{
    // 清理类型名字符串
    Utils::replaceAll(m_type, " ", "");        // 移除所有空格
    Utils::replaceAll(m_type, "Sammi::", "");  // 移除引擎命名空间前缀

    // 获取元数据中的默认值（若有）
    auto ret_string = Utils::getStringWithoutQuot(m_meta_data.getProperty("default"));
    m_default       = ret_string;  // 存储处理后的默认值
}

// 判断是否应参与代码生成
bool Field::shouldCompile(void) const
{
    return isAccessible();  // 直接委托给可访问性检查
}

// 判断字段是否可访问/是否需要生成
bool Field::isAccessible(void) const
{
    // 逻辑分解：
    // 情况1：父类启用字段生成 && 字段未被禁用
    bool case1 = (m_parent->m_meta_data.getFlag(NativeProperty::Fields) ||  // 父类标记生成所有字段
                  m_parent->m_meta_data.getFlag(NativeProperty::All)) &&    // 或父类标记生成所有内容
                 !m_meta_data.getFlag(NativeProperty::Disable);             // 且字段未标记禁用

    // 情况2：父类启用白名单模式 && 字段被显式启用
    bool case2 = m_parent->m_meta_data.getFlag(NativeProperty::WhiteListFields) &&  // 父类标记白名单模式
                 m_meta_data.getFlag(NativeProperty::Enable);                       // 且字段标记启用

    return case1 || case2;  // 满足任一条件即可访问
}
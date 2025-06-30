#include "common/precompiled.h"  // 预编译头文件
#include "parser/parser.h"       // 解析器基类（包含Cursor等定义）
#include "meta_info.h"           // 元信息类声明

// MetaInfo构造函数：从AST节点解析元数据（注解）
MetaInfo::MetaInfo(const Cursor& cursor)
{
    // 遍历光标（AST节点）的所有子节点
    for (auto& child : cursor.getChildren())
    {
        // 只处理注解属性节点（跳过其他类型）
        if (child.getKind() != CXCursor_AnnotateAttr)
            continue;

        // 从注解属性中提取键值对列表
        auto properties = extractProperties(child);

        // 将提取的属性存入成员Map
        for (auto& prop : properties)
            m_properties[prop.first] = prop.second;
    }
}

// 获取指定属性的值（字符串形式）
std::string MetaInfo::getProperty(const std::string& key) const
{
    auto search = m_properties.find(key);  // 查找键

    // 未找到时返回空字符串
    return search == m_properties.end() ? "" : search->second;
}

// 检查是否存在指定键（不论值为何）
bool MetaInfo::getFlag(const std::string& key) const
{
    return m_properties.find(key) != m_properties.end();
}

// 从注解节点提取属性键值对
std::vector<MetaInfo::Property> MetaInfo::extractProperties(const Cursor& cursor) const
{
    std::vector<Property> ret_list;  // 返回结果列表

    // 获取注解节点的完整显示名
    auto propertyList = cursor.getDisplayName();

    // 将多属性字符串分割为单个属性列表
    auto&& properties = Utils::split(propertyList, ",");

    // 预定义空白字符集（用于修剪字符串）
    static const std::string white_space_string = " \t\r\n";

    // 处理每个独立属性项
    for (auto& property_item : properties)
    {
        // 分割键值对（使用冒号分隔符）
        auto&& item_details = Utils::split(property_item, ":");

        // 处理键名：修剪空白字符
        auto&& temp_string  = Utils::trim(item_details[0], white_space_string);

        // 跳过空键名项
        if (temp_string.empty())
        {
            continue;
        }

        // 处理值（存在值时修剪，不存在时置空）
        std::string value = "";
        if (item_details.size() > 1)
        {
            value = Utils::trim(item_details[1], white_space_string);
        }

        // 保存为键值对
        ret_list.emplace_back(temp_string, value);
    }
    return ret_list;
}
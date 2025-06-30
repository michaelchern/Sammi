#pragma once

#include "cursor/cursor.h"  // 包含游标定义（用于AST节点访问）

// 元数据信息容器类
// 负责解析和存储源代码中的注解（attributes）
class MetaInfo
{
public:
    // 构造函数：从AST游标节点解析元数据
    MetaInfo(const Cursor& cursor);

    // 获取指定键的属性值
    // key: 属性名称
    // return: 属性值字符串（不存在时返回空字符串）
    std::string getProperty(const std::string& key) const;

    // 检查是否存在指定键（用于布尔标记型属性）
    // key: 标记名称
    // return: 存在返回true，否则false
    bool getFlag(const std::string& key) const;

private:
    // 属性键值对定义（first=键，second=值）
    typedef std::pair<std::string, std::string> Property;

    // 存储属性映射表（键→值）
    std::unordered_map<std::string, std::string> m_properties;

private:
    // 从游标节点提取所有属性键值对
    // cursor: 注解节点的游标
    // return: 解析出的属性列表
    std::vector<Property> extractProperties(const Cursor& cursor) const;
};
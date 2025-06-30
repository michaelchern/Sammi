#pragma once

#include "common/namespace.h"  // 命名空间工具类
#include "cursor/cursor.h"     // 源码解析游标类
#include "meta/meta_info.h"    // 元数据容器类
#include "parser/parser.h"     // 解析器基类

// 类型信息基类（所有类型相关类的抽象基类）
class TypeInfo
{
public:
    // 构造函数（必须由派生类调用）
    // cursor: 源码解析的当前位置游标
    // current_namespace: 当前所在的命名空间
    TypeInfo(const Cursor& cursor, const Namespace& current_namespace);

    virtual ~TypeInfo(void) {}  // 虚析构函数（确保派生类正确析构）

    // 获取关联的元数据集合
    const MetaInfo& getMetaData(void) const;

    // 获取声明该类型的源文件路径
    std::string getSourceFile(void) const;

    // 获取声明时的完整命名空间
    Namespace getCurrentNamespace() const;

    // 获取底层源码解析游标（读写权限）
    Cursor& getCurosr();

protected:
    MetaInfo m_meta_data;    // 元数据集合（注解/属性等）
    bool m_enabled;          // 标记是否启用该类型（代码生成/解析）
    std::string m_alias_cn;  // 中文别名（本地化支持）
    Namespace m_namespace;   // 类型所属的命名空间

private:
    Cursor m_root_cursor;  // 类型定义的原始游标（AST起始位置）
};
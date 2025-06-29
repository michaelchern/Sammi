#pragma once

#include "cursor_type.h"
// 包含自定义的光标类型声明

// 封装libclang的CXCursor对象
class Cursor
{
public:
    // 类型别名定义
    typedef std::vector<Cursor> List;  // 光标对象列表类型
    typedef CXCursorVisitor Visitor;   // libclang访问器函数类型

    // 构造函数：接受原始libclang光标句柄
    Cursor(const CXCursor& handle);

    //=== 基本属性查询 ===//
    // 获取光标类型(kind) - 如函数声明、类定义等
    CXCursorKind getKind(void) const;

    // 获取标识符文本(如变量名、函数名)
    std::string getSpelling(void) const;

    // 获取带上下文的显示名(如带命名空间的类名)
    std::string getDisplayName(void) const;

    // 获取源文件路径
    std::string getSourceFile(void) const;

    // 判断当前光标是否为定义(而不仅是声明)
    bool isDefinition(void) const;
    
    // 获取关联类型(如变量类型、函数返回类型)
    CursorType getType(void) const;

    //=== AST遍历接口 ===//
    // 获取所有子节点(返回列表)
    List getChildren(void) const;

    // 递归遍历子节点(性能更优的原生方式)
    void visitChildren(Visitor visitor, void* data = nullptr);

private:
    CXCursor m_handle;  // libclang原生的光标句柄
};
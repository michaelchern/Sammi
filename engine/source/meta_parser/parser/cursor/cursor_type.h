#pragma once

class Cursor;  // 前置声明（避免循环依赖）

// 封装libclang的CXType类型表示
class CursorType
{
public:
    // 构造函数：接受原始的CXType句柄
    CursorType(const CXType& handle);

    // 获取类型的可显示名称（如"int"、"std::vector<std::string>"）
    std::string GetDisplayName(void) const;

    // 获取类型的参数数量（适用于函数类型或模板类型）
    int GetArgumentCount(void) const;

    // 获取指定索引的类型参数（索引从0开始）
    CursorType GetArgument(unsigned index) const;

    // 获取规范类型（去除typedef等修饰后的底层类型）
    CursorType GetCanonicalType(void) const;

    // 获取该类型的声明光标（指向声明该类型的AST节点）
    Cursor GetDeclaration(void) const;

    // 获取类型种类（如CXType_Int, CXType_FunctionProto等）
    CXTypeKind GetKind(void) const;

    // 判断类型是否被const修饰
    bool IsConst(void) const;

private:
    CXType m_handle;  // 底层的libclang类型句柄
};
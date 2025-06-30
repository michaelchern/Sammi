#include "common/precompiled.h" // 包含项目预编译头
#include "cursor.h"             // 需要Cursor类的声明（用于GetDeclaration返回值）
#include "cursor_type.h"        // 当前类的声明

// 构造函数：保存原始类型句柄
CursorType::CursorType(const CXType& handle) : m_handle(handle) {}

// 获取类型的可读名称（如"const std::map<int, bool>&"）
std::string CursorType::GetDisplayName(void) const
{
    std::string display_name;
    // 调用libclang获取类型拼写，通过工具函数转换为std::string
    Utils::toString(clang_getTypeSpelling(m_handle), display_name);
    return display_name;
}

// 获取函数类型或模板类型的参数数量
// 对于非函数/模板类型返回0
int CursorType::GetArgumentCount(void) const
{
    return clang_getNumArgTypes(m_handle);
}

// 获取指定索引的参数类型（索引从0开始）
CursorType CursorType::GetArgument(unsigned index) const
{
    return clang_getArgType(m_handle, index);  // 返回封装的参数类型
}

// 获取规范类型（去除typedef/alias修饰的底层类型）
CursorType CursorType::GetCanonicalType(void) const
{
    return clang_getCanonicalType(m_handle);
}

// 获取声明该类型的AST节点（如类/函数声明）
Cursor CursorType::GetDeclaration(void) const
{
    // 注意：基础类型（如int）无声明，返回无效Cursor
    return clang_getTypeDeclaration(m_handle);
}

// 获取类型的基本分类（如指针、数组、结构体等）
CXTypeKind CursorType::GetKind(void) const
{
    return m_handle.kind;  // 直接返回底层结构的kind字段
}

// 检查类型是否有const限定符
bool CursorType::IsConst(void) const
{
    // 使用libclang的限定符检查函数
    return clang_isConstQualifiedType(m_handle) ? true : false;
}
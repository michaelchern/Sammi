#include "common/precompiled.h"
#include "meta/meta_utils.h"     // 包含元工具函数（如字符串转换）
#include "cursor.h"              // 包含当前类的声明

// 构造函数：接受并保存libclang的原始光标句柄
Cursor::Cursor(const CXCursor& handle) : m_handle(handle) {}

// 获取光标种类（如函数声明、类定义等）
CXCursorKind Cursor::getKind(void) const
{
    return m_handle.kind;  // 直接返回底层结构的kind字段
}

// 获取光标代表的标识符名称（如变量名、函数名）
std::string Cursor::getSpelling(void) const
{
    std::string spelling;
    // 使用工具函数将CXString安全转换为std::string
    Utils::toString(clang_getCursorSpelling(m_handle), spelling);

    return spelling;
}

// 获取带上下文的显示名称（如带命名空间的类名）
std::string Cursor::getDisplayName(void) const
{
    std::string display_name;

    Utils::toString(clang_getCursorDisplayName(m_handle), display_name);

    return display_name;
}

// 获取光标对应的源文件路径
std::string Cursor::getSourceFile(void) const
{
    // 获取光标标识符的源代码范围
    auto range = clang_Cursor_getSpellingNameRange(m_handle, 0, 0);
    auto start = clang_getRangeStart(range);  // 获取范围的起始位置

    CXFile   file;
    unsigned line, column, offset;
    // 解析位置获取文件信息
    clang_getFileLocation(start, &file, &line, &column, &offset);

    std::string filename;
    Utils::toString(clang_getFileName(file), filename);  // 文件名转换
    return filename;
}

// 判断光标是否为定义（而不仅是声明）
bool Cursor::isDefinition(void) const
{
    return clang_isCursorDefinition(m_handle);
}

// 获取光标的类型（如函数返回类型、变量类型等）
CursorType Cursor::getType(void) const
{
    return clang_getCursorType(m_handle);
}

// 获取光标的所有子节点（列表形式）
Cursor::List Cursor::getChildren(void) const
{
    List children;  // 存储子节点的容器

    auto visitor = [](CXCursor cursor, CXCursor parent, CXClientData data)
        {
            auto container = static_cast<List*>(data);  // 转换用户数据
            container->emplace_back(cursor);            // 添加子光标到容器

            // 检查是否到达预处理结束标记（停止遍历）
            if (cursor.kind == CXCursor_LastPreprocessing)
                return CXChildVisit_Break;  // 结束遍历

            return CXChildVisit_Continue;  // 继续遍历
        };

    // 遍历子节点，填充children容器
    clang_visitChildren(m_handle, visitor, &children);

    return children;
}

// 高效递归遍历子节点（直接使用libclang的访问器模式）
void Cursor::visitChildren(Visitor visitor, void* data)
{
    clang_visitChildren(m_handle, visitor, data);
}
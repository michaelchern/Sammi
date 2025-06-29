#include "common/precompiled.h" // ������ĿԤ����ͷ
#include "cursor.h"   // ��ҪCursor�������������GetDeclaration����ֵ��
#include "cursor_type.h"  // ��ǰ�������

// ���캯��������ԭʼ���;��
CursorType::CursorType(const CXType& handle) : m_handle(handle) {}

// ��ȡ���͵Ŀɶ����ƣ���"const std::map<int, bool>&"��
std::string CursorType::GetDisplayName(void) const
{
    std::string display_name;
    // ����libclang��ȡ����ƴд��ͨ�����ߺ���ת��Ϊstd::string
    Utils::toString(clang_getTypeSpelling(m_handle), display_name);
    return display_name;
}

// ��ȡ�������ͻ�ģ�����͵Ĳ�������
// ���ڷǺ���/ģ�����ͷ���0
int CursorType::GetArgumentCount(void) const
{
    return clang_getNumArgTypes(m_handle);
}

// ��ȡָ�������Ĳ������ͣ�������0��ʼ��
CursorType CursorType::GetArgument(unsigned index) const
{
    return clang_getArgType(m_handle, index);  // ���ط�װ�Ĳ�������
}

// ��ȡ�淶���ͣ�ȥ��typedef/alias���εĵײ����ͣ�
CursorType CursorType::GetCanonicalType(void) const
{
    return clang_getCanonicalType(m_handle);
}

// ��ȡ���������͵�AST�ڵ㣨����/����������
Cursor CursorType::GetDeclaration(void) const
{
    // ע�⣺�������ͣ���int����������������ЧCursor
    return clang_getTypeDeclaration(m_handle);
}

// ��ȡ���͵Ļ������ࣨ��ָ�롢���顢�ṹ��ȣ�
CXTypeKind CursorType::GetKind(void) const
{
    return m_handle.kind;  // ֱ�ӷ��صײ�ṹ��kind�ֶ�
}

// ��������Ƿ���const�޶���
bool CursorType::IsConst(void) const
{
    // ʹ��libclang���޶�����麯��
    return clang_isConstQualifiedType(m_handle) ? true : false;
}
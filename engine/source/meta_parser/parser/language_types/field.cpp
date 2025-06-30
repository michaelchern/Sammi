#include "common/precompiled.h"  // Ԥ����ͷ�ļ�
#include "class.h"               // �ඨ��ͷ�ļ�
#include "field.h"               // �ֶζ���ͷ�ļ�

// �ֶι��캯������AST�ڵ�����ֶ���Ϣ
Field::Field(const Cursor& cursor, const Namespace& current_namespace, Class* parent)
    : TypeInfo(cursor, current_namespace),                          // ��ʼ������
      m_is_const(cursor.getType().IsConst()),                       // ���const���η�
      m_parent(parent),                                             // ָ����������
      m_name(cursor.getSpelling()),                                 // ԭʼ�ֶ���
      m_display_name(Utils::getNameWithoutFirstM(m_name)),          // ��ʾ�����Ƴ�ǰ׺��
      m_type(Utils::getTypeNameWithoutNamespace(cursor.getType()))  // ԭʼ������
{
    // �����������ַ���
    Utils::replaceAll(m_type, " ", "");        // �Ƴ����пո�
    Utils::replaceAll(m_type, "Sammi::", "");  // �Ƴ����������ռ�ǰ׺

    // ��ȡԪ�����е�Ĭ��ֵ�����У�
    auto ret_string = Utils::getStringWithoutQuot(m_meta_data.getProperty("default"));
    m_default       = ret_string;  // �洢������Ĭ��ֵ
}

// �ж��Ƿ�Ӧ�����������
bool Field::shouldCompile(void) const
{
    return isAccessible();  // ֱ��ί�и��ɷ����Լ��
}

// �ж��ֶ��Ƿ�ɷ���/�Ƿ���Ҫ����
bool Field::isAccessible(void) const
{
    // �߼��ֽ⣺
    // ���1�����������ֶ����� && �ֶ�δ������
    bool case1 = (m_parent->m_meta_data.getFlag(NativeProperty::Fields) ||  // ���������������ֶ�
                  m_parent->m_meta_data.getFlag(NativeProperty::All)) &&    // ������������������
                 !m_meta_data.getFlag(NativeProperty::Disable);             // ���ֶ�δ��ǽ���

    // ���2���������ð�����ģʽ && �ֶα���ʽ����
    bool case2 = m_parent->m_meta_data.getFlag(NativeProperty::WhiteListFields) &&  // �����ǰ�����ģʽ
                 m_meta_data.getFlag(NativeProperty::Enable);                       // ���ֶα������

    return case1 || case2;  // ������һ�������ɷ���
}
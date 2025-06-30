#pragma once

#include "common/namespace.h"  // �����ռ乤����
#include "cursor/cursor.h"     // Դ������α���
#include "meta/meta_info.h"    // Ԫ����������
#include "parser/parser.h"     // ����������

// ������Ϣ���ࣨ�������������ĳ�����ࣩ
class TypeInfo
{
public:
    // ���캯������������������ã�
    // cursor: Դ������ĵ�ǰλ���α�
    // current_namespace: ��ǰ���ڵ������ռ�
    TypeInfo(const Cursor& cursor, const Namespace& current_namespace);

    virtual ~TypeInfo(void) {}  // ������������ȷ����������ȷ������

    // ��ȡ������Ԫ���ݼ���
    const MetaInfo& getMetaData(void) const;

    // ��ȡ���������͵�Դ�ļ�·��
    std::string getSourceFile(void) const;

    // ��ȡ����ʱ�����������ռ�
    Namespace getCurrentNamespace() const;

    // ��ȡ�ײ�Դ������α꣨��дȨ�ޣ�
    Cursor& getCurosr();

protected:
    MetaInfo m_meta_data;    // Ԫ���ݼ��ϣ�ע��/���Եȣ�
    bool m_enabled;          // ����Ƿ����ø����ͣ���������/������
    std::string m_alias_cn;  // ���ı��������ػ�֧�֣�
    Namespace m_namespace;   // ���������������ռ�

private:
    Cursor m_root_cursor;  // ���Ͷ����ԭʼ�α꣨AST��ʼλ�ã�
};
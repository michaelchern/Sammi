#pragma once

// ��׼�����
#include <memory>  // ����ָ��֧��
#include <string>  // �ַ�������
#include <vector>  // ��̬��������

namespace Sammi
{
    // ǰ�������ļ��ڵ���
    class EditorFileNode;
    // �����ļ��ڵ��������ͣ�����ָ��������
    using EditorFileNodeArray = std::vector<std::shared_ptr<EditorFileNode>>;

    /// �ļ��ڵ�ṹ����ʾ�ļ�ϵͳ�е��ļ�/�ļ��У�
    struct EditorFileNode
    {
        std::string         m_file_name;    // �ļ����ļ�������
        std::string         m_file_type;    // �ļ����ͣ���"Folder"��"Texture"�ȣ�
        std::string         m_file_path;    // �ļ�������·��
        int                 m_node_depth;   // �ڵ����ļ����е���ȣ����ڵ�Ϊ0��
        EditorFileNodeArray m_child_nodes;  // �ӽڵ��б��������νṹ��

        // Ĭ�Ϲ��캯��
        EditorFileNode() = default;

        // ���������캯��
        EditorFileNode(const std::string& name,
                       const std::string& type,
                       const std::string& path,
                       int depth) :
                       m_file_name(name),
                       m_file_type(type),
                       m_file_path(path),
                       m_node_depth(depth)
        {}
    };

    /// �༭���ļ����񣨹���༭���е��ļ�ϵͳ����
    class EditorFileService
    {
        EditorFileNodeArray m_file_node_array;  // �ļ��ڵ㼯�ϣ����������ļ�����
        EditorFileNode      m_root_node{ "asset", "Folder", "asset", -1 };// ���ڵ㣨Ĭ��·��Ϊ"asset"��

    private:
        // ��ȡ�����ڵ�ĸ��ڵ�ָ��
        EditorFileNode* getParentNodePtr(EditorFileNode* file_node);

        // ��鲢�����ļ��ڵ����飨���ڹ����ļ�����
        bool checkFileArray(EditorFileNode* file_node);

    public:
        // ��ȡ�༭���ļ����ĸ��ڵ�
        EditorFileNode* getEditorRootNode() { return m_file_node_array.empty() ? nullptr : m_file_node_array[0].get(); }

        // ����������Դ�ļ����ṹ
        void buildEngineFileTree();
    };
}
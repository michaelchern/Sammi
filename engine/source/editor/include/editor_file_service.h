#pragma once

// 标准库包含
#include <memory>  // 智能指针支持
#include <string>  // 字符串处理
#include <vector>  // 动态数组容器

namespace Sammi
{
    // 前向声明文件节点类
    class EditorFileNode;
    // 定义文件节点数组类型（智能指针向量）
    using EditorFileNodeArray = std::vector<std::shared_ptr<EditorFileNode>>;

    /// 文件节点结构（表示文件系统中的文件/文件夹）
    struct EditorFileNode
    {
        std::string         m_file_name;    // 文件或文件夹名称
        std::string         m_file_type;    // 文件类型（如"Folder"、"Texture"等）
        std::string         m_file_path;    // 文件的完整路径
        int                 m_node_depth;   // 节点在文件树中的深度（根节点为0）
        EditorFileNodeArray m_child_nodes;  // 子节点列表（构成树形结构）

        // 默认构造函数
        EditorFileNode() = default;

        // 参数化构造函数
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

    /// 编辑器文件服务（管理编辑器中的文件系统树）
    class EditorFileService
    {
        EditorFileNodeArray m_file_node_array;  // 文件节点集合（包含整个文件树）
        EditorFileNode      m_root_node{ "asset", "Folder", "asset", -1 };// 根节点（默认路径为"asset"）

    private:
        // 获取给定节点的父节点指针
        EditorFileNode* getParentNodePtr(EditorFileNode* file_node);

        // 检查并处理文件节点数组（用于构建文件树）
        bool checkFileArray(EditorFileNode* file_node);

    public:
        // 获取编辑器文件树的根节点
        EditorFileNode* getEditorRootNode() { return m_file_node_array.empty() ? nullptr : m_file_node_array[0].get(); }

        // 构建引擎资源文件树结构
        void buildEngineFileTree();
    };
}
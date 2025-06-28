#include "editor/include/editor_file_service.h"              // 文件服务头文件

#include "runtime/platform/file_service/file_service.h"      // 文件系统接口
#include "runtime/platform/path/path.h"                      // 路径处理工具

#include "runtime/resource/asset_manager/asset_manager.h"    // 资源管理器
#include "runtime/resource/config_manager/config_manager.h"  // 配置管理器

#include "runtime/function/global/global_context.h"          // 全局上下文

namespace Sammi
{
    /// 辅助函数：分割字符串并过滤特定子串
    std::vector<std::string> splitString(std::string input_string, const std::string& separator, const std::string& filter_string = "")
    {
        std::vector<std::string> output_string;
        int                      pos = input_string.find(separator);
        std::string              add_string;

        // 循环查找分隔符
        while (pos != std::string::npos)
        {
            add_string = input_string.substr(0, pos);
            if (!add_string.empty())
            {
                if (!filter_string.empty() && add_string == filter_string)
                {
                    // filter substring
                }
                else
                {
                    output_string.push_back(add_string);
                }
            }
            input_string.erase(0, pos + 1);
            pos = input_string.find(separator);
        }

        // 处理最后一段
        add_string = input_string;
        if (!add_string.empty())
        {
            output_string.push_back(add_string);
        }

        return output_string;
    }

    /// 构建引擎资源文件树结构
    void EditorFileService::buildEngineFileTree()
    {
        // 获取资源文件夹路径
        std::string                              asset_folder = g_runtime_global_context.m_config_manager->getAssetFolder().generic_string();

        // 获取资源文件夹下所有文件路径
        const std::vector<std::filesystem::path> file_paths = g_runtime_global_context.m_file_system->getFiles(asset_folder);

        // 存储所有文件的相对路径分段
        std::vector<std::vector<std::string>>    all_file_segments;

        // 处理每个文件路径
        for (const auto& path : file_paths)
        {
            // 获取相对路径并分段
            const std::filesystem::path& relative_path = Path::getRelativePath(asset_folder, path);
            all_file_segments.emplace_back(Path::getPathSegments(relative_path));
        }

        // 临时节点数组
        std::vector<std::shared_ptr<EditorFileNode>> node_array;

        // 重置文件节点数组
        m_file_node_array.clear();

        // 创建根节点
        auto root_node = std::make_shared<EditorFileNode>();
        *root_node     = m_root_node;
        m_file_node_array.push_back(root_node);

        // 遍历所有文件路径分段
        int all_file_segments_count = all_file_segments.size();
        for (int file_index = 0; file_index < all_file_segments_count; file_index++)
        {
            int depth = 0;
            node_array.clear();
            node_array.push_back(root_node);

            int file_segment_count = all_file_segments[file_index].size();
            for (int file_segment_index = 0; file_segment_index < file_segment_count; file_segment_index++)
            {
                // 创建新文件节点
                auto file_node         = std::make_shared<EditorFileNode>();
                file_node->m_file_name = all_file_segments[file_index][file_segment_index];

                // 设置节点类型
                if (depth < file_segment_count - 1)
                {
                    // 中间节点为文件夹
                    file_node->m_file_type = "Folder";
                }
                else
                {
                    // 最终节点为文件
                    const auto& extensions = Path::getFileExtensions(file_paths[file_index]);
                    file_node->m_file_type = std::get<0>(extensions);

                    if (file_node->m_file_type.size() == 0)
                        continue;

                    // 特殊处理JSON文件
                    if (file_node->m_file_type.compare(".json") == 0)
                    {
                        // 处理自定义类型文件
                        file_node->m_file_type = std::get<1>(extensions);
                        if (file_node->m_file_type.compare(".component") == 0)
                        {
                            // 组合扩展名
                            file_node->m_file_type = std::get<2>(extensions) + std::get<1>(extensions);
                        }
                    }

                    // 移除扩展名前的点
                    file_node->m_file_type = file_node->m_file_type.substr(1);

                    // 存储完整文件路径
                    file_node->m_file_path = file_paths[file_index].generic_string();
                }

                file_node->m_node_depth = depth;
                node_array.push_back(file_node);

                // 检查节点是否已存在
                bool node_exists = checkFileArray(file_node.get());
                if (node_exists == false)
                {
                    m_file_node_array.push_back(file_node);
                }

                // 查找父节点并添加子节点
                EditorFileNode* parent_node_ptr = getParentNodePtr(node_array[depth].get());
                if (parent_node_ptr != nullptr && node_exists == false)
                {
                    parent_node_ptr->m_child_nodes.push_back(file_node);
                }

                depth++;
            }
        }
    }

    /// 检查文件节点是否已存在
    bool EditorFileService::checkFileArray(EditorFileNode* file_node)
    {
        int editor_node_count = m_file_node_array.size();
        for (int file_node_index = 0; file_node_index < editor_node_count; file_node_index++)
        {
            // 检查同名同深度的节点
            if (m_file_node_array[file_node_index]->m_file_name == file_node->m_file_name &&
                m_file_node_array[file_node_index]->m_node_depth == file_node->m_node_depth)
            {
                return true;
            }
        }
        return false;
    }

    /// 获取父节点指针
    EditorFileNode* EditorFileService::getParentNodePtr(EditorFileNode* file_node)
    {
        int editor_node_count = m_file_node_array.size();
        for (int file_node_index = 0; file_node_index < editor_node_count; file_node_index++)
        {
            // 查找匹配节点
            if (m_file_node_array[file_node_index]->m_file_name == file_node->m_file_name &&
                m_file_node_array[file_node_index]->m_node_depth == file_node->m_node_depth)
            {
                return m_file_node_array[file_node_index].get();
            }
        }
        return nullptr;  // 未找到返回空指针
    }
}
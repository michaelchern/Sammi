#include "runtime/platform/file_service/file_service.h"  // 包含头文件

using namespace std;  // 使用标准命名空间

namespace Sammi
{
    // 获取目录下的所有文件路径（递归扫描）
    vector<filesystem::path> FileSystem::getFiles(const filesystem::path& directory)
    {
        // 创建存储文件路径的向量
        vector<filesystem::path> files;

        // 创建递归目录迭代器遍历目录及子目录
        // recursive_directory_iterator会深度优先遍历所有子目录
        for (auto const& directory_entry : filesystem::recursive_directory_iterator {directory})
        {
            // 检查当前条目是否为普通文件（非目录/符号链接等）
            if (directory_entry.is_regular_file())
            {
                // 将合法文件路径添加到结果向量中
                // directory_entry可以直接转换为path对象
                files.push_back(directory_entry);
            }
        }

        // 返回收集到的文件路径集合
        return files;
    }
}
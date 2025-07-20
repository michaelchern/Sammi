#pragma once

#include "runtime/core/base/macro.h"
#include "runtime/core/meta/serializer/serializer.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include "_generated/serializer/all_serializer.h"

namespace Sammi
{
    /**
     * @brief 资产管理类，负责资产的加载、保存及路径管理
     *
     * 该类提供模板接口，支持任意可被序列化的资产类型（需配合Serializer实现）
     */
    class AssetManager
    {
    public:
        /**
         * @brief 加载资产（从JSON文件反序列化为对象）
         * @tparam AssetType 待加载的资产类型（需支持Serializer反序列化）
         * @param asset_url 资产的相对路径（如"textures/stone.png"）
         * @param out_asset 输出参数，存储加载后的资产对象
         * @return 加载成功返回true，否则返回false
         *
         * 流程：
         * 1. 将相对路径转换为完整路径
         * 2. 打开文件并检查是否成功
         * 3. 读取文件全部内容到字符串
         * 4. 解析JSON文本，检查解析错误
         * 5. 使用Serializer将JSON对象反序列化为资产对象
         */
        template<typename AssetType>
        bool loadAsset(const std::string& asset_url, AssetType& out_asset) const
        {
            // 将相对路径转换为完整路径（如项目根目录+相对路径）
            std::filesystem::path asset_path = getFullPath(asset_url);

            // 打开JSON文件（只读模式）
            std::ifstream asset_json_file(asset_path);
            if (!asset_json_file)
            {
                LOG_ERROR("open file: {} failed!", asset_path.generic_string());
                return false;
            }

            // 读取文件全部内容到字符串流
            std::stringstream buffer;
            buffer << asset_json_file.rdbuf();
            std::string asset_json_text(buffer.str());

            // 解析JSON文本为Json对象（使用第三方JSON库，如nlohmann/json）
            std::string error;
            auto&&      asset_json = Json::parse(asset_json_text, error);
            if (!error.empty())
            {
                LOG_ERROR("parse json file {} failed!", asset_url);
                return false;
            }

            // 使用Serializer将JSON对象反序列化为资产对象（依赖特化的Serializer实现）
            Serializer::read(asset_json, out_asset);
            return true;
        }

        /**
         * @brief 保存资产（将对象序列化为JSON并写入文件）
         * @tparam AssetType 待保存的资产类型（需支持Serializer序列化）
         * @param out_asset 待保存的资产对象（通过引用传递）
         * @param asset_url 保存的目标相对路径（如"textures/new_stone.json"）
         * @return 保存成功返回true，否则返回false
         *
         * 流程：
         * 1. 创建输出文件流（覆盖模式）
         * 2. 检查文件是否成功打开
         * 3. 使用Serializer将资产对象序列化为JSON对象
         * 4. 将JSON对象转换为格式化字符串（带缩进）
         * 5. 将JSON字符串写入文件并刷新缓冲区
         */
        template<typename AssetType>
        bool saveAsset(const AssetType& out_asset, const std::string& asset_url) const
        {
            // 创建输出文件流（默认覆盖已有文件）
            std::ofstream asset_json_file(getFullPath(asset_url));
            if (!asset_json_file)
            {
                LOG_ERROR("open file {} failed!", asset_url);
                return false;
            }

            // 将资产对象序列化为JSON对象（依赖特化的Serializer实现）
            auto&&        asset_json      = Serializer::write(out_asset);
            // 将JSON对象转换为带缩进的字符串（提高可读性）
            std::string&& asset_json_text = asset_json.dump();  // dump(4)表示缩进4个空格

            // 将JSON字符串写入文件
            asset_json_file << asset_json_text;
            asset_json_file.flush();  // 强制刷新缓冲区，确保数据写入磁盘

            return true;
        }

        /**
         * @brief 获取相对路径对应的完整路径
         * @param relative_path 相对路径（相对于项目资源根目录）
         * @return 完整的文件系统路径（如"/project/assets/textures/stone.png"）
         *
         * 实现说明：
         * 具体实现可能基于项目配置的资源根目录，例如：
         * - 硬编码根目录（如"./assets"）
         * - 从配置文件读取根目录
         * - 结合可执行文件所在路径动态计算
         * 示例实现可能为：return std::filesystem::current_path() / "assets" / relative_path;
         */
        std::filesystem::path getFullPath(const std::string& relative_path) const;
    };
}

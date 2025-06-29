#pragma once

#include "runtime/core/base/macro.h"                  // 包含引擎基础宏定义
#include "runtime/core/meta/serializer/serializer.h"  // 包含序列化工具

#include <filesystem>  // C++17 文件系统库
#include <fstream>     // 文件流操作
#include <functional>  // 函数对象
#include <sstream>     // 字符串流
#include <string>      // 字符串操作

#include "_generated/serializer/all_serializer.h"  // 包含自动生成的序列化代码

namespace Sammi
{
    // 资产管理器类
    // 负责游戏中各种资源（资产）的加载和保存
    class AssetManager
    {
    public:
        // 加载指定类型的资产
        // 模板参数：
        //   AssetType - 资产类型（必须支持序列化）
        // 参数：
        //   asset_url - 资产的相对路径或标识符
        //   out_asset - 输出参数，加载的资产对象
        // 返回值：成功加载返回true，否则false
        template<typename AssetType>
        bool loadAsset(const std::string& asset_url, AssetType& out_asset) const
        {
            // 1. 获取资产完整路径
            std::filesystem::path asset_path = getFullPath(asset_url);

            // 2. 打开JSON文件
            std::ifstream asset_json_file(asset_path);
            if (!asset_json_file)
            {
                LOG_ERROR("open file: {} failed!", asset_path.generic_string());
                return false;
            }

            // 3. 读取文件内容到字符串
            std::stringstream buffer;
            buffer << asset_json_file.rdbuf();
            std::string asset_json_text(buffer.str());

            // 4. 解析JSON字符串
            std::string error;
            auto&&      asset_json = Json::parse(asset_json_text, error);  // 使用JSON库解析
            if (!error.empty())
            {
                LOG_ERROR("parse json file {} failed!", asset_url);
                return false;
            }

            // 5. 反序列化JSON到目标对象
            Serializer::read(asset_json, out_asset);
            return true;
        }

        // 保存指定类型的资产
        // 模板参数：
        //   AssetType - 要保存的资产类型
        // 参数：
        //   out_asset - 要保存的资产对象
        //   asset_url - 资产的相对路径或标识符
        // 返回值：成功保存返回true，否则false
        template<typename AssetType>
        bool saveAsset(const AssetType& out_asset, const std::string& asset_url) const
        {
            // 1. 打开目标文件
            std::ofstream asset_json_file(getFullPath(asset_url));
            if (!asset_json_file)
            {
                LOG_ERROR("open file {} failed!", asset_url);
                return false;
            }

            // 2. 序列化资产对象为JSON
            auto&& asset_json = Serializer::write(out_asset);

            // 3. 格式化为字符串
            std::string&& asset_json_text = asset_json.dump();

            // 4. 写入文件并刷新缓冲区
            asset_json_file << asset_json_text;
            asset_json_file.flush();

            return true;
        }

        // 获取资产的完整文件系统路径
        // 参数：
        //   relative_path - 资产的相对路径
        // 返回值：完整的文件系统路径
        std::filesystem::path getFullPath(const std::string& relative_path) const;
    };
}
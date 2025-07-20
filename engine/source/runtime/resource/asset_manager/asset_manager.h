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
     * @brief �ʲ������࣬�����ʲ��ļ��ء����漰·������
     *
     * �����ṩģ��ӿڣ�֧������ɱ����л����ʲ����ͣ������Serializerʵ�֣�
     */
    class AssetManager
    {
    public:
        /**
         * @brief �����ʲ�����JSON�ļ������л�Ϊ����
         * @tparam AssetType �����ص��ʲ����ͣ���֧��Serializer�����л���
         * @param asset_url �ʲ������·������"textures/stone.png"��
         * @param out_asset ����������洢���غ���ʲ�����
         * @return ���سɹ�����true�����򷵻�false
         *
         * ���̣�
         * 1. �����·��ת��Ϊ����·��
         * 2. ���ļ�������Ƿ�ɹ�
         * 3. ��ȡ�ļ�ȫ�����ݵ��ַ���
         * 4. ����JSON�ı�������������
         * 5. ʹ��Serializer��JSON�������л�Ϊ�ʲ�����
         */
        template<typename AssetType>
        bool loadAsset(const std::string& asset_url, AssetType& out_asset) const
        {
            // �����·��ת��Ϊ����·��������Ŀ��Ŀ¼+���·����
            std::filesystem::path asset_path = getFullPath(asset_url);

            // ��JSON�ļ���ֻ��ģʽ��
            std::ifstream asset_json_file(asset_path);
            if (!asset_json_file)
            {
                LOG_ERROR("open file: {} failed!", asset_path.generic_string());
                return false;
            }

            // ��ȡ�ļ�ȫ�����ݵ��ַ�����
            std::stringstream buffer;
            buffer << asset_json_file.rdbuf();
            std::string asset_json_text(buffer.str());

            // ����JSON�ı�ΪJson����ʹ�õ�����JSON�⣬��nlohmann/json��
            std::string error;
            auto&&      asset_json = Json::parse(asset_json_text, error);
            if (!error.empty())
            {
                LOG_ERROR("parse json file {} failed!", asset_url);
                return false;
            }

            // ʹ��Serializer��JSON�������л�Ϊ�ʲ����������ػ���Serializerʵ�֣�
            Serializer::read(asset_json, out_asset);
            return true;
        }

        /**
         * @brief �����ʲ������������л�ΪJSON��д���ļ���
         * @tparam AssetType ��������ʲ����ͣ���֧��Serializer���л���
         * @param out_asset ��������ʲ�����ͨ�����ô��ݣ�
         * @param asset_url �����Ŀ�����·������"textures/new_stone.json"��
         * @return ����ɹ�����true�����򷵻�false
         *
         * ���̣�
         * 1. ��������ļ���������ģʽ��
         * 2. ����ļ��Ƿ�ɹ���
         * 3. ʹ��Serializer���ʲ��������л�ΪJSON����
         * 4. ��JSON����ת��Ϊ��ʽ���ַ�������������
         * 5. ��JSON�ַ���д���ļ���ˢ�»�����
         */
        template<typename AssetType>
        bool saveAsset(const AssetType& out_asset, const std::string& asset_url) const
        {
            // ��������ļ�����Ĭ�ϸ��������ļ���
            std::ofstream asset_json_file(getFullPath(asset_url));
            if (!asset_json_file)
            {
                LOG_ERROR("open file {} failed!", asset_url);
                return false;
            }

            // ���ʲ��������л�ΪJSON���������ػ���Serializerʵ�֣�
            auto&&        asset_json      = Serializer::write(out_asset);
            // ��JSON����ת��Ϊ���������ַ�������߿ɶ��ԣ�
            std::string&& asset_json_text = asset_json.dump();  // dump(4)��ʾ����4���ո�

            // ��JSON�ַ���д���ļ�
            asset_json_file << asset_json_text;
            asset_json_file.flush();  // ǿ��ˢ�»�������ȷ������д�����

            return true;
        }

        /**
         * @brief ��ȡ���·����Ӧ������·��
         * @param relative_path ���·�����������Ŀ��Դ��Ŀ¼��
         * @return �������ļ�ϵͳ·������"/project/assets/textures/stone.png"��
         *
         * ʵ��˵����
         * ����ʵ�ֿ��ܻ�����Ŀ���õ���Դ��Ŀ¼�����磺
         * - Ӳ�����Ŀ¼����"./assets"��
         * - �������ļ���ȡ��Ŀ¼
         * - ��Ͽ�ִ���ļ�����·����̬����
         * ʾ��ʵ�ֿ���Ϊ��return std::filesystem::current_path() / "assets" / relative_path;
         */
        std::filesystem::path getFullPath(const std::string& relative_path) const;
    };
}

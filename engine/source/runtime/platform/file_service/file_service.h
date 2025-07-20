#pragma once

#include <filesystem>
#include <vector>

namespace Sammi
{
    // ��װ�ļ�ϵͳ�������࣬�ṩ���ļ�/Ŀ¼�����Ĺ��ܽӿ�
    class FileSystem 
    {
    public:
        /**
         * @brief ��ȡָ��Ŀ¼�µ������ļ�·������������Ŀ¼�е��ļ���
         *
         * @param directory Ŀ��Ŀ¼��·������std::filesystem::path���ͣ�
         * @return std::vector<std::filesystem::path> ����Ŀ��Ŀ¼�������ļ�·����vector����
         *
         * ����˵����
         *  - ����һ��Ŀ¼·������ "D:/data" �� "/home/user/docs"��
         *  - ������Ŀ¼�µ�ֱ�����ļ������ݹ���Ŀ¼��
         *  - ��ÿ���ļ���·����std::filesystem::path���ʹ���vector������
         * ע�⣺
         *  - ��Ŀ¼�����ڻ���Ȩ�޷��ʣ����ܷ��ؿ�vector��������Ϊ����ʵ�֣�
         *  - ·����ʽ֧�ֿ�ƽ̨��Windows/Linux/macOS��
         */
        std::vector<std::filesystem::path> getFiles(const std::filesystem::path& directory);
    };
}

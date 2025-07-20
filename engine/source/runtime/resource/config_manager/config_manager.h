#pragma once

#include <filesystem>

namespace Sammi
{
    // ǰ�����������ʼ�������ṹ�壨���嶨������������ļ��У�
    // ������������ʱ���ݶ����ʼ��ѡ����Զ�������·��������ģʽ�ȣ�
    struct EngineInitParams;

    /// ��������������õļ��������
    /// ������������ļ�����INI/JSON�����洢���ṩȫ�����ò�����·������ԴURL�ȣ�
    class ConfigManager
    {
    public:
        /// ��ʼ�����ù���������ָ�������ļ��������ã�
        /// @param config_file_path �����ļ�·������"Config/SammiEditor.ini"��
        void initialize(const std::filesystem::path& config_file_path);

        // ------------------------- ·����ȡ�ӿ� -------------------------
        /// ��ȡ������ļ���·������������·���Ļ�׼Ŀ¼��
        /// @return ���ļ���·������"/Project/Root/"��
        const std::filesystem::path& getRootFolder() const;

        /// ��ȡ��Դ�ļ���·�������ģ�͡��������ʵ���Դ��
        /// @return ��Դ�ļ���·������"/Project/Root/Assets/"��
        const std::filesystem::path& getAssetFolder() const;

        /// ��ȡģʽ�ļ���·�������XML Schema��ģʽ�����ļ���
        /// @return ģʽ�ļ���·������"/Project/Root/Schemas/"��
        const std::filesystem::path& getSchemaFolder() const;

        // �༭��ר��·�������ڱ༭��ģʽ��ʹ�ã�
        /// ��ȡ�༭����ͼ��·�������ڴ��ڱ�����/��������
        /// @return ��ͼ��·������"/Editor/Resources/Icons/EditorLarge.ico"��
        const std::filesystem::path& getEditorBigIconPath() const;

        /// ��ȡ�༭��Сͼ��·���������ļ���Դ��������С�ߴ���ʾ������
        /// @return Сͼ��·������"/Editor/Resources/Icons/EditorSmall.ico"��
        const std::filesystem::path& getEditorSmallIconPath() const;

        /// ��ȡ�༭������·�������ڽ����ı���Ⱦ��
        /// @return ����·������"/Editor/Resources/Fonts/Roboto-Regular.ttf"��
        const std::filesystem::path& getEditorFontPath() const;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        // ���������Ⱦ��ר��·�����������øù���ʱ��Ч��
        /// ��ȡJolt����������Դ�ļ���·���������ײ��״�����Ի�����Դ�ȣ�
        /// @return Jolt������Դ·������"/Engine/Physics/Jolt/Assets/"��
        const std::filesystem::path& getJoltPhysicsAssetFolder() const;
#endif

        // ------------------------- ��ԴURL��ȡ�ӿ� -------------------------
        /// ��ȡĬ������URL����������ʱ�Զ����صĳ����ļ�·����
        /// @return Ĭ������URL����"Scenes/MainLevel.sammi"��
        const std::string& getDefaultWorldUrl() const;

        /// ��ȡȫ����Ⱦ��ԴURL�������ɫ����������ͼ����Ⱦ�����Դ��
        /// @return ȫ����Ⱦ��ԴURL����"Resources/Rendering/Global.pak"��
        const std::string& getGlobalRenderingResUrl() const;

        /// ��ȡȫ��������ԴURL���������Ч��ģ�塢���������õȣ�
        /// @return ȫ��������ԴURL����"Resources/Particles/Global.pak"��
        const std::string& getGlobalParticleResUrl() const;

    private:
        // ------------------------- �ڲ��洢������ֵ -------------------------
        std::filesystem::path m_root_folder;             // ������ļ���·��
        std::filesystem::path m_asset_folder;            // ��Դ�ļ���·��
        std::filesystem::path m_schema_folder;           // ģʽ�ļ���·��
        std::filesystem::path m_editor_big_icon_path;    // �༭����ͼ��·��
        std::filesystem::path m_editor_small_icon_path;  // �༭��Сͼ��·��
        std::filesystem::path m_editor_font_path;        // �༭������·��

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        std::filesystem::path m_jolt_physics_asset_folder;  // Jolt������Դ·�����������룩
#endif

        std::string m_default_world_url;         // Ĭ������URL
        std::string m_global_rendering_res_url;  // ȫ����Ⱦ��ԴURL
        std::string m_global_particle_res_url;   // ȫ��������ԴURL
    };
}

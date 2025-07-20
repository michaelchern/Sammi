#pragma once

#include <memory>
#include <string>

namespace Sammi
{
    // ǰ������������ϵͳ�ࣨ�����������������������壩
    // ��Щ���������ĺ��Ĺ���ģ�飬����ʵ��������Դ�ļ���

    class LogSystem;          // ��־ϵͳ��ȫ����־��¼�����
    class InputSystem;        // ����ϵͳ���������/���������豸��
    class PhysicsManager;     // �����������ģ���������磬����塢��ײ��
    class FileSystem;         // �ļ�ϵͳ�������ļ���д����Դ·����
    class AssetManager;       // �ʲ�������������/����ģ�͡��������Դ��
    class ConfigManager;      // ���ù����������������ļ����細�ڳߴ硢���������
    class WorldManager;       // �����������������Ϸ����/������ά������ͼ��
    class RenderSystem;       // ��Ⱦϵͳ������3D/2Dͼ����Ⱦ��
    class WindowSystem;       // ����ϵͳ������/������Ϸ���ڣ��������¼���
    class ParticleManager;    // ���ӹ���������������Ч��������桢����
    class DebugDrawManager;   // ���Ի��ƹ���������ʾ��ײ�塢����߽�ȵ�����Ϣ��
    class RenderDebugConfig;  // ��Ⱦ�������ã����Ƶ��Թ��ܿ��أ�����ʾ����

    // �����ʼ�������ṹ�壨��ǰ����δչ�������ܰ��������ʼ��ѡ�
    // ���磺�Ƿ����õ���ģʽ���Զ�����Դ·���ȣ�Ԥ����չ��
    struct EngineInitParams;

    /// ��������ȫ����ϵͳ���������ڣ���������ʼ�������٣�
    /// ��Ϊ�����"����"���ṩͳһ��ڷ������к��Ĺ���ģ��
    class RuntimeGlobalContext
    {
    public:
        // ��������ȫ����ϵͳ��������˳���ʼ����
        // ������config_file_path - �����ļ�·������"Config/Engine.ini"��
        void startSystems(const std::string& config_file_path);

        // �ر�����ȫ����ϵͳ����������˳���ͷ���Դ��
        void shutdownSystems();

    public:
        std::shared_ptr<LogSystem>         m_logger_system;
        std::shared_ptr<InputSystem>       m_input_system;
        std::shared_ptr<FileSystem>        m_file_system;
        std::shared_ptr<AssetManager>      m_asset_manager;
        std::shared_ptr<ConfigManager>     m_config_manager;
        std::shared_ptr<WorldManager>      m_world_manager;
        std::shared_ptr<PhysicsManager>    m_physics_manager;
        std::shared_ptr<WindowSystem>      m_window_system;
        std::shared_ptr<RenderSystem>      m_render_system;
        std::shared_ptr<ParticleManager>   m_particle_manager;
        std::shared_ptr<DebugDrawManager>  m_debugdraw_manager;
        std::shared_ptr<RenderDebugConfig> m_render_debug_config;
    };

    // ȫ��������ʵ��������ģʽ��ȫ��Ψһ������ڣ�
    // ����ģ��ͨ����ʵ������������Ĺ��ܣ����ȡ����ϵͳ������ϵͳ�ȣ�
    extern RuntimeGlobalContext g_runtime_global_context;
}

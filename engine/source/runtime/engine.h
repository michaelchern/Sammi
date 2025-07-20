#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace Sammi
{
    // ȫ�ֱ༭��ģʽ��־�����ڿ��Ʊ༭����ع��ܵ�����/����״̬
    // �����������ڱ༭����������Sammi�༭������ʱ���˱�־�ᱻ����Ϊtrue
    extern bool g_is_editor_mode;

    // �༭��ģʽ����Ҫÿ֡���µ�������ͼ���
    // �洢������͵��ַ�����ʶ����"TransformComponent", "RenderComponent"��
    // �༭��ͨ���˼���ɸѡ��Ҫ�ڱ༭��tick�д�������������Ӱ������ʱ�߼�
    extern std::unordered_set<std::string> g_editor_tick_component_types;

    /**
     * @brief ������Ĺ����࣬����������������ڹ�����ѭ�����м����Ĺ���Э��
     *
     * �����װ���������������ʼ������ѭ�����С�֡���¡���Դ����Ⱥ��Ĺ��ܣ�
     * ������ϵͳ�ĺ�����ڵ㡣֧������ʱ״̬��ѯ����FPS���˳�״̬���ͱ༭��ģʽ���ɡ�
     */
    class SammiEngine
    {
        friend class SammiEditor;        // ����༭����ֱ�ӷ��������ڲ���Ա����˽��״̬��

        static const float s_fps_alpha;  // FPSƽ�������ָ���ƶ�ƽ��ϵ����0<alpha<1��
                                         // ���ڻ���֡�ʲ�����ʹ��ʾ��FPSֵ���ȶ������ⶨ�壩

    public:
        /**
         * @brief �������沢��������
         * @param config_file_path ���������ļ�·������"config/engine.json"��
         * ���ܣ���ʼ���ײ�ϵͳ������־����Դ�����������������ò�����׼�����л���
         * ע�⣺���ڵ���initialize()ǰ���ã������ڲ��Զ�������ʼ�����̣�����ʵ��������ƣ�
         */
        void startEngine(const std::string& config_file_path);

        /**
         * @brief �ر����沢�ͷ�������Դ
         * ���ܣ�ֹͣ���������е���ϵͳ������Ⱦ�������������ڴ���Դ�������Ҫ����
         * ע�⣺Ӧ�ڳ����˳�ǰ���ã�ȷ����Դ��ȷ�ͷţ������ڴ�й©
         */
        void shutdownEngine();

        /**
         * @brief ��ʼ�����������ϵͳ
         * ���ܣ���ʼ���߼�ģ�飨�����ϵͳ���¼�ϵͳ������Ⱦ������������Ⱥ������
         * ע�⣺ͨ����startEngine()����ã�����ΪstartEngine()���ڲ�����ִ��
         */
        void initialize();

        /**
         * @brief �������浱ǰ״̬���������ú���Դ��
         * ���ܣ������߼�״̬����������ʱ����ֹͣ�������������Ⱦ�������
         * ע�⣺�������������浽��ʼ������״̬�����ͷŵײ���Դ��������shutdownEngine��
         */
        void clear();

        /**
         * @brief ��ѯ�����Ƿ�Ӧ�˳���ѭ��
         * @return true ������Ϊ�˳���false ��������
         * ��;����ѭ�����ж��Ƿ���ֹ�������У����û�����رհ�ť�����quit()��
         */
        bool isQuit() const { return m_is_quit; }

        /**
         * @brief ����������ѭ��
         * ���̣���������tickOneFrame()����֡���£�ֱ��isQuit()����true
         * ע�⣺��ѭ����������ǰ�̣߳�ͨ��Ӧ�ڶ����߳������У������̣߳�
         */
        void run();

        /**
         * @brief ����֡���£��߼�+��Ⱦ��
         * @param delta_time ��ǰ֡����һ֡��ʱ�������룩����calculateDeltaTime()�ṩ
         * @return true ��֡���³ɹ���false ����ʧ�ܣ�����Ⱦ����
         * ���̣��ȵ���logicalTick()�����߼����ٵ���rendererTick()������Ⱦ
         */
        bool tickOneFrame(float delta_time);

        /**
         * @brief ��ȡ��ǰ����֡�ʣ�FPS��
         * @return ��ǰƽ�����֡��ֵ����������60��30��
         * ˵�������������֡��ʱ�������㣬ͨ��s_fps_alphaƽ��������ֵ���Ҳ���
         */
        int getFPS() const { return m_fps; }

    protected:
        /**
         * @brief ����֡�߼�����
         * @param delta_time ��ǰ֡ʱ�������룩
         * ���ܣ�������Ϸ�߼������ɫ�ƶ���AI���ߣ��������¼����������״̬
         * ע�⣺ÿ֡������һ�Σ���tickOneFrame()����
         */
        void logicalTick(float delta_time);

        /**
         * @brief ����֡��Ⱦ����
         * @param delta_time ��ǰ֡ʱ�������룩
         * @return true ��Ⱦ�ɹ���false ��Ⱦʧ�ܣ�����Դ��ʧ��
         * ���ܣ��ύ��Ⱦ�������UI�����Ƶ�����Ϣ
         * ע�⣺��Ⱦ�����������һ֡�������ʾ������������ȾAPI����OpenGL/Vulkan��
         */
        bool rendererTick(float delta_time);

        /**
         * @brief ���ڵ�ǰ֡ʱ��������FPS����
         * @param delta_time ��ǰ֡ʱ�������룩
         * �㷨��ʹ��ָ���ƶ�ƽ��ƽ��FPSֵ
         *      m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha
         *      m_fps = static_cast<int>(1.0f / m_average_duration)
         */
        void calculateFPS(float delta_time);

        /**
         * @brief ���㵱ǰ֡����һ֡��ʱ������delta time��
         * @return ��ǰ֡ʱ�������룬����ȡ����steady_clock��
         * ʵ�֣�ʹ��std::chrono::steady_clock��ȡ�߾���ʱ���������ʱ���
         * ע�⣺ÿ֡������ֻ�ܵ���һ�Σ�����ᵼ��ʱ��������
         */
        float calculateDeltaTime();

    protected:
        bool m_is_quit {false};  // �����˳���־��������ѭ����ֹ

        // ��һ֡tick��ʱ��������ڼ���delta_time��
        std::chrono::steady_clock::time_point m_last_tick_time_point {std::chrono::steady_clock::now()};

        float m_average_duration {0.f};  // ƽ�����ƽ��֡ʱ�䣨����FPS���㣩
        int   m_frame_count {0};         // ֡�����������ڵ��Ի������㣩
        int   m_fps {0};                 // ��ǰƽ�����֡��ֵ
    };

}

#pragma once

#include "runtime/core/math/vector2.h"
#include "runtime/function/render/render_pass_base.h"

#include <memory>
#include <vector>

namespace Sammi
{
    // ǰ��������������Ⱦϵͳ������
    class RHI;// ��ȾӲ���ӿڣ�����ײ�API��Vulkan/DirectX��
    class RenderResourceBase;// ��Ⱦ��Դ���ࣨ������������������Դ��
    class WindowUI;// UI��Ⱦ�����ࣨ����UI���ƣ�

    /**
     * @brief ��Ⱦ���߳�ʼ����Ϣ�ṹ��
     * ���ڴ�����Ⱦ���߳�ʼ����������ò�������Դ
     */
    struct RenderPipelineInitInfo
    {
        bool                                enable_fxaa {false};// �Ƿ�����FXAA����ݣ����ٽ��ƿ���ݣ�
        std::shared_ptr<RenderResourceBase> render_resource;// ��Ⱦ��Դ���������ṩ����/����������Դ���ʣ�
    };

    /**
     * @brief ��Ⱦ���߻��ࣨ����ӿڣ�
     * �������о�����Ⱦ���ߣ���ǰ����Ⱦ���ӳ���Ⱦ������ʵ�ֵĽӿڣ�
     * ����Э�������Ⱦͨ����RenderPass�������������Ⱦ���̡�
     * ��Ϊ��Ⱦϵͳ�ĺ���ģ�飬���������Ⱦ״̬����Դ���Ⱥ͸��׶���Ⱦ�����ִ�С�
     */
    class RenderPipelineBase
    {
        // ����RenderSystemΪ��Ԫ�ࣨ��������ʱ����˽��/������Ա��
        friend class RenderSystem;

    public:
        /**
         * @brief ����������
         * ȷ�����������ͨ������ָ������ʱ��ȷ������������
         */
        virtual ~RenderPipelineBase() {}

        /**
         * @brief ������Ⱦ������Դ
         * �ͷ�������Ⱦͨ����RHI��Դ�ȣ����ڹ������û�����˳�
         */
        virtual void clear() {};

        /**
         * @brief ��ʼ����Ⱦ���ߣ����麯����
         * @param init_info ��Ⱦ���߳�ʼ�����ã���������ݿ��ء���Ⱦ��Դ�ȣ�
         * �������ʵ�ִ˺�������ɾ���ĳ�ʼ���߼����紴����Ⱦͨ����������Դ��
         */
        virtual void initialize(RenderPipelineInitInfo init_info) = 0;

        /**
         * @brief ׼����Ⱦͨ������
         * @param render_resource ��Ⱦ��Դ���������ṩ����/����������Դ��
         * ����Ⱦǰ���¸���Ⱦͨ����Ҫ����ʱ���ݣ�����ղ������������ȣ�
         */
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);

        /**
         * @brief ִ��ǰ����Ⱦ���̣����麯����
         * @param rhi ��ȾӲ���ӿڣ����ʵײ�API��
         * @param render_resource ��Ⱦ��Դ������
         * ǰ����Ⱦ��������˳�������Ⱦ���ʺ�͸�����塢������Դ������
         * �������ʵ�ִ˺�������ɾ����ǰ����Ⱦ�߼�
         */
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

        /**
         * @brief ִ���ӳ���Ⱦ���̣����麯����
         * @param rhi ��ȾӲ���ӿ�
         * @param render_resource ��Ⱦ��Դ������
         * �ӳ���Ⱦ�������������ν׶δ洢GBuffer�����ս׶μ�����գ����ʺϸ��ӹ��ճ�����
         * �������ʵ�ִ˺�������ɾ�����ӳ���Ⱦ�߼�
         */
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

        /**
         * @brief ��ʼ��UI��Ⱦ���
         * @param window_ui UI���ڶ����ṩUI���������ģ�
         * ��UI��Ⱦ���ɵ�����Ⱦ�����У�������UI��ȾĿ�ꡢͬ���ֱ��ʵȣ�
         */
        void initializeUIRenderBackend(WindowUI* window_ui);

        /**
         * @brief ��ȡ��ʰȡ�����GUID�����麯����
         * @param picked_uv ��Ļ�ռ�UV���꣨�����λ�õĹ�һ�����꣩
         * ���ڽ�������������ѡ�����壩��������Ļ���귴�Ʊ�ѡ�е��������
         * @return ��ʰȡ�����Ψһ��ʶGUID����δ���з�����Чֵ��
         */
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) = 0;

    protected:
        std::shared_ptr<RHI> m_rhi;  // ��ȾӲ���ӿ�ʵ�������ʵײ�API�ĺ��ľ����

        // ��Ⱦͨ�����ϣ����׶���Ⱦ�����ִ���ߣ�
        std::shared_ptr<RenderPassBase> m_directional_light_pass;   // �������Ⱦͨ�������������Ӱ/���գ�
        std::shared_ptr<RenderPassBase> m_point_light_shadow_pass;  // ���Դ��Ӱ��Ⱦͨ�������ɵ��Դ��Ӱ��ͼ��
        std::shared_ptr<RenderPassBase> m_main_camera_pass;         // �������Ⱦͨ������Ⱦ�������ӽ����壩
        std::shared_ptr<RenderPassBase> m_color_grading_pass;       // ɫ�ʷּ�ͨ������������ɫ��/�Աȶȣ�
        std::shared_ptr<RenderPassBase> m_fxaa_pass;                // FXAA�����ͨ�����Ż���Ե��ݣ�
        std::shared_ptr<RenderPassBase> m_tone_mapping_pass;        // ɫ��ӳ��ͨ����HDRתLDR��
        std::shared_ptr<RenderPassBase> m_ui_pass;                  // UI��Ⱦͨ��������UIԪ�أ�
        std::shared_ptr<RenderPassBase> m_combine_ui_pass;          // UI�ϳ�ͨ�����ϲ�UI����Ϸ���棩
        std::shared_ptr<RenderPassBase> m_pick_pass;                // ʰȡ��Ⱦͨ������������ʰȡ�����/ID����
        std::shared_ptr<RenderPassBase> m_particle_pass;            // ����ϵͳ��Ⱦͨ������������Ч����
    };
}

#pragma once

#include "runtime/function/render/render_pipeline_base.h"

namespace Sammi
{
    /**
     * @brief ������Ⱦ����ʵ���ࣨ�̳���RenderPipelineBase��
     *
     * ����ʵ����RenderPipelineBase�ж�������д��麯�����ṩ�˾������Ⱦ�����߼���
     * ���������Ⱦͨ����ִ��˳����Դ��ʼ�����������ܣ������ʰȡ���Լ�������ز���������������ʾ����
     * ��Ϊ���������࣬ͨ��`override final`���η�ȷ���ӿ�ʵ�ֵ��ȶ��ԣ���ֹ�����������޸ĺ�����Ⱦ�߼���
     */
    class RenderPipeline : public RenderPipelineBase
    {
    public:
        /**
         * @brief ��ʼ����Ⱦ���ߣ�����ʵ�֣�
         * @param init_info ��Ⱦ���߳�ʼ�����ã���������ݿ��ء���Ⱦ��Դ�ȣ�
         * ʵ�ֻ���Ĵ��麯���������Ⱦͨ���Ĵ�������Դ�󶨡�RHI�����ĳ�ʼ���Ⱦ��������
         * ���磺��������Ⱦͨ��ʵ��������⡢������ȣ�������GPU��Դ�����ó�ʼ��Ⱦ״̬�ȡ�
         */
        virtual void initialize(RenderPipelineInitInfo init_info) override final;

        /**
         * @brief ִ��ǰ����Ⱦ���̣�����ʵ�֣�
         * @param rhi ��ȾӲ���ӿڣ����ʵײ�ͼ��API����Vulkan/DirectX��
         * @param render_resource ��Ⱦ��Դ���������ṩ��������������Դ��
         * ʵ�ֻ���Ĵ��麯�������ǰ����Ⱦ�ľ����߼���
         * - ��˳��ִ�и���Ⱦͨ�������������Ⱦ����Ӱ��Ⱦ��������Ⱦ�ȣ�
         * - Ӧ�ù��ռ��㡢������ɫ����Ȳ��Ե�ǰ����Ⱦ�ؼ�����
         * - ���������ɫ��֡����
         */
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource) override final;

        /**
         * @brief ִ���ӳ���Ⱦ���̣�����ʵ�֣�
         * @param rhi ��ȾӲ���ӿڣ����ʵײ�ͼ��API��
         * @param render_resource ��Ⱦ��Դ���������ṩ��������������Դ��
         * ʵ�ֻ���Ĵ��麯��������ӳ���Ⱦ�ľ����߼���
         * - ���ν׶Σ���������Ϣ��λ�á����ߡ����ʣ�д��GBuffer������ȾĿ�꣩
         * - ���ս׶Σ�����GBuffer���ݼ��㸴�ӹ��գ�����Դ���ӡ�ȫ�ֹ��գ�
         * - ���ڴ����ϲ�GBuffer��������Ч������ɫ�ʷּ�������ݣ�
         */
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource) override final;

        /**
         * @brief �������ؽ��������Ⱦͨ��
         * �����ڳߴ�仯��GPU�����Ķ�ʧ���½�������Swapchain����Ҫ�ؽ�ʱ��
         * ���ô˺������³�ʼ����Ⱦͨ����Ŀ�������ӿڵ���������������Դ��
         */
        void passUpdateAfterRecreateSwapchain();

        /**
         * @brief ��ȡ��ʰȡ�����GUID������ʵ�֣�
         * @param picked_uv ��Ļ�ռ�UV���꣨�����λ�õĹ�һ�����꣩
         * ʵ�ֻ���Ĵ��麯����ͨ����ȡʰȡ��Ⱦͨ�����ɵ����/ID����
         * ����Ļ����ת��Ϊ�����е�����ʵ��ID�����ڽ���ѡ������ѡ�����壩��
         * @return ��ʰȡ�����Ψһ��ʶGUID����δ���з�����Чֵ��
         */
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) override final;

        /**
         * @brief ����������Ŀɼ�״̬
         * @param state �ɼ�״̬��true=��ʾ��false=���أ�
         * ���Ʊ༭��ģʽ�������Ḩ������m_render_axis������Ⱦ���أ�
         * ���ڵ��Գ��������λ�úͳ���
         */
        void setAxisVisibleState(bool state);

        /**
         * @brief ���õ�ǰѡ�е�����������
         * @param selected_axis ѡ�е���������������0=X�ᣬ1=Y�ᣬ2=Z�ᣩ
         * ���ڱ༭�������и�����ʾѡ�е������ᣨ����ק��������λ��ʱ����
         * ͨ����UI�ؼ������������û�������
         */
        void setSelectedAxis(size_t selected_axis);
    };
}

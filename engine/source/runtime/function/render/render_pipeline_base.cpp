#include "runtime/function/render/render_pipeline_base.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"
#include "runtime/core/base/macro.h"
#include "runtime/function/global/global_context.h"

namespace Sammi
{
    void RenderPipelineBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        // ׼���������Ⱦͨ�������ݣ��������׶�塢ͶӰ���󡢿ɼ������б�ȣ�
        m_main_camera_pass->preparePassData(render_resource);

        // ׼��ʰȡ��Ⱦͨ�������ݣ��������������ʰȡ�����/ID����
        m_pick_pass->preparePassData(render_resource);

        // ׼���������Ⱦͨ�������ݣ�����Ӱ��ͼ��������ĵƹ���󡢼��������ȣ�
        m_directional_light_pass->preparePassData(render_resource);

        // ׼�����Դ��Ӱ��Ⱦͨ�������ݣ�����Դ��Ӱ��ͼ����������ͼ���ɲ�����
        m_point_light_shadow_pass->preparePassData(render_resource);

        // ׼������ϵͳ��Ⱦͨ�������ݣ�������λ�á��������ڡ����ʲ����ȣ�
        m_particle_pass->preparePassData(render_resource);

        // ����ȫ�ֵ��Ի��ƹ�����������׼��������������������Ļ��Ʋ�����
        g_runtime_global_context.m_debugdraw_manager->preparePassData(render_resource);
    }

    void RenderPipelineBase::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
    }

    void RenderPipelineBase::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
    }

    void RenderPipelineBase::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_ui_pass->initializeUIRenderBackend(window_ui);
    }
}

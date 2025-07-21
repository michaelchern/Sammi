#pragma once

#include "runtime/function/render/render_pass.h"

namespace Sammi
{
    class RenderResourceBase;

    /**
     * @brief 点光源阴影渲染通道类
     *
     * 继承自RenderPass，负责处理点光源阴影的具体渲染逻辑。
     * 在渲染流程中，该阶段会生成点光源所需的阴影贴图（Shadow Map），
     * 供后续光照计算阶段使用。
     */
    class PointLightShadowPass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* init_info) override final;
        void postInitialize() override final;
        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;
        void draw() override final;

        /**
         * @brief 设置每网格描述符集布局
         * @param layout 每个网格（Model）对应的描述符集布局（定义着色器资源绑定方式）
         * 存储网格级别的描述符集布局（如模型矩阵、纹理采样器等资源的绑定槽位）
         */
        void setPerMeshLayout(RHIDescriptorSetLayout* layout) { m_per_mesh_layout = layout; }

    private:
        void setupAttachments();
        void setupRenderPass();
        void setupFramebuffer();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
        void drawModel();

    private:
        // 每个网格的描述符集布局指针，用于绑定模型级资源
        RHIDescriptorSetLayout* m_per_mesh_layout;

        // 点光源阴影每帧存储缓冲区对象（SSBO）
        // 存储每帧动态变化的点光源相关数据（如光源位置、投影矩阵、视锥体参数等）
        MeshPointLightShadowPerframeStorageBufferObject m_mesh_point_light_shadow_perframe_storage_buffer_object;
    };
}

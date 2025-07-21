#include "runtime/function/render/render_pass.h"
#include "runtime/core/base/macro.h"
#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

// 声明RenderPass类的静态成员变量（全局可见节点容器）
// 用于存储当前帧中各类可见的渲染节点（如网格、轴节点等）
Sammi::VisiableNodes Sammi::RenderPass::m_visiable_nodes;

namespace Sammi
{
    void RenderPass::initialize(const RenderPassInitInfo* init_info)
    {
        // 将m_render_resource（std::shared_ptr<RenderResourceBase>）转换为具体类型RenderResource
        // 并获取其内部的m_global_render_resource指针（全局渲染资源管理器）
        // static_pointer_cast：安全地将基类智能指针转换为派生类智能指针（需确保类型正确）
        m_global_render_resource = &(std::static_pointer_cast<RenderResource>(m_render_resource)->m_global_render_resource);
    }

    void RenderPass::draw()
    {
    }

    void RenderPass::postInitialize()
    {
    }

    RHIRenderPass* RenderPass::getRenderPass() const 
    {
        return m_framebuffer.render_pass;
    }

    std::vector<RHIImageView*> RenderPass::getFramebufferImageViews() const
    {
        std::vector<RHIImageView*> image_views;
        // 遍历帧缓冲的所有附件（如GBuffer、深度、后处理缓冲区等）
        for (auto& attach : m_framebuffer.attachments)
        {
            // 将每个附件的图像视图（RHIImageView）加入列表
            image_views.push_back(attach.view);
        }
        return image_views;
    }

    std::vector<RHIDescriptorSetLayout*> RenderPass::getDescriptorSetLayouts() const
    {
        std::vector<RHIDescriptorSetLayout*> layouts;
        // 遍历所有描述符信息（包含布局和实例）
        for (auto& desc : m_descriptor_infos)
        {
            // 将每个描述符的布局（RHIDescriptorSetLayout）加入列表
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
}

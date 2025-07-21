#pragma once

#include "runtime/function/render/render_common.h"
#include "runtime/function/render/render_pass_base.h"
#include "runtime/function/render/render_resource.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Sammi
{
    class VulkanRHI;

    // ============================== 主相机渲染通道附件索引枚举 ==============================
    // 定义主相机渲染流程中使用的所有帧缓冲附件的索引（对应Vulkan的附件槽位）
    // 这些附件用于存储不同阶段的渲染结果（如GBuffer、深度、后处理中间结果等）
    enum
    {
        _main_camera_pass_gbuffer_a                     = 0,  // GBuffer A（通常存储位置信息）
        _main_camera_pass_gbuffer_b                     = 1,  // GBuffer B（通常存储法线信息）
        _main_camera_pass_gbuffer_c                     = 2,  // GBuffer C（通常存储材质/UV信息）
        _main_camera_pass_backup_buffer_odd             = 3,  // 备份缓冲区（奇数帧使用，用于双缓冲）
        _main_camera_pass_backup_buffer_even            = 4,  // 备份缓冲区（偶数帧使用）
        _main_camera_pass_post_process_buffer_odd       = 5,  // 后处理缓冲区（奇数帧中间结果）
        _main_camera_pass_post_process_buffer_even      = 6,  // 后处理缓冲区（偶数帧中间结果）
        _main_camera_pass_depth                         = 7,  // 深度缓冲区（存储深度信息）
        _main_camera_pass_swap_chain_image              = 8,  // 交换链图像（最终输出到屏幕的图像）
        _main_camera_pass_custom_attachment_count       = 5,  // 自定义附件数量（前5个非内置附件）
        _main_camera_pass_post_process_attachment_count = 2,  // 后处理附件数量（奇偶双缓冲）
        _main_camera_pass_attachment_count              = 9,  // 总附件数量（所有索引范围0~8）
    };

    // ============================== 主相机渲染子通道枚举 ==============================
    // 定义主相机渲染流程中的各个子阶段（子通道）
    // Vulkan中一个渲染通道可包含多个子通道，按顺序执行不同的渲染操作
    enum
    {
        _main_camera_subpass_basepass = 0,       // 基础通道：渲染不透明物体的GBuffer（位置/法线/材质）
        _main_camera_subpass_deferred_lighting,  // 延迟光照：基于GBuffer计算光照（适用于复杂光照场景）
        _main_camera_subpass_forward_lighting,   // 前向光照：渲染透明物体或需要逐物体光照的对象
        _main_camera_subpass_tone_mapping,       // 色调映射：将HDR颜色转换为LDR（适应屏幕显示）
        _main_camera_subpass_color_grading,      // 颜色分级：调整色彩风格（如对比度、饱和度）
        _main_camera_subpass_fxaa,               // FXAA抗锯齿：快速近似抗锯齿处理
        _main_camera_subpass_ui,                 // UI渲染：绘制2D界面元素
        _main_camera_subpass_combine_ui,         // UI合成：将UI与3D场景合并到最终图像
        _main_camera_subpass_count               // 子通道总数（用于数组大小或遍历）
    };

    // ============================== 可见节点信息结构体 ==============================
    // 存储当前帧中各类可见的渲染节点（用于优化渲染流程，仅渲染可见对象）
    struct VisiableNodes
    {
        // 方向光可见的网格节点
        std::vector<RenderMeshNode>* p_directional_light_visible_mesh_nodes{ nullptr };
        // 点光源可见的网格节点
        std::vector<RenderMeshNode>* p_point_lights_visible_mesh_nodes{ nullptr };
        // 主相机可见的网格节点
        std::vector<RenderMeshNode>* p_main_camera_visible_mesh_nodes{ nullptr };
        // 轴节点（用于调试绘制坐标轴）
        RenderAxisNode* p_axis_node{ nullptr };
    };

    // ============================== Vulkan渲染通道实现类 ==============================
    // 继承自RenderPassBase，实现基于Vulkan API的具体渲染流程
    // 负责管理帧缓冲、渲染管线、描述符集等Vulkan资源，并执行完整的渲染流程
    class RenderPass : public RenderPassBase
    {
    public:
        // ============================== Vulkan渲染相关结构体定义 ==============================
        // 描述单个帧缓冲附件（对应Vulkan的VkImageView和关联资源）
        struct FrameBufferAttachment
        {
            RHIImage* image;       // 图像对象（存储像素数据）
            RHIDeviceMemory* mem;  // 图像内存（Vulkan中图像需绑定显存）
            RHIImageView* view;    // 图像视图（描述图像的子资源范围）
            RHIFormat format;      // 图像格式（如VK_FORMAT_R8G8B8A8_UNORM）
        };

        // 描述完整的帧缓冲对象（对应Vulkan的VkFramebuffer）
        struct Framebuffer
        {
            int width;                    // 帧缓冲宽度（与交换链图像尺寸一致）
            int height;                   // 帧缓冲高度（与交换链图像尺寸一致）
            RHIFramebuffer* framebuffer;  // Vulkan帧缓冲对象（管理附件的组合）
            RHIRenderPass* render_pass;   // 关联的渲染通道对象（定义渲染流程）
            // 该帧缓冲的所有附件列表
            std::vector<FrameBufferAttachment> attachments;
        };

        // 描述描述符集布局及实例（用于绑定着色器资源）
        struct Descriptor
        {
            RHIDescriptorSetLayout* layout;    // 描述符集布局（定义资源类型与绑定点）
            RHIDescriptorSet* descriptor_set;  // 描述符集实例（存储实际的资源数据）
        };

        // 描述渲染管线的基础信息（Vulkan中管线由布局和具体管线组成）
        struct RenderPipelineBase
        {
            // 管线布局（定义着色器资源和渲染状态）
            RHIPipelineLayout* layout;
            // 具体的渲染管线对象（Vulkan的VkPipeline）
            RHIPipeline* pipeline;
        };

        // ============================== 成员变量 ==============================
        // 全局渲染资源指针（访问全局配置、相机等）
        GlobalRenderResource* m_global_render_resource{ nullptr };

        // 描述符集信息列表（存储所有需要的描述符布局和实例）
        std::vector<Descriptor>         m_descriptor_infos;
        // 渲染管线列表（存储不同阶段的管线，如光照、UI）
        std::vector<RenderPipelineBase> m_render_pipelines;
        // 当前帧缓冲对象（管理附件和渲染流程）
        Framebuffer                     m_framebuffer;

        // ============================== 核心接口实现 ==============================
        // 实现基类的初始化函数（Vulkan渲染通道的初始化逻辑）
        void initialize(const RenderPassInitInfo* init_info) override;

        // 实现基类的后初始化函数（完成资源绑定、管线编译等操作）
        void postInitialize() override;

        // 实现绘制函数（执行完整的渲染流程：从GBuffer到最终输出）
        virtual void draw();

        // ============================== 资源获取接口 ==============================
        // 获取当前渲染通道的Vulkan渲染通道对象
        virtual RHIRenderPass* getRenderPass() const;
        // 获取帧缓冲中所有图像视图的列表（用于后续渲染通道的输入附件）
        virtual std::vector<RHIImageView*> getFramebufferImageViews() const;
        // 获取所有描述符集布局的列表（用于动态绑定资源）
        virtual std::vector<RHIDescriptorSetLayout*> getDescriptorSetLayouts() const;

        // ============================== 静态成员 ==============================
        // 静态可见节点容器（全局可访问，存储当前帧可见对象）
        static VisiableNodes m_visiable_nodes;

    private:
    };
}

#pragma once

#include "runtime/function/render/interface/rhi.h"                         // 抽象RHI接口
#include "runtime/function/render/interface/vulkan/vulkan_rhi_resource.h"  // Vulkan资源实现

#include <vk_mem_alloc.h>   // Vulkan内存分配库头文件
#include <vulkan/vulkan.h>  // Vulkan核心头文件

#include <functional>
#include <map>
#include <vector>

namespace Sammi
{

    // Vulkan渲染硬件接口实现类（继承自抽象RHI接口）
    class VulkanRHI final : public RHI
    {
    public:
        // ===================== 初始化接口 =====================
        virtual void initialize(RHIInitInfo init_info) override final;  // 初始化Vulkan环境
        virtual void prepareContext() override final;                   // 准备渲染上下文

        // ===================== 资源创建接口 =====================
        bool allocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer* &pCommandBuffers) override;
        bool allocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet* &pDescriptorSets) override;
        void createSwapchain() override;                // 创建交换链
        void recreateSwapchain() override;              // 重新创建交换链（当窗口大小改变时）
        void createSwapchainImageViews() override;      // 创建交换链图像的视图
        void createFramebufferImageAndView() override;  // 创建帧缓冲图像和视图

        // 采样器管理
        RHISampler* getOrCreateDefaultSampler(RHIDefaultSamplerType type) override;      // 获取或创建默认采样器
        RHISampler* getOrCreateMipmapSampler(uint32_t width, uint32_t height) override;  // 获取或创建Mipmap采样器

        // 着色器管理
        RHIShader* createShaderModule(const std::vector<unsigned char>& shader_code) override;  // 创建着色器模块

        // 缓冲区管理
        void createBuffer(RHIDeviceSize size,
                          RHIBufferUsageFlags usage,
                          RHIMemoryPropertyFlags properties,
                          RHIBuffer* &buffer,
                          RHIDeviceMemory* &buffer_memory) override;

        void createBufferAndInitialize(RHIBufferUsageFlags usage,
                                       RHIMemoryPropertyFlags properties,
                                       RHIBuffer*& buffer,
                                       RHIDeviceMemory*& buffer_memory,
                                       RHIDeviceSize size,
                                       void* data = nullptr,
                                       int datasize = 0) override;

        // 使用VMA（Vulkan内存分配器）创建缓冲区
        bool createBufferVMA(VmaAllocator allocator,
                             const RHIBufferCreateInfo* pBufferCreateInfo,
                             const VmaAllocationCreateInfo* pAllocationCreateInfo,
                             RHIBuffer* &pBuffer,
                             VmaAllocation* pAllocation,
                             VmaAllocationInfo* pAllocationInfo) override;

        // 带对齐要求的VMA缓冲区创建
        bool createBufferWithAlignmentVMA(VmaAllocator allocator,
                                          const RHIBufferCreateInfo* pBufferCreateInfo,
                                          const VmaAllocationCreateInfo* pAllocationCreateInfo,
                                          RHIDeviceSize minAlignment,
                                          RHIBuffer* &pBuffer,
                                          VmaAllocation* pAllocation,
                                          VmaAllocationInfo* pAllocationInfo) override;

        // 缓冲区操作
        void copyBuffer(RHIBuffer* srcBuffer,
                        RHIBuffer* dstBuffer,
                        RHIDeviceSize srcOffset,
                        RHIDeviceSize dstOffset,
                        RHIDeviceSize size) override;

        // 图像管理
        void createImage(uint32_t image_width,
                         uint32_t image_height,
                         RHIFormat format,
                         RHIImageTiling image_tiling,
                         RHIImageUsageFlags image_usage_flags,
                         RHIMemoryPropertyFlags memory_property_flags,
                         RHIImage* &image,
                         RHIDeviceMemory* &memory,
                         RHIImageCreateFlags image_create_flags,
                         uint32_t array_layers,
                         uint32_t miplevels) override;

        void createImageView(RHIImage* image,
                             RHIFormat format,
                             RHIImageAspectFlags image_aspect_flags,
                             RHIImageViewType view_type,
                             uint32_t layout_count,
                             uint32_t miplevels,
                             RHIImageView* &image_view) override;

        // 全局图像（纹理）创建
        void createGlobalImage(RHIImage* &image,
                               RHIImageView* &image_view,
                               VmaAllocation& image_allocation,
                               uint32_t texture_image_width,
                               uint32_t texture_image_height,
                               void* texture_image_pixels,
                               RHIFormat texture_image_format,
                               uint32_t miplevels = 0) override;

        // 立方体贴图创建
        void createCubeMap(RHIImage* &image,
                           RHIImageView* &image_view,
                           VmaAllocation& image_allocation,
                           uint32_t texture_image_width,
                           uint32_t texture_image_height,
                           std::array<void*, 6> texture_image_pixels,
                           RHIFormat texture_image_format,
                           uint32_t miplevels) override;

        // 其他Vulkan对象创建
        bool createCommandPool(const RHICommandPoolCreateInfo* pCreateInfo,
                                     RHICommandPool* &pCommandPool) override;

        bool createDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo,
                                        RHIDescriptorPool* &pDescriptorPool) override;

        bool createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo,
                                             RHIDescriptorSetLayout* &pSetLayout) override;

        bool createFence(const RHIFenceCreateInfo* pCreateInfo,
                               RHIFence* &pFence) override;

        bool createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo,
                                     RHIFramebuffer* &pFramebuffer) override;

        bool createGraphicsPipelines(RHIPipelineCache* pipelineCache,
                                     uint32_t createInfoCount,
                               const RHIGraphicsPipelineCreateInfo* pCreateInfos,
                                     RHIPipeline* &pPipelines) override;

        bool createComputePipelines(RHIPipelineCache* pipelineCache,
                                    uint32_t createInfoCount,
                              const RHIComputePipelineCreateInfo* pCreateInfos,
                                    RHIPipeline*& pPipelines) override;

        bool createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo,
                                        RHIPipelineLayout* &pPipelineLayout) override;

        bool createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo,
                                    RHIRenderPass* &pRenderPass) override;

        bool createSampler(const RHISamplerCreateInfo* pCreateInfo,
                                 RHISampler* &pSampler) override;

        bool createSemaphore(const RHISemaphoreCreateInfo* pCreateInfo,
                                   RHISemaphore* &pSemaphore) override;

        // ===================== 命令操作接口 =====================
        // 同步原语操作 (使用函数指针扩展)
        bool waitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) override;
        bool resetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) override;
        bool resetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) override;
        // 命令缓冲区操作
        bool beginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) override;
        bool endCommandBufferPFN(RHICommandBuffer* commandBuffer) override;
        // 渲染通道操作
        void cmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, RHISubpassContents contents) override;
        void cmdNextSubpassPFN(RHICommandBuffer* commandBuffer, RHISubpassContents contents) override;
        void cmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) override;
        // 管线状态设置
        void cmdBindPipelinePFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) override;
        void cmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const RHIViewport* pViewports) override;
        void cmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const RHIRect2D* pScissors) override;
        // 资源绑定
        void cmdBindVertexBuffersPFN(RHICommandBuffer* commandBuffer,
                                     uint32_t firstBinding,
                                     uint32_t bindingCount,
                                     RHIBuffer* const* pBuffers,
                               const RHIDeviceSize* pOffsets) override;
        void cmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, RHIIndexType indexType) override;
        void cmdBindDescriptorSetsPFN(
            RHICommandBuffer* commandBuffer,
            RHIPipelineBindPoint pipelineBindPoint,
            RHIPipelineLayout* layout,
            uint32_t firstSet,
            uint32_t descriptorSetCount,
            const RHIDescriptorSet* const* pDescriptorSets,
            uint32_t dynamicOffsetCount,
            const uint32_t* pDynamicOffsets) override;

        // 绘制命令
        void cmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
        // 附件清除
        void cmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const RHIClearAttachment* pAttachments, uint32_t rectCount, const RHIClearRect* pRects) override;
        // 其他命令操作
        bool beginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) override;
        // 资源复制
        void cmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions) override;
        void cmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageAspectFlagBits srcFlag, RHIImage* dstImage, RHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) override;
        void cmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions) override;
        // 直接绘制/调度
        void cmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void cmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        void cmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) override;
        // 管线屏障（同步操作）
        void cmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const RHIImageMemoryBarrier* pImageMemoryBarriers) override;
        bool endCommandBuffer(RHICommandBuffer* commandBuffer) override;
        // 描述符集更新
        void updateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies) override;
        // 队列操作
        bool queueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence) override;
        bool queueWaitIdle(RHIQueue* queue) override;
        void resetCommandPool() override;
        void waitForFences() override;
        bool waitForFences(uint32_t fenceCount, const RHIFence* const* pFences, RHIBool32 waitAll, uint64_t timeout);

        // ===================== 状态查询接口 =====================
        void getPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties) override;
        RHICommandBuffer* getCurrentCommandBuffer() const override;
        RHICommandBuffer* const* getCommandBufferList() const override;
        RHICommandPool* getCommandPool() const override;
        RHIDescriptorPool* getDescriptorPool()const override;
        RHIFence* const* getFenceList() const override;
        QueueFamilyIndices getQueueFamilyIndices() const override;
        RHIQueue* getGraphicsQueue() const override;
        RHIQueue* getComputeQueue() const override;
        RHISwapChainDesc getSwapchainInfo() override;
        RHIDepthImageDesc getDepthImageInfo() const override;
        uint8_t getMaxFramesInFlight() const override;  // 获取最大帧数(通常为2-3)
        uint8_t getCurrentFrameIndex() const override;  // 获取当前帧索引
        void setCurrentFrameIndex(uint8_t index) override;  // 设置当前帧索引

        // ===================== 辅助操作接口 =====================
        RHICommandBuffer* beginSingleTimeCommands() override;  // 开始一次性命令（用于资源上传）
        void            endSingleTimeCommands(RHICommandBuffer* command_buffer) override;  // 提交并执行一次性命令
        bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) override;// 渲染通道前准备
        void submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) override;// 提交渲染命令
        // 调试标记
        void pushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) override;
        void popEvent(RHICommandBuffer* commond_buffer) override;

        // ===================== 资源销毁接口 =====================
        virtual ~VulkanRHI() override final;
        void clear() override;  // 清理所有资源
        void clearSwapchain() override;  // 清理交换链相关资源
        void destroyDefaultSampler(RHIDefaultSamplerType type) override;  // 销毁默认采样器
        void destroyMipmappedSampler() override;  // 销毁Mipmap采样器
        void destroyShaderModule(RHIShader* shader) override;  // 销毁着色器模块
        void destroySemaphore(RHISemaphore* semaphore) override;  // 销毁信号量
        void destroySampler(RHISampler* sampler) override;  // 销毁采样器
        void destroyInstance(RHIInstance* instance) override;  // 销毁Vulkan实例
        void destroyImageView(RHIImageView* imageView) override;  // 销毁图像视图
        void destroyImage(RHIImage* image) override;  // 销毁图像
        void destroyFramebuffer(RHIFramebuffer* framebuffer) override;  // 销毁帧缓冲
        void destroyFence(RHIFence* fence) override;  // 销毁栅栏
        void destroyDevice() override;  // 销毁逻辑设备
        void destroyCommandPool(RHICommandPool* commandPool) override;  // 销毁命令池
        void destroyBuffer(RHIBuffer* &buffer) override;  // 销毁缓冲区
        void freeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) override;  // 释放命令缓冲区

        // ===================== 内存操作接口 =====================
        void freeMemory(RHIDeviceMemory* &memory) override;  // 释放设备内存
        bool mapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) override;  // 映射设备内存
        void unmapMemory(RHIDeviceMemory* memory) override;  // 取消映射
        void invalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override;  // 使缓存失效
        void flushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override;  // 刷新缓存
        
        // ===================== 同步原语接口 =====================
        RHISemaphore* &getTextureCopySemaphore(uint32_t index) override;  // 获取纹理复制信号量
    public:
        static uint8_t const k_max_frames_in_flight {3};  // 最大帧数(防止CPU过提交GPU)

        // ===================== Vulkan核心对象 =====================
        RHIQueue* m_graphics_queue{ nullptr };  // 图形队列
        RHIQueue* m_compute_queue{ nullptr };  // 计算队列

        // 交换链属性
        
        
        
        RHIViewport m_viewport;                                // 视口设置
        

        #pragma region 1-Instance

        VkInstance m_instance{ nullptr };

        #pragma endregion




        #pragma region 3-WindowSurface

        VkSurfaceKHR m_surface{ nullptr };

        #pragma endregion



        #pragma region 4-SwapChain

        VkSwapchainKHR           m_swapchain{ nullptr };
        std::vector<VkImage>     m_swapchain_images;
        RHIFormat m_swapchain_image_format{ RHI_FORMAT_UNDEFINED };  // 交换链图像格式
        RHIExtent2D m_swapchain_extent;                        // 交换链尺寸
        RHIRect2D m_scissor;                                    // 裁剪区域
        std::vector<RHIImageView*> m_swapchain_imageviews;      // 交换链图像视图

        #pragma endregion



        #pragma region 5-Pipeline



        #pragma endregion


        // 深度缓冲属性
        RHIFormat m_depth_image_format{ RHI_FORMAT_UNDEFINED };   // 深度图像格式
        RHIImageView* m_depth_image_view = new VulkanImageView();  // 深度图像视图

        // 帧同步原语
        RHIFence* m_rhi_is_frame_in_flight_fences[k_max_frames_in_flight];

        RHIDescriptorPool* m_descriptor_pool = new VulkanDescriptorPool();

        RHICommandPool* m_rhi_command_pool; 

        RHICommandBuffer* m_command_buffers[k_max_frames_in_flight];
        RHICommandBuffer* m_current_command_buffer = new VulkanCommandBuffer();

        QueueFamilyIndices m_queue_indices;

        GLFWwindow*        m_window {nullptr};
        
        
        VkPhysicalDevice   m_physical_device {nullptr};
        VkDevice           m_device {nullptr};
        VkQueue            m_present_queue {nullptr};

        

        RHIImage*        m_depth_image = new VulkanImage();
        VkDeviceMemory m_depth_image_memory {nullptr};

        std::vector<VkFramebuffer> m_swapchain_framebuffers;

        // asset allocator use VMA library
        VmaAllocator m_assets_allocator;

        // function pointers
        PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT   _vkCmdEndDebugUtilsLabelEXT;
        PFN_vkWaitForFences         _vkWaitForFences;
        PFN_vkResetFences           _vkResetFences;
        PFN_vkResetCommandPool      _vkResetCommandPool;
        PFN_vkBeginCommandBuffer    _vkBeginCommandBuffer;
        PFN_vkEndCommandBuffer      _vkEndCommandBuffer;
        PFN_vkCmdBeginRenderPass    _vkCmdBeginRenderPass;
        PFN_vkCmdNextSubpass        _vkCmdNextSubpass;
        PFN_vkCmdEndRenderPass      _vkCmdEndRenderPass;
        PFN_vkCmdBindPipeline       _vkCmdBindPipeline;
        PFN_vkCmdSetViewport        _vkCmdSetViewport;
        PFN_vkCmdSetScissor         _vkCmdSetScissor;
        PFN_vkCmdBindVertexBuffers  _vkCmdBindVertexBuffers;
        PFN_vkCmdBindIndexBuffer    _vkCmdBindIndexBuffer;
        PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
        PFN_vkCmdDrawIndexed        _vkCmdDrawIndexed;
        PFN_vkCmdClearAttachments   _vkCmdClearAttachments;

        // global descriptor pool
        VkDescriptorPool m_vk_descriptor_pool;

        // command pool and buffers
        uint8_t              m_current_frame_index {0};
        VkCommandPool        m_command_pools[k_max_frames_in_flight];
        VkCommandBuffer      m_vk_command_buffers[k_max_frames_in_flight];
        VkSemaphore          m_image_available_for_render_semaphores[k_max_frames_in_flight];
        VkSemaphore          m_image_finished_for_presentation_semaphores[k_max_frames_in_flight];
        RHISemaphore*        m_image_available_for_texturescopy_semaphores[k_max_frames_in_flight];
        VkFence              m_is_frame_in_flight_fences[k_max_frames_in_flight];

        // TODO: set
        VkCommandBuffer   m_vk_current_command_buffer;

        uint32_t m_current_swapchain_image_index;

    private:
        const std::vector<char const*> m_validation_layers {"VK_LAYER_KHRONOS_validation"};
        uint32_t                       m_vulkan_api_version {VK_API_VERSION_1_0};

        std::vector<char const*> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // default sampler cache
        RHISampler* m_linear_sampler = nullptr;
        RHISampler* m_nearest_sampler = nullptr;
        std::map<uint32_t, RHISampler*> m_mipmap_sampler_map;

    private:
        void createInstance();
        void initializeDebugMessenger();
        void createWindowSurface();
        void initializePhysicalDevice();
        void createLogicalDevice();
        void createCommandPool() override;;
        void createCommandBuffers();
        void createDescriptorPool();
        void createSyncPrimitives();
        void createAssetAllocator();

    public:
        bool isPointLightShadowEnabled() override;

    private:
        bool m_enable_validation_Layers{ true };
        bool m_enable_debug_utils_label{ true };
        bool m_enable_point_light_shadow{ true };

        // used in descriptor pool creation
        uint32_t m_max_vertex_blending_mesh_count{ 256 };
        uint32_t m_max_material_count{ 256 };

        bool                     checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);



        VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
        VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);




        QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice physical_device);
        bool                    checkDeviceExtensionSupport(VkPhysicalDevice physical_device);
        bool                    isDeviceSuitable(VkPhysicalDevice physical_device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device);

        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

        VkSurfaceFormatKHR
        chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats);
        VkPresentModeKHR
                   chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);
    };
}
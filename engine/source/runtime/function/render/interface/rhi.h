#pragma once

// 引入 GLFW 和 Vulkan 库，GLFW_INCLUDE_VULKAN 宏确保 GLFW 包含 Vulkan 头文件
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// 引入Vulkan内存分配库 (VMA)
#include <vk_mem_alloc.h>

// 标准库头文件
#include <memory>
#include <vector>
#include <functional>

// 自定义RHI结构体定义
#include "rhi_struct.h"

namespace Sammi
{
    // 前向声明窗口系统类
    class WindowSystem;

    // RHI初始化信息结构体
    struct RHIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;  // 窗口系统智能指针
    };
    
    // 抽象RHI（渲染硬件接口）基类
    class RHI
    {
    public:
        // 虚析构函数（纯虚函数）
        virtual ~RHI() = 0;

        // 核心初始化方法
        virtual void initialize(RHIInitInfo initialize_info) = 0;
        // 准备渲染上下文
        virtual void prepareContext() = 0;

        // 功能查询
        virtual bool isPointLightShadowEnabled() = 0;  // 点光源阴影是否启用

        //--------- 资源创建与分配方法 ---------//
        virtual bool allocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer* &pCommandBuffers) = 0;
        virtual bool allocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet* &pDescriptorSets) = 0;
        // 创建交换链
        virtual void createSwapchain() = 0;
        // 重建交换链（窗口大小改变时）
        virtual void recreateSwapchain() = 0;
        // 创建交换链图像视图
        virtual void createSwapchainImageViews() = 0;
        // 创建帧缓冲图像和视图
        virtual void createFramebufferImageAndView() = 0;
        // 获取/创建默认采样器
        virtual RHISampler* getOrCreateDefaultSampler(RHIDefaultSamplerType type) = 0;
        // 获取/创建Mipmap采样器
        virtual RHISampler* getOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;
        // 创建着色器模块
        virtual RHIShader* createShaderModule(const std::vector<unsigned char>& shader_code) = 0;  
        // 创建缓冲区
        virtual void createBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer* &buffer, RHIDeviceMemory* &buffer_memory) = 0;
        // 带初始化的缓冲区创建
        virtual void createBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;
        // 使用VMA创建缓冲区
        virtual bool createBufferVMA(VmaAllocator allocator, const RHIBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, RHIBuffer*& pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo) = 0;
        // 带对齐要求的VMA缓冲区创建
        virtual bool createBufferWithAlignmentVMA(VmaAllocator allocator, const RHIBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, RHIDeviceSize minAlignment, RHIBuffer* &pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo) = 0;
        // 缓冲区拷贝
        virtual void copyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) = 0;
        // 创建图像
        virtual void createImage(uint32_t image_width, uint32_t image_height, RHIFormat format, RHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags, RHIImage* &image, RHIDeviceMemory* &memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) = 0;
        // 创建图像视图
        virtual void createImageView(RHIImage* image, RHIFormat format, RHIImageAspectFlags image_aspect_flags, RHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels, RHIImageView* &image_view) = 0;
        // 创建全局纹理图像
        virtual void createGlobalImage(RHIImage* &image, RHIImageView* &image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels = 0) = 0;
        // 创建立方体贴图
        virtual void createCubeMap(RHIImage* &image, RHIImageView* &image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels) = 0;
        // 创建命令池
        virtual void createCommandPool() = 0;
        virtual bool createCommandPool(const RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool) = 0;
        // 创建描述符池
        virtual bool createDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool* &pDescriptorPool) = 0;
        // 创建描述符集布局
        virtual bool createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout* &pSetLayout) = 0;
        // 创建栅栏
        virtual bool createFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence* &pFence) = 0;
        // 创建帧缓冲
        virtual bool createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer* &pFramebuffer) = 0;
        // 创建图形管线
        virtual bool createGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline* &pPipelines) = 0;
        // 创建计算管线
        virtual bool createComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline* &pPipelines) = 0;
        // 创建管线布局
        virtual bool createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout* &pPipelineLayout) = 0;
        // 创建渲染通道
        virtual bool createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass* &pRenderPass) = 0;
        // 创建采样器
        virtual bool createSampler(const RHISamplerCreateInfo* pCreateInfo, RHISampler* &pSampler) = 0;
        // 创建信号量
        virtual bool createSemaphore(const RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore* &pSemaphore) = 0;

        //--------- 命令记录与同步方法 ---------//
        // PFN后缀表示与Vulkan原生函数类似的接口
        // 等待栅栏
        virtual bool waitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) = 0;
        // 重置栅栏
        virtual bool resetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) = 0;
        // 重置命令池
        virtual bool resetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) = 0;
        // 开始命令缓冲记录
        virtual bool beginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) = 0;
        // 结束命令缓冲记录
        virtual bool endCommandBufferPFN(RHICommandBuffer* commandBuffer) = 0;
        // 开始渲染通道
        virtual void cmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, RHISubpassContents contents) = 0;
        // 进入下一子通道
        virtual void cmdNextSubpassPFN(RHICommandBuffer* commandBuffer, RHISubpassContents contents) = 0;
        // 结束渲染通道
        virtual void cmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) = 0;
        // 绑定管线
        virtual void cmdBindPipelinePFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) = 0;
        // 设置视口
        virtual void cmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const RHIViewport* pViewports) = 0;
        // 设置裁剪
        virtual void cmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const RHIRect2D* pScissors) = 0;
        // 绑定顶点缓冲区
        virtual void cmdBindVertexBuffersPFN(RHICommandBuffer* commandBuffer, uint32_t firstBinding, uint32_t bindingCount, RHIBuffer* const* pBuffers, const RHIDeviceSize* pOffsets) = 0;
        // 绑定索引缓冲区
        virtual void cmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, RHIIndexType indexType) = 0;
        // 绑定描述符集
        virtual void cmdBindDescriptorSetsPFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipelineLayout* layout, uint32_t firstSet, uint32_t descriptorSetCount, const RHIDescriptorSet* const* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) = 0;
        // 绘制索引几何体
        virtual void cmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
        // 清除附件
        virtual void cmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const RHIClearAttachment* pAttachments, uint32_t rectCount, const RHIClearRect* pRects) = 0;

        // 等效命令方法（非PFN后缀）
        virtual bool beginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) = 0;
        virtual void cmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions) = 0;
        virtual void cmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageAspectFlagBits srcFlag, RHIImage* dstImage, RHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) = 0;
        virtual void cmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions) = 0;
        virtual void cmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
        virtual void cmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void cmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) = 0;
        virtual void cmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const RHIImageMemoryBarrier* pImageMemoryBarriers) = 0;
        virtual bool endCommandBuffer(RHICommandBuffer* commandBuffer) = 0;
        virtual void updateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies) = 0;
        virtual bool queueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence) = 0;
        virtual bool queueWaitIdle(RHIQueue* queue) = 0;
        virtual void resetCommandPool() = 0;
        virtual void waitForFences() = 0;

        //--------- 状态查询方法 ---------//
        
        // 获取物理设备属性
        virtual void getPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties) = 0;
        // 获取当前命令缓冲
        virtual RHICommandBuffer* getCurrentCommandBuffer() const = 0;
        // 获取命令缓冲列表
        virtual RHICommandBuffer* const* getCommandBufferList() const = 0;
        // 获取命令池
        virtual RHICommandPool* getCommandPool() const = 0;
        // 获取描述符池（同上）
        virtual RHIDescriptorPool* getDescriptorPool() const = 0;
        // 获取栅栏列表
        virtual RHIFence* const* getFenceList() const = 0;
        // 获取队列族索引
        virtual QueueFamilyIndices getQueueFamilyIndices() const = 0;
        // 获取图形队列
        virtual RHIQueue* getGraphicsQueue() const = 0;
        // 获取计算队列
        virtual RHIQueue* getComputeQueue() const = 0;
        // 获取交换链信息
        virtual RHISwapChainDesc getSwapchainInfo() = 0;
        // 获取深度图像信息
        virtual RHIDepthImageDesc getDepthImageInfo() const = 0;
        // 获取最大帧并发数
        virtual uint8_t getMaxFramesInFlight() const = 0;
        // 获取当前帧索引
        virtual uint8_t getCurrentFrameIndex() const = 0;
        // 设置当前帧索引
        virtual void setCurrentFrameIndex(uint8_t index) = 0;

        //--------- 高级命令封装 ---------//

        // 开始一次性命令
        virtual RHICommandBuffer* beginSingleTimeCommands() = 0;
        // 提交一次性命令
        virtual void endSingleTimeCommands(RHICommandBuffer* command_buffer) = 0;
        // 渲染前准备
        virtual bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
        // 提交渲染命令
        virtual void submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
        // 开始调试事件
        virtual void pushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) = 0;
        // 结束调试事件
        virtual void popEvent(RHICommandBuffer* commond_buffer) = 0;

        //--------- 资源销毁方法 ---------//

        // 清理所有RHI资源
        virtual void clear() = 0;
        // 清理交换链资源
        virtual void clearSwapchain() = 0;
        // 销毁默认采样器
        virtual void destroyDefaultSampler(RHIDefaultSamplerType type) = 0;
        // 销毁Mipmap采样器
        virtual void destroyMipmappedSampler() = 0;
        // 销毁着色器模块
        virtual void destroyShaderModule(RHIShader* shader) = 0;
        // 销毁信号量
        virtual void destroySemaphore(RHISemaphore* semaphore) = 0;
        // 销毁采样器
        virtual void destroySampler(RHISampler* sampler) = 0;
        // 销毁RHI实例
        virtual void destroyInstance(RHIInstance* instance) = 0;
        // 销毁图像视图
        virtual void destroyImageView(RHIImageView* imageView) = 0;
        // 销毁图像
        virtual void destroyImage(RHIImage* image) = 0;
        // 销毁帧缓冲
        virtual void destroyFramebuffer(RHIFramebuffer* framebuffer) = 0;
        // 销毁栅栏
        virtual void destroyFence(RHIFence* fence) = 0;
        // 销毁逻辑设备
        virtual void destroyDevice() = 0;
        // 销毁命令池
        virtual void destroyCommandPool(RHICommandPool* commandPool) = 0;
        // 销毁缓冲区
        virtual void destroyBuffer(RHIBuffer* &buffer) = 0;
        // 释放命令缓冲
        virtual void freeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) = 0;

        //--------- 内存管理方法 ---------//

        // 释放设备内存
        virtual void freeMemory(RHIDeviceMemory* &memory) = 0;
        // 内存映射
        virtual bool mapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) = 0;
        // 解除内存映射
        virtual void unmapMemory(RHIDeviceMemory* memory) = 0;
        // 使映射内存无效
        virtual void invalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;
        // 刷新映射内存
        virtual void flushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;

        //--------- 同步原语访问 ---------//

        // 获取纹理拷贝信号量引用
        virtual RHISemaphore* &getTextureCopySemaphore(uint32_t index) = 0;

    private:
        // 无私有成员（接口类）
    };

    // 内联实现纯虚析构函数（避免未定义错误）
    inline RHI::~RHI() = default;
}
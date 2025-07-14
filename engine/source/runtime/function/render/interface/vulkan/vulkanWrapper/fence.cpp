
#include "fence.h"

namespace LearnVulkan::Wrapper
{
    /**
     * @brief 构造函数 - 创建Vulkan栅栏
     *
     * @param device 关联的逻辑设备
     * @param signaled 栅栏初始状态 (true=创建即为激发态, false=非激发态)
     */
    Fence::Fence(const Device::Ptr& device, bool signaled)
    {
        mDevice = device;  // 存储关联设备对象

        // 配置栅栏创建信息
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        // 调用Vulkan API创建栅栏
        if (vkCreateFence(mDevice->getDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Error:failed to create fence!");
        }
    }

    /// 析构函数 - 自动销毁栅栏资源
    Fence::~Fence()
    {
        if (mFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(mDevice->getDevice(), mFence, nullptr);
        }
    }

    /**
     * @brief 重置栅栏为非激发态
     *
     * 将栅栏从激发态(已触发)重置为非激发态(未触发)，
     * 通常在重复使用栅栏前调用。
     */
    void Fence::resetFence()
    {
        vkResetFences(mDevice->getDevice(), 1, &mFence);
    }

    /**
     * @brief 阻塞等待栅栏变为激发态
     *
     * @param timeout 超时时间(纳秒)，默认永久等待(UINT64_MAX)
     *
     * 注意：当前实现直接使用Vulkan API的阻塞等待，
     * 但未处理超时错误(应抛出异常)
     */
    void Fence::block(uint64_t timeout)
    {
        vkWaitForFences(mDevice->getDevice(),
                        1,         // 等待的栅栏数量
                        &mFence,   // 栅栏对象数组
                        VK_TRUE,   // 等待所有栅栏触发
                        timeout);  // 超时时间(纳秒)
    }

    void VulkanRHI::waitForFences()
    {
        VkResult res_wait_for_fences =
            _vkWaitForFences(m_device, 1, &m_is_frame_in_flight_fences[m_current_frame_index], VK_TRUE, UINT64_MAX);
        if (VK_SUCCESS != res_wait_for_fences)
        {
            LOG_ERROR("failed to synchronize!");
        }
    }

    bool VulkanRHI::waitForFences(uint32_t fenceCount, const RHIFence* const* pFences, RHIBool32 waitAll, uint64_t timeout)
    {
        //fence
        int fence_size = fenceCount;
        std::vector<VkFence> vk_fence_list(fence_size);
        for (int i = 0; i < fence_size; ++i)
        {
            const auto& rhi_fence_element = pFences[i];
            auto& vk_fence_element = vk_fence_list[i];

            vk_fence_element = ((VulkanFence*)rhi_fence_element)->getResource();
        };

        VkResult result = vkWaitForFences(m_device, fenceCount, vk_fence_list.data(), waitAll, timeout);

        if (result == VK_SUCCESS)
        {
            return RHI_SUCCESS;
        }
        else
        {
            LOG_ERROR("waitForFences failed");
            return false;
        }
    }

    bool VulkanRHI::createFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence)
    {
        VkFenceCreateInfo create_info{};
        create_info.sType = (VkStructureType)pCreateInfo->sType;
        create_info.pNext = (const void*)pCreateInfo->pNext;
        create_info.flags = (VkFenceCreateFlags)pCreateInfo->flags;

        pFence = new VulkanFence();
        VkFence vk_fence;
        VkResult result = vkCreateFence(m_device, &create_info, nullptr, &vk_fence);
        ((VulkanFence*)pFence)->setResource(vk_fence);

        if (result == VK_SUCCESS)
        {
            return RHI_SUCCESS;
        }
        else
        {
            LOG_ERROR("vkCreateFence failed!");
            return false;
        }
    }

    bool VulkanRHI::waitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFences, RHIBool32 waitAll, uint64_t timeout)
    {
        //fence
        int fence_size = fenceCount;
        std::vector<VkFence> vk_fence_list(fence_size);
        for (int i = 0; i < fence_size; ++i)
        {
            const auto& rhi_fence_element = pFences[i];
            auto& vk_fence_element = vk_fence_list[i];

            vk_fence_element = ((VulkanFence*)rhi_fence_element)->getResource();
        };

        VkResult result = _vkWaitForFences(m_device, fenceCount, vk_fence_list.data(), waitAll, timeout);

        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("_vkWaitForFences failed!");
            return false;
        }
    }

    bool VulkanRHI::resetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences)
    {
        //fence
        int fence_size = fenceCount;
        std::vector<VkFence> vk_fence_list(fence_size);
        for (int i = 0; i < fence_size; ++i)
        {
            const auto& rhi_fence_element = pFences[i];
            auto& vk_fence_element = vk_fence_list[i];

            vk_fence_element = ((VulkanFence*)rhi_fence_element)->getResource();
        };

        VkResult result = _vkResetFences(m_device, fenceCount, vk_fence_list.data());

        if (result == VK_SUCCESS)
        {
            return true;
        }
        else
        {
            LOG_ERROR("_vkResetFences failed!");
            return false;
        }
    }






















}
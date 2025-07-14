#include "commandPool.h"

namespace Sammi::Wrapper
{
    /**
     * @brief 创建Vulkan命令池（Command Pool）
     *
     * 命令池用于管理和分配命令缓冲（Command Buffer），是Vulkan中提交GPU命令的核心机制。
     * 此函数创建两类命令池：一个默认的图形命令池，以及多个用于帧同步的临时命令池。
     */
    CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag)
    {
        m_device = device;

        // --------------------------
        // 1. 创建默认图形命令池
        // --------------------------

        // 初始化Vulkan命令池对象（封装了Vulkan原生命令池句柄及相关操作）
        m_rhi_command_pool = new VulkanCommandPool();

        // Vulkan原生命令池句柄（用于后续API调用）
        VkCommandPool vk_command_pool;

        // 配置命令池创建信息（Vulkan API要求的结构体）
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;  // 结构体类型标识
        command_pool_create_info.pNext = NULL;                                        // 扩展指针（当前无扩展）
        // 关键标志：允许单独重置命令缓冲（无需重置整个命令池），适用于需要重复使用命令缓冲的场景
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // 指定命令池关联的队列族索引（此处为图形队列族，确保命令缓冲能被图形队列提交）
        command_pool_create_info.queueFamilyIndex = m_queue_indices.graphics_family.value();

        // 调用Vulkan API创建命令池
        if (vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &vk_command_pool) != VK_SUCCESS)
        {
            LOG_ERROR("failed to create command pool!");
        }

        // 将Vulkan原生命令池句柄保存到自定义命令池对象中（封装管理）
        ((VulkanCommandPool*)m_rhi_command_pool)->setResource(vk_command_pool);

        // --------------------------
        // 2. 创建其他帧同步用命令池（按最大飞行帧数创建）
        // --------------------------

        // 配置命令池创建信息（与默认命令池类似，但标志不同）
        VkCommandPoolCreateInfo command_pool_create_info;
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;  // 结构体类型标识
        command_pool_create_info.pNext = NULL;                                        // 扩展指针（当前无扩展）
        // 关键标志：命令池为"临时"性质（命令缓冲会被频繁分配/释放），减少内存碎片
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        // 指定命令池关联的队列族索引（仍为图形队列族）
        command_pool_create_info.queueFamilyIndex = m_queue_indices.graphics_family.value();

        // 按最大飞行帧数（k_max_frames_in_flight）创建多个命令池
        // 每帧使用独立的命令池，避免不同帧的命令缓冲相互干扰，便于资源管理和同步
        for (uint32_t i = 0; i < k_max_frames_in_flight; ++i)
        {
            // 调用Vulkan API创建命令池，结果存储到命令池数组中
            if (vkCreateCommandPool(m_device, &command_pool_create_info, NULL, &m_command_pools[i]) != VK_SUCCESS)
            {
                LOG_ERROR("failed to create command pool!");
            }
        }
    }

    CommandPool::~CommandPool()
    {
        /*if (mCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
        }*/
    }
}
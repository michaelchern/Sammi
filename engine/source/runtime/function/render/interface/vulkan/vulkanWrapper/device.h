#pragma once

#include "instance.h"
#include "windowSurface.h"

namespace Sammi::Wrapper
{
    const std::vector<const char*> deviceRequiredExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME
    };

    class Device
    {
    public:
        using Ptr = std::shared_ptr<Device>;

        static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface)
        {
            return std::make_shared<Device>(instance, surface);
        }

        Device(Instance::Ptr instance, WindowSurface::Ptr surface);

        ~Device();

        void initializePhysicalDevice();

        bool isDeviceSuitable(VkPhysicalDevice device);

        void initQueueFamilies(VkPhysicalDevice device);

        void createLogicalDevice();

        bool isQueueFamilyComplete();

		VkSampleCountFlags getMaxUsableSampleCount();

        [[nodiscard]] auto getDevice()             const { return mDevice; }
        [[nodiscard]] auto getPhysicalDevice()     const { return mPhysicalDevice; }

        [[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }
        [[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }

        [[nodiscard]] auto getGraphicQueue()       const { return mGraphicQueue; }
        [[nodiscard]] auto getPresentQueue()       const { return mPresentQueue; }
    private:
        VkPhysicalDevice   m_physical_device{ VK_NULL_HANDLE };
        Instance::Ptr      m_instance{ nullptr };
        WindowSurface::Ptr m_surface{ nullptr };

        QueueFamilyIndices m_queue_indices;

        bool m_enable_point_light_shadow{ true };

        RHIQueue* m_graphics_queue{ nullptr };
        RHIQueue* m_compute_queue{ nullptr };

        VkDevice m_device{ VK_NULL_HANDLE };




        //VkSampleCountFlagBits mMsaaSamples{ VK_SAMPLE_COUNT_1_BIT };
    };
}
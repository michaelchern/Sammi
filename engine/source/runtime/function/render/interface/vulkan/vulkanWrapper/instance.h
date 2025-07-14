#pragma once

#include "base.h"

namespace Sammi::Wrapper
{
    class Instance
    {
    public:
        using Ptr = std::shared_ptr<Instance>;

        static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

        Instance(bool enableValidationLayer);

        ~Instance();

        void printAvailableExtensions();

        std::vector<const char*> getRequiredExtensions();

        bool checkValidationLayerSupport();

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        void initializeDebugMessenger();

        [[nodiscard]] VkInstance getInstance() const { return m_instance; }

        [[nodiscard]] bool getEnableValidationLayer() const { return m_enable_validation_Layers; }

    private:
        VkInstance               m_instance{ VK_NULL_HANDLE };
        bool                     m_enable_validation_Layers{ false };
        bool                     m_enable_debug_utils_label{ true };
        VkDebugUtilsMessengerEXT m_debug_messenger{ VK_NULL_HANDLE };
    };
}
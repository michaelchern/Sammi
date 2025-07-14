
#include "vulkan_rhi.h"


namespace Sammi
{
    VulkanRHI::~VulkanRHI()
    {
        // TODO
    }


    void VulkanRHI::clear()
    {
        if (m_instance->getEnableValidationLayer())
        {
            destroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        }
    }





















    void VulkanRHI::initialize(RHIInitInfo init_info)
    {
        m_window = init_info.window_system->getWindow();

        std::array<int, 2> window_size = init_info.window_system->getWindowSize();

        m_viewport = { 0.0f, 0.0f, (float)window_size[0], (float)window_size[1], 0.0f, 1.0f };
        m_scissor = { {0, 0}, {(uint32_t)window_size[0], (uint32_t)window_size[1]} };

#ifndef NDEBUG
        m_enable_validation_Layers = true;
        m_enable_debug_utils_label = true;
#else
        m_enable_validation_Layers = false;
        m_enable_debug_utils_label = false;
#endif

#if defined(__GNUC__) && defined(__MACH__)
        m_enable_point_light_shadow = false;
#else
        m_enable_point_light_shadow = true;
#endif

#if defined(__GNUC__)
        // https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__linux__)
        char const* vk_layer_path = PICCOLO_XSTR(PICCOLO_VK_LAYER_PATH);
        setenv("VK_LAYER_PATH", vk_layer_path, 1);
#elif defined(__MACH__)
        // https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html
        char const* vk_layer_path = PICCOLO_XSTR(PICCOLO_VK_LAYER_PATH);
        char const* vk_icd_filenames = PICCOLO_XSTR(PICCOLO_VK_ICD_FILENAMES);
        setenv("VK_LAYER_PATH", vk_layer_path, 1);
        setenv("VK_ICD_FILENAMES", vk_icd_filenames, 1);
#else
#error Unknown Platform
#endif
#elif defined(_MSC_VER)
        // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
        char const* vk_layer_path = PICCOLO_XSTR(PICCOLO_VK_LAYER_PATH);
        SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
        SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
#else
#error Unknown Compiler
#endif

        m_instance = Wrapper::Instance::create(true);
		m_instance->initializeDebugMessenger();

        m_surface = Wrapper::WindowSurface::create(m_instance, m_window);


        createWindowSurface();

        initializePhysicalDevice();

        createLogicalDevice();

        createCommandPool();

        createCommandBuffers();

        createDescriptorPool();

        createSyncPrimitives();

        createSwapchain();

        createSwapchainImageViews();

        createFramebufferImageAndView();

        createAssetAllocator();
    }





















}

#include "vulkanWrapper/instance.h"

namespace Sammi
{
	class VulkanRHI final
	{
    public:
        virtual void initialize(RHIInitInfo init_info) override final;
        virtual void prepareContext() override final;






        virtual ~VulkanRHI() override final;
        void clear() override;













        
        
        Wrapper::Device::Ptr        mDevice{ nullptr };
        
        Wrapper::SwapChain::Ptr     mSwapChain{ nullptr };
        Wrapper::Pipeline::Ptr      mPipeline{ nullptr };
        Wrapper::RenderPass::Ptr    mRenderPass{ nullptr };

        Wrapper::CommandPool::Ptr                mCommandPool{ nullptr };
        std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};

        std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
        std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
        std::vector<Wrapper::Fence::Ptr>     mFences{};

        UniformManager::Ptr mUniformManager{ nullptr };
        Model::Ptr          mModel{ nullptr };
        VPMatrices          mVPMatrices;





    private:
        

	private:

        Wrapper::Window::Ptr        m_window{ nullptr };

        Wrapper::Instance::Ptr      m_instance{ nullptr };
        Wrapper::WindowSurface::Ptr m_surface{ nullptr };
        uint32_t                       m_vulkan_api_version{ VK_API_VERSION_1_0 };


        const std::vector<char const*> m_validation_layers{ "VK_LAYER_KHRONOS_validation" };

	};










}
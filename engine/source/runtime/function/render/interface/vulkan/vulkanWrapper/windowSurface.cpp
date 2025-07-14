#include"windowSurface.h"

namespace Sammi::Wrapper
{
    WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window)
    {
        m_instance = instance;
        if (glfwCreateWindowSurface(instance->getInstance(), window, nullptr, &m_surface) != VK_SUCCESS)
        {
            LOG_ERROR("failed to create surface");
        }
    }

    WindowSurface::~WindowSurface()
    {
        vkDestroySurfaceKHR(m_instance->getInstance(), m_surface, nullptr);
        m_instance.reset();
    }
}

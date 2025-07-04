#include "runtime/function/render/window_system.h"
#include "runtime/core/base/macro.h"

namespace Sammi
{
    // 析构函数：清理 GLFW 资源
    WindowSystem::~WindowSystem()
    {
        glfwDestroyWindow(m_window);  // 销毁 GLFW 窗口
        glfwTerminate();              // 终止 GLFW 库
    }

    // 初始化窗口系统
    void WindowSystem::initialize(WindowCreateInfo create_info)
    {
        // 初始化 GLFW 库
        if (!glfwInit())
        {
            LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
            return;
        }

        // 保存窗口尺寸
        m_width  = create_info.width;
        m_height = create_info.height;

        // 设置窗口提示：禁用默认的 OpenGL 上下文
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // 创建 GLFW 窗口（用于 Vulkan/OpenGL 渲染）
        m_window = glfwCreateWindow(create_info.width, create_info.height, create_info.title, nullptr, nullptr);

        if (!m_window)
        {
            LOG_FATAL(__FUNCTION__, "failed to create window");
            glfwTerminate();
            return;
        }

        // 将当前类实例指针存储到窗口用户数据中（用于回调函数中获取实例）
        glfwSetWindowUserPointer(m_window, this);

        // 绑定所有 GLFW 事件回调函数
        glfwSetKeyCallback(m_window, keyCallback);
        glfwSetCharCallback(m_window, charCallback);
        glfwSetCharModsCallback(m_window, charModsCallback);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        glfwSetCursorPosCallback(m_window, cursorPosCallback);
        glfwSetCursorEnterCallback(m_window, cursorEnterCallback);
        glfwSetScrollCallback(m_window, scrollCallback);
        glfwSetDropCallback(m_window, dropCallback);
        glfwSetWindowSizeCallback(m_window, windowSizeCallback);
        glfwSetWindowCloseCallback(m_window, windowCloseCallback);

        // 初始禁用原始鼠标输入（需要高精度时启用）
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }

    // 轮询事件：处理所有未决事件
    void WindowSystem::pollEvents() const { glfwPollEvents(); }

    // 检查窗口是否应关闭
    bool WindowSystem::shouldClose() const { return glfwWindowShouldClose(m_window); }

    // 设置窗口标题
    void WindowSystem::setTitle(const char* title) { glfwSetWindowTitle(m_window, title); }

    // 获取底层 GLFW 窗口指针
    GLFWwindow* WindowSystem::getWindow() const { return m_window; }

    // 获取当前窗口尺寸 [宽度, 高度]
    std::array<int, 2> WindowSystem::getWindowSize() const { return std::array<int, 2>({m_width, m_height}); }

    // 设置焦点模式（控制光标行为）
    void WindowSystem::setFocusMode(bool mode)
    {
        // 焦点模式：禁用光标（常见于游戏相机控制）
        // 非焦点模式：正常显示光标
        m_is_focus_mode = mode;
        glfwSetInputMode(m_window, GLFW_CURSOR, m_is_focus_mode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

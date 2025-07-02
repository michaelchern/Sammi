#include "runtime/function/render/render_pass_base.h"
#include "runtime/core/base/macro.h"

namespace Sammi
{
    // 渲染通道后初始化（默认空实现）
    void RenderPassBase::postInitialize()
    {
        // 1. 默认空实现
        // 2. 派生类可以重写这个方法
        // 3. 用于解决通道间的依赖关系（在initialize之后调用）
    }

    // 设置渲染通道的通用信息
    void RenderPassBase::setCommonInfo(RenderPassCommonInfo common_info)
    {
        // 1. 保存RHI引用（渲染硬件接口）
        m_rhi = common_info.rhi;

        // 2. 保存渲染资源管理器引用
        m_render_resource = common_info.render_resource;

        // 3. 这些引用将在派生类的初始化方法中使用
        // 4. 注意：必须在initialize之前调用
    }

    // 准备通道数据（默认空实现）
    void RenderPassBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        // 1. 默认空实现
        // 2. 派生类可以重写这个方法
        // 3. 每帧调用，用于准备通道需要的动态数据
    }

    // 初始化UI渲染后端（默认空实现）
    void RenderPassBase::initializeUIRenderBackend(WindowUI* window_ui)
    {
        // 1. 默认空实现
        // 2. 只有UI相关的通道需要重写这个方法
        // 3. 例如：UIPass会实现这个方法来初始化ImGui资源
    }
}

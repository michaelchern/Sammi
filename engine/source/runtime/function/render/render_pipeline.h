#pragma once

#include "runtime/function/render/render_pipeline_base.h"

namespace Sammi
{
    /**
     * @brief 具体渲染管线实现类（继承自RenderPipelineBase）
     *
     * 该类实现了RenderPipelineBase中定义的所有纯虚函数，提供了具体的渲染流程逻辑。
     * 负责管理渲染通道的执行顺序、资源初始化、交互功能（如鼠标拾取）以及调试相关操作（如坐标轴显示）。
     * 作为最终派生类，通过`override final`修饰符确保接口实现的稳定性，禁止后续派生类修改核心渲染逻辑。
     */
    class RenderPipeline : public RenderPipelineBase
    {
    public:
        /**
         * @brief 初始化渲染管线（具体实现）
         * @param init_info 渲染管线初始化配置（包含抗锯齿开关、渲染资源等）
         * 实现基类的纯虚函数，完成渲染通道的创建、资源绑定、RHI上下文初始化等具体操作。
         * 例如：创建各渲染通道实例（方向光、主相机等）、分配GPU资源、设置初始渲染状态等。
         */
        virtual void initialize(RenderPipelineInitInfo init_info) override final;

        /**
         * @brief 执行前向渲染流程（具体实现）
         * @param rhi 渲染硬件接口（访问底层图形API，如Vulkan/DirectX）
         * @param render_resource 渲染资源管理器（提供纹理、缓冲区等资源）
         * 实现基类的纯虚函数，完成前向渲染的具体逻辑：
         * - 按顺序执行各渲染通道（如主相机渲染、阴影渲染、粒子渲染等）
         * - 应用光照计算、材质着色、深度测试等前向渲染关键步骤
         * - 输出最终颜色到帧缓冲
         */
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource) override final;

        /**
         * @brief 执行延迟渲染流程（具体实现）
         * @param rhi 渲染硬件接口（访问底层图形API）
         * @param render_resource 渲染资源管理器（提供纹理、缓冲区等资源）
         * 实现基类的纯虚函数，完成延迟渲染的具体逻辑：
         * - 几何阶段：将物体信息（位置、法线、材质）写入GBuffer（多渲染目标）
         * - 光照阶段：基于GBuffer数据计算复杂光照（如多光源叠加、全局光照）
         * - 后期处理：合并GBuffer结果与后期效果（如色彩分级、抗锯齿）
         */
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource) override final;

        /**
         * @brief 交换链重建后更新渲染通道
         * 当窗口尺寸变化或GPU上下文丢失导致交换链（Swapchain）需要重建时，
         * 调用此函数重新初始化渲染通道的目标纹理、视口等依赖交换链的资源。
         */
        void passUpdateAfterRecreateSwapchain();

        /**
         * @brief 获取被拾取网格的GUID（具体实现）
         * @param picked_uv 屏幕空间UV坐标（鼠标点击位置的归一化坐标）
         * 实现基类的纯虚函数，通过读取拾取渲染通道生成的深度/ID纹理，
         * 将屏幕坐标转换为场景中的网格实例ID，用于交互选择（如点击选中物体）。
         * @return 被拾取网格的唯一标识GUID（若未命中返回无效值）
         */
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) override final;

        /**
         * @brief 设置坐标轴的可见状态
         * @param state 可见状态（true=显示，false=隐藏）
         * 控制编辑器模式下坐标轴辅助对象（m_render_axis）的渲染开关，
         * 用于调试场景物体的位置和朝向。
         */
        void setAxisVisibleState(bool state);

        /**
         * @brief 设置当前选中的坐标轴索引
         * @param selected_axis 选中的坐标轴索引（如0=X轴，1=Y轴，2=Z轴）
         * 用于编辑器交互中高亮显示选中的坐标轴（如拖拽调整物体位置时），
         * 通常与UI控件联动，反馈用户操作。
         */
        void setSelectedAxis(size_t selected_axis);
    };
}

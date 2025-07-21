#pragma once

#include "runtime/function/render/render_entity.h"
#include "runtime/function/render/render_guid_allocator.h"
#include "runtime/function/render/render_swap_context.h"
#include "runtime/function/render/render_type.h"

#include <array>
#include <memory>
#include <optional>

namespace Sammi
{
    // 前置声明相关类（避免头文件循环依赖）
    class WindowSystem;
    class RHI;
    class RenderResourceBase;
    class RenderPipelineBase;
    class RenderScene;
    class RenderCamera;
    class WindowUI;
    class DebugDrawManager;

    // 渲染系统初始化信息结构体（用于传递初始化所需的外部依赖）
    struct RenderSystemInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
        std::shared_ptr<DebugDrawManager> debugdraw_manager;
    };

    // 引擎内容视口结构体（定义渲染内容在窗口中的显示区域）
    struct EngineContentViewport
    {
        float x { 0.f};// 视口左上角x坐标（相对于窗口）
        float y { 0.f};// 视口左上角y坐标（相对于窗口）
        float width { 0.f};// 视口宽度
        float height { 0.f};// 视口高度
    };

    // 渲染系统核心类（负责管理渲染流程、资源、管线等）
    class RenderSystem
    {
    public:
        RenderSystem() = default;
        ~RenderSystem();

        // 初始化渲染系统（传入必要的依赖）
        // 参数：init_info - 初始化信息（包含窗口系统和调试绘制管理器）
        void initialize(RenderSystemInitInfo init_info);

        // 每帧更新（处理渲染逻辑、资源更新等）
        // 参数：delta_time - 上一帧到当前帧的时间间隔（秒）
        void tick(float delta_time);

        // 清理渲染系统资源（释放所有渲染相关对象）
        void clear();

        // 交换逻辑渲染数据（可能用于多线程渲染，交换逻辑线程和渲染线程的数据）
        void swapLogicRenderData();

        // 获取渲染交换上下文（用于线程间传递渲染命令和数据）
        // 返回值：渲染交换上下文引用
        RenderSwapContext& getSwapContext();

        // 获取当前渲染相机（场景中用于渲染的相机）
        // 返回值：渲染相机共享指针（可能为空，若未初始化）
        std::shared_ptr<RenderCamera> getRenderCamera() const;

        // 获取RHI实例（渲染硬件接口，用于调用底层图形API）
        // 返回值：RHI共享指针（可能为空，若未初始化）
        std::shared_ptr<RHI> getRHI() const;

        // 设置渲染管线类型（切换不同的渲染管线实现）
        // 参数：pipeline_type - 目标渲染管线类型（如延迟管线、前向管线）
        void setRenderPipelineType(RENDER_PIPELINE_TYPE pipeline_type);

        // 初始化UI渲染后端（为UI系统提供渲染支持）
        // 参数：window_ui - 窗口UI实例（需要绑定渲染后端）
        void initializeUIRenderBackend(WindowUI* window_ui);

        // 更新引擎内容视口的位置和尺寸（调整渲染内容在窗口中的显示区域）
        // 参数：offset_x - 视口水平偏移量（相对于窗口左上角）
        // 参数：offset_y - 视口垂直偏移量（相对于窗口左上角）
        // 参数：width - 视口宽度
        // 参数：height - 视口高度
        void updateEngineContentViewport(float offset_x, float offset_y, float width, float height);

        // 根据拾取的UV坐标获取被选中网格的GUID（全局唯一标识符）
        // 参数：picked_uv - 屏幕空间中的UV坐标（用于拾取判断）
        // 返回值：网格的GUID（若未命中则可能为无效值）
        uint32_t getGuidOfPickedMesh(const Vector2& picked_uv);

        // 通过网格ID获取对应的游戏对象ID（用于关联渲染网格和游戏逻辑对象）
        // 参数：mesh_id - 网格资源的唯一ID
        // 返回值：游戏对象ID（若不存在则可能为无效值）
        GObjectID getGObjectIDByMeshID(uint32_t mesh_id) const;

        // 获取当前引擎内容视口（包含位置和尺寸信息）
        // 返回值：引擎内容视口结构体（副本）
        EngineContentViewport getEngineContentViewport() const;

        // 创建坐标轴实体（用于场景中显示世界坐标系）
        // 参数：axis_entities - 包含X/Y/Z轴实体的数组（每个元素对应一个轴的渲染实体）
        // 参数：mesh_datas - 对应每个轴的网格数据（顶点、索引等信息）
        void createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas);

        // 设置可见的坐标轴（控制哪个轴在场景中显示）
        // 参数：axis - 可选的坐标轴实体（若为空则隐藏所有轴）
        void setVisibleAxis(std::optional<RenderEntity> axis);

        // 设置选中的坐标轴（用于交互选择，如编辑器中选择X/Y/Z轴）
        // 参数：selected_axis - 选中的轴索引（0=X轴，1=Y轴，2=Z轴）
        void setSelectedAxis(size_t selected_axis);

        // 获取游戏对象部件ID的GUID分配器（用于管理游戏对象部件的唯一标识）
        // 返回值：GUID分配器引用（管理GameObjectPartId类型）
        GuidAllocator<GameObjectPartId>& getGOInstanceIdAllocator();

        // 获取网格资源ID的GUID分配器（用于管理网格资源的唯一标识）
        // 返回值：GUID分配器引用（管理MeshSourceDesc类型）
        GuidAllocator<MeshSourceDesc>& getMeshAssetIdAllocator();

        // 清理关卡重新加载所需的资源（释放当前关卡相关的渲染资源，为重新加载做准备）
        void clearForLevelReloading();

    private:
        // 当前渲染管线类型（默认使用延迟渲染管线）
        RENDER_PIPELINE_TYPE m_render_pipeline_type {RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE};
        // 渲染交换上下文（存储逻辑线程和渲染线程之间交换的数据，如绘制命令、材质参数等）
        RenderSwapContext m_swap_context;

        // 渲染硬件接口实例（抽象底层图形API，负责实际绘制调用）
        std::shared_ptr<RHI> m_rhi;
        // 当前渲染场景使用的相机（定义视图矩阵、投影矩阵等）
        std::shared_ptr<RenderCamera> m_render_camera;
        // 渲染场景实例（管理场景中的所有可渲染对象、光照、雾效等）
        std::shared_ptr<RenderScene> m_render_scene;
        // 渲染资源管理器（管理纹理、网格、材质等资源的加载和释放）
        std::shared_ptr<RenderResourceBase> m_render_resource;
        // 渲染管线实例（根据m_render_pipeline_type创建的具体管线实现，如延迟渲染管线）
        std::shared_ptr<RenderPipelineBase> m_render_pipeline;

        // 处理交换的渲染数据（内部函数，可能用于将逻辑线程准备的数据传递给渲染线程执行）
        void processSwapData();
    };
}

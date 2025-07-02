#pragma once

#include "runtime/function/render/render_entity.h"          // 渲染实体定义
#include "runtime/function/render/render_guid_allocator.h"  // GUID 分配器
#include "runtime/function/render/render_swap_context.h"    // 渲染数据交换上下文
#include "runtime/function/render/render_type.h"            // 渲染类型定义

#include <array>
#include <memory>
#include <optional>

namespace Sammi
{
    // 前置声明（减少编译依赖）
    class WindowSystem;        // 窗口系统
    class RHI;                 // 渲染硬件接口层 (Rendering Hardware Interface)
    class RenderResourceBase;  // 渲染资源基类
    class RenderPipelineBase;  // 渲染管线基类
    class RenderScene;         // 渲染场景管理
    class RenderCamera;        // 渲染摄像机
    class WindowUI;            // UI系统接口
    class DebugDrawManager;    // 调试绘制管理器

    // 渲染系统初始化结构体
    struct RenderSystemInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;          // 窗口系统共享指针
        std::shared_ptr<DebugDrawManager> debugdraw_manager;  // 调试绘制管理器
    };

    // 引擎内容视口坐标信息
    struct EngineContentViewport
    {
        float x { 0.f};       // 视口左上角X坐标
        float y { 0.f};       // 视口左上角Y坐标
        float width { 0.f};   // 视口宽度
        float height { 0.f};  // 视口高度
    };

    // 核心渲染系统类
    class RenderSystem
    {
    public:
        RenderSystem() = default;
        ~RenderSystem();

        // 初始化渲染系统 (调用顺序严格)
        void initialize(RenderSystemInitInfo init_info);

        // 主渲染循环 (每帧调用)
        void tick(float delta_time);

        void clear();

        // 交换逻辑线程与渲染线程的数据
        void swapLogicRenderData();

        // 获取数据交换上下文
        RenderSwapContext& getSwapContext();

        // 获取当前渲染摄像机
        std::shared_ptr<RenderCamera> getRenderCamera() const;

        // 获取RHI实例
        std::shared_ptr<RHI> getRHI() const;

        // 设置渲染管线类型
        void setRenderPipelineType(RENDER_PIPELINE_TYPE pipeline_type);

        // 初始化UI渲染后端
        void initializeUIRenderBackend(WindowUI* window_ui);

        // 更新引擎内容视口区域
        void updateEngineContentViewport(float offset_x, float offset_y, float width, float height);

        // 通过拾取UV坐标获取网格的GUID
        uint32_t getGuidOfPickedMesh(const Vector2& picked_uv);

        // 通过网格ID获取关联的游戏对象ID
        GObjectID getGObjectIDByMeshID(uint32_t mesh_id) const;

        // 获取当前引擎内容视口配置
        EngineContentViewport getEngineContentViewport() const;

        // 创建坐标轴渲染对象 (用于编辑器/调试)
        void createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas);

        // 显示/隐藏坐标轴
        void setVisibleAxis(std::optional<RenderEntity> axis);

        // 设置被选中的坐标轴 (0=x, 1=y, 2=z)
        void setSelectedAxis(size_t selected_axis);

        // 获取游戏对象部件ID分配器
        GuidAllocator<GameObjectPartId>& getGOInstanceIdAllocator();

        // 获取网格资产ID分配器
        GuidAllocator<MeshSourceDesc>&   getMeshAssetIdAllocator();

        // 重新加载关卡前的资源清理
        void clearForLevelReloading();

    private:
        // 当前渲染管线类型（默认延迟渲染）
        RENDER_PIPELINE_TYPE m_render_pipeline_type {RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE};

        // 双缓冲数据交换上下文
        RenderSwapContext m_swap_context;

        // 核心渲染组件
        std::shared_ptr<RHI>                m_rhi;              // 渲染硬件接口
        std::shared_ptr<RenderCamera>       m_render_camera;    // 主渲染摄像机
        std::shared_ptr<RenderScene>        m_render_scene;     // 渲染场景数据
        std::shared_ptr<RenderResourceBase> m_render_resource;  // 资源管理器
        std::shared_ptr<RenderPipelineBase> m_render_pipeline;  // 当前渲染管线

        // 处理从逻辑线程交换过来的数据
        void processSwapData();
    };
}
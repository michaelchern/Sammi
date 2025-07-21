#pragma once

// 包含运行时框架中对象ID分配器头文件
#include "runtime/function/framework/object/object_id_allocator.h"

#include "runtime/function/render/light.h"
#include "runtime/function/render/render_common.h"
#include "runtime/function/render/render_entity.h"
#include "runtime/function/render/render_guid_allocator.h"
#include "runtime/function/render/render_object.h"

#include <optional>
#include <vector>

namespace Sammi
{
    // 前置声明渲染资源和相机类
    class RenderResource;
    class RenderCamera;

    /**
     * @brief 渲染场景类，负责管理场景中所有与渲染相关的资源和对象
     *
     * 该类是渲染模块的核心管理类，主要功能包括：
     * - 管理场景光照（环境光、方向光、点光源）
     * - 维护场景中的渲染实体集合
     * - 跟踪每帧可见的渲染对象（基于不同光源或相机的可见性计算）
     * - 管理资源ID分配（实例ID、网格资源ID、材质资源ID）
     * - 提供与游戏对象ID的映射接口
     */
    class RenderScene
    {
    public:
        // ====================== 光照相关成员 ======================
        AmbientLight      m_ambient_light;     // 场景环境光（全局基础照明）
        PDirectionalLight m_directional_light;  // 方向光指针（模拟太阳光等平行光源）
        PointLightList    m_point_light_list;   // 点光源列表（存储多个点光源对象）

        // ====================== 渲染实体 ======================
        std::vector<RenderEntity> m_render_entities;  // 场景中所有待渲染的实体集合

        // ====================== 编辑器辅助对象 ======================
        std::optional<RenderEntity> m_render_axis;  // 可选渲染轴实体（编辑器模式下显示坐标轴）

        // ====================== 每帧可见对象（动态更新） ======================
        // 不同光源或相机可见的网格节点集合（用于渲染通道筛选可见对象）
        std::vector<RenderMeshNode> m_directional_light_visible_mesh_nodes;// 方向光可见的网格节点
        std::vector<RenderMeshNode> m_point_lights_visible_mesh_nodes;// 点光源可见的网格节点
        std::vector<RenderMeshNode> m_main_camera_visible_mesh_nodes;// 主相机可见的网格节点
        RenderAxisNode              m_axis_node;// 轴节点（编辑器显示用）

        // ====================== 清理与更新接口 ======================
        /**
         * @brief 清空场景所有资源与数据
         * 重置光照、实体列表、可见对象等成员，用于场景重置或切换
         */
        void clear();

        /**
         * @brief 更新当前帧可见对象（核心可见性计算函数）
         * @param render_resource 渲染资源管理器（提供网格/材质等资源访问）
         * @param camera 当前使用的相机（决定视图投影矩阵）
         * 内部会根据不同渲染阶段（光照可见性、相机可见性）调用具体实现
         */
        void updateVisibleObjects(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);

        /**
         * @brief 设置可见节点在渲染通道中的引用
         * 供后续渲染流程（如绘制调用）直接访问已计算的可见对象集合
         */
        void setVisibleNodesReference();

        // ====================== ID分配器管理 ======================
        /**
         * @brief 获取实例ID分配器（管理GameObjectPartId类型的唯一标识）
         * @return 实例ID分配器引用
         */
        GuidAllocator<GameObjectPartId>& getInstanceIdAllocator();

        /**
         * @brief 获取网格资源ID分配器（管理MeshSourceDesc类型的唯一标识）
         * @return 网格资源ID分配器引用
         */
        GuidAllocator<MeshSourceDesc>& getMeshAssetIdAllocator();

        /**
         * @brief 获取材质资源ID分配器（管理MaterialSourceDesc类型的唯一标识）
         * @return 材质资源ID分配器引用
		 */
        GuidAllocator<MaterialSourceDesc>& getMaterialAssetdAllocator();

        // ====================== 游戏对象与渲染对象映射 ======================
        /**
         * @brief 记录网格ID到游戏对象ID的映射关系
         * @param instance_id 渲染实例ID（对应网格资源的实例）
         * @param go_id 关联的游戏对象ID（GameObjectID）
         */
        void addInstanceIdToMap(uint32_t instance_id, GObjectID go_id);

        /**
         * @brief 通过网格ID查询对应的游戏对象ID
         * @param mesh_id 网格资源ID（对应MeshSourceDesc的唯一标识）
         * @return 关联的游戏对象ID（若不存在则返回默认构造的GObjectID）
         */
        GObjectID getGObjectIDByMeshID(uint32_t mesh_id) const;

        /**
         * @brief 根据游戏对象ID删除对应实体
         * @param go_id 需要删除的游戏对象ID
         */
        void deleteEntityByGObjectID(GObjectID go_id);

        /**
         * @brief 清除场景数据（用于关卡重新加载前的清理）
         * 释放所有渲染资源引用，重置ID分配器，清空实体和可见对象集合
         */
        void clearForLevelReloading();

    private:
        // ====================== 私有成员变量 ======================
        GuidAllocator<GameObjectPartId>   m_instance_id_allocator;    // 实例ID分配器（管理GameObjectPartId）
        GuidAllocator<MeshSourceDesc>     m_mesh_asset_id_allocator;  // 网格资源ID分配器（管理MeshSourceDesc）
        GuidAllocator<MaterialSourceDesc> m_material_asset_id_allocator;// 材质资源ID分配器（管理MaterialSourceDesc）
        // 网格ID到游戏对象ID的快速映射表
        std::unordered_map<uint32_t, GObjectID> m_mesh_object_id_map;

        // ====================== 可见性更新私有实现 ======================
        /**
         * @brief 更新方向光可见的网格节点（基于阴影投射或视锥体裁剪）
         * @param render_resource 渲染资源管理器
         * @param camera 当前相机
         */
        void updateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);

        /**
         * @brief 更新点光源可见的网格节点（通常结合光照探针或逐像素计算）
         * @param render_resource 渲染资源管理器
         */
        void updateVisibleObjectsPointLight(std::shared_ptr<RenderResource> render_resource);

        /**
         * @brief 更新主相机可见的网格节点（基于相机视锥体裁剪）
         * @param render_resource 渲染资源管理器
         * @param camera 主相机对象
         */
        void updateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);

        /**
         * @brief 更新轴对象的可见性（编辑器模式下始终可见或根据设置控制）
         * @param render_resource 渲染资源管理器
         */
        void updateVisibleObjectsAxis(std::shared_ptr<RenderResource> render_resource);

        /**
         * @brief 更新粒子系统的可见性（预留接口，当前未实现具体逻辑）
         * @param render_resource 渲染资源管理器
         */
        void updateVisibleObjectsParticle(std::shared_ptr<RenderResource> render_resource);
    };
}

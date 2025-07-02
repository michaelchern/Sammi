#pragma once

// 包含粒子系统、渲染、资源等模块的头文件
#include "runtime/function/particle/emitter_id_allocator.h"
#include "runtime/function/particle/particle_desc.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_object.h"
#include "runtime/resource/res_type/global/global_particle.h"
#include "runtime/resource/res_type/global/global_rendering.h"

#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace Sammi
{
    // 基于图像的照明(IBL)资源描述
    struct LevelIBLResourceDesc
    {
        SkyBoxIrradianceMap m_skybox_irradiance_map;  // 天空盒辐照度贴图
        SkyBoxSpecularMap   m_skybox_specular_map;    // 天空盒镜面反射贴图
        std::string         m_brdf_map;               // BRDF查找表
    };

    // 颜色分级资源描述
    struct LevelColorGradingResourceDesc
    {
        std::string m_color_grading_map;  // 颜色分级查找表(LUT)
    };

    // 关卡资源描述
    struct LevelResourceDesc
    {
        LevelIBLResourceDesc          m_ibl_resource_desc;            // IBL相关资源
        LevelColorGradingResourceDesc m_color_grading_resource_desc;  // 颜色分级资源
    };

    // 相机交换数据（用于更新相机参数）
    struct CameraSwapData
    {
        std::optional<float>            m_fov_x;        // 水平视野角度（可选）
        std::optional<RenderCameraType> m_camera_type;  // 相机类型（可选）
        std::optional<Matrix4x4>        m_view_matrix;  // 视图矩阵（可选）
    };

    // 游戏对象资源描述（批量管理游戏对象）
    struct GameObjectResourceDesc
    {
        std::deque<GameObjectDesc> m_game_object_descs;  // 游戏对象描述队列

        // 添加游戏对象描述
        void add(GameObjectDesc& desc);
        // 从队列中弹出下一个对象
        void pop();
        // 检查队列是否为空
        bool isEmpty() const;
        // 获取下一个要处理的对象
        GameObjectDesc& getNextProcessObject();
    };

    // 粒子发射器提交请求
    struct ParticleSubmitRequest
    {
        std::vector<ParticleEmitterDesc> m_emitter_descs;  // 粒子发射器描述列表

        // 添加粒子发射器描述
        void add(ParticleEmitterDesc& desc);
        // 获取发射器数量
        unsigned int getEmitterCount() const;
        // 获取指定索引的发射器描述
        const ParticleEmitterDesc& getEmitterDesc(unsigned int index);
    };

    // 粒子发射器帧更新请求
    struct EmitterTickRequest
    {
        std::vector<ParticleEmitterID> m_emitter_indices;  // 需要更新的发射器ID列表
    };

    // 粒子发射器变换更新请求
    struct EmitterTransformRequest
    {
        std::vector<ParticleEmitterTransformDesc> m_transform_descs;  // 发射器变换描述列表

        // 添加变换描述
        void add(ParticleEmitterTransformDesc& desc);
        // 清空列表
        void clear();
        // 获取发射器数量
        unsigned int getEmitterCount() const;
        // 获取指定索引的变换描述
        const ParticleEmitterTransformDesc& getNextEmitterTransformDesc(unsigned int index);
    };

    // 渲染交换数据结构（包含所有可能的交换数据类型）
    struct RenderSwapData
    {
        std::optional<LevelResourceDesc>       m_level_resource_desc;        // 关卡资源更新
        std::optional<GameObjectResourceDesc>  m_game_object_resource_desc;  // 游戏对象资源更新
        std::optional<GameObjectResourceDesc>  m_game_object_to_delete;      // 待删除的游戏对象
        std::optional<CameraSwapData>          m_camera_swap_data;           // 相机参数更新
        std::optional<ParticleSubmitRequest>   m_particle_submit_request;    // 粒子发射器提交
        std::optional<EmitterTickRequest>      m_emitter_tick_request;       // 粒子发射器帧更新
        std::optional<EmitterTransformRequest> m_emitter_transform_request;  // 粒子发射器变换更新

        // 添加需要更新的游戏对象
        void addDirtyGameObject(GameObjectDesc&& desc);
        // 添加待删除的游戏对象
        void addDeleteGameObject(GameObjectDesc&& desc);
        // 添加新的粒子发射器
        void addNewParticleEmitter(ParticleEmitterDesc& desc);
        // 添加需要更新的粒子发射器
        void addTickParticleEmitter(ParticleEmitterID id);
        // 更新粒子发射器的变换
        void updateParticleTransform(ParticleEmitterTransformDesc& desc);
    };

    // 交换数据类型枚举
    enum SwapDataType : uint8_t
    {
        LogicSwapDataType = 0,  // 逻辑线程数据
        RenderSwapDataType,     // 渲染线程数据
        SwapDataTypeCount       // 类型总数
    };

    // 渲染交换上下文（双缓冲数据交换系统）
    class RenderSwapContext
    {
    public:
        // 获取逻辑线程交换数据引用
        RenderSwapData& getLogicSwapData();
        // 获取渲染线程交换数据引用
        RenderSwapData& getRenderSwapData();
        // 交换逻辑和渲染线程的数据
        void swapLogicRenderData();

        // 重置关卡资源交换数据
        void resetLevelRsourceSwapData();
        // 重置游戏对象资源交换数据
        void resetGameObjectResourceSwapData();
        // 重置待删除游戏对象数据
        void resetGameObjectToDelete();
        // 重置相机交换数据
        void resetCameraSwapData();
        // 重置粒子批次交换数据
        void resetPartilceBatchSwapData();
        // 重置粒子发射器更新数据
        void resetEmitterTickSwapData();
        // 重置粒子变换数据
        void resetEmitterTransformSwapData();

    private:
        uint8_t        m_logic_swap_data_index {LogicSwapDataType};    // 逻辑线程数据索引
        uint8_t        m_render_swap_data_index {RenderSwapDataType};  // 渲染线程数据索引
        RenderSwapData m_swap_data[SwapDataTypeCount];                 // 双缓冲数据存储

        // 检查是否准备好交换
        bool isReadyToSwap() const;
        // 执行数据交换
        void swap();
    };
}
#pragma once

#include "runtime/function/render/render_scene.h"
#include "runtime/function/render/render_swap_context.h"
#include "runtime/function/render/render_type.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Sammi
{
    // 前置声明（避免循环依赖）
    class RHI;
    class RenderScene;
    class RenderCamera;

    /**
     * @brief 渲染资源基类
     *
     * 定义渲染资源管理的核心接口，包括资源上传、更新、加载及缓存等功能。
     * 子类需实现纯虚函数以支持具体资源类型（如纹理、网格、材质）或渲染API（如Vulkan/DirectX）。
     */
    class RenderResourceBase
    {
    public:
        virtual ~RenderResourceBase() {}

        /**
         * @brief 清理资源
         *
         * 释放当前持有的渲染资源（如GPU内存、缓存数据等），用于资源重置或销毁场景前。
         */
        virtual void clear() = 0;

        /**
         * @brief 上传全局渲染资源
         *
         * 加载并上传关卡级别的全局资源（如天空盒、全局光照贴图等）到GPU。
         * @param rhi 渲染硬件接口实例（用于调用底层API）
         * @param level_resource_desc 关卡资源描述（包含资源路径、类型等信息）
         */
        virtual void uploadGlobalRenderResource(std::shared_ptr<RHI> rhi, LevelResourceDesc level_resource_desc) = 0;

        /**
         * @brief 上传游戏对象渲染资源（网格+材质）
         *
         * 为指定游戏对象上传其使用的网格数据和材质数据到GPU。
         * @param rhi 渲染硬件接口实例
         * @param render_entity 游戏对象实体ID
         * @param mesh_data 网格数据（顶点、索引等信息）
         * @param material_data 材质数据（着色器、纹理绑定等信息）
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi,
                                                    RenderEntity         render_entity,
                                                    RenderMeshData       mesh_data,
                                                    RenderMaterialData   material_data) = 0;

        /**
         * @brief 上传游戏对象渲染资源（仅网格）
         *
         * 为指定游戏对象上传网格数据到GPU（适用于仅需网格资源的对象，如静态场景）。
         * @param rhi 渲染硬件接口实例
         * @param render_entity 游戏对象实体ID
         * @param mesh_data 网格数据
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi,
                                                    RenderEntity         render_entity,
                                                    RenderMeshData       mesh_data) = 0;

        /**
         * @brief 上传游戏对象渲染资源（仅材质）
         *
         * 为指定游戏对象上传材质数据到GPU（适用于仅需材质资源的对象，如后期处理效果）。
         * @param rhi 渲染硬件接口实例
         * @param render_entity 游戏对象实体ID
         * @param material_data 材质数据
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi,
                                                    RenderEntity         render_entity,
                                                    RenderMaterialData   material_data) = 0;

        /**
         * @brief 更新每帧渲染缓冲区
         *
         * 更新每帧变化的动态缓冲区（如视图投影矩阵、光照参数等）。
         * @param render_scene 当前渲染场景（包含相机、光照等动态信息）
         * @param camera 当前渲染相机（提供视图投影矩阵）
         */
        virtual void updatePerFrameBuffer(std::shared_ptr<RenderScene>  render_scene,
                                          std::shared_ptr<RenderCamera> camera) = 0;

        // TODO: 数据缓存优化（后续可实现资源重复利用）

        /**
         * @brief 加载HDR纹理
         * @param file 纹理文件路径
         * @param desired_channels 期望的通道数（默认4，RGBA）
         * @return 纹理数据指针（加载失败返回空）
         */
        std::shared_ptr<TextureData> loadTextureHDR(std::string file, int desired_channels = 4);

        /**
         * @brief 加载普通纹理（支持sRGB格式）
         * @param file 纹理文件路径
         * @param is_srgb 是否为sRGB颜色空间（默认false）
         * @return 纹理数据指针（加载失败返回空）
         */
        std::shared_ptr<TextureData> loadTexture(std::string file, bool is_srgb = false);

        /**
         * @brief 加载网格数据并计算包围盒
         * @param source 网格资源描述（包含文件路径、加载参数等）
         * @param bounding_box 输出参数：网格的轴对齐包围盒（AABB）
         * @return 网格数据（包含顶点、索引、细分信息等）
         */
        RenderMeshData loadMeshData(const MeshSourceDesc& source, AxisAlignedBox& bounding_box);

        /**
         * @brief 加载材质数据
         * @param source 材质资源描述（包含着色器路径、纹理绑定规则等）
         * @return 材质数据（包含着色器程序、纹理采样参数等）
         */
        RenderMaterialData loadMaterialData(const MaterialSourceDesc& source);

        /**
         * @brief 获取缓存的网格包围盒
         * @param source 网格资源描述（用于查找缓存）
         * @return 缓存的轴对齐包围盒（未缓存时返回空）
         */
        AxisAlignedBox getCachedBoudingBox(const MeshSourceDesc& source) const;

    private:
        /**
         * @brief 加载静态网格数据（内部实现）
         * @param mesh_file 网格文件路径
         * @param bounding_box 输出参数：网格包围盒
         * @return 静态网格数据（仅包含顶点、索引等基础信息）
         */
        StaticMeshData loadStaticMesh(std::string mesh_file, AxisAlignedBox& bounding_box);

        // 缓存已加载网格的包围盒（避免重复计算）
        // 键：网格资源描述（MeshSourceDesc）
        // 值：对应的轴对齐包围盒（AxisAlignedBox）
        std::unordered_map<MeshSourceDesc, AxisAlignedBox> m_bounding_box_cache_map;
    };
}

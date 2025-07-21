#include "runtime/function/render/render_resource_base.h"
#include "runtime/core/base/macro.h"
#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"
#include "runtime/resource/res_type/data/mesh_data.h"
#include "runtime/function/global/global_context.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <algorithm>
#include <filesystem>
#include <vector>

namespace Sammi
{
    // ------------------------- 加载HDR纹理 -------------------------
    std::shared_ptr<TextureData> RenderResourceBase::loadTextureHDR(std::string file, int desired_channels)
    {
        // 获取全局资源管理器（用于解析资源完整路径）
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // 创建纹理数据对象
        std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

        int iw, ih, n;
        // 使用stb_image加载HDR浮点纹理
        // 参数说明：
        // - asset_manager->getFullPath(file).generic_string().c_str()：资源的完整路径（转换为通用字符串）
        // - &iw/&ih/&n：输出图像宽度、高度、通道数
        // - desired_channels：期望的通道数（如4表示RGBA）
        texture->m_pixels = stbi_loadf(asset_manager->getFullPath(file).generic_string().c_str(), &iw, &ih, &n, desired_channels);

        if (!texture->m_pixels)
            return nullptr;

        // 填充纹理元数据
        texture->m_width  = iw;  // 纹理宽度
        texture->m_height = ih;  // 纹理高度
        // 根据期望通道数设置RHI格式（RHIFormat枚举定义了不同纹理格式）
        switch (desired_channels)
        {
            case 2:
                // RG格式（32位浮点/通道）
                texture->m_format = RHIFormat::RHI_FORMAT_R32G32_SFLOAT;
                break;
            case 4:
                // RGBA格式（32位浮点/通道）
                texture->m_format = RHIFormat::RHI_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                // 不支持的通道数（如3通道）
                throw std::runtime_error("unsupported channels number");
                break;
        }
        // 其他纹理属性（深度、数组层数、MIP层级、图像类型）
        texture->m_depth        = 1;                                      // 深度（2D纹理为1）
        texture->m_array_layers = 1;                                      // 数组层数（单层）
        texture->m_mip_levels   = 1;                                      // MIP层级（仅1级，无MIPMAP）
        texture->m_type         = SAMMI_IMAGE_TYPE::SAMMI_IMAGE_TYPE_2D;  // 2D纹理

        return texture;
    }

    // ------------------------- 加载普通纹理（LDR） -------------------------
    std::shared_ptr<TextureData> RenderResourceBase::loadTexture(std::string file, bool is_srgb)
    {
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

        int iw, ih, n;
        // 使用stb_image加载8位无符号整数纹理（LDR）
        // 参数说明：
        // - asset_manager->getFullPath(file).generic_string().c_str()：资源完整路径
        // - &iw/&ih/&n：输出宽度、高度、实际通道数（stb_image会自动检测）
        // - 4：强制读取4个通道（RGBA）
        texture->m_pixels = stbi_load(asset_manager->getFullPath(file).generic_string().c_str(), &iw, &ih, &n, 4);

        if (!texture->m_pixels)
            return nullptr;

        // 填充纹理元数据
        texture->m_width        = iw;
        texture->m_height       = ih;
        // 根据是否sRGB设置格式（sRGB需要gamma校正）
        texture->m_format       = (is_srgb) ? RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB :
                                              RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
        texture->m_depth        = 1;
        texture->m_array_layers = 1;
        texture->m_mip_levels   = 1;
        texture->m_type         = SAMMI_IMAGE_TYPE::SAMMI_IMAGE_TYPE_2D;

        return texture;
    }

    RenderMeshData RenderResourceBase::loadMeshData(const MeshSourceDesc& source, AxisAlignedBox& bounding_box)
    {
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        RenderMeshData ret;

        // 根据文件扩展名判断网格格式
        if (std::filesystem::path(source.m_mesh_file).extension() == ".obj")
        {
            // 加载OBJ格式静态网格（调用内部实现）
            ret.m_static_mesh_data = loadStaticMesh(source.m_mesh_file, bounding_box);
        }
        else if (std::filesystem::path(source.m_mesh_file).extension() == ".json")
        {
            // 加载JSON格式预处理网格（从资源管理器加载）
            std::shared_ptr<MeshData> bind_data = std::make_shared<MeshData>();
            asset_manager->loadAsset<MeshData>(source.m_mesh_file, *bind_data);  // 从资源管理器加载MeshData

            // ------------------------- 顶点缓冲区 -------------------------
            size_t vertex_size = bind_data->vertex_buffer.size() * sizeof(MeshVertexDataDefinition);
            // 从资源管理器加载MeshData
            ret.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_size);
            // 将MeshData中的顶点数据复制到BufferData（转换格式）
            MeshVertexDataDefinition* vertex = (MeshVertexDataDefinition*)ret.m_static_mesh_data.m_vertex_buffer->m_data;
            for (size_t i = 0; i < bind_data->vertex_buffer.size(); i++)
            {
                vertex[i].x  = bind_data->vertex_buffer[i].px;
                vertex[i].y  = bind_data->vertex_buffer[i].py;
                vertex[i].z  = bind_data->vertex_buffer[i].pz;
                vertex[i].nx = bind_data->vertex_buffer[i].nx;
                vertex[i].ny = bind_data->vertex_buffer[i].ny;
                vertex[i].nz = bind_data->vertex_buffer[i].nz;
                vertex[i].tx = bind_data->vertex_buffer[i].tx;
                vertex[i].ty = bind_data->vertex_buffer[i].ty;
                vertex[i].tz = bind_data->vertex_buffer[i].tz;
                vertex[i].u  = bind_data->vertex_buffer[i].u;
                vertex[i].v  = bind_data->vertex_buffer[i].v;

                bounding_box.merge(Vector3(vertex[i].x, vertex[i].y, vertex[i].z));
            }

            // ------------------------- 索引缓冲区 -------------------------
            size_t index_size = bind_data->index_buffer.size() * sizeof(uint16_t);
            ret.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_size);
            uint16_t* index = (uint16_t*)ret.m_static_mesh_data.m_index_buffer->m_data;
            for (size_t i = 0; i < bind_data->index_buffer.size(); i++)
            {
                index[i] = static_cast<uint16_t>(bind_data->index_buffer[i]);
            }

            // ------------------------- 骨骼绑定缓冲区（可选） -------------------------
            size_t data_size = bind_data->bind.size() * sizeof(MeshVertexBindingDataDefinition);
            ret.m_skeleton_binding_buffer = std::make_shared<BufferData>(data_size);
            MeshVertexBindingDataDefinition* binding_data = reinterpret_cast<MeshVertexBindingDataDefinition*>(ret.m_skeleton_binding_buffer->m_data);
            for (size_t i = 0; i < bind_data->bind.size(); i++)
            {
                binding_data[i].m_index0  = bind_data->bind[i].index0;
                binding_data[i].m_index1  = bind_data->bind[i].index1;
                binding_data[i].m_index2  = bind_data->bind[i].index2;
                binding_data[i].m_index3  = bind_data->bind[i].index3;
                binding_data[i].m_weight0 = bind_data->bind[i].weight0;
                binding_data[i].m_weight1 = bind_data->bind[i].weight1;
                binding_data[i].m_weight2 = bind_data->bind[i].weight2;
                binding_data[i].m_weight3 = bind_data->bind[i].weight3;
            }
        }

        // 将当前网格的包围盒缓存（避免重复计算）
        m_bounding_box_cache_map.insert(std::make_pair(source, bounding_box));

        return ret;
    }

    RenderMaterialData RenderResourceBase::loadMaterialData(const MaterialSourceDesc& source)
    {
        RenderMaterialData ret;

        // 加载各类型纹理（调用loadTexture接口）
        ret.m_base_color_texture         = loadTexture(source.m_base_color_file, true);    // 基础颜色纹理（sRGB）
        ret.m_metallic_roughness_texture = loadTexture(source.m_metallic_roughness_file);  // 金属粗糙度纹理（线性空间）
        ret.m_normal_texture             = loadTexture(source.m_normal_file);              // 法线纹理（线性空间）
        ret.m_occlusion_texture          = loadTexture(source.m_occlusion_file);           // 遮挡纹理（线性空间）
        ret.m_emissive_texture           = loadTexture(source.m_emissive_file);            // 自发光纹理（sRGB）
        return ret;
    }

    AxisAlignedBox RenderResourceBase::getCachedBoudingBox(const MeshSourceDesc& source) const
    {
        // 在包围盒缓存中查找对应资源的包围盒
        auto find_it = m_bounding_box_cache_map.find(source);
        if (find_it != m_bounding_box_cache_map.end())
        {
            return find_it->second;
        }
        return AxisAlignedBox();
    }

    StaticMeshData RenderResourceBase::loadStaticMesh(std::string filename, AxisAlignedBox& bounding_box)
    {
        StaticMeshData mesh_data;

        // 使用tiny_obj_loader解析OBJ文件
        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.vertex_color = false;  // 不加载顶点颜色（假设材质已处理）

        // 解析OBJ文件
        if (!reader.ParseFromFile(filename, reader_config))
        {
            if (!reader.Error().empty())
            {
                LOG_ERROR("loadMesh {} failed, error: {}", filename, reader.Error());
            }
            assert(0);
        }

        if (!reader.Warning().empty())
        {
            LOG_WARN("loadMesh {} warning, warning: {}", filename, reader.Warning());
        }

        auto& attrib = reader.GetAttrib();  // 顶点属性（位置、法线、纹理坐标）
        auto& shapes = reader.GetShapes();  // 网格形状（面、索引）

        // 存储最终顶点数据
        std::vector<MeshVertexDataDefinition> mesh_vertices;

        // 遍历所有形状（通常一个OBJ文件包含多个子网格）
        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                bool with_normal   = true;  // 是否包含法线
                bool with_texcoord = true;  // 是否包含纹理坐标
                
                Vector3 vertex[3];  // 三角形的三个顶点位置
                Vector3 normal[3];  // 三角形的三个法线
                Vector2 uv[3];      // 三角形的三个纹理坐标

                // 仅处理三角形面（OBJ标准支持任意多边形，但渲染通常需要三角化）
                if (fv != 3)
                {
                    continue;  // 非三角形面跳过（实际项目中可能需要三角化）
                }

                // 展开顶点属性（OBJ索引可能共享顶点属性）
                for (size_t v = 0; v < fv; v++)
                {
                    auto idx = shapes[s].mesh.indices[index_offset + v];
                    auto vx  = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    auto vy  = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    auto vz  = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    vertex[v].x = static_cast<float>(vx);
                    vertex[v].y = static_cast<float>(vy);
                    vertex[v].z = static_cast<float>(vz);

                    // 更新包围盒
                    bounding_box.merge(Vector3(vertex[v].x, vertex[v].y, vertex[v].z));

                    // 法线（如果有）
                    if (idx.normal_index >= 0)
                    {
                        auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                        normal[v].x = static_cast<float>(nx);
                        normal[v].y = static_cast<float>(ny);
                        normal[v].z = static_cast<float>(nz);
                    }
                    else
                    {
                        with_normal = false;
                    }

                    // 纹理坐标（如果有）
                    if (idx.texcoord_index >= 0)
                    {
                        auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                        uv[v].x = static_cast<float>(tx);
                        uv[v].y = static_cast<float>(ty);
                    }
                    else
                    {
                        with_texcoord = false;
                    }
                }
                index_offset += fv;

                // 若缺少法线，自动生成面法线（所有顶点共享同一法线）
                if (!with_normal)
                {
                    Vector3 v0 = vertex[1] - vertex[0];
                    Vector3 v1 = vertex[2] - vertex[1];
                    normal[0]  = v0.crossProduct(v1).normalisedCopy();
                    normal[1]  = normal[0];
                    normal[2]  = normal[0];
                }

                // 若缺少纹理坐标，使用默认UV（中心点）
                if (!with_texcoord)
                {
                    uv[0] = Vector2(0.5f, 0.5f);
                    uv[1] = Vector2(0.5f, 0.5f);
                    uv[2] = Vector2(0.5f, 0.5f);
                }

                // 计算切线空间（用于法线贴图）
                Vector3 tangent {1, 0, 0};
                {
                    Vector3 edge1    = vertex[1] - vertex[0];
                    Vector3 edge2    = vertex[2] - vertex[1];
                    Vector2 deltaUV1 = uv[1] - uv[0];
                    Vector2 deltaUV2 = uv[2] - uv[1];

                    auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                    if (divide >= 0.0f && divide < 0.000001f)
                        divide = 0.000001f;
                    else if (divide < 0.0f && divide > -0.000001f)
                        divide = -0.000001f;

                    float df  = 1.0f / divide;
                    tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    tangent   = (tangent).normalisedCopy();
                }

                // 填充顶点数据（合并位置、法线、UV、切线）
                for (size_t i = 0; i < 3; i++)
                {
                    MeshVertexDataDefinition mesh_vert {};

                    mesh_vert.x = vertex[i].x;
                    mesh_vert.y = vertex[i].y;
                    mesh_vert.z = vertex[i].z;

                    mesh_vert.nx = normal[i].x;
                    mesh_vert.ny = normal[i].y;
                    mesh_vert.nz = normal[i].z;

                    mesh_vert.u = uv[i].x;
                    mesh_vert.v = uv[i].y;

                    mesh_vert.tx = tangent.x;
                    mesh_vert.ty = tangent.y;
                    mesh_vert.tz = tangent.z;

                    mesh_vertices.push_back(mesh_vert);
                }
            }
        }

        // 创建顶点缓冲区（分配显存并复制数据）
        uint32_t stride = sizeof(MeshVertexDataDefinition);  // 顶点步长（每个顶点的字节大小）
        mesh_data.m_vertex_buffer = std::make_shared<BufferData>(mesh_vertices.size() * stride);
        // 将顶点数据复制到显存
        mesh_data.m_index_buffer = std::make_shared<BufferData>(mesh_vertices.size() * sizeof(uint16_t));

        // 断言确保索引数量在uint16_t范围内（Vulkan等API通常限制索引为16位）
        assert(mesh_vertices.size() <= std::numeric_limits<uint16_t>::max());

        // 填充索引缓冲区（三角形面的顶点索引）
        uint16_t* indices = (uint16_t*)mesh_data.m_index_buffer->m_data;
        for (size_t i = 0; i < mesh_vertices.size(); i++)
        {
            // 复制顶点数据
            ((MeshVertexDataDefinition*)(mesh_data.m_vertex_buffer->m_data))[i] = mesh_vertices[i];
            // 索引为顶点数组的顺序索引
            indices[i] = static_cast<uint16_t>(i);
        }

        // 返回加载的静态网格数据
        return mesh_data;
    }
}

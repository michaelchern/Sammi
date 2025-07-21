#include "runtime/function/render/render_resource.h"

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_helper.h"

#include "runtime/function/render/render_mesh.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/interface/vulkan/vulkan_util.h"

#include "runtime/function/render/passes/main_camera_pass.h"

#include "runtime/core/base/macro.h"

#include <stdexcept>

namespace Sammi
{
    void RenderResource::clear()
    {
    }

    void RenderResource::uploadGlobalRenderResource(std::shared_ptr<RHI> rhi, LevelResourceDesc level_resource_desc)
    {
        // ������ӳ��ȫ�ִ洢�����������ڴ洢��Ⱦ�����ͳһ��Դ���ݣ�����ʲ������������õȣ�
        createAndMapStorageBuffer(rhi);

        // -------------------------- ��պз��նȣ�IBL���նȣ���Դ���� --------------------------
        // �ӹؿ���Դ��������ȡ��պз��ն���Դ��������Ϣ
        SkyBoxIrradianceMap skybox_irradiance_map = level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map;
        // �������������HDR���ն�������������ͼ�������棩
        // ����X/Y/Z�����Ӧ��պе������棬����ģ�⻷����������乱��
        std::shared_ptr<TextureData> irradiace_pos_x_map = loadTextureHDR(skybox_irradiance_map.m_positive_x_map);  // �ң�+X��
        std::shared_ptr<TextureData> irradiace_neg_x_map = loadTextureHDR(skybox_irradiance_map.m_negative_x_map);  // ��-X��
        std::shared_ptr<TextureData> irradiace_pos_y_map = loadTextureHDR(skybox_irradiance_map.m_positive_y_map);  // �ϣ�+Y��
        std::shared_ptr<TextureData> irradiace_neg_y_map = loadTextureHDR(skybox_irradiance_map.m_negative_y_map);  // �£�-Y��
        std::shared_ptr<TextureData> irradiace_pos_z_map = loadTextureHDR(skybox_irradiance_map.m_positive_z_map);  // ǰ��+Z��
        std::shared_ptr<TextureData> irradiace_neg_z_map = loadTextureHDR(skybox_irradiance_map.m_negative_z_map);  // ��-Z��

        // -------------------------- ��պи߹⣨IBL�߹⣩��Դ���� --------------------------
        // �ӹؿ���Դ��������ȡ��պи߹���Դ��������Ϣ�����ھ��淴����㣩
        SkyBoxSpecularMap skybox_specular_map = level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map;
        // �������������HDR�߹�������������ͼ�������棩
        // �߹�����ͨ���洢Ԥ�������гϵ���򻷾����ڱ���Ϣ�����ڿ��ټ��㾵�淴��
        std::shared_ptr<TextureData> specular_pos_x_map  = loadTextureHDR(skybox_specular_map.m_positive_x_map);
        std::shared_ptr<TextureData> specular_neg_x_map  = loadTextureHDR(skybox_specular_map.m_negative_x_map);
        std::shared_ptr<TextureData> specular_pos_y_map  = loadTextureHDR(skybox_specular_map.m_positive_y_map);
        std::shared_ptr<TextureData> specular_neg_y_map  = loadTextureHDR(skybox_specular_map.m_negative_y_map);
        std::shared_ptr<TextureData> specular_pos_z_map  = loadTextureHDR(skybox_specular_map.m_positive_z_map);
        std::shared_ptr<TextureData> specular_neg_z_map  = loadTextureHDR(skybox_specular_map.m_negative_z_map);

        // -------------------------- BRDF���ұ�LUT����Դ���� --------------------------
        // ����BRDFԤ������ұ�2D���������ڿ��ٲ�ѯ��ͬ�ֲڶȺ�������µ�BRDFֵ
        // ��������ɫ����ʵʱ���㸴�ӵĻ��֣�������Ⱦ����
        std::shared_ptr<TextureData> brdf_map = loadTextureHDR(level_resource_desc.m_ibl_resource_desc.m_brdf_map);

        // -------------------------- ����IBL������ --------------------------
        // ����IBL�������Ĳ�����������˷�ʽ��Ѱַģʽ�ȣ���ȷ�����������Ϊ��������
        createIBLSamplers(rhi);

        // -------------------------- ����IBL������������ͼ�� --------------------------
        // ע�⣺��������ͼ����������Ҫ���ض�˳�����У�ȡ���ڵײ�API��Ҫ����Vulkan��VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT��
        // ���ն����� [��, ��, ǰ, ��, ��, ��] ˳�����У���������ĳЩAPI����������ͼ��˳��Ҫ��
        std::array<std::shared_ptr<TextureData>, 6> irradiance_maps =
        {
            irradiace_pos_x_map,
            irradiace_neg_x_map,
            irradiace_pos_z_map,
            irradiace_neg_z_map,
            irradiace_pos_y_map,
            irradiace_neg_y_map
        };
        // �߹����� [��, ��, ǰ, ��, ��, ��] ˳�����У�����նȱ���һ�µ���˳��
        std::array<std::shared_ptr<TextureData>, 6> specular_maps =
        {
            specular_pos_x_map,
            specular_neg_x_map,
            specular_pos_z_map,
            specular_neg_z_map,
            specular_pos_y_map,
            specular_neg_y_map
        };

        // ����IBL������󣨰���ͼ��ͼ����ͼ���ڴ���䣩�������ص�HDR�����ϴ���GPU
        // ��Щ������Ϊȫ����Դ��������ɫ������ʹ��
        createIBLTextures(rhi, irradiance_maps, specular_maps);

        // -------------------------- ����BRDF LUT���� --------------------------
        // ��GPU�ϴ���BRDF���ұ��ȫ��ͼ����Դ������ͼ����ͼ���ڴ���䣩
        // ����˵����ͼ�����ͼ����ͼ���ڴ����������ȡ��߶ȡ��������ݡ������ʽ
        rhi->createGlobalImage(
            m_global_render_resource._ibl_resource._brdfLUT_texture_image,
            m_global_render_resource._ibl_resource._brdfLUT_texture_image_view,
            m_global_render_resource._ibl_resource._brdfLUT_texture_image_allocation,
            brdf_map->m_width,
            brdf_map->m_height,
            brdf_map->m_pixels,
            brdf_map->m_format);

        // -------------------------- ��ɫ�ּ���Color Grading����Դ���� --------------------------
        // ������ɫ�ּ�LUT�������ڵ��������ɫ�����Աȶȡ����Ͷȵȣ�
        std::shared_ptr<TextureData> color_grading_map = loadTexture(level_resource_desc.m_color_grading_resource_desc.m_color_grading_map);

        // -------------------------- ������ɫ�ּ�LUT���� --------------------------
        // ��GPU�ϴ�����ɫ�ּ����ұ��ȫ��ͼ����Դ������ͼ����ͼ���ڴ���䣩
        // ���������ں��ڴ���׶Σ������ջ��������ɫУ��
        rhi->createGlobalImage(
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image,
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_view,
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_allocation,
            color_grading_map->m_width,
            color_grading_map->m_height,
            color_grading_map->m_pixels,
            color_grading_map->m_format);
    }

    void RenderResource::uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data, RenderMaterialData material_data)
    {
        getOrCreateVulkanMesh(rhi, render_entity, mesh_data);
        getOrCreateVulkanMaterial(rhi, render_entity, material_data);
    }

    void RenderResource::uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data)
    {
        getOrCreateVulkanMesh(rhi, render_entity, mesh_data);
    }

    void RenderResource::uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMaterialData material_data)
    {
        getOrCreateVulkanMaterial(rhi, render_entity, material_data);
    }

    void RenderResource::updatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera)
    {
        // -------------------------- ���������� --------------------------
        Matrix4x4 view_matrix = camera->getViewMatrix();         // �����ͼ��������ռ������ռ䣩
        Matrix4x4 proj_matrix = camera->getPersProjMatrix();     // ���͸��ͶӰ��������ռ���ü��ռ䣩
        Vector3   camera_position = camera->position();          // ���������ռ��е�λ��
        Matrix4x4 proj_view_matrix = proj_matrix * view_matrix;  // ͶӰ-��ͼ���Ͼ�������ռ���ü��ռ䣩

        // -------------------------- ����������ȡ --------------------------
        Vector3  ambient_light = render_scene->m_ambient_light.m_irradiance;                                 // ��������նȣ�ȫ�ֻ������գ�
        uint32_t point_light_num = static_cast<uint32_t>(render_scene->m_point_light_list.m_lights.size());  // ���Դ����

        // -------------------------- ���������ײͨ��UBO --------------------------
        m_particle_collision_perframe_storage_buffer_object.view_matrix      = view_matrix;            // ��ͼ����
        m_particle_collision_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;       // ͶӰ-��ͼ����
        m_particle_collision_perframe_storage_buffer_object.proj_inv_matrix  = proj_matrix.inverse();  // ͶӰ�����棨������Ƚ���ȼ��㣩

        // -------------------------- ���������Ⱦͨ��UBO --------------------------
        m_mesh_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;  // ͶӰ-��ͼ���󣨹�������ɫ��ʹ�ã�
        m_mesh_perframe_storage_buffer_object.camera_position = camera_position;    // ���λ�ã������ռ���ʹ�ã�
        m_mesh_perframe_storage_buffer_object.ambient_light = ambient_light;        // ��������նȣ���PBR����ʹ�ã�
        m_mesh_perframe_storage_buffer_object.point_light_num = point_light_num;    // ���Դ����������ѭ�����ޣ�

        // -------------------------- �����Դ��Ӱͨ��UBO --------------------------
        m_mesh_point_light_shadow_perframe_storage_buffer_object.point_light_num = point_light_num;  // ���Դ����

        // -------------------------- �������Դ���� --------------------------
        for (uint32_t i = 0; i < point_light_num; i++)
        {
            // ��ȡ���Դλ�ú�ǿ�ȣ�ǿ��ͨ����ͨ��/4�й�һ��������PBR�����غ㣩
            Vector3 point_light_position  = render_scene->m_point_light_list.m_lights[i].m_position;
            Vector3 point_light_intensity = render_scene->m_point_light_list.m_lights[i].m_flux / (4.0f * Math_PI);

            // ������Դ�뾶�����ݹ�ǿ��˥��������̬������ȷ�����շ�Χ����
            float radius = render_scene->m_point_light_list.m_lights[i].calculateRadius();

            // ���������Ⱦͨ���ĵ��Դ���飨��Ƭ����ɫ��������
            m_mesh_perframe_storage_buffer_object.scene_point_lights[i].position  = point_light_position;
            m_mesh_perframe_storage_buffer_object.scene_point_lights[i].radius    = radius;
            m_mesh_perframe_storage_buffer_object.scene_point_lights[i].intensity = point_light_intensity;

            // �����Դ��Ӱͨ����λ��-�뾶���ݣ�������Ӱӳ�䣩
            m_mesh_point_light_shadow_perframe_storage_buffer_object.point_lights_position_and_radius[i] = Vector4(point_light_position, radius);
        }

        // -------------------------- ��䷽������� --------------------------
        // ����ⷽ�򣨵�λ��������ɫ������Ӱ�����ֱ�ӹ���ʹ�ã�
        m_mesh_perframe_storage_buffer_object.scene_directional_light.direction = render_scene->m_directional_light.m_direction.normalisedCopy();
        m_mesh_perframe_storage_buffer_object.scene_directional_light.color     = render_scene->m_directional_light.m_color;

        // -------------------------- ���ʰȡͨ��UBO --------------------------
        // ʰȡͨ��ʹ��������Ⱦ��ͬ��ͶӰ-��ͼ�������ڽ���Ļ����ת��Ϊ�������꣩
        m_mesh_inefficient_pick_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;

        // -------------------------- ������ӹ����ͨ��UBO --------------------------
        // �������Ҫ���������Ϣ��ʼ�������������/ǰ/�Ϸ������ڵ������ӳ���
        m_particlebillboard_perframe_storage_buffer_object.proj_view_matrix = proj_view_matrix;
        m_particlebillboard_perframe_storage_buffer_object.right_direction  = camera->right();
        m_particlebillboard_perframe_storage_buffer_object.foward_direction = camera->forward();
        m_particlebillboard_perframe_storage_buffer_object.up_direction     = camera->up();
    }

    void RenderResource::createIBLSamplers(std::shared_ptr<RHI> rhi)
    {
        // ������ָ��ת��ΪVulkan RHI�ľ���ʵ��ָ�루����RHI�Ķ�̬ʵ���У�VulkanRHI�Ǿ������ࣩ
        VulkanRHI* raw_rhi = static_cast<VulkanRHI*>(rhi.get());

        // -------------------------- ��ȡ�����豸���� --------------------------
        RHIPhysicalDeviceProperties physical_device_properties{};
        rhi->getPhysicalDeviceProperties(&physical_device_properties);
        // ���ڻ�ȡGPU֧�ֵ����������Լ���Ȳ�����Ӱ����������ã�

        // -------------------------- ��ʼ��������������Ϣ --------------------------
        RHISamplerCreateInfo samplerInfo{};
        samplerInfo.sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // ������˷�ʽ���Ŵ�/��С��ʹ�����Բ�ֵ��ƽ�����ɣ��ʺϻ�����ͼ��
        samplerInfo.magFilter = RHI_FILTER_LINEAR;
        samplerInfo.minFilter = RHI_FILTER_LINEAR;

        // �������곬��[0,1]��Χʱ�Ĵ���ʽ����Եǯ�ƣ������Ե�ڱ߻��ظ���
        samplerInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        // ���ø������Թ��ˣ������������ӽ���бʱ�Ĳ���������
        samplerInfo.anisotropyEnable = RHI_TRUE;
        // ���������Լ���ȡ��GPU֧�ֵ������豸���ԣ�ͨ��Ϊ2���ݴΣ���4/8/16��
        samplerInfo.maxAnisotropy = physical_device_properties.limits.maxSamplerAnisotropy;

        // �߽���ɫ������������ǯ�Ƶ���Եʱ��ʹ�ò�͸����ɫ�������ڻ�����ͼ��
        samplerInfo.borderColor = RHI_BORDER_COLOR_INT_OPAQUE_BLACK;
        // �Ƿ�ʹ�÷Ǳ�׼�����꣨�˴�ʹ�ñ�׼������[0,1]��
        samplerInfo.unnormalizedCoordinates = RHI_FALSE;
        // ������ȱȽϣ�����Ӱ����������ͨ�����������Ҫ�Ƚϲ�����
        samplerInfo.compareEnable = RHI_FALSE;
        samplerInfo.compareOp = RHI_COMPARE_OP_ALWAYS;
        // Mipmap���˷�ʽ�����Բ�ֵ��ƽ�����ɲ�ͬMip�㼶��
        samplerInfo.mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.maxLod = 0.0f;

        // -------------------------- ����/����BRDF LUT������ --------------------------
        // BRDF LUT��2D����ͨ������ҪMipmap���������������ʼ���Lod��Ϊ0
        if (m_global_render_resource._ibl_resource._brdfLUT_texture_sampler != RHI_NULL_HANDLE)
        {
            rhi->destroySampler(m_global_render_resource._ibl_resource._brdfLUT_texture_sampler);
        }

        // �����µ�BRDF LUT��������ʹ������ͨ�����ã�
        if (rhi->createSampler(&samplerInfo, m_global_render_resource._ibl_resource._brdfLUT_texture_sampler) != RHI_SUCCESS)
        {
            throw std::runtime_error("vk create sampler!");
        }

        // -------------------------- ���·��ն���ͼ������ --------------------------
        // ���ն���ͼ����������ͼ��ͨ�������༶Mipmap�����ڲ�ͬ����Ĳ�����
        samplerInfo.minLod = 0.0f;// ��СMipmap�㼶����0����ʼ��
        samplerInfo.maxLod = 8.0f; // ���Mipmap�㼶��������ն���ͼ��8����TODO�����ʵ�ʵ�����
        samplerInfo.mipLodBias = 0.0f;// Mipmap�㼶ƫ�ƣ��޶���ƫ�ƣ�  

        if (m_global_render_resource._ibl_resource._irradiance_texture_sampler != RHI_NULL_HANDLE)
        {
            rhi->destroySampler(m_global_render_resource._ibl_resource._irradiance_texture_sampler);
        }

        if (rhi->createSampler(&samplerInfo, m_global_render_resource._ibl_resource._irradiance_texture_sampler) != RHI_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }

        // -------------------------- ���¸߹���ͼ������ --------------------------
        // �߹���ͼ��Ԥ����Ļ������ڱλ���гϵ����ͨ������ն���ͼʹ����ͬ��������
        if (m_global_render_resource._ibl_resource._specular_texture_sampler != RHI_NULL_HANDLE)
        {
            rhi->destroySampler(m_global_render_resource._ibl_resource._specular_texture_sampler);
        }

        if (rhi->createSampler(&samplerInfo, m_global_render_resource._ibl_resource._specular_texture_sampler) != RHI_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }
    }

    void RenderResource::createIBLTextures(std::shared_ptr<RHI> rhi, std::array<std::shared_ptr<TextureData>, 6> irradiance_maps, std::array<std::shared_ptr<TextureData>, 6> specular_maps)
    {
        // -------------------------- ������ն���ͼ��Mipmap�㼶�� --------------------------
        // ���ն���ͼ����������ͼ����Mipmap�㼶�ɿ�ߵ����ֵ������log2(max(w,h))����ȡ�� +1��
        uint32_t irradiance_cubemap_miplevels = static_cast<uint32_t>(std::floor(log2(std::max(irradiance_maps[0]->m_width, irradiance_maps[0]->m_height)))) + 1;

        // ����Vulkan RHI�ӿڴ�����������ͼ��
        // - ͼ�������ͼ�����ڴ�����������������
        // - ��ߣ���������ͼÿ����Ϊ�����Σ�
        // - ��������������ݣ���˳���룩
        // - ��ʽ����R16G16B16A16_SFLOAT��
        // - Mipmap�㼶���������������в㼶��Mipmap��
        rhi->createCubeMap(
            m_global_render_resource._ibl_resource._irradiance_texture_image,
            m_global_render_resource._ibl_resource._irradiance_texture_image_view,
            m_global_render_resource._ibl_resource._irradiance_texture_image_allocation,
            irradiance_maps[0]->m_width,
            irradiance_maps[0]->m_height,
            { irradiance_maps[0]->m_pixels,   // +X����������
             irradiance_maps[1]->m_pixels,    // -X����������
             irradiance_maps[2]->m_pixels,    // +Z����������
             irradiance_maps[3]->m_pixels,    // -Z����������
             irradiance_maps[4]->m_pixels,    // +Y����������
             irradiance_maps[5]->m_pixels },  // -Y����������
            irradiance_maps[0]->m_format,     // �����ʽ
            irradiance_cubemap_miplevels);    // Mipmap�㼶��

        // -------------------------- ����߹���ͼ��Mipmap�㼶�� --------------------------
        uint32_t specular_cubemap_miplevels = static_cast<uint32_t>(std::floor(log2(std::max(specular_maps[0]->m_width, specular_maps[0]->m_height)))) + 1;

        // -------------------------- �����߹���ͼ��������ͼ --------------------------
        rhi->createCubeMap(
            m_global_render_resource._ibl_resource._specular_texture_image,
            m_global_render_resource._ibl_resource._specular_texture_image_view,
            m_global_render_resource._ibl_resource._specular_texture_image_allocation,
            specular_maps[0]->m_width,
            specular_maps[0]->m_height,
            { specular_maps[0]->m_pixels,
             specular_maps[1]->m_pixels,
             specular_maps[2]->m_pixels,
             specular_maps[3]->m_pixels,
             specular_maps[4]->m_pixels,
             specular_maps[5]->m_pixels },
            specular_maps[0]->m_format,
            specular_cubemap_miplevels);
    }

    VulkanMesh& RenderResource::getOrCreateVulkanMesh(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMeshData mesh_data)
    {
        size_t assetid = entity.m_mesh_asset_id;  // ������Դ��Ψһ��ʶ���ʲ�ID��

        // -------------------------- �����Դ�Ƿ��Ѵ��� --------------------------
        auto it = m_vulkan_meshes.find(assetid);  // ���ѹ����Vulkan����ӳ���в���
        if (it != m_vulkan_meshes.end())  // ���ҵ��Ѵ��ڵ���Դ
        {
            return it->second;  // ֱ�ӷ���������Դ
        }
        else
        {
            VulkanMesh temp; // ��ʱVulkanMesh�������ڲ���ӳ�䣩
            auto       res = m_vulkan_meshes.insert(std::make_pair(assetid, std::move(temp)));// ��������Դ��ӳ��
            assert(res.second);// ���Բ���ɹ��������ظ����룩

            // -------------------------- ��ȡ����/�������������� --------------------------
            uint32_t index_buffer_size = static_cast<uint32_t>(mesh_data.m_static_mesh_data.m_index_buffer->m_size);  // ������������С���ֽڣ�
            void* index_buffer_data = mesh_data.m_static_mesh_data.m_index_buffer->m_data;// ��������������ָ��

            uint32_t vertex_buffer_size = static_cast<uint32_t>(mesh_data.m_static_mesh_data.m_vertex_buffer->m_size);// ���㻺������С���ֽڣ�
            MeshVertexDataDefinition* vertex_buffer_data = reinterpret_cast<MeshVertexDataDefinition*>(mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);// ���㻺��������ָ�루ת��Ϊ�Զ���ṹ�壩

            // -------------------------- ��ȡ��ǰ�´�����������Դ���� --------------------------
            VulkanMesh& now_mesh = res.first->second;

            // -------------------------- ������������ݣ����У� --------------------------
            if (mesh_data.m_skeleton_binding_buffer)  // �����ڹ����󶨻����������ɫģ�͵Ĺ���Ȩ��/������
            {
                uint32_t joint_binding_buffer_size = (uint32_t)mesh_data.m_skeleton_binding_buffer->m_size;  // �����󶨻�������С
                MeshVertexBindingDataDefinition* joint_binding_buffer_data = reinterpret_cast<MeshVertexBindingDataDefinition*>(mesh_data.m_skeleton_binding_buffer->m_data);

                // ���ø��º������ϴ�����/����/�������ݵ�GPU��ʹ�ù��������ݣ�
                updateMeshData(rhi,
                               true,
                               index_buffer_size,
                               index_buffer_data,
                               vertex_buffer_size,
                               vertex_buffer_data,
                               joint_binding_buffer_size,
                               joint_binding_buffer_data,
                               now_mesh);
            }
            else
            {
                // ���ø��º��������ϴ�����/�������ݣ��޹����󶨣�
                updateMeshData(rhi,
                               false,
                               index_buffer_size,
                               index_buffer_data,
                               vertex_buffer_size,
                               vertex_buffer_data,
                               0,
                               NULL,
                               now_mesh);
            }

            return now_mesh;  // �����´�����������Դ
        }
    }

    VulkanPBRMaterial& RenderResource::getOrCreateVulkanMaterial(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMaterialData material_data)
    {
        VulkanRHI* vulkan_context = static_cast<VulkanRHI*>(rhi.get());

        // �Ӳ���ʵ������ȡ�ʲ� ID��������Դ��Ψһ��ʶ�����ڻ�����ң�
        size_t assetid = entity.m_material_asset_id;

        // -------------------- ���� 1���������Ƿ��ѻ��� --------------------
        // ���ѹ���� Vulkan PBR ���������в����Ƿ���ڸ��ʲ��Ĳ���
        auto it = m_vulkan_pbr_materials.find(assetid);
        if (it != m_vulkan_pbr_materials.end())
        {
            return it->second;
        }
        else
        {
            // -------------------- ���� 2��������ʱ���ʶ������� --------------------
            VulkanPBRMaterial temp;
            // �����²��ʵ�������ʹ�� emplace �� insert ���⿽����
            auto res = m_vulkan_pbr_materials.insert(std::make_pair(assetid, std::move(temp)));
            assert(res.second);

            // -------------------- ���� 3����ʼ��������ز�����Ĭ��ֵ + ʵ���������ݣ� --------------------
            // ����Ϊ��������������ɫ�������ֲڶȵȣ����������ݡ��ߴ�͸�ʽ�ĳ�ʼ��
            // Ĭ��ʹ�� 1x1 �Ļ�ɫ���أ�RGBA ��ͨ�� 0.5f������ʽΪ SRGB���ʺ� sRGB ����ռ䣩

            float empty_image[] = { 0.5f, 0.5f, 0.5f, 0.5f };

            // ������ɫ�������
            void*     base_color_image_pixels = empty_image;  // ��������ָ�루Ĭ�Ͽ�ͼ��
            uint32_t  base_color_image_width  = 1;            // �����ȣ�Ĭ�� 1��
            uint32_t  base_color_image_height = 1;            // ����߶ȣ�Ĭ�� 1��
            RHIFormat base_color_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB;  // ��ʽ��Ĭ�� SRGB��
            if (material_data.m_base_color_texture)           // �����ڻ�����ɫ����
            {
                // ʹ��ʵ���������ݸ���Ĭ��ֵ
                base_color_image_pixels = material_data.m_base_color_texture->m_pixels;
                base_color_image_width  = static_cast<uint32_t>(material_data.m_base_color_texture->m_width);
                base_color_image_height = static_cast<uint32_t>(material_data.m_base_color_texture->m_height);
                base_color_image_format = material_data.m_base_color_texture->m_format;
            }

            // ����-�ֲڶ�������������ƻ�����ɫ����Ĵ����߼���
            void*     metallic_roughness_image_pixels = empty_image;
            uint32_t  metallic_roughness_width        = 1;
            uint32_t  metallic_roughness_height       = 1;
            RHIFormat metallic_roughness_format       = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_metallic_roughness_texture)
            {
                metallic_roughness_image_pixels = material_data.m_metallic_roughness_texture->m_pixels;
                metallic_roughness_width        = static_cast<uint32_t>(material_data.m_metallic_roughness_texture->m_width);
                metallic_roughness_height       = static_cast<uint32_t>(material_data.m_metallic_roughness_texture->m_height);
                metallic_roughness_format       = material_data.m_metallic_roughness_texture->m_format;
            }

            // ����-�ֲڶ����������ע�⣺�˴�����Ϊ����ͨ�����������ʽΪ RGB�����������߼�����
            void* normal_roughness_image_pixels = empty_image;
            uint32_t           normal_roughness_width = 1;
            uint32_t           normal_roughness_height = 1;
            RHIFormat normal_roughness_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_normal_texture)
            {
                normal_roughness_image_pixels = material_data.m_normal_texture->m_pixels;
                normal_roughness_width = static_cast<uint32_t>(material_data.m_normal_texture->m_width);
                normal_roughness_height = static_cast<uint32_t>(material_data.m_normal_texture->m_height);
                normal_roughness_format = material_data.m_normal_texture->m_format;
            }

            // �ڵ��������
            void* occlusion_image_pixels = empty_image;
            uint32_t           occlusion_image_width = 1;
            uint32_t           occlusion_image_height = 1;
            RHIFormat occlusion_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_occlusion_texture)
            {
                occlusion_image_pixels = material_data.m_occlusion_texture->m_pixels;
                occlusion_image_width = static_cast<uint32_t>(material_data.m_occlusion_texture->m_width);
                occlusion_image_height = static_cast<uint32_t>(material_data.m_occlusion_texture->m_height);
                occlusion_image_format = material_data.m_occlusion_texture->m_format;
            }

            // �Է����������
            void* emissive_image_pixels = empty_image;
            uint32_t           emissive_image_width = 1;
            uint32_t           emissive_image_height = 1;
            RHIFormat emissive_image_format = RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            if (material_data.m_emissive_texture)
            {
                emissive_image_pixels = material_data.m_emissive_texture->m_pixels;
                emissive_image_width  = static_cast<uint32_t>(material_data.m_emissive_texture->m_width);
                emissive_image_height = static_cast<uint32_t>(material_data.m_emissive_texture->m_height);
                emissive_image_format = material_data.m_emissive_texture->m_format;
            }

            VulkanPBRMaterial& now_material = res.first->second;

            // -------------------- ���� 4����ʼ������ͳһ��������Uniform Buffer�� --------------------
            // ͳһ���������ڴ洢���ʲ��������ϱ�־��˫����Ⱦ����ɫ���ӵȣ�������ɫ������
            {
                // ͳһ��������С������ MeshPerMaterialUniformBufferObject �ṹ���С��
                RHIDeviceSize buffer_size = sizeof(MeshPerMaterialUniformBufferObject);

                // ������ʱ�ݴ滺���������ڽ����ݴ������ڴ渴�Ƶ� GPU �ڴ棩
                // ʹ�� HOST_VISIBLE �� HOST_COHERENT ���ԣ���������ֱ�Ӷ�д�������ֶ�ˢ�»���
                RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
                RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
                rhi->createBuffer(
                    buffer_size,
                    RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    inefficient_staging_buffer,
                    inefficient_staging_buffer_memory);

                // ӳ���ݴ滺�����ڴ浽������ַ�ռ䣨��ȡָ����д�����ݣ�
                void* staging_buffer_data = nullptr;
                rhi->mapMemory(
                    inefficient_staging_buffer_memory,
                    0,
                    buffer_size,
                    0,
                    &staging_buffer_data);
                
                // ���ͳһ���������ݣ����ڲ���ʵ������ԣ�
                MeshPerMaterialUniformBufferObject& material_uniform_buffer_info = (*static_cast<MeshPerMaterialUniformBufferObject*>(staging_buffer_data));
                material_uniform_buffer_info.is_blend = entity.m_blend;// �Ƿ����û��
                material_uniform_buffer_info.is_double_sided = entity.m_double_sided;// �Ƿ�˫����Ⱦ
                material_uniform_buffer_info.baseColorFactor = entity.m_base_color_factor;// ��ɫ���ӣ�RGBA��
                material_uniform_buffer_info.metallicFactor = entity.m_metallic_factor;// ����������
                material_uniform_buffer_info.roughnessFactor = entity.m_roughness_factor;// �ֲڶ�����
                material_uniform_buffer_info.normalScale = entity.m_normal_scale;// ������ͼ����
                material_uniform_buffer_info.occlusionStrength = entity.m_occlusion_strength;// �ڵ�ǿ��
                material_uniform_buffer_info.emissiveFactor = entity.m_emissive_factor;// �Է�������

                // ����ڴ�ӳ�䣨�ͷ�������ָ�룬������ͬ���� GPU��
                rhi->unmapMemory(inefficient_staging_buffer_memory);

                // ���� GPU ר��ͳһ��������ʹ�� VMA ������ڴ���䣩
                // ��������;��UNIFORM_BUFFER����ɫ��ͳһ�������� + TRANSFER_DST�����մ������ݣ�
                RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                bufferInfo.size = buffer_size;
                bufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

                // VMA �ڴ������ԣ��� GPU �ɼ������� CPU ���ʿ�����
                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

                // ���仺���������ڴ棨ʹ�ö���Ҫ������ Vulkan ����Լ����
                rhi->createBufferWithAlignmentVMA(
                    vulkan_context->m_assets_allocator,// VMA ������ʵ��
                    &bufferInfo,// ������������Ϣ
                    &allocInfo,// �ڴ�������
                    m_global_render_resource._storage_buffer._min_uniform_buffer_offset_alignment,// ��С����Ҫ��
                    now_material.material_uniform_buffer,
                    &now_material.material_uniform_buffer_allocation,
                    NULL);

                // use the data from staging buffer
                rhi->copyBuffer(inefficient_staging_buffer, now_material.material_uniform_buffer, 0, 0, buffer_size);

                // release staging buffer
                rhi->destroyBuffer(inefficient_staging_buffer);
                rhi->freeMemory(inefficient_staging_buffer_memory);
            }

            TextureDataToUpdate update_texture_data;
            update_texture_data.base_color_image_pixels         = base_color_image_pixels;
            update_texture_data.base_color_image_width          = base_color_image_width;
            update_texture_data.base_color_image_height         = base_color_image_height;
            update_texture_data.base_color_image_format         = base_color_image_format;
            update_texture_data.metallic_roughness_image_pixels = metallic_roughness_image_pixels;
            update_texture_data.metallic_roughness_image_width  = metallic_roughness_width;
            update_texture_data.metallic_roughness_image_height = metallic_roughness_height;
            update_texture_data.metallic_roughness_image_format = metallic_roughness_format;
            update_texture_data.normal_roughness_image_pixels   = normal_roughness_image_pixels;
            update_texture_data.normal_roughness_image_width    = normal_roughness_width;
            update_texture_data.normal_roughness_image_height   = normal_roughness_height;
            update_texture_data.normal_roughness_image_format   = normal_roughness_format;
            update_texture_data.occlusion_image_pixels          = occlusion_image_pixels;
            update_texture_data.occlusion_image_width           = occlusion_image_width;
            update_texture_data.occlusion_image_height          = occlusion_image_height;
            update_texture_data.occlusion_image_format          = occlusion_image_format;
            update_texture_data.emissive_image_pixels           = emissive_image_pixels;
            update_texture_data.emissive_image_width            = emissive_image_width;
            update_texture_data.emissive_image_height           = emissive_image_height;
            update_texture_data.emissive_image_format           = emissive_image_format;
            update_texture_data.now_material                    = &now_material;

            updateTextureImageData(rhi, update_texture_data);

            RHIDescriptorSetAllocateInfo material_descriptor_set_alloc_info;
            material_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            material_descriptor_set_alloc_info.pNext = NULL;
            material_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
            material_descriptor_set_alloc_info.descriptorSetCount = 1;
            material_descriptor_set_alloc_info.pSetLayouts        = m_material_descriptor_set_layout;

            if (RHI_SUCCESS != rhi->allocateDescriptorSets(
                &material_descriptor_set_alloc_info,
                now_material.material_descriptor_set))
            {
                throw std::runtime_error("allocate material descriptor set");
            }

            RHIDescriptorBufferInfo material_uniform_buffer_info = {};
            material_uniform_buffer_info.offset = 0;
            material_uniform_buffer_info.range = sizeof(MeshPerMaterialUniformBufferObject);
            material_uniform_buffer_info.buffer = now_material.material_uniform_buffer;

            RHIDescriptorImageInfo base_color_image_info = {};
            base_color_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            base_color_image_info.imageView = now_material.base_color_image_view;
            base_color_image_info.sampler = rhi->getOrCreateMipmapSampler(base_color_image_width,
                                                                          base_color_image_height);

            RHIDescriptorImageInfo metallic_roughness_image_info = {};
            metallic_roughness_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            metallic_roughness_image_info.imageView = now_material.metallic_roughness_image_view;
            metallic_roughness_image_info.sampler = rhi->getOrCreateMipmapSampler(metallic_roughness_width,
                                                                                  metallic_roughness_height);

            RHIDescriptorImageInfo normal_roughness_image_info = {};
            normal_roughness_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normal_roughness_image_info.imageView = now_material.normal_image_view;
            normal_roughness_image_info.sampler = rhi->getOrCreateMipmapSampler(normal_roughness_width,
                                                                                normal_roughness_height);

            RHIDescriptorImageInfo occlusion_image_info = {};
            occlusion_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            occlusion_image_info.imageView = now_material.occlusion_image_view;
            occlusion_image_info.sampler = rhi->getOrCreateMipmapSampler(occlusion_image_width,occlusion_image_height);

            RHIDescriptorImageInfo emissive_image_info = {};
            emissive_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            emissive_image_info.imageView = now_material.emissive_image_view;
            emissive_image_info.sampler = rhi->getOrCreateMipmapSampler(emissive_image_width, emissive_image_height);

            RHIWriteDescriptorSet mesh_descriptor_writes_info[6];

            mesh_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_descriptor_writes_info[0].pNext = NULL;
            mesh_descriptor_writes_info[0].dstSet = now_material.material_descriptor_set;
            mesh_descriptor_writes_info[0].dstBinding = 0;
            mesh_descriptor_writes_info[0].dstArrayElement = 0;
            mesh_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            mesh_descriptor_writes_info[0].descriptorCount = 1;
            mesh_descriptor_writes_info[0].pBufferInfo = &material_uniform_buffer_info;

            mesh_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_descriptor_writes_info[1].pNext = NULL;
            mesh_descriptor_writes_info[1].dstSet = now_material.material_descriptor_set;
            mesh_descriptor_writes_info[1].dstBinding = 1;
            mesh_descriptor_writes_info[1].dstArrayElement = 0;
            mesh_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            mesh_descriptor_writes_info[1].descriptorCount = 1;
            mesh_descriptor_writes_info[1].pImageInfo = &base_color_image_info;

            mesh_descriptor_writes_info[2] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[2].dstBinding = 2;
            mesh_descriptor_writes_info[2].pImageInfo = &metallic_roughness_image_info;

            mesh_descriptor_writes_info[3] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[3].dstBinding = 3;
            mesh_descriptor_writes_info[3].pImageInfo = &normal_roughness_image_info;

            mesh_descriptor_writes_info[4] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[4].dstBinding = 4;
            mesh_descriptor_writes_info[4].pImageInfo = &occlusion_image_info;

            mesh_descriptor_writes_info[5] = mesh_descriptor_writes_info[1];
            mesh_descriptor_writes_info[5].dstBinding = 5;
            mesh_descriptor_writes_info[5].pImageInfo = &emissive_image_info;

            rhi->updateDescriptorSets(6, mesh_descriptor_writes_info, 0, nullptr);

            return now_material;
        }
    }

    void RenderResource::updateMeshData(std::shared_ptr<RHI>                   rhi,
                                        bool                                   enable_vertex_blending,
                                        uint32_t                               index_buffer_size,
                                        void*                                  index_buffer_data,
                                        uint32_t                               vertex_buffer_size,
                                        MeshVertexDataDefinition const*        vertex_buffer_data,
                                        uint32_t                               joint_binding_buffer_size,
                                        MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                        VulkanMesh&                            now_mesh)
    {
        now_mesh.enable_vertex_blending = enable_vertex_blending;
        assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
        now_mesh.mesh_vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
        updateVertexBuffer(rhi,
                           enable_vertex_blending,
                           vertex_buffer_size,
                           vertex_buffer_data,
                           joint_binding_buffer_size,
                           joint_binding_buffer_data,
                           index_buffer_size,
                           reinterpret_cast<uint16_t*>(index_buffer_data),
                           now_mesh);
        assert(0 == (index_buffer_size % sizeof(uint16_t)));
        now_mesh.mesh_index_count = index_buffer_size / sizeof(uint16_t);
        updateIndexBuffer(rhi, index_buffer_size, index_buffer_data, now_mesh);
    }

    void RenderResource::updateVertexBuffer(std::shared_ptr<RHI>                   rhi,
                                            bool                                   enable_vertex_blending,
                                            uint32_t                               vertex_buffer_size,
                                            MeshVertexDataDefinition const*        vertex_buffer_data,
                                            uint32_t                               joint_binding_buffer_size,
                                            MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                            uint32_t                               index_buffer_size,
                                            uint16_t*                              index_buffer_data,
                                            VulkanMesh&                            now_mesh)
    {
        VulkanRHI* vulkan_context = static_cast<VulkanRHI*>(rhi.get());

        if (enable_vertex_blending)
        {
            assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
            uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);
            assert(0 == (index_buffer_size % sizeof(uint16_t)));
            uint32_t index_count = index_buffer_size / sizeof(uint16_t);

            RHIDeviceSize vertex_position_buffer_size = sizeof(MeshVertex::VulkanMeshVertexPostition) * vertex_count;
            RHIDeviceSize vertex_varying_enable_blending_buffer_size =
                sizeof(MeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
            RHIDeviceSize vertex_varying_buffer_size = sizeof(MeshVertex::VulkanMeshVertexVarying) * vertex_count;
            RHIDeviceSize vertex_joint_binding_buffer_size =
                sizeof(MeshVertex::VulkanMeshVertexJointBinding) * index_count;

            RHIDeviceSize vertex_position_buffer_offset = 0;
            RHIDeviceSize vertex_varying_enable_blending_buffer_offset =
                vertex_position_buffer_offset + vertex_position_buffer_size;
            RHIDeviceSize vertex_varying_buffer_offset =
                vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;
            RHIDeviceSize vertex_joint_binding_buffer_offset = vertex_varying_buffer_offset + vertex_varying_buffer_size;

            // temporary staging buffer
            RHIDeviceSize inefficient_staging_buffer_size =
                vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size +
                vertex_joint_binding_buffer_size;
            RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
            RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
            rhi->createBuffer(inefficient_staging_buffer_size,
                              RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

            void* inefficient_staging_buffer_data;
            rhi->mapMemory(inefficient_staging_buffer_memory,
                           0,
                           RHI_WHOLE_SIZE,
                           0,
                           &inefficient_staging_buffer_data);

            MeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
                reinterpret_cast<MeshVertex::VulkanMeshVertexPostition*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
            MeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
                    vertex_varying_enable_blending_buffer_offset);
            MeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVarying*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);
            MeshVertex::VulkanMeshVertexJointBinding* mesh_vertex_joint_binding =
                reinterpret_cast<MeshVertex::VulkanMeshVertexJointBinding*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_joint_binding_buffer_offset);

            for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
            {
                Vector3 normal = Vector3(vertex_buffer_data[vertex_index].nx,
                    vertex_buffer_data[vertex_index].ny,
                    vertex_buffer_data[vertex_index].nz);
                Vector3 tangent = Vector3(vertex_buffer_data[vertex_index].tx,
                    vertex_buffer_data[vertex_index].ty,
                    vertex_buffer_data[vertex_index].tz);

                mesh_vertex_positions[vertex_index].position = Vector3(vertex_buffer_data[vertex_index].x,
                    vertex_buffer_data[vertex_index].y,
                    vertex_buffer_data[vertex_index].z);

                mesh_vertex_blending_varyings[vertex_index].normal = normal;
                mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

                mesh_vertex_varyings[vertex_index].texcoord =
                    Vector2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
            }

            for (uint32_t index_index = 0; index_index < index_count; ++index_index)
            {
                uint32_t vertex_buffer_index = index_buffer_data[index_index];

                // TODO: move to assets loading process

                mesh_vertex_joint_binding[index_index].indices[0] = joint_binding_buffer_data[vertex_buffer_index].m_index0;
                mesh_vertex_joint_binding[index_index].indices[1] = joint_binding_buffer_data[vertex_buffer_index].m_index1;
                mesh_vertex_joint_binding[index_index].indices[2] = joint_binding_buffer_data[vertex_buffer_index].m_index2;
                mesh_vertex_joint_binding[index_index].indices[3] = joint_binding_buffer_data[vertex_buffer_index].m_index3;

                float inv_total_weight = joint_binding_buffer_data[vertex_buffer_index].m_weight0 +
                                         joint_binding_buffer_data[vertex_buffer_index].m_weight1 +
                                         joint_binding_buffer_data[vertex_buffer_index].m_weight2 +
                                         joint_binding_buffer_data[vertex_buffer_index].m_weight3;

                inv_total_weight = (inv_total_weight != 0.0) ? 1 / inv_total_weight : 1.0;

                mesh_vertex_joint_binding[index_index].weights =
                    Vector4(joint_binding_buffer_data[vertex_buffer_index].m_weight0 * inv_total_weight,
                        joint_binding_buffer_data[vertex_buffer_index].m_weight1 * inv_total_weight,
                        joint_binding_buffer_data[vertex_buffer_index].m_weight2 * inv_total_weight,
                        joint_binding_buffer_data[vertex_buffer_index].m_weight3 * inv_total_weight);
            }

            rhi->unmapMemory(inefficient_staging_buffer_memory);

            // use the vmaAllocator to allocate asset vertex buffer
            RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.size = vertex_position_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_position_buffer,
                                 &now_mesh.mesh_vertex_position_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_enable_blending_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_enable_blending_buffer,
                                 &now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_buffer,
                                 &now_mesh.mesh_vertex_varying_buffer_allocation,
                                 NULL);

            bufferInfo.usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.size = vertex_joint_binding_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_joint_binding_buffer,
                                 &now_mesh.mesh_vertex_joint_binding_buffer_allocation,
                                 NULL);

            // use the data from staging buffer
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_position_buffer,
                            vertex_position_buffer_offset,
                            0,
                            vertex_position_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_enable_blending_buffer,
                            vertex_varying_enable_blending_buffer_offset,
                            0,
                            vertex_varying_enable_blending_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_buffer,
                            vertex_varying_buffer_offset,
                            0,
                            vertex_varying_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_joint_binding_buffer,
                            vertex_joint_binding_buffer_offset,
                            0,
                            vertex_joint_binding_buffer_size);

            // release staging buffer
            rhi->destroyBuffer(inefficient_staging_buffer);
            rhi->freeMemory(inefficient_staging_buffer_memory);

            // update descriptor set
            RHIDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
                RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts        = m_mesh_descriptor_set_layout;

            if (RHI_SUCCESS != rhi->allocateDescriptorSets(
                &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                now_mesh.mesh_vertex_blending_descriptor_set))
            {
                throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
            }

            RHIDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
            mesh_vertex_Joint_binding_storage_buffer_info.offset = 0;
            mesh_vertex_Joint_binding_storage_buffer_info.range = vertex_joint_binding_buffer_size;
            mesh_vertex_Joint_binding_storage_buffer_info.buffer = now_mesh.mesh_vertex_joint_binding_buffer;
            assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
                m_global_render_resource._storage_buffer._max_storage_buffer_range);

            RHIDescriptorSet* descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

            RHIWriteDescriptorSet descriptor_writes[1];

            RHIWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
                descriptor_writes[0];
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
                RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = NULL;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet = descriptor_set_to_write;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
                RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
                &mesh_vertex_Joint_binding_storage_buffer_info;

            rhi->updateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                                      descriptor_writes,
                                      0,
                                      NULL);
        }
        else
        {
            assert(0 == (vertex_buffer_size % sizeof(MeshVertexDataDefinition)));
            uint32_t vertex_count = vertex_buffer_size / sizeof(MeshVertexDataDefinition);

            RHIDeviceSize vertex_position_buffer_size = sizeof(MeshVertex::VulkanMeshVertexPostition) * vertex_count;
            RHIDeviceSize vertex_varying_enable_blending_buffer_size =
                sizeof(MeshVertex::VulkanMeshVertexVaryingEnableBlending) * vertex_count;
            RHIDeviceSize vertex_varying_buffer_size = sizeof(MeshVertex::VulkanMeshVertexVarying) * vertex_count;

            RHIDeviceSize vertex_position_buffer_offset = 0;
            RHIDeviceSize vertex_varying_enable_blending_buffer_offset =
                vertex_position_buffer_offset + vertex_position_buffer_size;
            RHIDeviceSize vertex_varying_buffer_offset =
                vertex_varying_enable_blending_buffer_offset + vertex_varying_enable_blending_buffer_size;

            // temporary staging buffer
            RHIDeviceSize inefficient_staging_buffer_size =
                vertex_position_buffer_size + vertex_varying_enable_blending_buffer_size + vertex_varying_buffer_size;
            RHIBuffer* inefficient_staging_buffer = RHI_NULL_HANDLE;
            RHIDeviceMemory* inefficient_staging_buffer_memory = RHI_NULL_HANDLE;
            rhi->createBuffer(inefficient_staging_buffer_size,
                              RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              inefficient_staging_buffer,
                              inefficient_staging_buffer_memory);

            void* inefficient_staging_buffer_data;
            rhi->mapMemory(inefficient_staging_buffer_memory,
                           0,
                           RHI_WHOLE_SIZE,
                           0,
                           &inefficient_staging_buffer_data);

            MeshVertex::VulkanMeshVertexPostition* mesh_vertex_positions =
                reinterpret_cast<MeshVertex::VulkanMeshVertexPostition*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_position_buffer_offset);
            MeshVertex::VulkanMeshVertexVaryingEnableBlending* mesh_vertex_blending_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVaryingEnableBlending*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) +
                    vertex_varying_enable_blending_buffer_offset);
            MeshVertex::VulkanMeshVertexVarying* mesh_vertex_varyings =
                reinterpret_cast<MeshVertex::VulkanMeshVertexVarying*>(
                    reinterpret_cast<uintptr_t>(inefficient_staging_buffer_data) + vertex_varying_buffer_offset);

            for (uint32_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
            {
                Vector3 normal = Vector3(vertex_buffer_data[vertex_index].nx,
                    vertex_buffer_data[vertex_index].ny,
                    vertex_buffer_data[vertex_index].nz);
                Vector3 tangent = Vector3(vertex_buffer_data[vertex_index].tx,
                    vertex_buffer_data[vertex_index].ty,
                    vertex_buffer_data[vertex_index].tz);

                mesh_vertex_positions[vertex_index].position = Vector3(vertex_buffer_data[vertex_index].x,
                    vertex_buffer_data[vertex_index].y,
                    vertex_buffer_data[vertex_index].z);

                mesh_vertex_blending_varyings[vertex_index].normal = normal;
                mesh_vertex_blending_varyings[vertex_index].tangent = tangent;

                mesh_vertex_varyings[vertex_index].texcoord =
                    Vector2(vertex_buffer_data[vertex_index].u, vertex_buffer_data[vertex_index].v);
            }

            rhi->unmapMemory(inefficient_staging_buffer_memory);

            // use the vmaAllocator to allocate asset vertex buffer
            RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            bufferInfo.size = vertex_position_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_position_buffer,
                                 &now_mesh.mesh_vertex_position_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_enable_blending_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_enable_blending_buffer,
                                 &now_mesh.mesh_vertex_varying_enable_blending_buffer_allocation,
                                 NULL);
            bufferInfo.size = vertex_varying_buffer_size;
            rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 now_mesh.mesh_vertex_varying_buffer,
                                 &now_mesh.mesh_vertex_varying_buffer_allocation,
                                 NULL);

            // use the data from staging buffer
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_position_buffer,
                            vertex_position_buffer_offset,
                            0,
                            vertex_position_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_enable_blending_buffer,
                            vertex_varying_enable_blending_buffer_offset,
                            0,
                            vertex_varying_enable_blending_buffer_size);
            rhi->copyBuffer(inefficient_staging_buffer,
                            now_mesh.mesh_vertex_varying_buffer,
                            vertex_varying_buffer_offset,
                            0,
                            vertex_varying_buffer_size);

            // release staging buffer
            rhi->destroyBuffer(inefficient_staging_buffer);
            rhi->freeMemory(inefficient_staging_buffer_memory);

            // update descriptor set
            RHIDescriptorSetAllocateInfo mesh_vertex_blending_per_mesh_descriptor_set_alloc_info;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.sType =
                RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pNext = NULL;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorPool = vulkan_context->m_descriptor_pool;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.descriptorSetCount = 1;
            mesh_vertex_blending_per_mesh_descriptor_set_alloc_info.pSetLayouts        = m_mesh_descriptor_set_layout;

            if (RHI_SUCCESS != rhi->allocateDescriptorSets(
                &mesh_vertex_blending_per_mesh_descriptor_set_alloc_info,
                now_mesh.mesh_vertex_blending_descriptor_set))
            {
                throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
            }

            RHIDescriptorBufferInfo mesh_vertex_Joint_binding_storage_buffer_info = {};
            mesh_vertex_Joint_binding_storage_buffer_info.offset = 0;
            mesh_vertex_Joint_binding_storage_buffer_info.range = 1;
            mesh_vertex_Joint_binding_storage_buffer_info.buffer =
                m_global_render_resource._storage_buffer._global_null_descriptor_storage_buffer;
            assert(mesh_vertex_Joint_binding_storage_buffer_info.range <
                m_global_render_resource._storage_buffer._max_storage_buffer_range);

            RHIDescriptorSet* descriptor_set_to_write = now_mesh.mesh_vertex_blending_descriptor_set;

            RHIWriteDescriptorSet descriptor_writes[1];

            RHIWriteDescriptorSet& mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info =
                descriptor_writes[0];
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.sType =
                RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pNext = NULL;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstSet = descriptor_set_to_write;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstBinding = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.dstArrayElement = 0;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorType =
                RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.descriptorCount = 1;
            mesh_vertex_blending_vertex_Joint_binding_storage_buffer_write_info.pBufferInfo =
                &mesh_vertex_Joint_binding_storage_buffer_info;

            rhi->updateDescriptorSets((sizeof(descriptor_writes) / sizeof(descriptor_writes[0])),
                                      descriptor_writes,
                                      0,
                                      NULL);
        }
    }

    void RenderResource::updateIndexBuffer(std::shared_ptr<RHI> rhi,
                                           uint32_t             index_buffer_size,
                                           void*                index_buffer_data,
                                           VulkanMesh&          now_mesh)
    {
        VulkanRHI* vulkan_context = static_cast<VulkanRHI*>(rhi.get());

        // temp staging buffer
        RHIDeviceSize buffer_size = index_buffer_size;

        RHIBuffer* inefficient_staging_buffer;
        RHIDeviceMemory* inefficient_staging_buffer_memory;
        rhi->createBuffer(buffer_size,
                          RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          inefficient_staging_buffer,
                          inefficient_staging_buffer_memory);

        void* staging_buffer_data;
        rhi->mapMemory(inefficient_staging_buffer_memory, 0, buffer_size, 0, &staging_buffer_data);
        memcpy(staging_buffer_data, index_buffer_data, (size_t)buffer_size);
        rhi->unmapMemory(inefficient_staging_buffer_memory);

        // use the vmaAllocator to allocate asset index buffer
        RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = buffer_size;
        bufferInfo.usage = RHI_BUFFER_USAGE_INDEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        rhi->createBufferVMA(vulkan_context->m_assets_allocator,
                             &bufferInfo,
                             &allocInfo,
                             now_mesh.mesh_index_buffer,
                             &now_mesh.mesh_index_buffer_allocation,
                             NULL);

        // use the data from staging buffer
        rhi->copyBuffer( inefficient_staging_buffer, now_mesh.mesh_index_buffer, 0, 0, buffer_size);

        // release temp staging buffer
        rhi->destroyBuffer(inefficient_staging_buffer);
        rhi->freeMemory(inefficient_staging_buffer_memory);
    }

    void RenderResource::updateTextureImageData(std::shared_ptr<RHI> rhi, const TextureDataToUpdate& texture_data)
    {
        rhi->createGlobalImage(
            texture_data.now_material->base_color_texture_image,
            texture_data.now_material->base_color_image_view,
            texture_data.now_material->base_color_image_allocation,
            texture_data.base_color_image_width,
            texture_data.base_color_image_height,
            texture_data.base_color_image_pixels,
            texture_data.base_color_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->metallic_roughness_texture_image,
            texture_data.now_material->metallic_roughness_image_view,
            texture_data.now_material->metallic_roughness_image_allocation,
            texture_data.metallic_roughness_image_width,
            texture_data.metallic_roughness_image_height,
            texture_data.metallic_roughness_image_pixels,
            texture_data.metallic_roughness_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->normal_texture_image,
            texture_data.now_material->normal_image_view,
            texture_data.now_material->normal_image_allocation,
            texture_data.normal_roughness_image_width,
            texture_data.normal_roughness_image_height,
            texture_data.normal_roughness_image_pixels,
            texture_data.normal_roughness_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->occlusion_texture_image,
            texture_data.now_material->occlusion_image_view,
            texture_data.now_material->occlusion_image_allocation,
            texture_data.occlusion_image_width,
            texture_data.occlusion_image_height,
            texture_data.occlusion_image_pixels,
            texture_data.occlusion_image_format);

        rhi->createGlobalImage(
            texture_data.now_material->emissive_texture_image,
            texture_data.now_material->emissive_image_view,
            texture_data.now_material->emissive_image_allocation,
            texture_data.emissive_image_width,
            texture_data.emissive_image_height,
            texture_data.emissive_image_pixels,
            texture_data.emissive_image_format);
    }

    VulkanMesh& RenderResource::getEntityMesh(RenderEntity entity)
    {
        size_t assetid = entity.m_mesh_asset_id;

        auto it = m_vulkan_meshes.find(assetid);
        if (it != m_vulkan_meshes.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("failed to get entity mesh");
        }
    }

    VulkanPBRMaterial& RenderResource::getEntityMaterial(RenderEntity entity)
    {
        size_t assetid = entity.m_material_asset_id;

        auto it = m_vulkan_pbr_materials.find(assetid);
        if (it != m_vulkan_pbr_materials.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("failed to get entity material");
        }
    }

    void RenderResource::resetRingBufferOffset(uint8_t current_frame_index)
    {
        m_global_render_resource._storage_buffer._global_upload_ringbuffers_end[current_frame_index] =
            m_global_render_resource._storage_buffer._global_upload_ringbuffers_begin[current_frame_index];
    }

    void RenderResource::createAndMapStorageBuffer(std::shared_ptr<RHI> rhi)
    {
        VulkanRHI* raw_rhi = static_cast<VulkanRHI*>(rhi.get());
        StorageBuffer& _storage_buffer = m_global_render_resource._storage_buffer;
        uint32_t       frames_in_flight = raw_rhi->k_max_frames_in_flight;

        RHIPhysicalDeviceProperties properties;
        rhi->getPhysicalDeviceProperties(&properties);

        _storage_buffer._min_uniform_buffer_offset_alignment =
            static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
        _storage_buffer._min_storage_buffer_offset_alignment =
            static_cast<uint32_t>(properties.limits.minStorageBufferOffsetAlignment);
        _storage_buffer._max_storage_buffer_range = properties.limits.maxStorageBufferRange;
        _storage_buffer._non_coherent_atom_size = properties.limits.nonCoherentAtomSize;

        // In Vulkan, the storage buffer should be pre-allocated.
        // The size is 128MB in NVIDIA D3D11
        // driver(https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0).
        uint32_t global_storage_buffer_size = 1024 * 1024 * 128;
        rhi->createBuffer(global_storage_buffer_size,
                          RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          _storage_buffer._global_upload_ringbuffer,
                          _storage_buffer._global_upload_ringbuffer_memory);

        _storage_buffer._global_upload_ringbuffers_begin.resize(frames_in_flight);
        _storage_buffer._global_upload_ringbuffers_end.resize(frames_in_flight);
        _storage_buffer._global_upload_ringbuffers_size.resize(frames_in_flight);
        for (uint32_t i = 0; i < frames_in_flight; ++i)
        {
            _storage_buffer._global_upload_ringbuffers_begin[i] = (global_storage_buffer_size * i) / frames_in_flight;
            _storage_buffer._global_upload_ringbuffers_size[i] =
                (global_storage_buffer_size * (i + 1)) / frames_in_flight -
                (global_storage_buffer_size * i) / frames_in_flight;
        }

        // axis
        rhi->createBuffer(sizeof(AxisStorageBufferObject),
                          RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          _storage_buffer._axis_inefficient_storage_buffer,
                          _storage_buffer._axis_inefficient_storage_buffer_memory);

        // null descriptor
        rhi->createBuffer(64,
                          RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          0,
                          _storage_buffer._global_null_descriptor_storage_buffer,
                          _storage_buffer._global_null_descriptor_storage_buffer_memory);

        // TODO: Unmap when program terminates
        rhi->mapMemory(_storage_buffer._global_upload_ringbuffer_memory,
                       0,
                       RHI_WHOLE_SIZE,
                       0,
                       &_storage_buffer._global_upload_ringbuffer_memory_pointer);

        rhi->mapMemory(_storage_buffer._axis_inefficient_storage_buffer_memory,
                       0,
                       RHI_WHOLE_SIZE,
                       0,
                       &_storage_buffer._axis_inefficient_storage_buffer_memory_pointer);

        static_assert(64 >= sizeof(MeshVertex::VulkanMeshVertexJointBinding), "");
    }
}

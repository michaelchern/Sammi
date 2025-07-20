#pragma once

#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_type.h"
#include "runtime/function/render/interface/rhi.h"

#include "runtime/function/render/render_common.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <map>
#include <vector>
#include <cmath>

namespace Sammi
{
    class RHI;
    class RenderPassBase;
    class RenderCamera;

    // -------------------------- IBL������ͼ��Ĺ��գ���Դ�ṹ�� --------------------------
    /// �洢IBL������ͼ��Ĺ��գ������GPU��Դ
    struct IBLResource
    {
        RHIImage*     _brdfLUT_texture_image;                // BRDF LUT��˫����ֲ��������ұ�ͼ�����
        RHIImageView* _brdfLUT_texture_image_view;           // BRDF LUTͼ����ͼ�����ڷ���ͼ�����ݣ�
        RHISampler*   _brdfLUT_texture_sampler;              // BRDF LUT���������������������ʽ��
        VmaAllocation _brdfLUT_texture_image_allocation;     // BRDF LUTͼ���Vulkan�ڴ������

        RHIImage*     _irradiance_texture_image;             // ���ն���ͼ�������������䲿�֣�ͼ�����
        RHIImageView* _irradiance_texture_image_view;        // ���ն���ͼͼ����ͼ
        RHISampler*   _irradiance_texture_sampler;           // ���ն���ͼ������
        VmaAllocation _irradiance_texture_image_allocation;  // ���ն���ͼ�ڴ����

        RHIImage*     _specular_texture_image;               // ���淴����ͼ�������⾵�淴�䲿�֣�ͼ�����
        RHIImageView* _specular_texture_image_view;          // ���淴����ͼͼ����ͼ
        RHISampler*   _specular_texture_sampler;             // ���淴����ͼ������
        VmaAllocation _specular_texture_image_allocation;    // ���淴����ͼ�ڴ����
    };

    /// �洢IBL��Դ��ԭʼ�������ݣ����ڴ�CPU�ϴ���GPU��
    struct IBLResourceData
    {
        void*                _brdfLUT_texture_image_pixels;     // BRDF LUTͼ����������ָ��
        uint32_t             _brdfLUT_texture_image_width;      // BRDF LUTͼ����
        uint32_t             _brdfLUT_texture_image_height;     // BRDF LUTͼ��߶�
        RHIFormat            _brdfLUT_texture_image_format;     // BRDF LUTͼ���ʽ����RGBA8��

        std::array<void*, 6> _irradiance_texture_image_pixels;  // ���ն���ͼ��6���棨��������ͼ����������
        uint32_t             _irradiance_texture_image_width;   // ���ն���ͼ���
        uint32_t             _irradiance_texture_image_height;  // ���ն���ͼ�߶�
        RHIFormat            _irradiance_texture_image_format;  // ���ն���ͼ��ʽ

        std::array<void*, 6> _specular_texture_image_pixels;    // ���淴����ͼ��6������������
        uint32_t             _specular_texture_image_width;     // ���淴����ͼ���
        uint32_t             _specular_texture_image_height;    // ���淴����ͼ�߶�
        RHIFormat            _specular_texture_image_format;    // ���淴����ͼ��ʽ
    };

    // -------------------------- ��ɫ�ּ���Դ�ṹ�� --------------------------
    /// �洢��ɫ�ּ���Color Grading�������GPU��Դ
    struct ColorGradingResource
    {
        RHIImage*     _color_grading_LUT_texture_image;             // ��ɫ�ּ�LUTͼ��������ڵ���ɫ��/���Ͷȵȣ�
        RHIImageView* _color_grading_LUT_texture_image_view;        // ��ɫ�ּ�LUTͼ����ͼ
        VmaAllocation _color_grading_LUT_texture_image_allocation;  // �ڴ������
    };

    /// �洢��ɫ�ּ���Դ��ԭʼ��������
    struct ColorGradingResourceData
    {
        void*     _color_grading_LUT_texture_image_pixels;  // ��ɫ�ּ�LUT��������ָ��
        uint32_t  _color_grading_LUT_texture_image_width;   // ͼ����
        uint32_t  _color_grading_LUT_texture_image_height;  // ͼ��߶�
        RHIFormat _color_grading_LUT_texture_image_format;  // ͼ���ʽ
    };

    // -------------------------- �洢�������ṹ�� --------------------------
    /// �洢���໺������Դ����Ӳ������
    struct StorageBuffer
    {
        // Ӳ�����ƣ����������豸�����ԣ�
        uint32_t _min_uniform_buffer_offset_alignment{ 256 };  // ��Сͳһ������ƫ�ƶ���
        uint32_t _min_storage_buffer_offset_alignment{ 256 };  // ��С�洢������ƫ�ƶ���
        uint32_t _max_storage_buffer_range{ 1 << 27 };         // ���洢��������Χ��128MB��
        uint32_t _non_coherent_atom_size{ 256 };               // ��һ���ڴ�ԭ�Ӳ�����С

        // ȫ���ϴ��������������ڸ�Ч�ϴ���ʱ���ݵ�GPU��
        RHIBuffer* _global_upload_ringbuffer;                  // ���λ���������
        RHIDeviceMemory* _global_upload_ringbuffer_memory;     // �������������豸�ڴ�
        void* _global_upload_ringbuffer_memory_pointer;        // �ڴ�ָ�루����CPUֱ��д�룩

        // ���λ������ֶι������ٸ�֡���ݵ���ʼ/����λ�úʹ�С��
        std::vector<uint32_t> _global_upload_ringbuffers_begin;
        std::vector<uint32_t> _global_upload_ringbuffers_end;
        std::vector<uint32_t> _global_upload_ringbuffers_size;

        // ���������洢��������������Ҫ�ǿհ󶨵���ɫ����Դ��
        RHIBuffer* _global_null_descriptor_storage_buffer;               // �մ洢����������
        RHIDeviceMemory* _global_null_descriptor_storage_buffer_memory;  // �����ڴ�

        // ��洢���������洢��������ϵ�������ȳ������ݣ�
        RHIBuffer* _axis_inefficient_storage_buffer;               // ��洢����������
        RHIDeviceMemory* _axis_inefficient_storage_buffer_memory;  // �����ڴ�
        void* _axis_inefficient_storage_buffer_memory_pointer;     // �ڴ�ָ��
    };

    // -------------------------- ȫ����Ⱦ��Դ�ṹ�� --------------------------
    /// ����ȫ�ֹ������Ⱦ��Դ��IBL����ɫ�ּ����洢��������
    struct GlobalRenderResource
    {
        IBLResource          _ibl_resource;            // IBL������Դ
        ColorGradingResource _color_grading_resource;  // ��ɫ�ּ���Դ
        StorageBuffer        _storage_buffer;          // �洢����������
    };

    // -------------------------- ��Ⱦ��Դ������ --------------------------
    /// ���������Ⱦ��Դ���ϴ������¡����棩
    class RenderResource : public RenderResourceBase
    {
    public:
        /// ����������Ⱦ��Դ���ͷ�GPU�ڴ桢���ٶ���ȣ�
        void clear() override final;

        /**
         * @brief �ϴ�ȫ����Ⱦ��Դ��GPU
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param level_resource_desc �ؿ���Դ����������IBL����ɫ�ּ������ݣ�
         */
        virtual void uploadGlobalRenderResource(std::shared_ptr<RHI> rhi, LevelResourceDesc level_resource_desc) override final;

        /**
         * @brief �ϴ���Ϸ�����������Դ�����ذ汾��
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param render_entity ��Ϸʵ��
         * @param mesh_data �������ݣ����㡢�����ȣ�
         * @param material_data �������ݣ���ɫ������������ȣ�
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data, RenderMaterialData material_data) override final;

        /**
         * @brief �ϴ���Ϸ�����������Դ�����ذ汾��
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param render_entity ��Ϸʵ��
         * @param mesh_data �������ݣ����㡢�����ȣ�
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMeshData mesh_data) override final;

        /**
         * @brief �ϴ���Ϸ����Ĳ�����Դ�����ذ汾��
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param render_entity ��Ϸʵ��
         * @param material_data �������ݣ���ɫ������������ȣ�
         */
        virtual void uploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, RenderMaterialData material_data) override final;

        /**
         * @brief ����ÿ֡���µĻ��������ݣ���������󡢹�����Ϣ��
         * @param render_scene ��ǰ��Ⱦ����
         * @param camera ��ǰ���
         */
        virtual void updatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) override final;

        /// ��ȡʵ���Vulkan������󣨻������أ�
        VulkanMesh& getEntityMesh(RenderEntity entity);

        /// ��ȡʵ���Vulkan PBR���ʶ��󣨻������أ�
        VulkanPBRMaterial& getEntityMaterial(RenderEntity entity);

        /// ����ָ��֡�����Ļ��λ�����ƫ������������һ֡�����ϴ���
        void resetRingBufferOffset(uint8_t current_frame_index);

        // ȫ����Ⱦ��Դ���ᴩ������Ⱦ���̵ĺ�����Դ��
        GlobalRenderResource m_global_render_resource;

        // ����ÿ֡���µĴ洢���������󣨰����ܷ��ࣩ
        MeshPerframeStorageBufferObject m_mesh_perframe_storage_buffer_object;                                                 // ����ÿ֡����
        MeshPointLightShadowPerframeStorageBufferObject m_mesh_point_light_shadow_perframe_storage_buffer_object;              // ���Դ��Ӱÿ֡����
        MeshDirectionalLightShadowPerframeStorageBufferObject m_mesh_directional_light_shadow_perframe_storage_buffer_object;  // �������Ӱÿ֡����
        AxisStorageBufferObject m_axis_storage_buffer_object;                                                                  // ������
        MeshInefficientPickPerframeStorageBufferObject m_mesh_inefficient_pick_perframe_storage_buffer_object;                 // ��Чʰȡÿ֡����
        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;                       // ���ӹ�����ÿ֡����
        ParticleCollisionPerframeStorageBufferObject m_particle_collision_perframe_storage_buffer_object;                      // ������ײÿ֡����

        // ���������Ͳ��ʣ������ظ�������
        std::map<size_t, VulkanMesh>        m_vulkan_meshes;         // ����ʵ���ϣ��ֵ��Vulkan�������
        std::map<size_t, VulkanPBRMaterial> m_vulkan_pbr_materials;  // ����ʵ���ϣ��ֵ��Vulkan PBR���ʶ���

        // ������������ָ�루������Դ�ϴ�ʱ�Ĳ��ְ󶨣�
        RHIDescriptorSetLayout* const* m_mesh_descriptor_set_layout {nullptr};      // ����������������
        RHIDescriptorSetLayout* const* m_material_descriptor_set_layout {nullptr};  // ����������������

    private:
        /**
         * @brief ������ӳ��洢��������CPU�ڴ棨���ڸ�Ч�����ϴ���
         * @param rhi ��ȾӲ���ӿ�ʵ��
         */
        void createAndMapStorageBuffer(std::shared_ptr<RHI> rhi);

        /**
         * @brief ����IBL��Դ�Ĳ������������������/Ѱַģʽ��
         * @param rhi ��ȾӲ���ӿ�ʵ��
         */
        void createIBLSamplers(std::shared_ptr<RHI> rhi);

        /**
         * @brief ����IBL�����ϴ����ݵ�GPU
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param irradiance_maps ���ն���ͼ��6�������ݣ���������ͼ��
         * @param specular_maps ���淴����ͼ��6��������
         */
        void createIBLTextures(std::shared_ptr<RHI> rhi, std::array<std::shared_ptr<TextureData>, 6> irradiance_maps, std::array<std::shared_ptr<TextureData>, 6> specular_maps);

        /**
         * @brief ��ȡ�򴴽�Vulkan������󣨻�����ƣ�
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param entity ��Ϸʵ��
         * @param mesh_data ��������
         * @return VulkanMesh& �Ѵ��ڵĻ��´������������
         */
        VulkanMesh& getOrCreateVulkanMesh(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMeshData mesh_data);

        /**
         * @brief ��ȡ�򴴽�Vulkan PBR���ʶ��󣨻�����ƣ�
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param entity ��Ϸʵ��
         * @param material_data ��������
         * @return VulkanPBRMaterial& �Ѵ��ڵĻ��´����Ĳ��ʶ���
         */
        VulkanPBRMaterial& getOrCreateVulkanMaterial(std::shared_ptr<RHI> rhi, RenderEntity entity, RenderMaterialData material_data);

        /**
         * @brief �����������ݵ�GPU�����������㡢�����ȣ�
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param enable_vertex_blending �Ƿ����ö����ϣ�����������
         * @param index_buffer_size ������������С���ֽڣ�
         * @param index_buffer_data ��������ָ��
         * @param vertex_buffer_size ���㻺������С���ֽڣ�
         * @param vertex_buffer_data ��������ָ��
         * @param joint_binding_buffer_size �ؽڰ󶨻�������С
         * @param joint_binding_buffer_data �ؽڰ�����ָ��
         * @param now_mesh Ŀ���������
         */
        void updateMeshData(std::shared_ptr<RHI>                          rhi,
                            bool                                          enable_vertex_blending,
                            uint32_t                                      index_buffer_size,
                            void*                                         index_buffer_data,
                            uint32_t                                      vertex_buffer_size,
                            struct MeshVertexDataDefinition const*        vertex_buffer_data,
                            uint32_t                                      joint_binding_buffer_size,
                            struct MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                            VulkanMesh&                                   now_mesh);

        /**
         * @brief ���¶��㻺��������
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param enable_vertex_blending �Ƿ����ö�����
         * @param vertex_buffer_size ���㻺������С
         * @param vertex_buffer_data ��������ָ��
         * @param joint_binding_buffer_size �ؽڰ󶨻�������С
         * @param joint_binding_buffer_data �ؽڰ�����ָ��
         * @param index_buffer_size ������������С
         * @param index_buffer_data ��������ָ�루uint16_t���ͣ�
         * @param now_mesh Ŀ���������
         */
        void updateVertexBuffer(std::shared_ptr<RHI>                          rhi,
                                bool                                          enable_vertex_blending,
                                uint32_t                                      vertex_buffer_size,
                                struct MeshVertexDataDefinition const*        vertex_buffer_data,
                                uint32_t                                      joint_binding_buffer_size,
                                struct MeshVertexBindingDataDefinition const* joint_binding_buffer_data,
                                uint32_t                                      index_buffer_size,
                                uint16_t*                                     index_buffer_data,
                                VulkanMesh&                                   now_mesh);

        /**
         * @brief ������������������
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param index_buffer_size ������������С���ֽڣ�
         * @param index_buffer_data ��������ָ�루uint16_t���ͣ�
         * @param now_mesh Ŀ���������
         */
        void updateIndexBuffer(std::shared_ptr<RHI> rhi,
                               uint32_t             index_buffer_size,
                               void*                index_buffer_data,
                               VulkanMesh&          now_mesh);

        /**
         * @brief ��������ͼ�����ݣ���CPU�ϴ���GPU��
         * @param rhi ��ȾӲ���ӿ�ʵ��
         * @param texture_data ���������������ݺ�Ԫ��Ϣ�Ľṹ��
         */
        void updateTextureImageData(std::shared_ptr<RHI> rhi, const TextureDataToUpdate& texture_data);
    };
}

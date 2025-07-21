#pragma once

// ��������ʱ����ж���ID������ͷ�ļ�
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
    // ǰ��������Ⱦ��Դ�������
    class RenderResource;
    class RenderCamera;

    /**
     * @brief ��Ⱦ�����࣬�������������������Ⱦ��ص���Դ�Ͷ���
     *
     * ��������Ⱦģ��ĺ��Ĺ����࣬��Ҫ���ܰ�����
     * - ���������գ������⡢����⡢���Դ��
     * - ά�������е���Ⱦʵ�弯��
     * - ����ÿ֡�ɼ�����Ⱦ���󣨻��ڲ�ͬ��Դ������Ŀɼ��Լ��㣩
     * - ������ԴID���䣨ʵ��ID��������ԴID��������ԴID��
     * - �ṩ����Ϸ����ID��ӳ��ӿ�
     */
    class RenderScene
    {
    public:
        // ====================== ������س�Ա ======================
        AmbientLight      m_ambient_light;     // ���������⣨ȫ�ֻ���������
        PDirectionalLight m_directional_light;  // �����ָ�루ģ��̫�����ƽ�й�Դ��
        PointLightList    m_point_light_list;   // ���Դ�б��洢������Դ����

        // ====================== ��Ⱦʵ�� ======================
        std::vector<RenderEntity> m_render_entities;  // ���������д���Ⱦ��ʵ�弯��

        // ====================== �༭���������� ======================
        std::optional<RenderEntity> m_render_axis;  // ��ѡ��Ⱦ��ʵ�壨�༭��ģʽ����ʾ�����ᣩ

        // ====================== ÿ֡�ɼ����󣨶�̬���£� ======================
        // ��ͬ��Դ������ɼ�������ڵ㼯�ϣ�������Ⱦͨ��ɸѡ�ɼ�����
        std::vector<RenderMeshNode> m_directional_light_visible_mesh_nodes;// �����ɼ�������ڵ�
        std::vector<RenderMeshNode> m_point_lights_visible_mesh_nodes;// ���Դ�ɼ�������ڵ�
        std::vector<RenderMeshNode> m_main_camera_visible_mesh_nodes;// ������ɼ�������ڵ�
        RenderAxisNode              m_axis_node;// ��ڵ㣨�༭����ʾ�ã�

        // ====================== ��������½ӿ� ======================
        /**
         * @brief ��ճ���������Դ������
         * ���ù��ա�ʵ���б��ɼ�����ȳ�Ա�����ڳ������û��л�
         */
        void clear();

        /**
         * @brief ���µ�ǰ֡�ɼ����󣨺��Ŀɼ��Լ��㺯����
         * @param render_resource ��Ⱦ��Դ���������ṩ����/���ʵ���Դ���ʣ�
         * @param camera ��ǰʹ�õ������������ͼͶӰ����
         * �ڲ�����ݲ�ͬ��Ⱦ�׶Σ����տɼ��ԡ�����ɼ��ԣ����þ���ʵ��
         */
        void updateVisibleObjects(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);

        /**
         * @brief ���ÿɼ��ڵ�����Ⱦͨ���е�����
         * ��������Ⱦ���̣�����Ƶ��ã�ֱ�ӷ����Ѽ���Ŀɼ����󼯺�
         */
        void setVisibleNodesReference();

        // ====================== ID���������� ======================
        /**
         * @brief ��ȡʵ��ID������������GameObjectPartId���͵�Ψһ��ʶ��
         * @return ʵ��ID����������
         */
        GuidAllocator<GameObjectPartId>& getInstanceIdAllocator();

        /**
         * @brief ��ȡ������ԴID������������MeshSourceDesc���͵�Ψһ��ʶ��
         * @return ������ԴID����������
         */
        GuidAllocator<MeshSourceDesc>& getMeshAssetIdAllocator();

        /**
         * @brief ��ȡ������ԴID������������MaterialSourceDesc���͵�Ψһ��ʶ��
         * @return ������ԴID����������
		 */
        GuidAllocator<MaterialSourceDesc>& getMaterialAssetdAllocator();

        // ====================== ��Ϸ��������Ⱦ����ӳ�� ======================
        /**
         * @brief ��¼����ID����Ϸ����ID��ӳ���ϵ
         * @param instance_id ��Ⱦʵ��ID����Ӧ������Դ��ʵ����
         * @param go_id ��������Ϸ����ID��GameObjectID��
         */
        void addInstanceIdToMap(uint32_t instance_id, GObjectID go_id);

        /**
         * @brief ͨ������ID��ѯ��Ӧ����Ϸ����ID
         * @param mesh_id ������ԴID����ӦMeshSourceDesc��Ψһ��ʶ��
         * @return ��������Ϸ����ID�����������򷵻�Ĭ�Ϲ����GObjectID��
         */
        GObjectID getGObjectIDByMeshID(uint32_t mesh_id) const;

        /**
         * @brief ������Ϸ����IDɾ����Ӧʵ��
         * @param go_id ��Ҫɾ������Ϸ����ID
         */
        void deleteEntityByGObjectID(GObjectID go_id);

        /**
         * @brief ����������ݣ����ڹؿ����¼���ǰ������
         * �ͷ�������Ⱦ��Դ���ã�����ID�����������ʵ��Ϳɼ����󼯺�
         */
        void clearForLevelReloading();

    private:
        // ====================== ˽�г�Ա���� ======================
        GuidAllocator<GameObjectPartId>   m_instance_id_allocator;    // ʵ��ID������������GameObjectPartId��
        GuidAllocator<MeshSourceDesc>     m_mesh_asset_id_allocator;  // ������ԴID������������MeshSourceDesc��
        GuidAllocator<MaterialSourceDesc> m_material_asset_id_allocator;// ������ԴID������������MaterialSourceDesc��
        // ����ID����Ϸ����ID�Ŀ���ӳ���
        std::unordered_map<uint32_t, GObjectID> m_mesh_object_id_map;

        // ====================== �ɼ��Ը���˽��ʵ�� ======================
        /**
         * @brief ���·����ɼ�������ڵ㣨������ӰͶ�����׶��ü���
         * @param render_resource ��Ⱦ��Դ������
         * @param camera ��ǰ���
         */
        void updateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);

        /**
         * @brief ���µ��Դ�ɼ�������ڵ㣨ͨ����Ϲ���̽��������ؼ��㣩
         * @param render_resource ��Ⱦ��Դ������
         */
        void updateVisibleObjectsPointLight(std::shared_ptr<RenderResource> render_resource);

        /**
         * @brief ����������ɼ�������ڵ㣨���������׶��ü���
         * @param render_resource ��Ⱦ��Դ������
         * @param camera ���������
         */
        void updateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);

        /**
         * @brief ���������Ŀɼ��ԣ��༭��ģʽ��ʼ�տɼ���������ÿ��ƣ�
         * @param render_resource ��Ⱦ��Դ������
         */
        void updateVisibleObjectsAxis(std::shared_ptr<RenderResource> render_resource);

        /**
         * @brief ��������ϵͳ�Ŀɼ��ԣ�Ԥ���ӿڣ���ǰδʵ�־����߼���
         * @param render_resource ��Ⱦ��Դ������
         */
        void updateVisibleObjectsParticle(std::shared_ptr<RenderResource> render_resource);
    };
}

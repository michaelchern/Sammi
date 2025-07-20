#pragma once

#include "runtime/core/math/axis_aligned.h"
#include "runtime/core/math/matrix4.h"

// ������׼������������
#include <cstdint>  // ��ȷ����������ͣ���uint32_t��
#include <vector>   // ��̬��������

namespace Sammi
{
    /**
     * @brief ��Ⱦ�����еĻ���ʵ���࣬����һ������Ⱦ����ĺ������Ժ�״̬
     *
     * �����װ����Ⱦʵ������Ĺؼ���Ϣ������ģ�ͱ任������������/������Դ��
     * ��Ⱦ���ԣ����ϡ�˫����Ⱦ���Լ�PBR�������������Ⱦ����ز�����
     */
    class RenderEntity
    {
    public:
        uint32_t  m_instance_id {0};                     // ʵ��Ψһʵ��ID�����ڱ�ʶ�͹���ͬ��Ⱦ����
        Matrix4x4 m_model_matrix {Matrix4x4::IDENTITY};  // ģ�;��󣨾ֲ��ռ� -> ����ռ�ı任����

        // �����������
        size_t                 m_mesh_asset_id {0};               // ������������ԴID��ָ����ص����������ʲ���
        bool                   m_enable_vertex_blending {false};  // �Ƿ����ö����ϣ������������أ�
        std::vector<Matrix4x4> m_joint_matrices;                  // �ؽڱ任�������飨���������и��ؽڵľֲ��任��
        AxisAlignedBox         m_bounding_box;                    // ������Χ�У�����ռ��е���ײ/�ü��߽磩

        // �����������
        size_t  m_material_asset_id {0};  // �����Ĳ�����ԴID��ָ����صĲ��ʲ����ʲ���
        bool    m_blend {false};          // �Ƿ�������ɫ��ϣ�͸������Ⱦ���أ�
        bool    m_double_sided {false};   // �Ƿ�˫����Ⱦ����������ɼ����أ�

        // PBR�������������Ⱦ�����ʲ���
        Vector4 m_base_color_factor {1.0f, 1.0f, 1.0f, 1.0f};  // ������ɫ���ӣ�������/��������ɫ��RGBA��
        float   m_metallic_factor {1.0f};                      // ���������ӣ�0=��Ե�壬1=������
        float   m_roughness_factor {1.0f};                     // �ֲڶ����ӣ�0=�⻬��1=�ֲڣ�
        float   m_normal_scale {1.0f};                         // ������ͼ�������ӣ���������ϸ��ǿ�ȣ�
        float   m_occlusion_strength {1.0f};                   // �������ڱ�ǿ�ȣ�����AO�Թ��յ�Ӱ�죩
        Vector3 m_emissive_factor {0.0f, 0.0f, 0.0f};          // �Է������ӣ�RGB��ɫ��ǿ�������Ⱦ�����
    };
}
